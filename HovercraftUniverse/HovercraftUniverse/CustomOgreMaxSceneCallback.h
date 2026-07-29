#ifndef CUSTOMOGREMAXSCENECALLBACK_H
#define CUSTOMOGREMAXSCENECALLBACK_H

#include "OgreMaxTypesExtension.h"
#include "Version.hpp"

namespace HovUni {

    /** 
	 * A interface that receives notifications during the loading of the scene 
	 */
    class CustomOgreMaxSceneCallback
    {
    public:

        /**
		 * Destructor
		 */
		virtual ~CustomOgreMaxSceneCallback() {
		}

		// MISC //

		/**
		 * Called when the scene file was found
		 * @param filename
		 * @param resourcegroupname
		 */
		virtual void onSceneFile( const Ogre::String& fileName, Ogre::String& resourceGroupName) = 0;

		/** 
		 * Called before a scene is to be loaded. 
		 * At this stage the xml has been parsed successfuly and is placed in memory.
		 */
        virtual void StartedLoad() = 0;

        /**
         * Called after a scene is loaded.
         */
        virtual void FinishedLoad( bool success ) = 0;

 	    /** 
		 * Called when Progress is made
		 * @param progress
		 */
        virtual void UpdatedLoadProgress( Ogre::Real progress ) = 0;

		// OGRE MISC //

		/**
		 * When resource location loaded
		 * @param name
		 * @param type
		 * @param recursive
		 */
		virtual void onResourceLocation ( const Ogre::String& name, const Ogre::String& type, bool recursive) = 0;

		// USER DATA //

		/**
		 * Called when the scene tag has been parsed, a check to Ogre version has been done here
		 * @param formatVersion, version of scene format
		 * @param minOgreVersion
		 * @param ogreMaxVersion
		 * @param author
		 * @param upaxis
		 * @param unitsPerMeter
		 * @param unitType
		 */
		virtual void onSceneData( const OgreMax::Version& formatVersion, const OgreMax::Version& minOgreVersion, const OgreMax::Version& ogreMaxVersion, const Ogre::String& author, const Ogre::String& application, OgreMax::Types::UpAxis upAxis, Ogre::Real unitsPerMeter, const Ogre::String& unitType) = 0;

	    /**
		 * Called when scene user data is loaded. Will only be called if there is user data.
		 * Scene user data is unique for the scene so this method will be called only once.
		 * @param userdataref
		 * @param userdata
		 */
        virtual void onSceneUserData(const Ogre::String& userDataReference, const Ogre::String& userData) = 0;

		// ExternalUserData //

		/**
		 * A scene can hav a list of external data. This is flat text that doesn't have any positional data.
		 * @param externalud
		 */
		virtual void onExternalUserData( OgreMax::Types::ExternalUserData& externalud)= 0;

		/**
		 * Called when external item is loaded, a external item is external data but contains much more info.
		 * It has a position, scale, orientation and optionaly a bounding box. 
		 * It can be seen as a scene node that should not be used in ogre.
		 * A externalitem can also have user data.
		 * @param externalitem
		 */
		virtual void onExternal( OgreMax::Types::ExternalItem& externalitem) = 0;

		// OGRE ENVIROMENT //

		/**
		 * Called when clipping info is loaded
		 * @param environmentNear, near clipping distance
		 * @param environmentFar, far clipping distance
		 */
		virtual void onClipping( Ogre::Real environmentNear, Ogre::Real environmentFar) = 0;

		/**
		 * Called when general shadow properties are loaded
		 * @param shadowparameters
		 */
		virtual void onShadowProperties( OgreMax::Types::ShadowParameters& shadowparameters ) = 0;

		/**
		 * Called when the genereal background color of the viewports are loaded
		 * @param colour
		 */
		virtual void onBackgroundColour( const Ogre::ColourValue& colour ) = 0;

		/**
		 * Called when the world ambient lightning color is loaded
		 * @param colour
		 */
		virtual void onAmbientColour( const Ogre::ColourValue& colour ) = 0;

