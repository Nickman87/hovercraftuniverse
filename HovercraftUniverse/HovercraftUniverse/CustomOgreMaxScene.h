#ifndef CUSTOMOGREMAXSCENE_INCLUDED
#define CUSTOMOGREMAXSCENE_INCLUDED

#include "OgreMaxTypes.hpp"
#include "OgreMaxPlatform.hpp"
#include "ProgressCalculator.hpp"
#include "Version.hpp"
#include "CustomOgreMaxSceneCallback.h"

namespace HovUni {

/**
 * Manages the loading of a OgreMax .scene file.
 * Use a CustomOgreMaxSceneCallback to set up your scene:
 * CustomOgreMaxScene scene;
 * CustomOgreMaxSceneCallback * callback = new MyCallback(someogrepointers);
 * scene.load(file, OPTIONS, callback);
 * @author PJ
 */
class CustomOgreMaxScene
{

public:
    CustomOgreMaxScene();

	virtual ~CustomOgreMaxScene();

    /**
     * Options that may be passed to Load()
     */
    typedef int LoadOptions;
    enum
    {
        NO_OPTIONS = 0,

        /** Skips the environment settings in the file. Skipping the environment also skips shadows */
        SKIP_ENVIRONMENT = 0x1,

        /** Skips the shadow settings in the file */
        SKIP_SHADOWS = 0x2,

        /** Skips the sky settings in the file */
        SKIP_SKY = 0x4,

        /** Skips the nodes in the file */
        SKIP_NODES = 0x8,

        /** Skips the externals in the file */
        SKIP_EXTERNALS = 0x10,

        /** Skips the terrain in the file */
        SKIP_TERRAIN = 0x20,

        /** Skips scene level query flags in the file */
        SKIP_QUERY_FLAG_ALIASES = 0x200,

        /** Skips scene level visibility flags in the file */
        SKIP_VISIBILITY_FLAG_ALIASES = 0x400,

        /** Skips scene level resource locations in the file */
        SKIP_RESOURCE_LOCATIONS = 0x800,

        /** Indicates animation states should not be created for node animation tracks */
        NO_ANIMATION_STATES = 0x1000,

        /**
         * Indicates scene 'externals' should not be stored. They will, however, still
         * be loaded and the CreatedExternal() scene callback will be called
         */
        NO_EXTERNALS = 0x2000,

        /**
         * By default, the OgreMaxScene::Load() checks to see if the file that was passed
         * in exists in the file system (outside of the configured resource locations).
         * This flag disables that logic
         */
        NO_FILE_SYSTEM_CHECK = 0x4000,

        /**
         * Indicates that a light should be created if the loaded scene doesn't contain a light
         * Any lights created as a result of setting the default lighting are not passed to
         * the scene callback
         */
        SET_DEFAULT_LIGHTING = 0x8000,
    };

    /**
     * Loads a scene from the specified file.
     * @param fileNameOrContent [in] - The name of the file to load.
     * @param loadOptions [in] - Options used to specify the loading behavior. Optional.
	 * @param callback [in] - Pointer to a callback object. Optional.
     * @param defaultResourceGroupName [in] - The resource group name that is used by default when
     * creating new resources. Optional.
     */
    void Load ( const Ogre::String& fileName, CustomOgreMaxSceneCallback * callback = 0, LoadOptions loadOptions = NO_OPTIONS,
        const Ogre::String& defaultResourceGroupName = Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME
        );

private:

	Ogre::String CustomOgreMaxScene::getUniqueName();
    const Ogre::Vector3& GetUpVector() const;
    void Load ( TiXmlElement* objectElement, LoadOptions loadOptions = NO_OPTIONS, CustomOgreMaxSceneCallback * callback = 0, const Ogre::String& defaultResourceGroupName = Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME );
    Ogre::String GetNewObjectName(const TiXmlElement* objectElement);
	void LoadObjectNames(const TiXmlElement* objectElement, const Ogre::String& elementName, std::vector<Ogre::String>& names);

