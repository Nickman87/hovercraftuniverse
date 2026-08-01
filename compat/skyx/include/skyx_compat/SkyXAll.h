// hu_skyx_compat -- drop-in SkyX 0.1 API shim (sky/cloud/star rendering
// disabled). Owned by this porting effort (compat/skyx/**), not a port of
// anything under HovercraftUniverse/. See docs/porting/skyx-compat.md.
//
// Why this exists: the game is built against SkyX 0.1 (LGPL, Xavier
// Verguin Gonzalez), a small Ogre add-on for sky/cloud/star rendering.
// Only prebuilt VC9 binaries + headers survive under
// HovercraftUniverse/dependencies/SkyX/ (see archive-mirror/SkyX_0_1.rar --
// confirmed to contain only headers + VC9 .lib/.dll, no .cpp sources), and
// SkyX is a real C++-ABI library (virtual calls, no C shim), so the VC9
// binaries cannot be relinked against MSVC v143. A real SkyX 0.4 source
// mirror was found (github.com/TommyTeaVee/ogre_skyx) and considered, but
// its public API already diverged from the 0.1 headers this game was
// written against (SkyX::SkyX takes a Controller* instead of a bare
// Ogre::Camera*, and per-camera updates moved to a separate
// notifyCameraRender()/RenderTargetListener step) -- porting it would mean
// both an Ogre 1.7->14 API-gap pass *and* an adapter layer in
// InGameState.cpp, plus unverified compatibility between its GPUManager's
// shader-parameter conventions and this game's existing
// data/levels/materials/programs/SkyX.material (authored for 0.1). Given
// the game's entire SkyX usage is 6 call sites in one file
// (InGameState.cpp/.h: construct, create(), getAtmosphereManager()
// get/setOptions(), remove(), update()), a from-scratch API-compatible
// shim (this file) is the lower-risk, faster path to LINK+RUN. Sky
// rendering is inert as a result -- honestly logged once, never faked.
//
// Mirrors the single-umbrella-header pattern established by
// compat/havok/include/havok_compat/HavokAll.h: one header with every
// class body, instead of trying to keep ~10 tiny per-class headers (like
// real SkyX splits its API across) in sync by hand.

#ifndef HU_SKYX_COMPAT_SKYXALL_H
#define HU_SKYX_COMPAT_SKYXALL_H

#include <OgreCamera.h>
#include <OgreSceneManager.h>
#include <OgreVector2.h>
#include <OgreVector3.h>

namespace skyx_compat {
// Logs a distinct "TODO(phaseB): <msg>" line once per distinct call site
// (matching zshim::todoPhaseBOnce()/havok_compat::todoPhaseBOnce()). SkyX's
// entire feature surface is Phase B here (there is no Phase A "wire real
// behavior" step for a sky renderer the way there was for physics/
// networking -- either it draws a sky or it doesn't), so every non-trivial
// entry point below logs through this once.
void todoPhaseBOnce(const char* site, const std::string& msg);
} // namespace skyx_compat

namespace SkyX {

class SkyX;

// ---------------------------------------------------------------------------
// AtmosphereManager -- matches HovercraftUniverse/dependencies/SkyX/include/
// AtmosphereManager.h's Options struct field-for-field (same defaults) and
// its getOptions()/setOptions() pair, since InGameState.cpp constructs an
// Options by value from getOptions(), mutates a couple of fields, and passes
// it back to setOptions(). Storage only -- no shader parameters exist to
// push the values to (no sky dome is created), so setOptions() just logs
// once and keeps the values (harmless, matches "never fake visuals").
// ---------------------------------------------------------------------------
class AtmosphereManager {
public:
    struct Options {
        Ogre::Vector3 Time;
        Ogre::Vector2 EastPosition;
        Ogre::Real InnerRadius;
        Ogre::Real OuterRadius;
        Ogre::Real HeightPosition;
        Ogre::Real RayleighMultiplier;
        Ogre::Real MieMultiplier;
        Ogre::Real SunIntensity;
        Ogre::Vector3 WaveLength;
        Ogre::Real G;
        Ogre::Real Exposure;
        int NumberOfSamples;

        Options()
            : Time(14.0f, 7.50f, 20.50f)
            , EastPosition(0, 1)
            , InnerRadius(9.77501f)
            , OuterRadius(10.2963f)
            , HeightPosition(0.01f)
            , RayleighMultiplier(0.0022f)
            , MieMultiplier(0.000675f)
            , SunIntensity(30)
            , WaveLength(0.57f, 0.54f, 0.44f)
            , G(-0.991f)
            , Exposure(2.0f)
            , NumberOfSamples(4) {}
    };

    explicit AtmosphereManager(SkyX* parent) : mSkyX(parent) {}

    inline void setOptions(const Options& newOptions) {
        mOptions = newOptions;
        skyx_compat::todoPhaseBOnce(
            "AtmosphereManager::setOptions",
            "SkyX sky rendering is stubbed (compat/skyx shim, no real SkyX "
            "port); atmosphere options are stored but never rendered.");
    }