		/**
		 * Called when the general fog is loaded 
		 * @param fogparameters
		 */
		virtual void onFog ( OgreMax::Types::FogParameters& fogparameters ) = 0;

		// OGRE SCENE //

		/**
		 * Called when info is found for the root node.
		 * The rootnode cannot have userdata, use scene user data instead!
		 * @param position
		 * @param rotation
		 * @param scale
		 */
		virtual void onRootNode ( const Ogre::Vector3& position, const Ogre::Quaternion& rotation, const Ogre::Vector3& scale ) = 0;

		/**
		 * Called when al node info is loaded but before any child node is attached
		 * @param nodeparameters
		 * @param nodeanimation, 0 if no animation
		 * @param parent, 0 if parent is rootnode
		 */
		virtual void onNode( OgreMax::Types::NodeParameters& nodeparameters, std::vector<OgreMax::Types::NodeAnimation> * nodeanimation, const OgreMax::Types::NodeParameters* parent) = 0;

		/**
		 * Called when the skybox is loaded
		 * @param skyBoxParameters
		 * @param animation, 0 if not defined
		 */
		virtual void onSkyBox( OgreMax::Types::SkyBoxParameters& skyBoxParameters, std::vector<OgreMax::Types::NodeAnimation> * animation ) = 0;

		/**
		 * Called when the skydome is loaded
		 * @param skyDomeParameters
		 * @param animation, 0 if not defined
		 */
        virtual void onSkyDome( OgreMax::Types::SkyDomeParameters& skyDomeParameters, std::vector<OgreMax::Types::NodeAnimation>* animation) = 0;

		/**
		 * Called when the skyplane is loaded
		 * @param skyPlaneParameters
		 * @param animation, 0 if not defined
		 */
        virtual void onSkyPlane( OgreMax::Types::SkyPlaneParameters& skyPlaneParameters, std::vector<OgreMax::Types::NodeAnimation> * animation) = 0;

		/**
		 * Called when a light is loaded
		 * @param lightparameters
		 * @param parent, 0 if parent rootnode
		 */
		virtual void onLight( OgreMax::Types::LightParameters& lightparameters, const OgreMax::Types::Attachable * parent ) = 0;
        
		/**
		 * Called when a camera is loaded
		 * @param cameraparameters
		 * @param parent, 0 if parent rootnode
		 */
		virtual void onCamera(OgreMax::Types::CameraParameters& params, const OgreMax::Types::Attachable * parent ) = 0;
        
		/**
		 * Called when an ogre entity is loaded
		 * @param entityparameters
		 * @param parent, 0 if parent rootnode
		 */
		virtual void onEntity( OgreMax::Types::EntityParameters& entityparameters, const OgreMax::Types::Attachable * parent ) = 0;
        
		/**
		 * Called when a particle system is loaded
		 * @param particleSystem
		 * @param parent, 0 if parent rootnode
		 */
		virtual void onParticleSystem( OgreMax::Types::ParticleSystemParameters& particleSystem, const OgreMax::Types::Attachable * parent) = 0;
        
		/**
		 * Called when a billboardset is loaded
		 * @param bilboardsetparameters
		 * @param billboardset
		 * @param custom data for each billboard
		 * @param parent, 0 if parent rootnode
		 */
		virtual void onBillboardSet( OgreMax::Types::BillboardSetParameters& bilboardsetparameters, std::vector<OgreMax::Types::Billboard>& billboardset, std::vector<OgreMax::Types::CustomParameter>& customParameters, const OgreMax::Types::Attachable * parent ) = 0;
        
		/**
		 * Called when a plane is loaded
		 * @param planeparameters
		 * @param parent, 0 if parent rootnode
		 */
		virtual void onPlane( OgreMax::Types::PlaneParameters planeparameters, const OgreMax::Types::Attachable * parent) = 0;
    };

}

#endif