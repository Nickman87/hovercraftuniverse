#Requires -Version 5.1
<#
.SYNOPSIS
    Reconstructs the build/runtime dependency layout for Hovercraft Universe
    (HovercraftUniverse\dependencies, HovercraftUniverse\bin, HovercraftUniverse\data)
    on a fresh clone of this repository.

.DESCRIPTION
    HovercraftUniverse/dependencies (and the runtime bits under bin/ and data/) are
    not checked into git -- they were originally distributed as downloadable RAR/lib
    packages on the Google Code Archive. This script reproduces that setup exactly,
    by hand-validated procedure, corresponding to README.md's Phase 0 (rescue the
    archive packages) and Phase 2 step 2 ("Add a scripted dependency bootstrap").

    Sources:
      - If a local `archive-mirror\` folder exists at the repo root, files are taken
        from there.
      - Otherwise each needed file is downloaded from the Google Code Archive mirror
        (storage.googleapis.com) into `archive-mirror\`, so re-runs become local.
    Every file (local or freshly downloaded) is SHA1-verified before use.

    Extraction requires 7-Zip (7z.exe) specifically -- do NOT substitute tar/bsdtar,
    which is known to corrupt these particular RAR archives (CRC errors).

    Extraction order matters, because later archives/files intentionally overwrite
    files from earlier ones (Hikari updates, the LuaBind release-lib fix):
      1. Win-dependencies 2.0.rar   (base dependencies\ tree)
      2. SkyX_0_1.rar               (bin\, dependencies\SkyX\)
      3. LuaBind-Fix.rar            (luabind.debug.lib fix)
      4. luabind.release.lib        (copied over the release lib fix)
      5. Win-Hikari-Update.rar, _2, _3, in that order (bin\, dependencies\Hikari\)
      6. Win-runtime 2.0.rar        (bin\debug, bin\release, data\)

    After extraction, a verification pass checks that the expected directories
    exist and that the LuaBind release-lib fix actually took (exact byte size).

.PARAMETER MirrorDir
    Folder holding (or to receive) the archive packages. Defaults to
    "<repo root>\archive-mirror".

.PARAMETER SkipDownload
    Do not download anything; only use files already present in -MirrorDir. Fails
    with a clear error if a required file is missing.

.EXAMPLE
    .\scripts\bootstrap-dependencies.ps1

.EXAMPLE
    .\scripts\bootstrap-dependencies.ps1 -SkipDownload
#>

[CmdletBinding()]
param(
    [string]$MirrorDir,
    [switch]$SkipDownload
)

Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'

# ---------------------------------------------------------------------------
# Paths
# ---------------------------------------------------------------------------

$RepoRoot = (Resolve-Path (Join-Path $PSScriptRoot '..')).Path
$TargetDir = Join-Path $RepoRoot 'HovercraftUniverse'

if (-not $MirrorDir) {
    $MirrorDir = Join-Path $RepoRoot 'archive-mirror'
}

if (-not (Test-Path -LiteralPath $TargetDir -PathType Container)) {
    throw "Expected game folder not found: $TargetDir"
}

# ---------------------------------------------------------------------------
# Required files, in the exact order they must be extracted/applied
# ---------------------------------------------------------------------------

$BaseUrl = 'https://storage.googleapis.com/google-code-archive-downloads/v2/code.google.com/uhasseltaacgua/'

# 'Kind' is one of: Archive (extract with 7z into $TargetDir), File (copy into place)
$RequiredFiles = @(
    [pscustomobject]@{ Name = 'Win-dependencies 2.0.rar'; Sha1 = 'bc81ea58a7e5e05f17be813e03c8e53202c3cea5'; Kind = 'Archive' }
    [pscustomobject]@{ Name = 'SkyX_0_1.rar';             Sha1 = '90bf79b63531b52f2114d9c852da6494f5bf9ccc'; Kind = 'Archive' }
    [pscustomobject]@{ Name = 'LuaBind-Fix.rar';          Sha1 = 'b416223e35cb0c10667f681430665baf82fca6e0'; Kind = 'Archive' }
    [pscustomobject]@{ Name = 'luabind.release.lib';      Sha1 = 'cd1942e1a2cb731101cfeab7df33d6c746f1f94d'; Kind = 'File'
                        Destination = 'dependencies\luabind\msvc-9.0-sp1\lib-x86\luabind.release.lib' }
    [pscustomobject]@{ Name = 'Win-Hikari-Update.rar';    Sha1 = 'beb9110545e79f05df46b6ee84f1d61d0566e9d9'; Kind = 'Archive' }
    [pscustomobject]@{ Name = 'Win-Hikari-Update_2.rar';  Sha1 = '82965a9c8ac2e97872797c67f8d4d8a6a58f751e'; Kind = 'Archive' }
    [pscustomobject]@{ Name = 'Win-Hikari-Update_3.rar';  Sha1 = '877173a5e620df88cee4bde3cf469c3afb50d473'; Kind = 'Archive' }
    [pscustomobject]@{ Name = 'Win-runtime 2.0.rar';      Sha1 = '4b9b063d8de4b58eed57907239937fc9b7b754a6'; Kind = 'Archive' }
)

# Directories that must exist under HovercraftUniverse\dependencies\ once everything
# above has been extracted.
$ExpectedDependencyDirs = @(
    'Havoc\Source'
    'Havoc\Lib\win32_net_9-0\debug_multithreaded_dll'
    'Havoc\Lib\win32_net_9-0\release_multithreaded_dll'
    'OGRE\include\OGRE'
    'OGRE\include\OIS'
    'OGRE\lib\Debug'
    'OGRE\lib\release'
    'Hikari\includes'
    'Hikari\lib'
    'FMOD\includes'
    'FMOD\lib'
    'zoidcom\include'
    'zoidcom\lib\debug'
    'zoidcom\lib\release'
    'lua\include'
    'lua\msvc-9.0-sp1\lib-x86'
    'luabind\include'
    'luabind\msvc-9.0-sp1\lib-x86'
    'boost\include'
    'boost\lib\debug'
    'boost\lib\release'
    'SkyX\include'
    'SkyX\lib'
)

$LuaBindReleaseLibRelPath = 'luabind\msvc-9.0-sp1\lib-x86\luabind.release.lib'
$LuaBindReleaseLibExpectedSize = 14163840

# ---------------------------------------------------------------------------
# Helpers
# ---------------------------------------------------------------------------

function Write-Step {
    param([string]$Message)
    Write-Host "==> $Message" -ForegroundColor Cyan
}

function Write-Info {
    param([string]$Message)
    Write-Host "    $Message"
}

function Get-Sha1Hash {
    param([Parameter(Mandatory)][string]$Path)
    (Get-FileHash -LiteralPath $Path -Algorithm SHA1).Hash.ToLowerInvariant()
}

function Resolve-SevenZip {
    $candidatePaths = @(
        'C:\Program Files\7-Zip\7z.exe'
        'C:\Program Files (x86)\7-Zip\7z.exe'
    )
    foreach ($candidate in $candidatePaths) {
        if (Test-Path -LiteralPath $candidate -PathType Leaf) {
            return $candidate
        }
    }

    $onPath = Get-Command '7z' -ErrorAction SilentlyContinue
    if ($onPath) {
        return $onPath.Source
    }

    Write-Host ''
    Write-Host '7-Zip (7z.exe) was not found on this machine.' -ForegroundColor Red
    Write-Host 'These dependency archives are RAR files that must be extracted with' -ForegroundColor Red
    Write-Host 'real 7-Zip -- tar.exe/bsdtar is known to corrupt them (CRC errors).' -ForegroundColor Red
    Write-Host ''
    Write-Host 'Install 7-Zip and re-run this script:' -ForegroundColor Yellow
    Write-Host '    winget install 7zip.7zip' -ForegroundColor Yellow
    Write-Host ''
    throw '7-Zip is required but was not found (checked Program Files and PATH).'
}

function Confirm-FileHash {
    param(
        [Parameter(Mandatory)][string]$Path,
        [Parameter(Mandatory)][string]$ExpectedSha1,
        [Parameter(Mandatory)][string]$DisplayName
    )
    $actual = Get-Sha1Hash -Path $Path
    if ($actual -ne $ExpectedSha1.ToLowerInvariant()) {
        throw "SHA1 mismatch for '$DisplayName' (path: $Path)`n" +
              "  expected: $ExpectedSha1`n" +
              "  actual:   $actual`n" +
              "The file is corrupt, truncated, or the wrong version. Delete it from " +
              "the mirror folder and re-run this script to re-download it."
    }
}