	/** Updates the progress for the specified progress calculator. */
	void UpdateLoadProgress(OgreMax::ProgressCalculator* calculator, Ogre::Real amount);

    void LoadScene(const TiXmlElement* objectElement);
    
	//Load Resource DONE
    void LoadResourceLocations(const TiXmlElement* objectElement);

	//Node DONE
    void LoadNodes(const TiXmlElement* objectElement);
	void LoadNode(const TiXmlElement* objectElement, OgreMax::Types::NodeParameters * parent);

	//Load Entity DONE
    void LoadEntity(const TiXmlElement* objectElement, OgreMax::Types::Attachable * parent);
	void LoadBoneAttachments(const TiXmlElement* objectElement, OgreMax::Types::EntityParameters * entity);
    void LoadBoneAttachment(const TiXmlElement* objectElement, OgreMax::Types::EntityParameters * entity);

	//Load Light DONE
	void LoadLight(const TiXmlElement* objectElement, OgreMax::Types::Attachable * parent);
	void LoadLightRange(const TiXmlElement* objectElement, OgreMax::Types::LightParameters& light);
    void LoadLightAttenuation(const TiXmlElement* objectElement, OgreMax::Types::LightParameters& light);

	//Load Camera DONE
	void LoadCamera(const TiXmlElement* objectElement, OgreMax::Types::Attachable * parent);

	//Load ParticleSystem DONE
    void LoadParticleSystem(const TiXmlElement* objectElement, OgreMax::Types::Attachable * parent);
    
	//Load Plane DONE
    void LoadPlane(const TiXmlElement* objectElement, OgreMax::Types::Attachable * parent);
    
	//Bilboards DONE
	void LoadBillboardSet(const TiXmlElement* objectElement, OgreMax::Types::Attachable * parent);
	void LoadBillboard(const TiXmlElement* objectElement, std::vector<OgreMax::Types::Billboard>& billboardset);

	//animation DONE
	std::vector<OgreMax::Types::NodeAnimation> LoadNodeAnimations(const TiXmlElement* objectElement);
    OgreMax::Types::NodeAnimation LoadNodeAnimation(const TiXmlElement* objectElement);
	void LoadNodeAnimationKeyFrame(const TiXmlElement* objectElement, std::vector<OgreMax::Types::KeyFrame>& animationTrack);

	//External DONE
    void LoadExternals(const TiXmlElement* objectElement);
    void LoadExternalUserDatas(const TiXmlElement* objectElement);
	void LoadExternalItem(const TiXmlElement* objectElement );
	void LoadExternalUserData(const TiXmlElement* objectElement, OgreMax::Types::ExternalUserData& userData);

	//Enviroment DONE
    void LoadEnvironment(const TiXmlElement* objectElement);        		
    void LoadFog(const TiXmlElement* objectElement);
    void LoadSkyBox(const TiXmlElement* objectElement);
    void LoadSkyDome(const TiXmlElement* objectElement);
    void LoadSkyPlane(const TiXmlElement* objectElement);
    void LoadClipping(const TiXmlElement* objectElement, Ogre::Real& nearClip, Ogre::Real& farClip);
    void LoadShadows(const TiXmlElement* objectElement);

	//Load Flags TODO
	void LoadQueryFlagAliases(const TiXmlElement* objectElement);
    void LoadVisibilityFlagAliases(const TiXmlElement* objectElement);

protected:
    LoadOptions loadOptions;

    CustomOgreMaxSceneCallback* callback;

	OgreMax::Types::UpAxis upAxis;

	Ogre::String defaultResourceGroupName;

    bool loadedFromFileSystem;

    /** Tracks scene loading progress. */
	class LoadSceneProgressCalculator : public OgreMax::ProgressCalculator
    {
    public:
        LoadSceneProgressCalculator()
        {
            this->nodes = AddCalculator("nodes");
        }

    public:
        //Child calculators
        ProgressCalculator* nodes;
    };
    LoadSceneProgressCalculator loadProgress;

	int mUniqueID;
};

}

#endif