    inline const Options& getOptions() const { return mOptions; }

private:
    Options mOptions;
    SkyX* mSkyX;
};

// ---------------------------------------------------------------------------
// CloudsManager / VCloudsManager / MoonManager / ColorGradient -- not called
// by any current game code path (InGameState.cpp only ever reaches
// getAtmosphereManager(); its one getCloudsManager() call is commented out,
// see the source comment there), but declared per the task brief ("whatever
// the game actually calls" plus these named types) so the API surface reads
// as complete, and so any future re-enablement of the commented-out cloud
// layer call compiles against a real (if inert) type instead of needing yet
// another shim pass.
// ---------------------------------------------------------------------------
class ColorGradient {
public:
    void addCFrame(void* /*unused*/) {
        skyx_compat::todoPhaseBOnce("ColorGradient::addCFrame",
                                     "SkyX ColorGradient is stubbed; no gradient keyframes are stored.");
    }
};

class CloudsManager {
public:
    struct CloudLayerOptions {};

    explicit CloudsManager(SkyX* /*parent*/) {}

    void* add(const CloudLayerOptions&) {
        skyx_compat::todoPhaseBOnce("CloudsManager::add",
                                     "SkyX CloudsManager is stubbed; no cloud layer geometry is created.");
        return nullptr;
    }

    void removeAll() {}
};

class VCloudsManager {
public:
    explicit VCloudsManager(SkyX* /*parent*/) {}

    void create() {
        skyx_compat::todoPhaseBOnce("VCloudsManager::create",
                                     "SkyX VCloudsManager is stubbed; no volumetric cloud field is created.");
    }
    void remove() {}
    void _updateWindSpeedConfig() {}
};

class MoonManager {
public:
    explicit MoonManager(SkyX* /*parent*/) {}

    void setAutoUpdate(bool) {}
};

// ---------------------------------------------------------------------------
// SkyX -- the class InGameState.cpp actually instantiates. Constructor
// signature matches the 0.1 header exactly: (Ogre::SceneManager*,
// Ogre::Camera*). create()/remove()/update() are all real, cheap no-ops
// (they intentionally do NOT create any scene node, material, or texture --
// "make it inert and clearly logged", not fake visuals). Logs once at
// create() so a real playtest session shows, one time, that sky rendering
// is absent.
// ---------------------------------------------------------------------------
class SkyX {
public:
    enum LightingMode { LM_LDR = 0, LM_HDR = 1 };

    SkyX(Ogre::SceneManager* sm, Ogre::Camera* cam)
        : mSceneManager(sm)
        , mCamera(cam)
        , mCreated(false)
        , mLightingMode(LM_LDR)
        , mStarfield(true)
        , mTimeMultiplier(0.0f)
        , mAtmosphereManager(this)
        , mCloudsManager(this)
        , mVCloudsManager(this)
        , mMoonManager(this) {}

    ~SkyX() { remove(); }

    void create() {
        if (mCreated) return;
        mCreated = true;
        skyx_compat::todoPhaseBOnce(
            "SkyX::create",
            "Real SkyX source (0.1, matching this game's API) could not be "
            "recovered -- archive-mirror/SkyX_0_1.rar contains only VC9 "
            "headers/binaries, no .cpp sources. This is compat/skyx, a "
            "from-scratch API-compatible shim: no sky dome, sun, moon, "
            "stars, or clouds are rendered. See docs/porting/skyx-compat.md.");
    }

    void remove() { mCreated = false; }

    void update(const Ogre::Real& /*timeSinceLastFrame*/) {
        // Real SkyX advances sun/moon/star/cloud animation state here.
        // Nothing is created, so there is nothing to advance.
    }

    inline bool isCreated() const { return mCreated; }

    inline void setTimeMultiplier(const Ogre::Real& t) { mTimeMultiplier = t; }
    inline const Ogre::Real& getTimeMultiplier() const { return mTimeMultiplier; }

    inline AtmosphereManager* getAtmosphereManager() { return &mAtmosphereManager; }
    inline CloudsManager* getCloudsManager() { return &mCloudsManager; }
    inline VCloudsManager* getVCloudsManager() { return &mVCloudsManager; }
    inline MoonManager* getMoonManager() { return &mMoonManager; }

    void setLightingMode(const LightingMode& lm) { mLightingMode = lm; }
    inline const LightingMode& getLightingMode() const { return mLightingMode; }

    void setStarfieldEnabled(const bool& enabled) { mStarfield = enabled; }
    inline const bool& isStarfieldEnabled() const { return mStarfield; }

    inline Ogre::SceneManager* getSceneManager() { return mSceneManager; }
    inline Ogre::Camera* getCamera() { return mCamera; }

private:
    Ogre::SceneManager* mSceneManager;
    Ogre::Camera* mCamera;
    bool mCreated;
    LightingMode mLightingMode;
    bool mStarfield;
    Ogre::Real mTimeMultiplier;

    AtmosphereManager mAtmosphereManager;
    CloudsManager mCloudsManager;
    VCloudsManager mVCloudsManager;
    MoonManager mMoonManager;
};

} // namespace SkyX

#endif // HU_SKYX_COMPAT_SKYXALL_H