function Get-RequiredFile {
    <#
        Ensures $MirrorDir\$($FileInfo.Name) exists and matches its expected SHA1.
        Downloads it first if missing and -SkipDownload was not specified.
        Returns the full local path.
    #>
    param([Parameter(Mandatory)]$FileInfo)

    $localPath = Join-Path $MirrorDir $FileInfo.Name

    if (Test-Path -LiteralPath $localPath -PathType Leaf) {
        Write-Info "Found in mirror: $($FileInfo.Name)"
    }
    elseif ($SkipDownload) {
        throw "Missing required file '$($FileInfo.Name)' in mirror '$MirrorDir' and " +
              "-SkipDownload was specified, so it cannot be downloaded."
    }
    else {
        if (-not (Test-Path -LiteralPath $MirrorDir -PathType Container)) {
            New-Item -ItemType Directory -Path $MirrorDir -Force | Out-Null
        }

        $encodedName = [uri]::EscapeDataString($FileInfo.Name)
        $url = $BaseUrl + $encodedName
        Write-Info "Downloading $($FileInfo.Name) ..."
        Write-Info "  from $url"

        $tempPath = "$localPath.download"
        try {
            Invoke-WebRequest -Uri $url -OutFile $tempPath -UseBasicParsing
        }
        catch {
            if (Test-Path -LiteralPath $tempPath) {
                Remove-Item -LiteralPath $tempPath -Force -ErrorAction SilentlyContinue
            }
            throw "Failed to download '$($FileInfo.Name)' from $url : $($_.Exception.Message)"
        }
        Move-Item -LiteralPath $tempPath -Destination $localPath -Force
    }

    Confirm-FileHash -Path $localPath -ExpectedSha1 $FileInfo.Sha1 -DisplayName $FileInfo.Name
    return $localPath
}

