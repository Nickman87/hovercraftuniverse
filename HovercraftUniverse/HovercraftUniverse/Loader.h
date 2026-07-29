#ifndef LOADER_H
#define LOADER_H

#include "RaceState.h"
#include "CustomOgreMaxSceneCallback.h"

namespace HovUni {

/**
 * An base class for all loaders
 */
class Loader: public CustomOgreMaxSceneCallback {
protected:

	// The race state
	RaceState* mRaceState;

public:

	/**
	 * Constructor
	 */
	Loader(void);

	/**
	 * Destructor
	 */
	virtual ~Loader(void);

	/**
	 * Load given scene file
	 * @param filename
	 */
	virtual void load(const Ogre::String& filename);

	/**
	 * Loads the scene file from the scene stored in the race state.
	 */
	virtual void load();

	/**
	 * Set the race state the loader belongs to
	 *
	 * @param state the race state
	 */
	inline virtual void setRaceState(RaceState * state) {
		mRaceState = state;
	}

	/**
	 * Get the race state the loader belongs to
	 *
	 * @return the race state
	 */
	inline virtual RaceState* getRaceState() {
		return mRaceState;
	}

	// All callbacks do nothing by default

	virtual void StartedLoad() {
	}

	virtual void FinishedLoad(bool success) {
	}

	virtual void UpdatedLoadProgress(Ogre::Real progress) {
	}

	virtual void onSceneUserData(const Ogre::String& userDataReference, const Ogre::String& userData) {
	}
	;

	virtual void onExternal(OgreMax::Types::ExternalItem& externalitem) {
	}
	;

	virtual void onSceneData(const OgreMax::Version& formatVersion, const OgreMax::Version& minOgreVersion,
			const OgreMax::Version& ogreMaxVersion, const Ogre::String& author, const Ogre::String& application,
			OgreMax::Types::UpAxis upAxis, Ogre::Real unitsPerMeter, const Ogre::String& unitType) {
	}

	virtual void onSkyBox(OgreMax::Types::SkyBoxParameters& skyBoxParameters,
			std::vector<OgreMax::Types::NodeAnimation> * animation) {
	}

	virtual void onSkyDome(OgreMax::Types::SkyDomeParameters& skyDomeParameters,
			std::vector<OgreMax::Types::NodeAnimation> * animation) {
	}

	virtual void onSkyPlane(OgreMax::Types::SkyPlaneParameters& skyPlaneParameters,
			std::vector<OgreMax::Types::NodeAnimation> * animation) {
	}

	virtual void onClipping(Ogre::Real environmentNear, Ogre::Real environmentFar) {
	}

	virtual void onFog(OgreMax::Types::FogParameters& fog) {
	}

	virtual void onAmbientColour(const Ogre::ColourValue& colour) {
	}

	virtual void onShadowProperties(OgreMax::Types::ShadowParameters& parameter) {
	}

	virtual void onBackgroundColour(const Ogre::ColourValue& colour) {
	}

	virtual void onRootNode(const Ogre::Vector3& position, const Ogre::Quaternion& rotation, const Ogre::Vector3& scale) {
	}

	virtual void onLight(OgreMax::Types::LightParameters& light, const OgreMax::Types::Attachable * parent) {
	}

	virtual void onCamera(OgreMax::Types::CameraParameters& params, const OgreMax::Types::Attachable * parent) {
	}

	virtual void onBillboardSet(OgreMax::Types::BillboardSetParameters& bilboardsetparameters, std::vector<
			OgreMax::Types::Billboard>& billboardset, std::vector<OgreMax::Types::CustomParameter>& customParameters,
			const OgreMax::Types::Attachable * parent) {
	}

	virtual void onParticleSystem(OgreMax::Types::ParticleSystemParameters& particleSystem,
			const OgreMax::Types::Attachable * parent) {
	}

	virtual void onPlane(OgreMax::Types::PlaneParameters planeparameters, const OgreMax::Types::Attachable * parent) {
	}

	virtual void onSceneFile(const Ogre::String& fileName, Ogre::String& resourceGroupName) {
	}

	virtual void onResourceLocation(const Ogre::String& name, const Ogre::String& type, bool recursive) {
	}

	virtual void onExternalUserData(OgreMax::Types::ExternalUserData& externalud) {
	}

	virtual void onEntity(OgreMax::Types::EntityParameters& entityparameters, const OgreMax::Types::Attachable * parent) {
	}

	virtual void onNode(OgreMax::Types::NodeParameters& nodeparameters, std::vector<OgreMax::Types::NodeAnimation> * animation,
			const OgreMax::Types::NodeParameters* parent) {
	}
};

}

#endif
