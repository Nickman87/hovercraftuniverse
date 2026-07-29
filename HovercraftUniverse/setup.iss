[Setup]
OutputDir=bin\installer
Compression=lzma/ultra64
AppName=Hovercraft Universe
AppVerName=Hovercraft Universe v1.0
DefaultDirName={pf}\HovercraftUniverse
DefaultGroupName=Hovercraft Universe
AlwaysShowDirOnReadyPage=true
UninstallDisplayName=Hovercraft Universe
AppVersion=1.0
AppID={{3F2DDF97-F798-426F-81C7-2C9D4A6EAB05}
UninstallDisplayIcon={app}\HovercraftUniverse.exe
AllowNoIcons=true
VersionInfoCompany=University of Antwerp
VersionInfoDescription=Hovercraft Universe
VersionInfoCopyright=Hovercraft Universe
AppPublisher=Hovercraft Universe
AppPublisherURL=http://uhasseltaacgua.googlecode.com/
AppSupportURL=http://uhasseltaacgua.googlecode.com/
AppUpdatesURL=http://uhasseltaacgua.googlecode.com/
OutputBaseFilename=HovercraftUniverseSetup
WizardImageFile=bin\installer\setup-large.bmp
WizardImageBackColor=clMaroon
WizardSmallImageFile=bin\installer\setup-small.bmp
;SetupIconFile=bin\installer\HovercraftUniverse.ico
VersionInfoVersion=1.0
VersionInfoTextVersion=1.0
PrivilegesRequired=none
InfoAfterFile=bin\installer\ReadmeForInstaller.txt
[Icons]
Name: {commondesktop}\Hovercraft Universe; Filename: {app}\HovercraftUniverse.exe; WorkingDir: {app}; Comment: Play Hovercraft Universe; IconIndex: 0
Name: {commondesktop}\Hovercraft Universe Dedicated Server; Filename: {app}\HovercraftUniverse.exe; Parameters: --server; WorkingDir: {app}; Comment: Run Hovercraft Universe Dedicated Server; IconIndex: 0
[Run]
Filename: {app}\HovercraftUniverse.exe; Description: Start playing Hovercraft Universe!; Flags: postinstall skipifsilent unchecked
[Files]
;Take the INIs from the bin\installer folder
Source: bin\installer\HovercraftUniverse.ini; DestDir: {app}
Source: bin\installer\Server.ini; DestDir: {app}

;Executables and DLL from the release folder
Source: bin\release\HovercraftUniverse.exe; DestDir: {app}
Source: bin\release\lua51.dll; DestDir: {app}
Source: bin\release\OgreMain.dll; DestDir: {app}
Source: bin\release\OIS.dll; DestDir: {app}
Source: bin\release\zoidcom_vc.dll; DestDir: {app}
Source: bin\release\fmod_event.dll; DestDir: {app}
Source: bin\release\fmodex.dll; DestDir: {app}
Source: bin\release\Hikari.dll; DestDir: {app}
Source: bin\release\lua5.1.dll; DestDir: {app}
Source: bin\release\SkyX.dll; DestDir: {app}
;Source: bin\release\d3dx9_42.dll; DestDir: {app}

;Data folder - without the debug plugins
Source: data\characters\materials\textures\cloudera.jpg; DestDir: {app}\data\characters\materials\textures
Source: data\characters\materials\Cloudera.material; DestDir: {app}\data\characters\materials
Source: data\characters\models\Cloudera.mesh; DestDir: {app}\data\characters\models
Source: data\characters\models\Cloudera.skeleton; DestDir: {app}\data\characters\models
Source: data\controls\Controls.ini; DestDir: {app}\data\controls
Source: data\entities\Entities.ini; DestDir: {app}\data\entities
Source: data\gui\background.swf; DestDir: {app}\data\gui
Source: data\gui\chat.swf; DestDir: {app}\data\gui
Source: data\gui\countdown.swf; DestDir: {app}\data\gui
Source: data\gui\direction.swf; DestDir: {app}\data\gui
Source: data\gui\GUIConfig.xml; DestDir: {app}\data\gui
Source: data\gui\lapTimer.swf; DestDir: {app}\data\gui
Source: data\gui\loader.swf; DestDir: {app}\data\gui
Source: data\gui\lobby.swf; DestDir: {app}\data\gui
Source: data\gui\menuButton.swf; DestDir: {app}\data\gui
Source: data\gui\messagebox.swf; DestDir: {app}\data\gui
Source: data\gui\position.swf; DestDir: {app}\data\gui
Source: data\gui\raceresults.swf; DestDir: {app}\data\gui
Source: data\gui\serverMenu.swf; DestDir: {app}\data\gui
Source: data\gui\speedometer.swf; DestDir: {app}\data\gui
Source: data\gui\title.swf; DestDir: {app}\data\gui
Source: data\hovercraft\materials\scripts\HippyCraft.material; DestDir: {app}\data\hovercraft\materials\scripts
Source: data\hovercraft\materials\scripts\hover1.material; DestDir: {app}\data\hovercraft\materials\scripts
Source: data\hovercraft\materials\scripts\hoverjeep.material; DestDir: {app}\data\hovercraft\materials\scripts
Source: data\hovercraft\materials\textures\armygreen.jpg; DestDir: {app}\data\hovercraft\materials\textures
Source: data\hovercraft\materials\textures\cogsmetallic.jpg; DestDir: {app}\data\hovercraft\materials\textures
Source: data\hovercraft\materials\textures\hippycraftbody.jpg; DestDir: {app}\data\hovercraft\materials\textures
Source: data\hovercraft\materials\textures\hovercogscabin.jpg; DestDir: {app}\data\hovercraft\materials\textures
Source: data\hovercraft\materials\textures\hovercraftbody.jpg; DestDir: {app}\data\hovercraft\materials\textures
Source: data\hovercraft\materials\textures\hoverjeeptires.jpg; DestDir: {app}\data\hovercraft\materials\textures
Source: data\hovercraft\materials\textures\jeeparmycolours.jpg; DestDir: {app}\data\hovercraft\materials\textures
Source: data\hovercraft\materials\textures\jeepdashboard.jpg; DestDir: {app}\data\hovercraft\materials\textures
Source: data\hovercraft\materials\textures\jeephoverhood.jpg; DestDir: {app}\data\hovercraft\materials\textures
Source: data\hovercraft\materials\textures\jeepseat.jpg; DestDir: {app}\data\hovercraft\materials\textures
Source: data\hovercraft\materials\textures\seat.jpg; DestDir: {app}\data\hovercraft\materials\textures
Source: data\hovercraft\materials\textures\steeringwheel.jpg; DestDir: {app}\data\hovercraft\materials\textures
Source: data\hovercraft\materials\textures\tires.jpg; DestDir: {app}\data\hovercraft\materials\textures
Source: data\hovercraft\models\hippycraft.mesh; DestDir: {app}\data\hovercraft\models
Source: data\hovercraft\models\hippycraft.skeleton; DestDir: {app}\data\hovercraft\models
Source: data\hovercraft\models\hover1.mesh; DestDir: {app}\data\hovercraft\models
Source: data\hovercraft\models\hover1.skeleton; DestDir: {app}\data\hovercraft\models
Source: data\hovercraft\models\hoverjeep.mesh; DestDir: {app}\data\hovercraft\models
Source: data\hovercraft\models\hoverJeep.skeleton; DestDir: {app}\data\hovercraft\models
Source: data\hovercraft\HippyCraft.hkx; DestDir: {app}\data\hovercraft
Source: data\hovercraft\HippyCraft.scene; DestDir: {app}\data\hovercraft
Source: data\hovercraft\hover1.hkx; DestDir: {app}\data\hovercraft
Source: data\hovercraft\hover1.scene; DestDir: {app}\data\hovercraft
Source: data\hovercraft\USAHovercraft.hkx; DestDir: {app}\data\hovercraft
Source: data\hovercraft\USAHovercraft.scene; DestDir: {app}\data\hovercraft
Source: data\hovercraft\HoverJeep.hkx; DestDir: {app}\data\hovercraft
Source: data\hovercraft\HoverJeep.scene; DestDir: {app}\data\hovercraft

;Crazy planet level
Source: data\levels\Bavaraf\materials\scripts\Bavaraf.material; DestDir: {app}\data\levels\Bavaraf\materials\scripts
;Source: data\levels\Bavaraf\materials\scripts\Kopie van Bavaraf.material; DestDir: {app}\data\levels\Bavaraf\materials\scripts
Source: data\levels\Bavaraf\models\Checkpoint0Geom.mesh; DestDir: {app}\data\levels\Bavaraf\models
Source: data\levels\Bavaraf\models\Checkpoint1Geom.mesh; DestDir: {app}\data\levels\Bavaraf\models
Source: data\levels\Bavaraf\models\Checkpoint2Geom.mesh; DestDir: {app}\data\levels\Bavaraf\models
Source: data\levels\Bavaraf\models\Checkpoint3Geom.mesh; DestDir: {app}\data\levels\Bavaraf\models
Source: data\levels\Bavaraf\models\PlanetGeom.mesh; DestDir: {app}\data\levels\Bavaraf\models
Source: data\levels\Bavaraf\models\StartGeom.mesh; DestDir: {app}\data\levels\Bavaraf\models
Source: data\levels\Bavaraf\textures\checkpointsign.jpg; DestDir: {app}\data\levels\Bavaraf\textures
Source: data\levels\Bavaraf\textures\dirt.jpg; DestDir: {app}\data\levels\Bavaraf\textures
Source: data\levels\Bavaraf\textures\grass.jpg; DestDir: {app}\data\levels\Bavaraf\textures
Source: data\levels\Bavaraf\textures\PlanetGeomDiffuseMap.png; DestDir: {app}\data\levels\Bavaraf\textures
Source: data\levels\Bavaraf\textures\rock-5_sml2.jpg; DestDir: {app}\data\levels\Bavaraf\textures
Source: data\levels\Bavaraf\textures\rock.jpg; DestDir: {app}\data\levels\Bavaraf\textures
Source: data\levels\Bavaraf\textures\terrain0.jpg; DestDir: {app}\data\levels\Bavaraf\textures
Source: data\levels\Bavaraf\textures\trunk.png; DestDir: {app}\data\levels\Bavaraf\textures
Source: data\levels\Bavaraf\textures\wall_512_5_05.jpg; DestDir: {app}\data\levels\Bavaraf\textures

;The junk yard
Source: data\levels\junkyard\materials\scripts\Junkyard.material; DestDir: {app}\data\levels\junkyard\materials\scripts
Source: data\levels\junkyard\materials\scripts\Junkyard2.material; DestDir: {app}\data\levels\junkyard\materials\scripts
Source: data\levels\junkyard\models\Asteroid01.mesh; DestDir: {app}\data\levels\junkyard\models
Source: data\levels\junkyard\models\Asteroid02.mesh; DestDir: {app}\data\levels\junkyard\models
Source: data\levels\junkyard\models\Asteroid03.mesh; DestDir: {app}\data\levels\junkyard\models
Source: data\levels\junkyard\models\Asteroid04.mesh; DestDir: {app}\data\levels\junkyard\models
Source: data\levels\junkyard\models\Barrel01.mesh; DestDir: {app}\data\levels\junkyard\models
Source: data\levels\junkyard\models\Barrel02.mesh; DestDir: {app}\data\levels\junkyard\models
Source: data\levels\junkyard\models\Barrel03.mesh; DestDir: {app}\data\levels\junkyard\models
Source: data\levels\junkyard\models\Box01.mesh; DestDir: {app}\data\levels\junkyard\models
Source: data\levels\junkyard\models\Box02.mesh; DestDir: {app}\data\levels\junkyard\models
Source: data\levels\junkyard\models\Box03.mesh; DestDir: {app}\data\levels\junkyard\models
Source: data\levels\junkyard\models\Check01.mesh; DestDir: {app}\data\levels\junkyard\models
Source: data\levels\junkyard\models\Check02.mesh; DestDir: {app}\data\levels\junkyard\models
Source: data\levels\junkyard\models\Check03.mesh; DestDir: {app}\data\levels\junkyard\models
Source: data\levels\junkyard\models\Check04.mesh; DestDir: {app}\data\levels\junkyard\models
Source: data\levels\junkyard\models\Engine01.mesh; DestDir: {app}\data\levels\junkyard\models
Source: data\levels\junkyard\models\Fighter.mesh; DestDir: {app}\data\levels\junkyard\models
Source: data\levels\junkyard\models\Jump01.mesh; DestDir: {app}\data\levels\junkyard\models
Source: data\levels\junkyard\models\Jump02.mesh; DestDir: {app}\data\levels\junkyard\models
Source: data\levels\junkyard\models\Jump03.mesh; DestDir: {app}\data\levels\junkyard\models
Source: data\levels\junkyard\models\Jump04.mesh; DestDir: {app}\data\levels\junkyard\models
Source: data\levels\junkyard\models\Jump05.mesh; DestDir: {app}\data\levels\junkyard\models
Source: data\levels\junkyard\models\Jump06.mesh; DestDir: {app}\data\levels\junkyard\models
Source: data\levels\junkyard\models\Planet01.mesh; DestDir: {app}\data\levels\junkyard\models
Source: data\levels\junkyard\models\Rock01.mesh; DestDir: {app}\data\levels\junkyard\models
Source: data\levels\junkyard\models\Rock02.mesh; DestDir: {app}\data\levels\junkyard\models
Source: data\levels\junkyard\models\Rock03.mesh; DestDir: {app}\data\levels\junkyard\models
Source: data\levels\junkyard\models\Rock05.mesh; DestDir: {app}\data\levels\junkyard\models
Source: data\levels\junkyard\models\Rock06.mesh; DestDir: {app}\data\levels\junkyard\models
Source: data\levels\junkyard\models\Rock07.mesh; DestDir: {app}\data\levels\junkyard\models
Source: data\levels\junkyard\models\tree01.mesh; DestDir: {app}\data\levels\junkyard\models
Source: data\levels\junkyard\models\tree02.mesh; DestDir: {app}\data\levels\junkyard\models
Source: data\levels\junkyard\models\tree03.mesh; DestDir: {app}\data\levels\junkyard\models
Source: data\levels\junkyard\models\tree2_01.mesh; DestDir: {app}\data\levels\junkyard\models
Source: data\levels\junkyard\models\tree2_02.mesh; DestDir: {app}\data\levels\junkyard\models
Source: data\levels\junkyard\models\tunnel01.mesh; DestDir: {app}\data\levels\junkyard\models
Source: data\levels\junkyard\models\tunnel02.mesh; DestDir: {app}\data\levels\junkyard\models
Source: data\levels\junkyard\models\tunnel03.mesh; DestDir: {app}\data\levels\junkyard\models
Source: data\levels\junkyard\models\Wall01.mesh; DestDir: {app}\data\levels\junkyard\models
Source: data\levels\junkyard\models\Wall02.mesh; DestDir: {app}\data\levels\junkyard\models
Source: data\levels\junkyard\models\Wall03.mesh; DestDir: {app}\data\levels\junkyard\models
Source: data\levels\junkyard\models\Wall04.mesh; DestDir: {app}\data\levels\junkyard\models
Source: data\levels\junkyard\models\Wall05.mesh; DestDir: {app}\data\levels\junkyard\models
Source: data\levels\junkyard\models\Wall06.mesh; DestDir: {app}\data\levels\junkyard\models
Source: data\levels\junkyard\models\Wall07.mesh; DestDir: {app}\data\levels\junkyard\models
Source: data\levels\junkyard\models\Wall08.mesh; DestDir: {app}\data\levels\junkyard\models
Source: data\levels\junkyard\models\Wall09.mesh; DestDir: {app}\data\levels\junkyard\models
Source: data\levels\junkyard\models\Wing_Part_01.mesh; DestDir: {app}\data\levels\junkyard\models
Source: data\levels\junkyard\models\Wing_Part_02.mesh; DestDir: {app}\data\levels\junkyard\models
Source: data\levels\junkyard\textures\arrow.png; DestDir: {app}\data\levels\junkyard\textures
Source: data\levels\junkyard\textures\Asteroid01DiffuseMap.png; DestDir: {app}\data\levels\junkyard\textures
Source: data\levels\junkyard\textures\Asteroid02DiffuseMap.png; DestDir: {app}\data\levels\junkyard\textures
Source: data\levels\junkyard\textures\Asteroid03DiffuseMap.png; DestDir: {app}\data\levels\junkyard\textures
Source: data\levels\junkyard\textures\Asteroid04DiffuseMap.png; DestDir: {app}\data\levels\junkyard\textures
Source: data\levels\junkyard\textures\Barrel.png; DestDir: {app}\data\levels\junkyard\textures
Source: data\levels\junkyard\textures\box.png; DestDir: {app}\data\levels\junkyard\textures
Source: data\levels\junkyard\textures\checkpoint_1.png; DestDir: {app}\data\levels\junkyard\textures
Source: data\levels\junkyard\textures\checkpoint_3.png; DestDir: {app}\data\levels\junkyard\textures
Source: data\levels\junkyard\textures\dirt.jpg; DestDir: {app}\data\levels\junkyard\textures
Source: data\levels\junkyard\textures\engine-texture.png; DestDir: {app}\data\levels\junkyard\textures
Source: data\levels\junkyard\textures\engine-wing.png; DestDir: {app}\data\levels\junkyard\textures
Source: data\levels\junkyard\textures\fighter.png; DestDir: {app}\data\levels\junkyard\textures
Source: data\levels\junkyard\textures\grass.jpg; DestDir: {app}\data\levels\junkyard\textures
Source: data\levels\junkyard\textures\jump.png; DestDir: {app}\data\levels\junkyard\textures
Source: data\levels\junkyard\textures\leaves3.png; DestDir: {app}\data\levels\junkyard\textures
Source: data\levels\junkyard\textures\Planet01DiffuseMap.png; DestDir: {app}\data\levels\junkyard\textures
Source: data\levels\junkyard\textures\rock-texture.png; DestDir: {app}\data\levels\junkyard\textures
Source: data\levels\junkyard\textures\rock.jpg; DestDir: {app}\data\levels\junkyard\textures
Source: data\levels\junkyard\textures\trunk.png; DestDir: {app}\data\levels\junkyard\textures
Source: data\levels\junkyard\textures\tunnel.png; DestDir: {app}\data\levels\junkyard\textures
Source: data\levels\junkyard\textures\wing.png; DestDir: {app}\data\levels\junkyard\textures
Source: data\levels\materials\programs\asteroid.program; DestDir: {app}\data\levels\materials\programs
Source: data\levels\materials\programs\asteroidPS.cg; DestDir: {app}\data\levels\materials\programs
Source: data\levels\materials\programs\asteroidVS.cg; DestDir: {app}\data\levels\materials\programs

;Halfpipe level
Source: data\levels\Ramp\materials\scripts\Ramp.material; DestDir: {app}\data\levels\Ramp\materials\scripts
Source: data\levels\Ramp\models\Box01.mesh; DestDir: {app}\data\levels\Ramp\models
Source: data\levels\Ramp\models\Finish.mesh; DestDir: {app}\data\levels\Ramp\models
Source: data\levels\Ramp\models\Planet01.mesh; DestDir: {app}\data\levels\Ramp\models
Source: data\levels\Ramp\textures\ramp.png; DestDir: {app}\data\levels\Ramp\textures

;SimpleTrack2
Source: data\levels\SimpleTrack2\materials\scripts\ShaderExample.txt; DestDir: {app}\data\levels\SimpleTrack2\materials\scripts
Source: data\levels\SimpleTrack2\materials\scripts\SimpleTrack2.material; DestDir: {app}\data\levels\SimpleTrack2\materials\scripts
Source: data\levels\SimpleTrack2\materials\scripts\soccer.material; DestDir: {app}\data\levels\SimpleTrack2\materials\scripts
Source: data\levels\SimpleTrack2\models\Asteroid01.mesh; DestDir: {app}\data\levels\SimpleTrack2\models
Source: data\levels\SimpleTrack2\models\Asteroid02.mesh; DestDir: {app}\data\levels\SimpleTrack2\models
Source: data\levels\SimpleTrack2\models\Barrel01.mesh; DestDir: {app}\data\levels\SimpleTrack2\models
Source: data\levels\SimpleTrack2\models\Barrel02.mesh; DestDir: {app}\data\levels\SimpleTrack2\models
Source: data\levels\SimpleTrack2\models\Barrel03.mesh; DestDir: {app}\data\levels\SimpleTrack2\models
Source: data\levels\SimpleTrack2\models\Box01.mesh; DestDir: {app}\data\levels\SimpleTrack2\models
Source: data\levels\SimpleTrack2\models\Box02.mesh; DestDir: {app}\data\levels\SimpleTrack2\models
Source: data\levels\SimpleTrack2\models\Box03.mesh; DestDir: {app}\data\levels\SimpleTrack2\models
Source: data\levels\SimpleTrack2\models\Check01.mesh; DestDir: {app}\data\levels\SimpleTrack2\models
Source: data\levels\SimpleTrack2\models\Check02.mesh; DestDir: {app}\data\levels\SimpleTrack2\models
Source: data\levels\SimpleTrack2\models\Check03.mesh; DestDir: {app}\data\levels\SimpleTrack2\models
Source: data\levels\SimpleTrack2\models\Check04.mesh; DestDir: {app}\data\levels\SimpleTrack2\models
Source: data\levels\SimpleTrack2\models\Engine01.mesh; DestDir: {app}\data\levels\SimpleTrack2\models
Source: data\levels\SimpleTrack2\models\Fighter.mesh; DestDir: {app}\data\levels\SimpleTrack2\models
Source: data\levels\SimpleTrack2\models\Jump01.mesh; DestDir: {app}\data\levels\SimpleTrack2\models
Source: data\levels\SimpleTrack2\models\Jump02.mesh; DestDir: {app}\data\levels\SimpleTrack2\models
Source: data\levels\SimpleTrack2\models\Planet01.mesh; DestDir: {app}\data\levels\SimpleTrack2\models
Source: data\levels\SimpleTrack2\models\Rock01.mesh; DestDir: {app}\data\levels\SimpleTrack2\models
Source: data\levels\SimpleTrack2\models\Rock02.mesh; DestDir: {app}\data\levels\SimpleTrack2\models
Source: data\levels\SimpleTrack2\models\Rock03.mesh; DestDir: {app}\data\levels\SimpleTrack2\models
Source: data\levels\SimpleTrack2\models\Wing_Part_01.mesh; DestDir: {app}\data\levels\SimpleTrack2\models
Source: data\levels\SimpleTrack2\models\Wing_Part_02.mesh; DestDir: {app}\data\levels\SimpleTrack2\models
Source: data\levels\SimpleTrack2\textures\Asteroid01DiffuseMap.png; DestDir: {app}\data\levels\SimpleTrack2\textures
Source: data\levels\SimpleTrack2\textures\Asteroid02DiffuseMap.png; DestDir: {app}\data\levels\SimpleTrack2\textures
Source: data\levels\SimpleTrack2\textures\Barrel.png; DestDir: {app}\data\levels\SimpleTrack2\textures
Source: data\levels\SimpleTrack2\textures\box.png; DestDir: {app}\data\levels\SimpleTrack2\textures
Source: data\levels\SimpleTrack2\textures\checkpoint_1.png; DestDir: {app}\data\levels\SimpleTrack2\textures
Source: data\levels\SimpleTrack2\textures\checkpoint_3.png; DestDir: {app}\data\levels\SimpleTrack2\textures
Source: data\levels\SimpleTrack2\textures\dirt.jpg; DestDir: {app}\data\levels\SimpleTrack2\textures
Source: data\levels\SimpleTrack2\textures\engine-texture.png; DestDir: {app}\data\levels\SimpleTrack2\textures
Source: data\levels\SimpleTrack2\textures\engine-wing.png; DestDir: {app}\data\levels\SimpleTrack2\textures
Source: data\levels\SimpleTrack2\textures\fighter.png; DestDir: {app}\data\levels\SimpleTrack2\textures
Source: data\levels\SimpleTrack2\textures\grass.jpg; DestDir: {app}\data\levels\SimpleTrack2\textures
Source: data\levels\SimpleTrack2\textures\jump.png; DestDir: {app}\data\levels\SimpleTrack2\textures
Source: data\levels\SimpleTrack2\textures\Planet01DiffuseMap.png; DestDir: {app}\data\levels\SimpleTrack2\textures
Source: data\levels\SimpleTrack2\textures\rock-texture.png; DestDir: {app}\data\levels\SimpleTrack2\textures
Source: data\levels\SimpleTrack2\textures\rock.jpg; DestDir: {app}\data\levels\SimpleTrack2\textures
Source: data\levels\SimpleTrack2\textures\wing.png; DestDir: {app}\data\levels\SimpleTrack2\textures

;The soccer field
Source: data\levels\SoccerField\materials\scripts\SoccerField.material; DestDir: {app}\data\levels\SoccerField\materials\scripts
Source: data\levels\SoccerField\models\Ball.mesh; DestDir: {app}\data\levels\SoccerField\models
Source: data\levels\SoccerField\models\Box01.mesh; DestDir: {app}\data\levels\SoccerField\models
Source: data\levels\SoccerField\models\Box02.mesh; DestDir: {app}\data\levels\SoccerField\models
Source: data\levels\SoccerField\models\Checkpoint0.mesh; DestDir: {app}\data\levels\SoccerField\models
Source: data\levels\SoccerField\models\field.mesh; DestDir: {app}\data\levels\SoccerField\models
Source: data\levels\SoccerField\models\finish.mesh; DestDir: {app}\data\levels\SoccerField\models
Source: data\levels\SoccerField\models\FinishRoom.mesh; DestDir: {app}\data\levels\SoccerField\models
Source: data\levels\SoccerField\models\FootballPlanet.mesh; DestDir: {app}\data\levels\SoccerField\models
Source: data\levels\SoccerField\models\Goal01.mesh; DestDir: {app}\data\levels\SoccerField\models
Source: data\levels\SoccerField\models\Goal02.mesh; DestDir: {app}\data\levels\SoccerField\models
Source: data\levels\SoccerField\models\Red01.mesh; DestDir: {app}\data\levels\SoccerField\models
Source: data\levels\SoccerField\models\Blue01.mesh; DestDir: {app}\data\levels\SoccerField\models
Source: data\levels\SoccerField\models\Red.mesh; DestDir: {app}\data\levels\SoccerField\models
Source: data\levels\SoccerField\models\Wall.mesh; DestDir: {app}\data\levels\SoccerField\models
Source: data\levels\SoccerField\textures\darktexture.jpg; DestDir: {app}\data\levels\SoccerField\textures
Source: data\levels\SoccerField\textures\goalfence.jpg; DestDir: {app}\data\levels\SoccerField\textures
Source: data\levels\SoccerField\textures\soccer.png; DestDir: {app}\data\levels\SoccerField\textures
Source: data\levels\SoccerField\textures\grass.jpg; DestDir: {app}\data\levels\SoccerField\textures
Source: data\levels\SoccerField\textures\soccerfield.jpg; DestDir: {app}\data\levels\SoccerField\textures
Source: data\levels\SoccerField\textures\walltexture.jpg; DestDir: {app}\data\levels\SoccerField\textures

;Levels
Source: data\levels\Bavaraf.hkx; DestDir: {app}\data\levels
Source: data\levels\Bavaraf.path; DestDir: {app}\data\levels
Source: data\levels\Bavaraf.scene; DestDir: {app}\data\levels

Source: data\levels\Junkyard.hkx; DestDir: {app}\data\levels
Source: data\levels\Junkyard.path; DestDir: {app}\data\levels
Source: data\levels\Junkyard.scene; DestDir: {app}\data\levels

Source: data\levels\Ramp.hkx; DestDir: {app}\data\levels
Source: data\levels\Ramp.path; DestDir: {app}\data\levels
Source: data\levels\Ramp.scene; DestDir: {app}\data\levels

Source: data\levels\SimpleTrack2.hkx; DestDir: {app}\data\levels
Source: data\levels\SimpleTrack2.path; DestDir: {app}\data\levels
Source: data\levels\SimpleTrack2.scene; DestDir: {app}\data\levels

Source: data\levels\SoccerField.hkx; DestDir: {app}\data\levels
Source: data\levels\SoccerField.path; DestDir: {app}\data\levels
Source: data\levels\SoccerField.scene; DestDir: {app}\data\levels




; Ogre stuff
Source: data\ogre\fonts\badabb.ttf; DestDir: {app}\data\ogre\fonts
Source: data\ogre\fonts\Badaboom.fontdef; DestDir: {app}\data\ogre\fonts
Source: data\ogre\fonts\Lynx.fontdef; DestDir: {app}\data\ogre\fonts
Source: data\ogre\fonts\Lynx.ttf; DestDir: {app}\data\ogre\fonts
Source: data\ogre\fonts\solo5.ttf; DestDir: {app}\data\ogre\fonts
Source: data\ogre\fonts\StarWars.fontdef; DestDir: {app}\data\ogre\fonts
Source: data\ogre\materials\NicknameOverlay.material; DestDir: {app}\data\ogre\materials
Source: data\ogre\particle\aureola.material; DestDir: {app}\data\ogre\particle
Source: data\ogre\particle\aureola.png; DestDir: {app}\data\ogre\particle
Source: data\ogre\particle\bigflame.png; DestDir: {app}\data\ogre\particle
Source: data\ogre\particle\black.png; DestDir: {app}\data\ogre\particle
Source: data\ogre\particle\darksmoke.png; DestDir: {app}\data\ogre\particle
Source: data\ogre\particle\Dust.particle; DestDir: {app}\data\ogre\particle
Source: data\ogre\particle\explosion.png; DestDir: {app}\data\ogre\particle
Source: data\ogre\particle\flare.png; DestDir: {app}\data\ogre\particle
Source: data\ogre\particle\flaretrail.png; DestDir: {app}\data\ogre\particle
Source: data\ogre\particle\PE_materials.material; DestDir: {app}\data\ogre\particle
Source: data\ogre\particle\PurpleFountain.particle; DestDir: {app}\data\ogre\particle
Source: data\ogre\particle\ring_flare.png; DestDir: {app}\data\ogre\particle
Source: data\ogre\particle\ring_flare2.png; DestDir: {app}\data\ogre\particle
Source: data\ogre\particle\smoke.png; DestDir: {app}\data\ogre\particle
Source: data\ogre\particle\Smokey.particle; DestDir: {app}\data\ogre\particle
Source: data\ogre\particle\Spark.particle; DestDir: {app}\data\ogre\particle
Source: data\ogre\particle\streak.png; DestDir: {app}\data\ogre\particle
Source: data\ogre\particle\Vortex.particle; DestDir: {app}\data\ogre\particle
Source: data\ogre\cursor.png; DestDir: {app}\data\ogre
Source: data\plugins_release\Plugin_BSPSceneManager.dll; DestDir: {app}\data\plugins_release
Source: data\plugins_release\Plugin_CgProgramManager.dll; DestDir: {app}\data\plugins_release
Source: data\plugins_release\Plugin_OctreeSceneManager.dll; DestDir: {app}\data\plugins_release
Source: data\plugins_release\Plugin_OctreeZone.dll; DestDir: {app}\data\plugins_release
Source: data\plugins_release\Plugin_ParticleFX.dll; DestDir: {app}\data\plugins_release
Source: data\plugins_release\Plugin_PCZSceneManager.dll; DestDir: {app}\data\plugins_release
Source: data\plugins_release\RenderSystem_Direct3D10.dll; DestDir: {app}\data\plugins_release
Source: data\plugins_release\RenderSystem_Direct3D9.dll; DestDir: {app}\data\plugins_release
Source: data\plugins_release\RenderSystem_GL.dll; DestDir: {app}\data\plugins_release
Source: data\scripts\AI\PathFollowing.lua; DestDir: {app}\data\scripts\AI
Source: data\scripts\AI\Utils.lua; DestDir: {app}\data\scripts\AI
Source: data\SkyX\c22.png; DestDir: {app}\data\SkyX
Source: data\SkyX\c22n.png; DestDir: {app}\data\SkyX
Source: data\SkyX\Cloud1.png; DestDir: {app}\data\SkyX
Source: data\SkyX\Cloud1_Normal.png; DestDir: {app}\data\SkyX
Source: data\SkyX\DensityClouds1.png; DestDir: {app}\data\SkyX
Source: data\SkyX\Noise.jpg; DestDir: {app}\data\SkyX
Source: data\SkyX\SkyX.material; DestDir: {app}\data\SkyX
Source: data\SkyX\SkyX_Clouds.hlsl; DestDir: {app}\data\SkyX
Source: data\SkyX\SkyX_Ground.hlsl; DestDir: {app}\data\SkyX
Source: data\SkyX\SkyX_Moon.hlsl; DestDir: {app}\data\SkyX
Source: data\SkyX\SkyX_Moon.png; DestDir: {app}\data\SkyX
Source: data\SkyX\SkyX_Skydome.hlsl; DestDir: {app}\data\SkyX
Source: data\SkyX\SkyX_Starfield.png; DestDir: {app}\data\SkyX
Source: data\SkyX\SkyX_VolClouds.hlsl; DestDir: {app}\data\SkyX
Source: data\sound\HovSound.fev; DestDir: {app}\data\sound
Source: data\sound\HovSound.h; DestDir: {app}\data\sound
Source: data\sound\HovUniv.fsb; DestDir: {app}\data\sound
Source: data\user\game.userdatatypes.xml; DestDir: {app}\data\user
Source: data\user\types.userdatatypes.xml; DestDir: {app}\data\user
Source: data\cg.dll; DestDir: {app}\data\
Source: data\engine_settings.cfg; DestDir: {app}\data\
Source: data\Flash.ocx; DestDir: {app}\data\
Source: data\plugins_release.cfg; DestDir: {app}\data\
Source: data\resources.cfg; DestDir: {app}\data\
Source: data\SingleplayerServer.ini; DestDir: {app}\data\

[Dirs]
Name: {app}\data
Name: {app}\data\characters
Name: {app}\data\characters\materials
Name: {app}\data\characters\materials\scripts
Name: {app}\data\characters\materials\textures
Name: {app}\data\characters\models
Name: {app}\data\controls
Name: {app}\data\entities
Name: {app}\data\gui
Name: {app}\data\hovercraft
Name: {app}\data\hovercraft\materials
Name: {app}\data\hovercraft\materials\scripts
Name: {app}\data\hovercraft\materials\textures
Name: {app}\data\hovercraft\models
Name: {app}\data\levels
Name: {app}\data\levels\Bavaraf
Name: {app}\data\levels\Bavaraf\materials
Name: {app}\data\levels\Bavaraf\materials\scripts
Name: {app}\data\levels\Bavaraf\models
Name: {app}\data\levels\Bavaraf\textures
Name: {app}\data\levels\junkyard
Name: {app}\data\levels\junkyard\materials
Name: {app}\data\levels\junkyard\materials\scripts
Name: {app}\data\levels\junkyard\models
Name: {app}\data\levels\junkyard\textures
Name: {app}\data\levels\materials
Name: {app}\data\levels\materials\programs
Name: {app}\data\levels\Ramp
Name: {app}\data\levels\Ramp\materials
Name: {app}\data\levels\Ramp\materials\scripts
Name: {app}\data\levels\Ramp\models
Name: {app}\data\levels\Ramp\textures
Name: {app}\data\levels\SimpleTrack2
Name: {app}\data\levels\SimpleTrack2\materials
Name: {app}\data\levels\SimpleTrack2\materials\scripts
Name: {app}\data\levels\SimpleTrack2\models
Name: {app}\data\levels\SimpleTrack2\textures
Name: {app}\data\levels\SoccerField
Name: {app}\data\levels\SoccerField\materials
Name: {app}\data\levels\SoccerField\materials\scripts
Name: {app}\data\levels\SoccerField\models
Name: {app}\data\levels\SoccerField\textures
Name: {app}\data\ogre
Name: {app}\data\ogre\fonts
Name: {app}\data\ogre\materials
Name: {app}\data\ogre\particle
Name: {app}\data\plugins_release
Name: {app}\data\scripts
Name: {app}\data\scripts\AI
Name: {app}\data\SkyX
Name: {app}\data\sound
Name: {app}\data\user