function Expand-ArchiveWith7z {
    param(
        [Parameter(Mandatory)][string]$SevenZipExe,
        [Parameter(Mandatory)][string]$ArchivePath,
        [Parameter(Mandatory)][string]$DestinationDir
    )

    $arguments = @('x', $ArchivePath, "-o$DestinationDir", '-aoa', '-y')
    $output = & $SevenZipExe @arguments 2>&1
    if ($LASTEXITCODE -ne 0) {
        Write-Host ($output | Out-String)
        throw "7z.exe exited with code $LASTEXITCODE while extracting '$ArchivePath'."
    }
}

# ---------------------------------------------------------------------------
# Main
# ---------------------------------------------------------------------------

Write-Step "Repo root:    $RepoRoot"
Write-Step "Target dir:   $TargetDir"
Write-Step "Mirror dir:   $MirrorDir"
if ($SkipDownload) {
    Write-Step "Downloads disabled (-SkipDownload); using local mirror only."
}

$sevenZip = Resolve-SevenZip
Write-Step "Using 7-Zip:  $sevenZip"

# --- 1. Win-dependencies 2.0.rar -------------------------------------------
Write-Step "1/6 Win-dependencies 2.0.rar (base dependencies\ tree)"
$file = $RequiredFiles | Where-Object { $_.Name -eq 'Win-dependencies 2.0.rar' }
$path = Get-RequiredFile -FileInfo $file
Expand-ArchiveWith7z -SevenZipExe $sevenZip -ArchivePath $path -DestinationDir $TargetDir

# --- 2. SkyX_0_1.rar ---------------------------------------------------------
Write-Step "2/6 SkyX_0_1.rar (bin\, dependencies\SkyX\)"
$file = $RequiredFiles | Where-Object { $_.Name -eq 'SkyX_0_1.rar' }
$path = Get-RequiredFile -FileInfo $file
Expand-ArchiveWith7z -SevenZipExe $sevenZip -ArchivePath $path -DestinationDir $TargetDir

# --- 3. LuaBind-Fix.rar -------------------------------------------------------
Write-Step "3/6 LuaBind-Fix.rar (luabind.debug.lib fix)"
$file = $RequiredFiles | Where-Object { $_.Name -eq 'LuaBind-Fix.rar' }
$path = Get-RequiredFile -FileInfo $file
Expand-ArchiveWith7z -SevenZipExe $sevenZip -ArchivePath $path -DestinationDir $TargetDir

# --- 4. luabind.release.lib ---------------------------------------------------
Write-Step "4/6 luabind.release.lib (overwrite release lib with the fixed build)"
$file = $RequiredFiles | Where-Object { $_.Name -eq 'luabind.release.lib' }
$path = Get-RequiredFile -FileInfo $file
$destPath = Join-Path $TargetDir $file.Destination
$destDir = Split-Path -Parent $destPath
if (-not (Test-Path -LiteralPath $destDir -PathType Container)) {
    New-Item -ItemType Directory -Path $destDir -Force | Out-Null
}
Copy-Item -LiteralPath $path -Destination $destPath -Force
Write-Info "Copied to $destPath"

# --- 5. Hikari updates, in order ----------------------------------------------
Write-Step "5/6 Hikari updates (bin\, dependencies\Hikari\) -- Win-Hikari-Update.rar, _2, _3"
foreach ($name in @('Win-Hikari-Update.rar', 'Win-Hikari-Update_2.rar', 'Win-Hikari-Update_3.rar')) {
    $file = $RequiredFiles | Where-Object { $_.Name -eq $name }
    $path = Get-RequiredFile -FileInfo $file
    Write-Info "Extracting $name ..."
    Expand-ArchiveWith7z -SevenZipExe $sevenZip -ArchivePath $path -DestinationDir $TargetDir
}

# --- 6. Win-runtime 2.0.rar ----------------------------------------------------
Write-Step "6/6 Win-runtime 2.0.rar (bin\debug, bin\release, data\)"
$file = $RequiredFiles | Where-Object { $_.Name -eq 'Win-runtime 2.0.rar' }
$path = Get-RequiredFile -FileInfo $file
Expand-ArchiveWith7z -SevenZipExe $sevenZip -ArchivePath $path -DestinationDir $TargetDir

# ---------------------------------------------------------------------------
# Verification
# ---------------------------------------------------------------------------

Write-Step "Verifying dependency layout"

$dependenciesRoot = Join-Path $TargetDir 'dependencies'
$allOk = $true
$rows = New-Object System.Collections.Generic.List[object]

foreach ($relDir in $ExpectedDependencyDirs) {
    $fullDir = Join-Path $dependenciesRoot $relDir
    $ok = Test-Path -LiteralPath $fullDir -PathType Container
    if (-not $ok) { $allOk = $false }
    $rows.Add([pscustomobject]@{
        Check  = "dependencies\$relDir"
        Status = if ($ok) { [char]0x2713 } else { [char]0x2717 }
        Ok     = $ok
    })
}

$luaBindReleaseLibPath = Join-Path $dependenciesRoot $LuaBindReleaseLibRelPath
$luaBindOk = $false
if (Test-Path -LiteralPath $luaBindReleaseLibPath -PathType Leaf) {
    $actualSize = (Get-Item -LiteralPath $luaBindReleaseLibPath).Length
    $luaBindOk = ($actualSize -eq $LuaBindReleaseLibExpectedSize)
    $sizeNote = if ($luaBindOk) {
        "$actualSize bytes"
    } else {
        "$actualSize bytes (expected $LuaBindReleaseLibExpectedSize -- fix did not overwrite the base package's copy)"
    }
}
else {
    $sizeNote = 'file not found'
}
if (-not $luaBindOk) { $allOk = $false }
$rows.Add([pscustomobject]@{
    Check  = "dependencies\$LuaBindReleaseLibRelPath size == $LuaBindReleaseLibExpectedSize ($sizeNote)"
    Status = if ($luaBindOk) { [char]0x2713 } else { [char]0x2717 }
    Ok     = $luaBindOk
})

Write-Host ''
$rows | ForEach-Object {
    $color = if ($_.Ok) { 'Green' } else { 'Red' }
    Write-Host ("  [{0}] {1}" -f $_.Status, $_.Check) -ForegroundColor $color
}
Write-Host ''

if ($allOk) {
    Write-Host 'All dependency checks passed.' -ForegroundColor Green
    exit 0
}
else {
    Write-Host 'One or more dependency checks FAILED. See the table above.' -ForegroundColor Red
    exit 1
}
