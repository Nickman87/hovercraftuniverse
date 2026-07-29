//Includes---------------------------------------------------------------------
#include "CustomOgreMaxScene.h"
#include "OgreMaxUtilities.hpp"
using namespace Ogre;

namespace HovUni {

//Implementation---------------------------------------------------------------

CustomOgreMaxScene::CustomOgreMaxScene()
{
	this->callback = 0;
	this->loadedFromFileSystem = false;
	this->loadOptions = NO_OPTIONS;
}

CustomOgreMaxScene::~CustomOgreMaxScene()
{
}

const Vector3& CustomOgreMaxScene::GetUpVector() const
{
	return this->upAxis == OgreMax::Types::UP_AXIS_Y ? Vector3::UNIT_Y : Vector3::UNIT_Z;
}

//load
void CustomOgreMaxScene::Load ( const String& fileName, CustomOgreMaxSceneCallback* callback, LoadOptions loadOptions, const String& defaultResourceGroupName ) {
	this->mUniqueID = 0;
	this->callback = callback;


    //Notify callback, possibly getting a new resource group name
    String resourceGroupName = defaultResourceGroupName;
	if (this->callback != 0){
		this->callback->onSceneFile(fileName, resourceGroupName);
	}

    //Load the XML document
    TiXmlDocument document;
	OgreMax::OgreMaxUtilities::LoadXmlDocument(fileName, document, resourceGroupName);
    Load(document.RootElement(), loadOptions, callback, resourceGroupName);
}

void CustomOgreMaxScene::Load ( TiXmlElement* objectElement, LoadOptions loadOptions, CustomOgreMaxSceneCallback* callback, const String& defaultResourceGroupName ) {
    this->callback = callback;
	this->defaultResourceGroupName = defaultResourceGroupName; 

	//Send "start" notification to callback
	if (this->callback != 0){
		this->callback->StartedLoad();
	}

    try {
        //Perform scene loading
        LoadScene(objectElement);
    }
    catch (...) {
        //Send "ended with failure" notification to callback
        if (this->callback != 0)
            this->callback->FinishedLoad(false);

        throw;
    }

    //Send "ended with success" notification to callback
	if (this->callback != 0){
		this->callback->FinishedLoad(true);
	}
}



void CustomOgreMaxScene::LoadScene(const TiXmlElement* objectElement)
{
	static const OgreMax::Version CURRENT_OGRE_VERSION(OGRE_VERSION_MAJOR, OGRE_VERSION_MINOR, OGRE_VERSION_PATCH);
    static const String CURRENT_OGRE_VERSION_STRING = CURRENT_OGRE_VERSION.ToString();

	OgreMax::Types::ObjectExtraData sceneExtraData;
	sceneExtraData.id = OgreMax::OgreMaxUtilities::GetStringAttribute(objectElement, "id");
    OgreMax::Version formatVersion = OgreMax::Version(OgreMax::OgreMaxUtilities::GetStringAttribute(objectElement, "formatVersion"));
    OgreMax::Version minOgreVersion = OgreMax::Version(OgreMax::OgreMaxUtilities::GetStringAttribute(objectElement, "minOgreVersion"));
    OgreMax::Version ogreMaxVersion = OgreMax::Version(OgreMax::OgreMaxUtilities::GetStringAttribute(objectElement, "ogreMaxVersion"));
    String author = OgreMax::OgreMaxUtilities::GetStringAttribute(objectElement, "author");
    String application = OgreMax::OgreMaxUtilities::GetStringAttribute(objectElement, "application");
	this->upAxis = OgreMax::Types::UP_AXIS_Y;
	String upAxisText = OgreMax::OgreMaxUtilities::GetStringAttribute(objectElement, "upAxis");
	if (!upAxisText.empty()){
		this->upAxis = OgreMax::OgreMaxUtilities::ParseUpAxis(upAxisText);
	}
    Ogre::Real unitsPerMeter = OgreMax::OgreMaxUtilities::GetRealAttribute(objectElement, "unitsPerMeter", 0);
    Ogre::String unitType = OgreMax::OgreMaxUtilities::GetStringAttribute(objectElement, "unitType");

    //Make sure the Ogre version used to compile this viewer supports the Ogre version
    //required by the scene file
    if (minOgreVersion > CURRENT_OGRE_VERSION)
    {
        StringUtil::StrStreamType errorMessage;
        errorMessage << "The scene file's required Ogre version (" << minOgreVersion.ToString()
            << ") is not supported by the Ogre version this viewer uses ( " << CURRENT_OGRE_VERSION_STRING << ")";

        OGRE_EXCEPT
            (
            Exception::ERR_NOT_IMPLEMENTED,
		    errorMessage.str(),
		    "CustomOgreMaxScene::LoadScene"
            );
    }

	//callback
	if ( this->callback != 0 ){
		this->callback->onSceneData( formatVersion, minOgreVersion, ogreMaxVersion, author, application, upAxis, unitsPerMeter, unitType);
	}

    //Load scene user data
    const TiXmlElement* userDataReferenceElement = objectElement->FirstChildElement("userDataReference");
    const TiXmlElement* userDataElement = objectElement->FirstChildElement("userData");
    if (userDataReferenceElement != 0 || userDataElement != 0) {
        if (userDataReferenceElement != 0)
            OgreMax::OgreMaxUtilities::LoadUserDataReference(userDataReferenceElement, sceneExtraData.userDataReference);
        if (userDataElement != 0)
            OgreMax::OgreMaxUtilities::GetChildText(userDataElement, sceneExtraData.userData);

        if (this->callback != 0)
            this->callback->onSceneUserData(sceneExtraData.userDataReference, sceneExtraData.userData);
    }    

	//Load resource locations
    if ((this->loadOptions & SKIP_RESOURCE_LOCATIONS) == 0) {
        const TiXmlElement* resourceLocationsElement = objectElement->FirstChildElement("resourceLocations");
        if (resourceLocationsElement != 0 || this->loadedFromFileSystem) {
			LoadResourceLocations(resourceLocationsElement);
        }
    }

    //Load environment settings
    const TiXmlElement* environmentElement = objectElement->FirstChildElement("environment");
	if (environmentElement != 0){
		LoadEnvironment(environmentElement);
	}

    //Parse child elements
    String elementName;
    const TiXmlElement* childElement = 0;
    while (childElement = OgreMax::OgreMaxUtilities::IterateChildElements(objectElement, childElement)) {
        elementName = childElement->Value();

		if (elementName == "nodes") {
            LoadNodes(childElement);
		}
		else if (elementName == "externals") {
            LoadExternals(childElement);
		}
		else if (elementName == "externalUserData") {
            LoadExternalUserDatas(childElement);
		}
		/*else if (elementName == "terrain"){
            LoadTerrain(childElement);
		}*/
		else if (elementName == "light") {
            LoadLight(childElement,0);
		}
        else if (elementName == "camera") {
            LoadCamera(childElement,0);
		}
        else if (elementName == "queryFlags") {
            LoadQueryFlagAliases(childElement);
		}
        else if (elementName == "visibilityFlags") {
            LoadVisibilityFlagAliases(childElement);
		}
        /*else if (elementName == "instancedGeometries") {
            instancedGeometriesElement = childElement;
		}
        /*else if (elementName == "staticGeometries") {
            staticGeometriesElement = childElement;
		}
        else if (elementName == "portalConnectedZones") {
            portalConnectedZonesElement = childElement;
		}*/
    }
}

//misc
void CustomOgreMaxScene::LoadResourceLocations(const TiXmlElement* objectElement)
{
    //Exit early if resource locations are not allowed
	if (!this->loadedFromFileSystem) {
        return;
	}

    //Iterate over all the resource groups
    String elementName;
    const TiXmlElement* childElement = 0;
    while (childElement = OgreMax::OgreMaxUtilities::IterateChildElements(objectElement, childElement)) {
        elementName = childElement->Value();

        if (elementName == "resourceLocation")
        {
            //Name
            String name = OgreMax::OgreMaxUtilities::GetStringAttribute(childElement, "name");

            //Type
            String type = OgreMax::OgreMaxUtilities::GetStringAttribute(childElement, "type");

            //Recursive
            bool recursive = OgreMax::OgreMaxUtilities::GetBoolAttribute(childElement, "recursive", false);

			if ( this->callback ){
				this->callback->onResourceLocation(name,type,recursive);
			}
        }
    }
}

Ogre::String CustomOgreMaxScene::getUniqueName() {
	mUniqueID++;
	return "no_name_" + Ogre::StringConverter::toString(mUniqueID);
}

Ogre::String CustomOgreMaxScene::GetNewObjectName(const TiXmlElement* objectElement)
{
    //Get the name from either the "name" attribute or the node
    String name = OgreMax::OgreMaxUtilities::GetStringAttribute(objectElement, "name");
    if (name.empty() )
        name = getUniqueName();

    return name;
}



void CustomOgreMaxScene::UpdateLoadProgress(OgreMax::ProgressCalculator* calculator, Real amount)
{
    //Update the specified calculator
    calculator->Update(amount);

    //Send progress notification to callback
	if (this->callback != 0){
		this->callback->UpdatedLoadProgress(this->loadProgress.GetProgress());
	}
}

//TODO
void CustomOgreMaxScene::LoadQueryFlagAliases(const TiXmlElement* objectElement)
{
/*    //Exit early if skip option is set
    if ((this->loadOptions & SKIP_QUERY_FLAG_ALIASES) != 0)
        return;

    //Allocate the query flags
    size_t oldSize = this->queryFlags.size();
    size_t queryFlagCount = OgreMax::OgreMaxUtilities::GetElementCount(objectElement, "queryFlag");
    this->queryFlags.resize(oldSize + queryFlagCount);

    //Iterate over all the query flags
    size_t index = 0;
    String elementName;
    const TiXmlElement* childElement = 0;
    while (childElement = OgreMax::OgreMaxUtilities::IterateChildElements(objectElement, childElement))
    {
        this->queryFlags[oldSize + index].name = OgreMax::OgreMaxUtilities::GetStringAttribute(childElement, "name");
        this->queryFlags[oldSize + index].bit = OgreMax::OgreMaxUtilities::GetIntAttribute(childElement, "bit", 0);
        index++;
    }*/
}

void CustomOgreMaxScene::LoadVisibilityFlagAliases(const TiXmlElement* objectElement) {
/*    //Exit early if skip option is set
    if ((this->loadOptions & SKIP_VISIBILITY_FLAG_ALIASES) != 0)
        return;

    //Allocate the visibility flags
    size_t oldSize = this->queryFlags.size();
    size_t visibilityFlagCount = OgreMax::OgreMaxUtilities::GetElementCount(objectElement, "visibilityFlag");
    this->visibilityFlags.resize(oldSize + visibilityFlagCount);

    //Iterate over all the visibility flags
    size_t index = 0;
    String elementName;
    const TiXmlElement* childElement = 0;
    while (childElement = OgreMax::OgreMaxUtilities::IterateChildElements(objectElement, childElement))
    {
        this->visibilityFlags[oldSize + index].name = OgreMax::OgreMaxUtilities::GetStringAttribute(childElement, "name");
        this->visibilityFlags[oldSize + index].bit = OgreMax::OgreMaxUtilities::GetIntAttribute(childElement, "bit", 0);
        index++;
    }*/
}



//externals
void CustomOgreMaxScene::LoadExternals(const TiXmlElement* objectElement)
{
    //Exit early if skip option is set
	if ((this->loadOptions & SKIP_EXTERNALS) != 0){
        return;
	}

    //Iterate over all the node children
    size_t index = 0;
    String elementName;
    const TiXmlElement* childElement = 0;
    while (childElement = OgreMax::OgreMaxUtilities::IterateChildElements(objectElement, childElement)) {
        elementName = childElement->Value();

        if (elementName == "item") {
            LoadExternalItem(childElement);
        }
    }
}

void CustomOgreMaxScene::LoadExternalUserDatas(const TiXmlElement* objectElement) {
    //Iterate over all the node children
    size_t index = 0;
    String elementName;
    const TiXmlElement* childElement = 0;
    while (childElement = OgreMax::OgreMaxUtilities::IterateChildElements(objectElement, childElement)) {
        elementName = childElement->Value();

		OgreMax::Types::ExternalUserData ud;

		if (elementName == "userData") {
			LoadExternalUserData(childElement,ud);
		}
    }
}

void CustomOgreMaxScene::LoadExternalItem(const TiXmlElement* objectElement ) {
	OgreMax::Types::ExternalItem item;

    item.name = OgreMax::OgreMaxUtilities::GetStringAttribute(objectElement, "name");
    item.type = OgreMax::OgreMaxUtilities::GetStringAttribute(objectElement, "type");

    //Parse child elements
    String elementName;
    const TiXmlElement* childElement = 0;
    while (childElement = OgreMax::OgreMaxUtilities::IterateChildElements(objectElement, childElement)) {
        elementName = childElement->Value();

		if (elementName == "file") {
            item.file = OgreMax::OgreMaxUtilities::GetStringAttribute(childElement, "name");
		}
		else if (elementName == "position") {
            item.position = OgreMax::OgreMaxUtilities::LoadXYZ(childElement);
		}
		else if (elementName == "rotation") {
            item.rotation = OgreMax::OgreMaxUtilities::LoadRotation(childElement);
		}
		else if (elementName == "scale") {
            item.scale = OgreMax::OgreMaxUtilities::LoadXYZ(childElement);
		}
		else if (elementName == "boundingVolume") {
            OgreMax::OgreMaxUtilities::LoadBoundingVolume(childElement, item.boundingVolume);
		}
		else if (elementName == "userDataReference") {
            OgreMax::OgreMaxUtilities::LoadUserDataReference(childElement, item.userDataReference);
		}
		else if (elementName == "userData") {
            OgreMax::OgreMaxUtilities::GetChildText(childElement, item.userData);
		}
		else if (elementName == "noteTracks") {
            OgreMax::OgreMaxUtilities::LoadNoteTracks(childElement, item.noteTracks);
		}
    }

    //Notify callback
	if (this->callback != 0) {
		this->callback->onExternal(item);
	}
}

void CustomOgreMaxScene::LoadExternalUserData(const TiXmlElement* objectElement, OgreMax::Types::ExternalUserData& userData) {
    userData.type = OgreMax::OgreMaxUtilities::GetStringAttribute(objectElement, "type");
    userData.name = OgreMax::OgreMaxUtilities::GetStringAttribute(objectElement, "name");
    userData.id = OgreMax::OgreMaxUtilities::GetStringAttribute(objectElement, "id");
    userData.userDataReference = OgreMax::OgreMaxUtilities::GetStringAttribute(objectElement, "userDataReference");
    OgreMax::OgreMaxUtilities::GetChildText(objectElement, userData.userData);

	//Notify callback
	if (this->callback != 0) {
		this->callback->onExternalUserData(userData);
	}
}


//enviroment
void CustomOgreMaxScene::LoadEnvironment(const TiXmlElement* objectElement)
{
    //Exit early if skip option is set
	if ((this->loadOptions & SKIP_ENVIRONMENT) != 0) {
		return;
	}

    //Iterate over all the node children
    String elementName;
    const TiXmlElement* childElement = 0;
    while (childElement = OgreMax::OgreMaxUtilities::IterateChildElements(objectElement, childElement)) {
        elementName = childElement->Value();

		if (elementName == "fog"){
			LoadFog(childElement);
		}
		else if (elementName == "skyBox") {
			LoadSkyBox(childElement);
		}
		else if (elementName == "skyDome") {
            LoadSkyDome(childElement);
		}
		else if (elementName == "skyPlane") {
            LoadSkyPlane(childElement);
		}
		else if (elementName == "clipping") {
			Ogre::Real environmentNear;
			Ogre::Real environmentFar;
			OgreMax::OgreMaxUtilities::LoadClipping(childElement, environmentNear, environmentFar);
			if ( this->callback ) {
				this->callback->onClipping(environmentNear, environmentFar);
			}
		}
		else if (elementName == "colourAmbient") {
			if ( this->callback ) {
				this->callback->onAmbientColour(OgreMax::OgreMaxUtilities::LoadColor(childElement));
			}
		}
        else if (elementName == "colourBackground") {
			if ( this->callback ){
				this->callback->onBackgroundColour(OgreMax::OgreMaxUtilities::LoadColor(childElement));
			}
        }
		else if (elementName == "shadows") {
			LoadShadows(childElement);
		}
    }
}
void CustomOgreMaxScene::LoadFog(const TiXmlElement* objectElement) {
	OgreMax::Types::FogParameters fogParameters;

    fogParameters.expDensity = OgreMax::OgreMaxUtilities::GetRealAttribute(objectElement, "expDensity", fogParameters.expDensity);
    fogParameters.linearStart = OgreMax::OgreMaxUtilities::GetRealAttribute(objectElement, "linearStart", fogParameters.linearStart);
    fogParameters.linearEnd = OgreMax::OgreMaxUtilities::GetRealAttribute(objectElement, "linearEnd", fogParameters.linearEnd);

    String fogModeText = OgreMax::OgreMaxUtilities::GetStringAttribute(objectElement, "mode", "none");
	if (!fogModeText.empty()) {
		fogParameters.mode = OgreMax::OgreMaxUtilities::ParseFogMode(fogModeText);
	}

    const TiXmlElement* colorElement = objectElement->FirstChildElement("colourDiffuse");
	if (colorElement != 0) {
		fogParameters.color = OgreMax::OgreMaxUtilities::LoadColor(colorElement);
	}

	if ( this->callback ){
		this->callback->onFog(fogParameters);
	}
}

void CustomOgreMaxScene::LoadSkyBox(const TiXmlElement* objectElement) {
    //Exit early if skip option is set
	if ((this->loadOptions & SKIP_SKY) != 0) {
		return;
	}

    OgreMax::Types::SkyBoxParameters parameters;
    parameters.enabled = OgreMax::OgreMaxUtilities::GetBoolAttribute(objectElement, "enable", true);
    parameters.material = OgreMax::OgreMaxUtilities::GetStringAttribute(objectElement, "material");
    parameters.distance = OgreMax::OgreMaxUtilities::GetRealAttribute(objectElement, "distance", 5000);
    parameters.drawFirst = OgreMax::OgreMaxUtilities::GetBoolAttribute(objectElement, "drawFirst", true);
    parameters.resourceGroupName = this->defaultResourceGroupName;

    //Parse child elements
    String elementName;
    const TiXmlElement* childElement = 0;
    const TiXmlElement* animationsElement = 0;
    while (childElement = OgreMax::OgreMaxUtilities::IterateChildElements(objectElement, childElement)) {
        elementName = childElement->Value();

		if (elementName == "rotation") {
			parameters.rotation = OgreMax::OgreMaxUtilities::LoadRotation(childElement);
		}
		else if (elementName == "userDataReference") {
            OgreMax::OgreMaxUtilities::LoadUserDataReference(childElement, parameters.extraData.userDataReference);
		}
		else if (elementName == "userData") {
			OgreMax::OgreMaxUtilities::GetChildText(childElement, parameters.extraData.userData);
		}
		else if (elementName == "animations") {
			animationsElement = childElement;
		}
    }

	if ( animationsElement ){
		std::vector<OgreMax::Types::NodeAnimation> animation = LoadNodeAnimations(animationsElement);
		if (this->callback != 0){
			this->callback->onSkyBox(parameters,&animation);
		}	
	}
	else {
		//Notify callback
		if (this->callback != 0){
			this->callback->onSkyBox(parameters,0);
		}
	}

}

void CustomOgreMaxScene::LoadSkyDome(const TiXmlElement* objectElement) {
    //Exit early if skip option is set
	if ((this->loadOptions & SKIP_SKY) != 0) {
		return;
	}

    OgreMax::Types::SkyDomeParameters parameters;
    parameters.enabled = OgreMax::OgreMaxUtilities::GetBoolAttribute(objectElement, "enable", true);
    parameters.material = OgreMax::OgreMaxUtilities::GetStringAttribute(objectElement, "material");
    parameters.curvature = OgreMax::OgreMaxUtilities::GetRealAttribute(objectElement, "curvature", 10);
    parameters.tiling = OgreMax::OgreMaxUtilities::GetRealAttribute(objectElement, "tiling", 8);
    parameters.distance = OgreMax::OgreMaxUtilities::GetRealAttribute(objectElement, "distance", 4000);
    parameters.drawFirst = OgreMax::OgreMaxUtilities::GetBoolAttribute(objectElement, "drawFirst", true);
    parameters.xSegments = OgreMax::OgreMaxUtilities::GetIntAttribute(objectElement, "xSegments", 16);
    parameters.ySegments = OgreMax::OgreMaxUtilities::GetIntAttribute(objectElement, "ySegments", 16);
    parameters.resourceGroupName = this->defaultResourceGroupName;

    //Parse child elements
    String elementName;
    const TiXmlElement* childElement = 0;
    const TiXmlElement* animationsElement = 0;
    while (childElement = OgreMax::OgreMaxUtilities::IterateChildElements(objectElement, childElement)) {
        elementName = childElement->Value();

		if (elementName == "rotation") {
			parameters.rotation = OgreMax::OgreMaxUtilities::LoadRotation(childElement);
		}
		else if (elementName == "userDataReference") {
			OgreMax::OgreMaxUtilities::LoadUserDataReference(childElement, parameters.extraData.userDataReference);
		}
		else if (elementName == "userData") {
			OgreMax::OgreMaxUtilities::GetChildText(childElement, parameters.extraData.userData);
		}
		else if (elementName == "animations") {
			animationsElement = childElement;
		}
    }

	if ( animationsElement ) {
		std::vector<OgreMax::Types::NodeAnimation> animation = LoadNodeAnimations(animationsElement);
		if (this->callback != 0) {
			this->callback->onSkyDome(parameters,&animation);
		}	
	}
	else {
		//Notify callback
		if (this->callback != 0) {
			this->callback->onSkyDome(parameters,0);
		}
	}
}

void CustomOgreMaxScene::LoadSkyPlane(const TiXmlElement* objectElement) {
    //Exit early if skip option is set
	if ((this->loadOptions & SKIP_SKY) != 0) {
		return;
	}

    OgreMax::Types::SkyPlaneParameters parameters;
    parameters.enabled = OgreMax::OgreMaxUtilities::GetBoolAttribute(objectElement, "enable", true);
    parameters.material = OgreMax::OgreMaxUtilities::GetStringAttribute(objectElement, "material");
    parameters.plane = OgreMax::OgreMaxUtilities::GetPlaneAttributes(objectElement, 0, -1, 0, 5000);
    parameters.scale = OgreMax::OgreMaxUtilities::GetRealAttribute(objectElement, "scale", 1000);
    parameters.bow = OgreMax::OgreMaxUtilities::GetRealAttribute(objectElement, "bow", 0);
    parameters.tiling = OgreMax::OgreMaxUtilities::GetRealAttribute(objectElement, "tiling", 10);
    parameters.drawFirst = OgreMax::OgreMaxUtilities::GetBoolAttribute(objectElement, "drawFirst", true);
    parameters.xSegments = OgreMax::OgreMaxUtilities::GetIntAttribute(objectElement, "xSegments", 1);
    parameters.ySegments = OgreMax::OgreMaxUtilities::GetIntAttribute(objectElement, "ySegments", 1);
    parameters.resourceGroupName = this->defaultResourceGroupName;

    //Parse child elements
    String elementName;
    const TiXmlElement* childElement = 0;
    const TiXmlElement* animationsElement = 0;
    while (childElement = OgreMax::OgreMaxUtilities::IterateChildElements(objectElement, childElement)) {
        elementName = childElement->Value();

		if (elementName == "rotation") {
			parameters.rotation = OgreMax::OgreMaxUtilities::LoadRotation(childElement);
		}
		else if (elementName == "userDataReference") {
			OgreMax::OgreMaxUtilities::LoadUserDataReference(childElement, parameters.extraData.userDataReference);
		}
		else if (elementName == "userData") {
			OgreMax::OgreMaxUtilities::GetChildText(childElement, parameters.extraData.userData);
		}
		else if (elementName == "animations") {
            animationsElement = childElement;
		}
    }

	if ( animationsElement ){
		std::vector<OgreMax::Types::NodeAnimation> animation = LoadNodeAnimations(animationsElement);
		if (this->callback != 0) {
			this->callback->onSkyPlane(parameters,&animation);
		}	
	}
	else {
		//Notify callback
		if (this->callback != 0) {
			this->callback->onSkyPlane(parameters,0);
		}
	}
}

void CustomOgreMaxScene::LoadShadows(const TiXmlElement* objectElement)
{
    //Exit early if skip option is set
	if ((this->loadOptions & SKIP_SHADOWS) != 0) {
        return;
	}

    OgreMax::Types::ShadowParameters params;
    String techniqueText = OgreMax::OgreMaxUtilities::GetStringAttribute(objectElement, "technique", "none");
	if (!techniqueText.empty()) {
		params.shadowTechnique = OgreMax::OgreMaxUtilities::ParseShadowTechnique(techniqueText);
	}
    params.selfShadow = OgreMax::OgreMaxUtilities::GetBoolAttribute(objectElement, "selfShadow", params.selfShadow);
    params.farDistance = OgreMax::OgreMaxUtilities::GetRealAttribute(objectElement, "farDistance", params.farDistance);

    //Parse child elements
    String elementName;
    const TiXmlElement* childElement = 0;
    while (childElement = OgreMax::OgreMaxUtilities::IterateChildElements(objectElement, childElement)) {
        elementName = childElement->Value();

        if (elementName == "shadowTextures") {
            params.textureSize = OgreMax::OgreMaxUtilities::GetIntAttribute(childElement, "size", params.textureSize);
            params.textureCount = OgreMax::OgreMaxUtilities::GetIntAttribute(childElement, "count", params.textureCount);
            params.textureOffset = OgreMax::OgreMaxUtilities::GetRealAttribute(childElement, "offset", params.textureOffset);
            params.textureFadeStart = OgreMax::OgreMaxUtilities::GetRealAttribute(childElement, "fadeStart", params.textureFadeStart);
            params.textureFadeEnd = OgreMax::OgreMaxUtilities::GetRealAttribute(childElement, "fadeEnd", params.textureFadeEnd);
            params.textureShadowCasterMaterial = OgreMax::OgreMaxUtilities::GetStringAttribute(childElement, "shadowCasterMaterial");
            params.textureShadowReceiverMaterial = OgreMax::OgreMaxUtilities::GetStringAttribute(childElement, "shadowReceiverMaterial");
        }
		else if (elementName == "colourShadow") {
			params.shadowColor = OgreMax::OgreMaxUtilities::LoadColor(childElement);
		}
        else if (elementName == "shadowCameraSetup") {
            params.cameraSetup = OgreMax::OgreMaxUtilities::GetStringAttribute(childElement, "type", "lispsm");
            const Vector3& upVector = GetUpVector();
            params.optimalPlane = OgreMax::OgreMaxUtilities::GetPlaneAttributes(objectElement, upVector.x, upVector.y, upVector.z, 0);
        }
    }

	if ( this->callback ){
		this->callback->onShadowProperties(params);
    }
}


//nodes
void CustomOgreMaxScene::LoadNodes(const TiXmlElement* objectElement) {
    //Exit early if skip option is set
	if ((this->loadOptions & SKIP_NODES) != 0) {
		return;
	}

	//root node positions
	Ogre::Vector3 position(0,0,0);
	Ogre::Quaternion rotation;
	Ogre::Vector3 scale(1,1,1);

    //Initialize progress counters
    size_t nodeCount = OgreMax::OgreMaxUtilities::GetElementCount(objectElement, "node");
    this->loadProgress.nodes->SetRange(nodeCount);

    //Iterate over all the node children
    String elementName;
    const TiXmlElement* childElement = 0;
    while (childElement = OgreMax::OgreMaxUtilities::IterateChildElements(objectElement, childElement)) {
        elementName = childElement->Value();

		if (elementName == "position") {
			position = OgreMax::OgreMaxUtilities::LoadXYZ(childElement);
		}
		else if (elementName == "rotation") {
			rotation = OgreMax::OgreMaxUtilities::LoadRotation(childElement);
		}
		else if (elementName == "scale") {
			scale = OgreMax::OgreMaxUtilities::LoadXYZ(childElement);
		}
    }

	if ( this->callback ){
		this->callback->onRootNode(position,rotation,scale);
	}

	while (childElement = OgreMax::OgreMaxUtilities::IterateChildElements(objectElement, childElement)) {
        elementName = childElement->Value();

		if (elementName == "node" || elementName == "modelInstance") {
            LoadNode(childElement,0);
		}
	}
}

void CustomOgreMaxScene::LoadNode(const TiXmlElement* objectElement, OgreMax::Types::NodeParameters * parent)
{
    OgreMax::Types::ObjectExtraData extraData;
	
	OgreMax::Types::NodeParameters parameters;

	parameters.name = OgreMax::OgreMaxUtilities::GetStringAttribute(objectElement, "name");;

    /*
	TODO
	//Get/load the model

	parameters.modelFile = OgreMax::OgreMaxUtilities::GetStringAttribute(objectElement, "modelFile");

    if (parameters.modelFile.empty())
        parameters.modelFile = OgreMax::OgreMaxUtilities::GetStringAttribute(objectElement, "modelName");
    OgreMaxModel* model = modelFileName.empty() ? 0 : GetModel(parameters.modelFile);

    extraData.id = OgreMax::OgreMaxUtilities::GetStringAttribute(objectElement, "id");

    //Create the node
    SceneNode* node = name.empty() ? parentNode->createChildSceneNode() : parentNode->createChildSceneNode(name);

    //Notify callback
    if (this->callback != 0)
        this->callback->StartedCreatingNode(this, node);

    //Create the model instance if there is one
    if (model != 0)
    {
        //Determine options
        OgreMaxModel::InstanceOptions instanceOptions = OgreMaxModel::NO_INITIAL_TRANSFORMATION;
        if (this->loadOptions & NO_ANIMATION_STATES)
            instanceOptions |= OgreMaxModel::NO_ANIMATION_STATES;

        //Create the node that will contain the instance
        SceneNode* modelNode = node->createChildSceneNode();

        //Create the instance
        SceneModelInstanceCallback sceneModelInstanceCallback(this, this->callback);
        model->CreateInstance
            (
            this->sceneManager,
            name,
            (this->callback != 0) ? &sceneModelInstanceCallback : 0,
            instanceOptions,
            node,
            this->defaultResourceGroupName,
            modelNode,
            this
            );
    }*/

    //Iterate over all the node children
    
	bool hasanimation = false;
    String elementName;
    const TiXmlElement* childElement = 0;
	const TiXmlElement* animationElement = 0;

	//fully load the node parameters
    while (childElement = OgreMax::OgreMaxUtilities::IterateChildElements(objectElement, childElement)) {
        elementName = childElement->Value();

		if (elementName == "userDataReference") {
			OgreMax::OgreMaxUtilities::LoadUserDataReference(childElement, extraData.userDataReference);
		}
		else if (elementName == "userData") {
			OgreMax::OgreMaxUtilities::GetChildText(childElement, extraData.userData);
		}
		else if (elementName == "position") {
			parameters.position = OgreMax::OgreMaxUtilities::LoadXYZ(childElement);
		}
		else if (elementName == "rotation") {
			parameters.orientation = OgreMax::OgreMaxUtilities::LoadRotation(childElement); 
		}          
		else if (elementName == "scale") {
			parameters.scale = OgreMax::OgreMaxUtilities::LoadXYZ(childElement);
		}
        else if (elementName == "animations") {
			animationElement = childElement;
        }
	}

	//Set the node's visibility
    String visibilityText = OgreMax::OgreMaxUtilities::GetStringAttribute(objectElement, "visibility");
	parameters.visibility = OgreMax::OgreMaxUtilities::ParseNodeVisibility(visibilityText);

	if ( animationElement == 0 ) {
		//Notify callback
		if (this->callback != 0) {
			this->callback->onNode(parameters, 0, parent);
		}
	} else {
		std::vector<OgreMax::Types::NodeAnimation> animations = LoadNodeAnimations(animationElement);
		//Notify callback
		if (this->callback != 0) {
			this->callback->onNode(parameters, &animations, parent);
		}
	}	

	OgreMax::Types::SceneNode attachable;
	attachable.name = parameters.name;

	//get on with it
	childElement = 0;
	while (childElement = OgreMax::OgreMaxUtilities::IterateChildElements(objectElement, childElement))
    {
		elementName = childElement->Value();
        //else if (elementName == "lookTarget")
            //LoadLookTarget(childElement, node, 0);
        //else if (elementName == "trackTarget")
            //LoadTrackTarget(childElement, node, 0);
		if (elementName == "node" || elementName == "modelInstance") {
			LoadNode(childElement, &parameters);
		}
		else if (elementName == "entity") {
			LoadEntity(childElement, &attachable);
		}
		else if (elementName == "light") {
			LoadLight(childElement, &attachable);
		}
		else if (elementName == "camera") {
			LoadCamera(childElement, &attachable);
		}
		else if (elementName == "particleSystem") {
			LoadParticleSystem(childElement, &attachable);
		}
		else if (elementName == "billboardSet") {
			LoadBillboardSet(childElement, &attachable);
		}
		else if (elementName == "plane") {
			LoadPlane(childElement,&attachable);
		}
    }

    //Update progress counter
    UpdateLoadProgress(this->loadProgress.nodes, 1);
}

void CustomOgreMaxScene::LoadEntity(const TiXmlElement* objectElement, OgreMax::Types::Attachable * parent)
{
    OgreMax::Types::ObjectExtraDataPtr objectExtraData(new OgreMax::Types::ObjectExtraData);

    OgreMax::Types::EntityParameters parameters;
    parameters.name = GetNewObjectName(objectElement);
    parameters.queryFlags = OgreMax::OgreMaxUtilities::GetUIntAttribute(objectElement, "queryFlags", 0);
    parameters.visibilityFlags = OgreMax::OgreMaxUtilities::GetUIntAttribute(objectElement, "visibilityFlags", 0);
    parameters.visibility = OgreMax::OgreMaxUtilities::GetObjectVisibilityAttribute(objectElement, "visible");
    parameters.meshFile = OgreMax::OgreMaxUtilities::GetStringAttribute(objectElement, "meshFile");
    parameters.materialFile = OgreMax::OgreMaxUtilities::GetStringAttribute(objectElement, "materialFile");
    parameters.castShadows = OgreMax::OgreMaxUtilities::GetBoolAttribute(objectElement, "castShadows", true);

    String renderQueue = OgreMax::OgreMaxUtilities::GetStringAttribute(objectElement, "renderQueue");
    parameters.renderQueue = OgreMax::OgreMaxUtilities::ParseRenderQueue(renderQueue);

    parameters.renderingDistance = OgreMax::OgreMaxUtilities::GetRealAttribute(objectElement, "renderingDistance", 0);

    parameters.resourceGroupName = this->defaultResourceGroupName;

    objectExtraData->id = OgreMax::OgreMaxUtilities::GetStringAttribute(objectElement, "id");
    objectExtraData->receiveShadows = OgreMax::OgreMaxUtilities::GetBoolAttribute(objectElement, "receiveShadows", true);

    //Parse child elements
    const TiXmlElement* boneAttachmentsElement = 0;
    String elementName;
    const TiXmlElement* childElement = 0;
    while (childElement = OgreMax::OgreMaxUtilities::IterateChildElements(objectElement, childElement)) {
        elementName = childElement->Value();

		if (elementName == "vertexBuffer") {
			OgreMax::OgreMaxUtilities::LoadBufferUsage(childElement, parameters.vertexBufferUsage, parameters.vertexBufferShadowed);
		}
		else if (elementName == "indexBuffer") {
			OgreMax::OgreMaxUtilities::LoadBufferUsage(childElement, parameters.indexBufferUsage, parameters.indexBufferShadowed);
		}
		else if (elementName == "userDataReference") {
			OgreMax::OgreMaxUtilities::LoadUserDataReference(childElement, objectExtraData->userDataReference);
		}
		else if (elementName == "userData") {
			OgreMax::OgreMaxUtilities::GetChildText(childElement, objectExtraData->userData);
		}
        else if (elementName == "noteTracks") {
            objectExtraData->noteTracks = SharedPtr<OgreMax::Types::NoteTracks>(new OgreMax::Types::NoteTracks);
            OgreMax::OgreMaxUtilities::LoadNoteTracks(childElement, *objectExtraData->noteTracks.get());
        }
		else if (elementName == "customParameters") {
			OgreMax::OgreMaxUtilities::LoadCustomParameters(childElement, parameters.customParameters);
		}
		else if (elementName == "subentities") {
			OgreMax::OgreMaxUtilities::LoadSubentities(childElement, parameters.subentities);
		}
		else if (elementName == "boneAttachments") {
			boneAttachmentsElement = childElement;
		}
    }

    parameters.extraData = objectExtraData;

	//Load bone attachments
	if (boneAttachmentsElement != 0) {
		LoadBoneAttachments(boneAttachmentsElement, &parameters);
	}

    //Notify callback
	if (this->callback != 0) {
		this->callback->onEntity(parameters, parent);
	}
}

void CustomOgreMaxScene::LoadLight(const TiXmlElement* objectElement, OgreMax::Types::Attachable * parent)
{
    OgreMax::Types::ObjectExtraDataPtr objectExtraData(new OgreMax::Types::ObjectExtraData);

	OgreMax::Types::LightParameters parameters;
	parameters.objectType = OgreMax::Types::ObjectParameters::LIGHT;	

	parameters.name = GetNewObjectName(objectElement);
    objectExtraData->id = OgreMax::OgreMaxUtilities::GetStringAttribute(objectElement, "id");
	parameters.queryFlags = OgreMax::OgreMaxUtilities::GetUIntAttribute(objectElement, "queryFlags", 0);
	parameters.visibilityFlags = OgreMax::OgreMaxUtilities::GetUIntAttribute(objectElement, "visibilityFlags", 0);
    parameters.visibility = OgreMax::OgreMaxUtilities::GetObjectVisibilityAttribute(objectElement, "visible");


	Ogre::String lighttypetext = OgreMax::OgreMaxUtilities::GetStringAttribute(objectElement, "type");
	//TODO check types output
	if ( lighttypetext == "point" ) {
		parameters.lightType = Ogre::Light::LT_POINT;
	}
	else if ( lighttypetext == "spot" ) {
		parameters.lightType = Ogre::Light::LT_SPOTLIGHT;		
	}
	else {
		parameters.lightType = Ogre::Light::LT_DIRECTIONAL;
	}

   
    parameters.castShadows = OgreMax::OgreMaxUtilities::GetBoolAttribute(objectElement, "castShadows", true);
    parameters.power = OgreMax::OgreMaxUtilities::GetRealAttribute(objectElement, "power", 1);

    //Parse child elements
    String elementName;
    const TiXmlElement* childElement = 0;
    while (childElement = OgreMax::OgreMaxUtilities::IterateChildElements(objectElement, childElement)) {
        elementName = childElement->Value();

		if (elementName == "colourDiffuse") {
			parameters.diffuseColor = OgreMax::OgreMaxUtilities::LoadColor(childElement);
		}
		else if (elementName == "colourSpecular") {
			parameters.specularColor = OgreMax::OgreMaxUtilities::LoadColor(childElement);
		}
		else if (elementName == "lightRange") {
			LoadLightRange(childElement, parameters);
		}
		else if (elementName == "lightAttenuation") {
			LoadLightAttenuation(childElement, parameters);
		}
		else if (elementName == "position") {
			parameters.direction = OgreMax::OgreMaxUtilities::LoadXYZ(childElement);
		}
		else if (elementName == "normal") {
			parameters.direction = OgreMax::OgreMaxUtilities::LoadXYZ(childElement);
		}
		else if (elementName == "userDataReference") {
			OgreMax::OgreMaxUtilities::LoadUserDataReference(childElement, objectExtraData->userDataReference);
		}
		else if (elementName == "userData") {
			OgreMax::OgreMaxUtilities::GetChildText(childElement, objectExtraData->userData);
		}
        else if (elementName == "noteTracks") {
            objectExtraData->noteTracks = SharedPtr<OgreMax::Types::NoteTracks>(new OgreMax::Types::NoteTracks);
            OgreMax::OgreMaxUtilities::LoadNoteTracks(childElement, *objectExtraData->noteTracks.get());
        }
    }

	parameters.extraData = objectExtraData;

    //Notify callback
	if (this->callback != 0) {
		this->callback->onLight(parameters, parent);
	}
}

void CustomOgreMaxScene::LoadCamera(const TiXmlElement* objectElement, OgreMax::Types::Attachable * parent)
{
    OgreMax::Types::ObjectExtraDataPtr objectExtraData(new OgreMax::Types::ObjectExtraData);

	OgreMax::Types::CameraParameters paramaters;

	objectExtraData->id = OgreMax::OgreMaxUtilities::GetStringAttribute(objectElement, "id");

    paramaters.name = GetNewObjectName(objectElement);    
    paramaters.queryFlags = OgreMax::OgreMaxUtilities::GetUIntAttribute(objectElement, "queryFlags", 0);
    paramaters.visibilityFlags = OgreMax::OgreMaxUtilities::GetUIntAttribute(objectElement, "visibilityFlags", 0);
    paramaters.visibility = OgreMax::OgreMaxUtilities::GetObjectVisibilityAttribute(objectElement, "visible");

    paramaters.fov = OgreMax::OgreMaxUtilities::GetRealAttribute(objectElement, "fov", Math::PI/2);
    paramaters.aspectRatio = OgreMax::OgreMaxUtilities::GetRealAttribute(objectElement, "aspectRatio", (Real)1.33);

	String projectiontypetext = OgreMax::OgreMaxUtilities::GetStringAttribute(objectElement, "type");
	if ( projectiontypetext == "orthograpic" ) {
		paramaters.projectionType = Ogre::PT_ORTHOGRAPHIC;
	}
	else {
		paramaters.projectionType = Ogre::PT_PERSPECTIVE;			
	}

   //Parse child elements
    String elementName;
    const TiXmlElement* childElement = 0;
    while (childElement = OgreMax::OgreMaxUtilities::IterateChildElements(objectElement, childElement)) {
        elementName = childElement->Value();

        if (elementName == "clipping") {
			OgreMax::OgreMaxUtilities::LoadClipping(childElement, paramaters.nearClip, paramaters.farClip);
        }
		else if (elementName == "position") {
			paramaters.position = OgreMax::OgreMaxUtilities::LoadXYZ(childElement);       
		}     
		else if (elementName == "rotation") {
			paramaters.orientation = OgreMax::OgreMaxUtilities::LoadRotation(childElement);  
		}          
		else if (elementName == "normal") {
			paramaters.direction = OgreMax::OgreMaxUtilities::LoadXYZ(childElement);
		}   
        /*else if (elementName == "lookTarget")
            LoadLookTarget(childElement, 0, camera);
        else if (elementName == "trackTarget")
            LoadTrackTarget(childElement, 0, camera);*/
		else if (elementName == "userDataReference") {
			OgreMax::OgreMaxUtilities::LoadUserDataReference(childElement, objectExtraData->userDataReference);
		}
		else if (elementName == "userData") {
			OgreMax::OgreMaxUtilities::GetChildText(childElement, objectExtraData->userData);
		}
        else if (elementName == "noteTracks") {
            objectExtraData->noteTracks = SharedPtr<OgreMax::Types::NoteTracks>(new OgreMax::Types::NoteTracks);
            OgreMax::OgreMaxUtilities::LoadNoteTracks(childElement, *objectExtraData->noteTracks.get());
        }
    }
	paramaters.extraData = objectExtraData;

    //Notify callback
	if (this->callback != 0) {
		this->callback->onCamera(paramaters,parent);
	}
}

void CustomOgreMaxScene::LoadParticleSystem(const TiXmlElement* objectElement, OgreMax::Types::Attachable * parent)
{
    OgreMax::Types::ObjectExtraDataPtr objectExtraData(new OgreMax::Types::ObjectExtraData);

	OgreMax::Types::ParticleSystemParameters params;

    params.name = GetNewObjectName(objectElement);
    objectExtraData->id = OgreMax::OgreMaxUtilities::GetStringAttribute(objectElement, "id");
    params.queryFlags = OgreMax::OgreMaxUtilities::GetUIntAttribute(objectElement, "queryFlags", 0);
    params.visibilityFlags = OgreMax::OgreMaxUtilities::GetUIntAttribute(objectElement, "visibilityFlags", 0);
    params.visibility = OgreMax::OgreMaxUtilities::GetObjectVisibilityAttribute(objectElement, "visible");
    String renderQueue = OgreMax::OgreMaxUtilities::GetStringAttribute(objectElement, "renderQueue");
    params.renderQueue = OgreMax::OgreMaxUtilities::ParseRenderQueue(renderQueue);
    params.renderingDistance = OgreMax::OgreMaxUtilities::GetRealAttribute(objectElement, "renderingDistance", 0);
    params.file = OgreMax::OgreMaxUtilities::GetStringAttribute(objectElement, "file");

    //Parse child elements
    String elementName;
    const TiXmlElement* childElement = 0;
    while (childElement = OgreMax::OgreMaxUtilities::IterateChildElements(objectElement, childElement)) {
        elementName = childElement->Value();

		if (elementName == "userDataReference") {
			OgreMax::OgreMaxUtilities::LoadUserDataReference(childElement, objectExtraData->userDataReference);
		}
		else if (elementName == "userData") {
			OgreMax::OgreMaxUtilities::GetChildText(childElement, objectExtraData->userData);
		}
        else if (elementName == "noteTracks") {
            objectExtraData->noteTracks = SharedPtr<OgreMax::Types::NoteTracks>(new OgreMax::Types::NoteTracks);
            OgreMax::OgreMaxUtilities::LoadNoteTracks(childElement, *objectExtraData->noteTracks.get());
        }
    }

	params.extraData = objectExtraData;

    //Notify callback
	if (this->callback != 0) {
		this->callback->onParticleSystem(params,parent);
	}
}

void CustomOgreMaxScene::LoadBillboardSet(const TiXmlElement* objectElement, OgreMax::Types::Attachable * parent )
{
	OgreMax::Types::ObjectExtraDataPtr objectExtraData(new OgreMax::Types::ObjectExtraData);

	OgreMax::Types::BillboardSetParameters params;

    params.name = GetNewObjectName(objectElement);
    objectExtraData->id = OgreMax::OgreMaxUtilities::GetStringAttribute(objectElement, "id");
    params.queryFlags = OgreMax::OgreMaxUtilities::GetUIntAttribute(objectElement, "queryFlags", 0);
    params.visibilityFlags = OgreMax::OgreMaxUtilities::GetUIntAttribute(objectElement, "visibilityFlags", 0);
    params.visibility = OgreMax::OgreMaxUtilities::GetObjectVisibilityAttribute(objectElement, "visible");
    params.material = OgreMax::OgreMaxUtilities::GetStringAttribute(objectElement, "material");
    params.width = OgreMax::OgreMaxUtilities::GetRealAttribute(objectElement, "width", 10);
    params.height = OgreMax::OgreMaxUtilities::GetRealAttribute(objectElement, "height", 10);
	params.billboardType = OgreMax::OgreMaxUtilities::ParseBillboardType(OgreMax::OgreMaxUtilities::GetStringAttribute(objectElement, "type", "point"));
    params.origin = OgreMax::OgreMaxUtilities::ParseBillboardOrigin(OgreMax::OgreMaxUtilities::GetStringAttribute(objectElement, "origin", "center"));
    params.rotationType = OgreMax::OgreMaxUtilities::ParseBillboardRotationType(OgreMax::OgreMaxUtilities::GetStringAttribute(objectElement, "rotationType", "vertex"));
    params.poolSize = OgreMax::OgreMaxUtilities::GetUIntAttribute(objectElement, "poolSize", 0);
    params.autoExtendPool = OgreMax::OgreMaxUtilities::GetBoolAttribute(objectElement, "autoExtendPool", true);
    params.cullIndividual = OgreMax::OgreMaxUtilities::GetBoolAttribute(objectElement, "cullIndividual", false);
    params.sort = OgreMax::OgreMaxUtilities::GetBoolAttribute(objectElement, "sort", false);
    params.accurateFacing = OgreMax::OgreMaxUtilities::GetBoolAttribute(objectElement, "accurateFacing", false);
    params.renderQueue = OgreMax::OgreMaxUtilities::ParseRenderQueue(OgreMax::OgreMaxUtilities::GetStringAttribute(objectElement, "renderQueue"));
    params.renderingDistance = OgreMax::OgreMaxUtilities::GetRealAttribute(objectElement, "renderingDistance", 0);
    
	std::vector<OgreMax::Types::CustomParameter> customParameters;
	std::vector<OgreMax::Types::Billboard> billboardset;

     //Parse child elements
    String elementName;
    const TiXmlElement* childElement = 0;
    while (childElement = OgreMax::OgreMaxUtilities::IterateChildElements(objectElement, childElement)) {
        elementName = childElement->Value();

		if (elementName == "billboard") {
			LoadBillboard(childElement,billboardset);
		}
        else if (elementName == "commonDirection") {
			params.commonDirection = OgreMax::OgreMaxUtilities::LoadXYZ(childElement);            
        }
        else if (elementName == "commonUpVector") {
			params.commonUpVector = OgreMax::OgreMaxUtilities::LoadXYZ(childElement); 
        }
		else if (elementName == "userDataReference") {
			OgreMax::OgreMaxUtilities::LoadUserDataReference(childElement, objectExtraData->userDataReference);
		}
		else if (elementName == "userData") {
			OgreMax::OgreMaxUtilities::GetChildText(childElement, objectExtraData->userData);
		}
        else if (elementName == "noteTracks") {
			objectExtraData->noteTracks = Ogre::SharedPtr<OgreMax::Types::NoteTracks>(new OgreMax::Types::NoteTracks);
            OgreMax::OgreMaxUtilities::LoadNoteTracks(childElement, *objectExtraData->noteTracks.get());
        }
		else if (elementName == "customParameters") {
			OgreMax::OgreMaxUtilities::LoadCustomParameters(childElement, customParameters);
		}
    }

    //Notify callback
	if (this->callback != 0) {
		this->callback->onBillboardSet(params,billboardset,customParameters,parent);
	}
}

void CustomOgreMaxScene::LoadPlane(const TiXmlElement* objectElement, OgreMax::Types::Attachable * parent )
{
    OgreMax::Types::ObjectExtraDataPtr objectExtraData(new OgreMax::Types::ObjectExtraData);

    OgreMax::Types::PlaneParameters parameters;
    parameters.name = GetNewObjectName(objectElement);
    parameters.queryFlags = OgreMax::OgreMaxUtilities::GetUIntAttribute(objectElement, "queryFlags", 0);
    parameters.visibilityFlags = OgreMax::OgreMaxUtilities::GetUIntAttribute(objectElement, "visibilityFlags", 0);
    parameters.visibility = OgreMax::OgreMaxUtilities::GetObjectVisibilityAttribute(objectElement, "visible");
    parameters.planeName = parameters.name;
    parameters.distance = OgreMax::OgreMaxUtilities::GetRealAttribute(objectElement, "distance", 0);
    parameters.width = OgreMax::OgreMaxUtilities::GetRealAttribute(objectElement, "width", 10);
    parameters.height = OgreMax::OgreMaxUtilities::GetRealAttribute(objectElement, "height", 10);
    parameters.xSegments = OgreMax::OgreMaxUtilities::GetIntAttribute(objectElement, "xSegments", 1);
    parameters.ySegments = OgreMax::OgreMaxUtilities::GetIntAttribute(objectElement, "ySegments", 1);
    parameters.numTexCoordSets = OgreMax::OgreMaxUtilities::GetIntAttribute(objectElement, "numTexCoordSets", 1);
    parameters.uTile = OgreMax::OgreMaxUtilities::GetRealAttribute(objectElement, "uTile", 1);
    parameters.vTile = OgreMax::OgreMaxUtilities::GetRealAttribute(objectElement, "vTile", 1);
    parameters.material = OgreMax::OgreMaxUtilities::GetStringAttribute(objectElement, "material");
    parameters.normals = OgreMax::OgreMaxUtilities::GetBoolAttribute(objectElement, "normals", true);
    parameters.createMovablePlane = OgreMax::OgreMaxUtilities::GetBoolAttribute(objectElement, "movablePlane", true);
    parameters.castShadows = OgreMax::OgreMaxUtilities::GetBoolAttribute(objectElement, "castShadows", true);
    parameters.resourceGroupName = this->defaultResourceGroupName;

    String renderQueue = OgreMax::OgreMaxUtilities::GetStringAttribute(objectElement, "renderQueue");
    parameters.renderQueue = OgreMax::OgreMaxUtilities::ParseRenderQueue(renderQueue);

    parameters.renderingDistance = OgreMax::OgreMaxUtilities::GetRealAttribute(objectElement, "renderingDistance", 0);

    objectExtraData->id = OgreMax::OgreMaxUtilities::GetStringAttribute(objectElement, "id");
    objectExtraData->receiveShadows = OgreMax::OgreMaxUtilities::GetBoolAttribute(objectElement, "receiveShadows", true);

    //Parse child elements
    String elementName;
    const TiXmlElement* childElement = 0;
	while (childElement = OgreMax::OgreMaxUtilities::IterateChildElements(objectElement, childElement)) {
        elementName = childElement->Value();

		if (elementName == "normal") {
			parameters.normal = OgreMax::OgreMaxUtilities::LoadXYZ(childElement);
		}
		else if (elementName == "upVector") {
			parameters.upVector = OgreMax::OgreMaxUtilities::LoadXYZ(childElement);
		}
		else if (elementName == "vertexBuffer") {
			OgreMax::OgreMaxUtilities::LoadBufferUsage(childElement, parameters.vertexBufferUsage, parameters.vertexBufferShadowed);
		}
		else if (elementName == "indexBuffer") {
			OgreMax::OgreMaxUtilities::LoadBufferUsage(childElement, parameters.indexBufferUsage, parameters.indexBufferShadowed);
		}
		else if (elementName == "userDataReference") {
			OgreMax::OgreMaxUtilities::LoadUserDataReference(childElement, objectExtraData->userDataReference);
		}
		else if (elementName == "userData") {
			OgreMax::OgreMaxUtilities::GetChildText(childElement, objectExtraData->userData);
		}
        else if (elementName == "noteTracks") {
            objectExtraData->noteTracks = SharedPtr<OgreMax::Types::NoteTracks>(new OgreMax::Types::NoteTracks);
            OgreMax::OgreMaxUtilities::LoadNoteTracks(childElement, *objectExtraData->noteTracks.get());
        }
		else if (elementName == "customParameters") {
			OgreMax::OgreMaxUtilities::LoadCustomParameters(childElement, parameters.customParameters);
		}
    }

    parameters.extraData = objectExtraData;

    //Notify callback
	if (this->callback != 0) {
		this->callback->onPlane(parameters,parent);
	}
}

void CustomOgreMaxScene::LoadBoneAttachments(const TiXmlElement* objectElement, OgreMax::Types::EntityParameters * entity)
{
    const TiXmlElement* childElement = 0;
	while (childElement = OgreMax::OgreMaxUtilities::IterateChildElements(objectElement, childElement)) {
        LoadBoneAttachment(childElement, entity);
	}
}

void CustomOgreMaxScene::LoadBoneAttachment(const TiXmlElement* objectElement, OgreMax::Types::EntityParameters * entity)
{
    bool attachedSomething = false;

	OgreMax::Types::TagPoint tagpoint;
	tagpoint.name = entity->name;
    tagpoint.boneName = OgreMax::OgreMaxUtilities::GetStringAttribute(objectElement, "bone");
	tagpoint.tagName = OgreMax::OgreMaxUtilities::GetStringAttribute(objectElement, "name");

    String elementName;
    const TiXmlElement* childElement = 0;
    while (childElement = OgreMax::OgreMaxUtilities::IterateChildElements(objectElement, childElement)) {
        elementName = childElement->Value();
		if (elementName == "position") {
			tagpoint.attachPosition = OgreMax::OgreMaxUtilities::LoadXYZ(childElement);
		}
		else if (elementName == "rotation") {
			tagpoint.attachRotation = OgreMax::OgreMaxUtilities::LoadRotation(childElement);
		}
		else if (elementName == "scale") {
			tagpoint.attachScale = OgreMax::OgreMaxUtilities::LoadXYZ(childElement);
		}
	}

	childElement = 0;
	while (childElement = OgreMax::OgreMaxUtilities::IterateChildElements(objectElement, childElement)) {
        elementName = childElement->Value();

		if (elementName == "entity") {
            LoadEntity(childElement,&tagpoint);
            attachedSomething = true;
        }
        else if (elementName == "light") {
            LoadLight(childElement,&tagpoint);
            attachedSomething = true;
        }
        else if (elementName == "camera") {
            LoadCamera(childElement,&tagpoint);
            attachedSomething = true;
        }
        else if (elementName == "particleSystem") {
            LoadParticleSystem(childElement,&tagpoint);
            attachedSomething = true;
        }
		else if (elementName == "billboardSet") {
            LoadBillboardSet(childElement,&tagpoint);
            attachedSomething = true;
        }
        else if (elementName == "plane") {
            LoadPlane(childElement,&tagpoint);
            attachedSomething = true;
        }
    }

	//TODO
    //if (!attachedSomething)
	//	owner.AttachEmpty(name);
}

void CustomOgreMaxScene::LoadBillboard(const TiXmlElement* objectElement, std::vector<OgreMax::Types::Billboard>& billboardset)
{
	OgreMax::Types::Billboard bb;

    bb.width = OgreMax::OgreMaxUtilities::GetRealAttribute(objectElement, "width", 0);
    bb.height = OgreMax::OgreMaxUtilities::GetRealAttribute(objectElement, "height", 0);
    bb.rotationAngle = Ogre::Radian(OgreMax::OgreMaxUtilities::GetRealAttribute(objectElement, "rotation", 0));

    bb.position = Ogre::Vector3::ZERO;
    bb.color = Ogre::ColourValue::White;
	bb.texCoordRectangle = Ogre::FloatRect(0, 0, 0, 0);

    //Parse child elements
    Ogre::String elementName;
    const TiXmlElement* childElement = 0;
    while (childElement = OgreMax::OgreMaxUtilities::IterateChildElements(objectElement, childElement)) {
        elementName = childElement->Value();

		if (elementName == "position") {
			bb.position = OgreMax::OgreMaxUtilities::LoadXYZ(childElement);
		}
        else if (elementName == "rotation") {
            Quaternion rotation = OgreMax::OgreMaxUtilities::LoadRotation(childElement);
            Vector3 rotationAxis;
            rotation.ToAngleAxis(bb.rotationAngle, rotationAxis);
        }
		else if (elementName == "colourDiffuse") {
			bb.color = OgreMax::OgreMaxUtilities::LoadColor(childElement);
		}
		else if (elementName == "texCoordRectangle") {
			bb.texCoordRectangle = OgreMax::OgreMaxUtilities::LoadFloatRectangle(childElement);
		}
    }

	billboardset.push_back(bb);
}

void CustomOgreMaxScene::LoadLightRange(const TiXmlElement* objectElement, OgreMax::Types::LightParameters& light)
{
	if (light.lightType == Light::LT_SPOTLIGHT) {
        String value;

        value = OgreMax::OgreMaxUtilities::GetStringAttribute(objectElement, "inner");
		if (!value.empty()) {
			light.spotlightInnerAngle = Radian(StringConverter::parseReal(value));
		}

        value = OgreMax::OgreMaxUtilities::GetStringAttribute(objectElement, "outer");
		if (!value.empty()) {
			light.spotlightOuterAngle = Radian(StringConverter::parseReal(value));
		}

        value = OgreMax::OgreMaxUtilities::GetStringAttribute(objectElement, "falloff");
		if (!value.empty()) {
			light.spotlightFalloff = StringConverter::parseReal(value);
		}
    }
}

void CustomOgreMaxScene::LoadLightAttenuation(const TiXmlElement* objectElement, OgreMax::Types::LightParameters& light)
{
	light.attenuationRange = Ogre::StringConverter::parseReal(OgreMax::OgreMaxUtilities::GetStringAttribute(objectElement, "range"));
    light.attenuationConstant = Ogre::StringConverter::parseReal(OgreMax::OgreMaxUtilities::GetStringAttribute(objectElement, "constant"));
	light.attenuationLinear = Ogre::StringConverter::parseReal(OgreMax::OgreMaxUtilities::GetStringAttribute(objectElement, "linear"));
	light.attenuationQuadric = Ogre::StringConverter::parseReal(OgreMax::OgreMaxUtilities::GetStringAttribute(objectElement, "quadric"));   
}

std::vector<OgreMax::Types::NodeAnimation> CustomOgreMaxScene::LoadNodeAnimations(const TiXmlElement* objectElement)
{
	std::vector<OgreMax::Types::NodeAnimation> result;

    //Parse child elements
    String elementName;
    const TiXmlElement* childElement = 0;
    while (childElement = OgreMax::OgreMaxUtilities::IterateChildElements(objectElement, childElement)) {
        elementName = childElement->Value();

		if (elementName == "animation") {
			result.push_back(LoadNodeAnimation(childElement));
		}
    }

	return result;
}

OgreMax::Types::NodeAnimation CustomOgreMaxScene::LoadNodeAnimation(const TiXmlElement* objectElement)
{
	OgreMax::Types::NodeAnimation animation;

    //Get enabled and looping states
	animation.parameters.enable = OgreMax::OgreMaxUtilities::GetBoolAttribute(objectElement, "enable", true);
    animation.parameters.looping = OgreMax::OgreMaxUtilities::GetBoolAttribute(objectElement, "loop", false);

    //Animation name
    animation.parameters.name = OgreMax::OgreMaxUtilities::GetStringAttribute(objectElement, "name");

    //Length
    animation.parameters.length = OgreMax::OgreMaxUtilities::GetRealAttribute(objectElement, "length", 0);

    //Interpolation mode
    String interpolationModeText = OgreMax::OgreMaxUtilities::GetStringAttribute(objectElement, "interpolationMode");
	if (!interpolationModeText.empty()) {
        animation.parameters.interpolationMode = OgreMax::OgreMaxUtilities::ParseAnimationInterpolationMode(interpolationModeText);
	}

    //Rotation interpolation mode
    String rotationInterpolationModeText = OgreMax::OgreMaxUtilities::GetStringAttribute(objectElement, "rotationInterpolationMode");
	if (!rotationInterpolationModeText.empty()) {
        animation.parameters.rotationInterpolationMode = OgreMax::OgreMaxUtilities::ParseAnimationRotationInterpolationMode(rotationInterpolationModeText);
	}

    //Load animation keyframes
    String elementName;
    const TiXmlElement* childElement = 0;
    while (childElement = OgreMax::OgreMaxUtilities::IterateChildElements(objectElement, childElement)) {
        elementName = childElement->Value();

		if (elementName == "keyframe") {
			LoadNodeAnimationKeyFrame(childElement, animation.animationTrack);
		}
    }

	return animation;
}

void CustomOgreMaxScene::LoadNodeAnimationKeyFrame(const TiXmlElement* objectElement, std::vector<OgreMax::Types::KeyFrame>& animationTrack)
{
	//Create the key frame
    OgreMax::Types::KeyFrame keyframe;

	//Key time
    keyframe.keyTime = OgreMax::OgreMaxUtilities::GetRealAttribute(objectElement, "time", 0);

    //Parse child elements
    String elementName;
    const TiXmlElement* childElement = 0;
    while (childElement = OgreMax::OgreMaxUtilities::IterateChildElements(objectElement, childElement)) {
        elementName = childElement->Value();
		if (elementName == "translation") {
			keyframe.translation = OgreMax::OgreMaxUtilities::LoadXYZ(childElement);  
		}          
		else if (elementName == "rotation") {
			keyframe.rotation = OgreMax::OgreMaxUtilities::LoadRotation(childElement);  
		}          
		else if (elementName == "scale") {
			keyframe.scale = OgreMax::OgreMaxUtilities::LoadXYZ(childElement);
		}
    }

	animationTrack.push_back(keyframe);
}

void CustomOgreMaxScene::LoadObjectNames(const TiXmlElement* objectElement, const String& elementName, std::vector<String>& names) {
    size_t objectCount = OgreMax::OgreMaxUtilities::GetElementCount(objectElement, elementName);
    names.resize(objectCount);

    size_t index = 0;
    const TiXmlElement* childElement = 0;
	while (childElement = OgreMax::OgreMaxUtilities::IterateChildElements(objectElement, childElement)) {
		names[index++] = childElement->Attribute("name");
	}
}

/*
		void CustomOgreMaxScene::LoadInstancedGeometries(const TiXmlElement* objectElement)
		{
			//Ensure instancing is supported
			if (!Root::getSingleton().getRenderSystem()->getCapabilities()->hasCapability(RSC_VERTEX_PROGRAM))
			{
				OGRE_EXCEPT
					(
					Exception::ERR_INVALIDPARAMS,
					"Instanced geometry is not supported by the current render system and/or video card",
					"CustomOgreMaxScene::LoadInstancedGeometry"
					);
			}

			//Read all the instanced geometries
			String elementName;
			const TiXmlElement* childElement = 0;
			while (childElement = OgreMaxUtilities::IterateChildElements(objectElement, childElement))
			{
				elementName = childElement->Value();

				if (elementName == "instancedGeometry")
					LoadInstancedGeometry(childElement);
			}
		}

		void CustomOgreMaxScene::LoadInstancedGeometry(const TiXmlElement* objectElement)
		{
			String name = OgreMaxUtilities::GetStringAttribute(objectElement, "name");
			bool castShadows = OgreMaxUtilities::GetBoolAttribute(objectElement, "castShadows", true);
			ObjectVisibility visibility = OgreMaxUtilities::GetObjectVisibilityAttribute(objectElement, "visible");
			unsigned int batchCount = OgreMaxUtilities::GetUIntAttribute(objectElement, "batchCount", 0);
			String renderQueue = OgreMaxUtilities::GetStringAttribute(objectElement, "renderQueue");
			Real renderingDistance = OgreMaxUtilities::GetRealAttribute(objectElement, "renderingDistance", 0);
			Vector3 origin = Vector3::ZERO;
			Vector3 dimensions(1000000, 1000000, 1000000);
			const TiXmlElement* entitiesElement = 0;

			//Iterate over all the child elements
			String elementName;
			const TiXmlElement* childElement = 0;
			while (childElement = OgreMaxUtilities::IterateChildElements(objectElement, childElement))
			{
				elementName = childElement->Value();

				if (elementName == "origin")
					origin = OgreMaxUtilities::LoadXYZ(childElement);
				else if (elementName == "dimensions")
					dimensions = OgreMaxUtilities::LoadXYZ(childElement);
				else if (elementName == "entities")
					entitiesElement = childElement;
			}

			//Create the instanced geometry
			InstancedGeometry* instancedGeometry = this->sceneManager->createInstancedGeometry(name);
			instancedGeometry->setCastShadows(castShadows);
			OgreMaxUtilities::SetObjectVisibility(instancedGeometry, visibility);
			instancedGeometry->setOrigin(origin);
			instancedGeometry->setBatchInstanceDimensions(dimensions);
			if (!renderQueue.empty())
				instancedGeometry->setRenderQueueGroup(OgreMaxUtilities::ParseRenderQueue(renderQueue));
			instancedGeometry->setRenderingDistance(renderingDistance);

			//Add the entities
			childElement = 0;
			while (childElement = OgreMaxUtilities::IterateChildElements(entitiesElement, childElement))
			{
				elementName = childElement->Value();

				if (elementName == "entity")
					LoadInstancedGeometryEntity(childElement, instancedGeometry);
			}

			//Build the instanced geometry
			instancedGeometry->build();

			//Add additional batch instances
			for (unsigned int batchIndex = 0; batchIndex < batchCount; batchIndex++)
				instancedGeometry->addBatchInstance();
		}

		void CustomOgreMaxScene::LoadInstancedGeometryEntity(const TiXmlElement* objectElement)
		{
			static const String TEMP_ENTITY_NAME("__instancedGeometryEntity");

			String meshFile = OgreMaxUtilities::GetStringAttribute(objectElement, "meshFile");
			Vector3 position = Vector3::ZERO;
			Quaternion rotation = Quaternion::IDENTITY;
			Vector3 scale = Vector3::UNIT_SCALE;
			std::vector<EntityParameters::Subentity> subentities;

			String elementName;
			const TiXmlElement* childElement = 0;
			while (childElement = OgreMaxUtilities::IterateChildElements(objectElement, childElement))
			{
				elementName = childElement->Value();

				if (elementName == "position")
					position = OgreMaxUtilities::LoadXYZ(childElement);
				else if (elementName == "rotation")
					rotation = OgreMaxUtilities::LoadRotation(childElement);
				else if (elementName == "scale")
					scale = OgreMaxUtilities::LoadXYZ(childElement);
				else if (elementName == "subentities")
					OgreMaxUtilities::LoadSubentities(childElement, subentities);
			}

			if (!meshFile.empty())
			{
				//Create temporary entity
				Entity* entity = OgreMaxUtilities::CreateEntity(this->sceneManager, TEMP_ENTITY_NAME, meshFile, subentities);

				//Add entity to the static geometry
				instancedGeometry->addEntity(entity, position, rotation, scale);

				//Destroy entity
				this->sceneManager->destroyEntity(entity);
			}
		}

		void CustomOgreMaxScene::LoadStaticGeometries(const TiXmlElement* objectElement)
		{
			//Read all the static geometries
			String elementName;
			const TiXmlElement* childElement = 0;
			while (childElement = OgreMaxUtilities::IterateChildElements(objectElement, childElement))
			{
				elementName = childElement->Value();

				if (elementName == "staticGeometry")
					LoadStaticGeometry(childElement);
			}
		}

		void CustomOgreMaxScene::LoadStaticGeometry(const TiXmlElement* objectElement)
		{
			String name = OgreMaxUtilities::GetStringAttribute(objectElement, "name");
			bool castShadows = OgreMaxUtilities::GetBoolAttribute(objectElement, "castShadows", true);
			ObjectVisibility visibility = OgreMaxUtilities::GetObjectVisibilityAttribute(objectElement, "visible");
			String renderQueue = OgreMaxUtilities::GetStringAttribute(objectElement, "renderQueue");
			Real renderingDistance = OgreMaxUtilities::GetRealAttribute(objectElement, "renderingDistance", 0);
			Vector3 origin = Vector3::ZERO;
			Vector3 dimensions(1000000, 1000000, 1000000);
			const TiXmlElement* entitiesElement = 0;

			//Iterate over all the child elements
			String elementName;
			const TiXmlElement* childElement = 0;
			while (childElement = OgreMaxUtilities::IterateChildElements(objectElement, childElement))
			{
				elementName = childElement->Value();

				if (elementName == "origin")
					origin = OgreMaxUtilities::LoadXYZ(childElement);
				else if (elementName == "dimensions")
					dimensions = OgreMaxUtilities::LoadXYZ(childElement);
				else if (elementName == "entities")
					entitiesElement = childElement;
			}

			//Create the instanced geometry
			StaticGeometry* staticGeometry = this->sceneManager->createStaticGeometry(name);
			staticGeometry->setCastShadows(castShadows);
			OgreMaxUtilities::SetObjectVisibility(staticGeometry, visibility);
			staticGeometry->setOrigin(origin);
			staticGeometry->setRegionDimensions(dimensions);
			if (!renderQueue.empty())
				staticGeometry->setRenderQueueGroup(OgreMaxUtilities::ParseRenderQueue(renderQueue));
			staticGeometry->setRenderingDistance(renderingDistance);

			//Add the entities
			childElement = 0;
			while (childElement = OgreMaxUtilities::IterateChildElements(entitiesElement, childElement))
			{
				elementName = childElement->Value();

				if (elementName == "entity")
					LoadStaticGeometryEntity(childElement, staticGeometry);
			}

			//Build the static geometry
			staticGeometry->build();
		}

		void CustomOgreMaxScene::LoadStaticGeometryEntity(const TiXmlElement* objectElement)
		{
			static const String TEMP_ENTITY_NAME("__staticGeometryEntity");

			String meshFile = OgreMaxUtilities::GetStringAttribute(objectElement, "meshFile");
			Vector3 position = Vector3::ZERO;
			Quaternion rotation = Quaternion::IDENTITY;
			Vector3 scale = Vector3::UNIT_SCALE;
			std::vector<EntityParameters::Subentity> subentities;

			String elementName;
			const TiXmlElement* childElement = 0;
			while (childElement = OgreMaxUtilities::IterateChildElements(objectElement, childElement))
			{
				elementName = childElement->Value();

				if (elementName == "position")
					position = OgreMaxUtilities::LoadXYZ(childElement);
				else if (elementName == "rotation")
					rotation = OgreMaxUtilities::LoadRotation(childElement);
				else if (elementName == "scale")
					scale = OgreMaxUtilities::LoadXYZ(childElement);
				else if (elementName == "subentities")
					OgreMaxUtilities::LoadSubentities(childElement, subentities);
			}

			if (!meshFile.empty())
			{
				//Create temporary entity
				Entity* entity = OgreMaxUtilities::CreateEntity(this->sceneManager, TEMP_ENTITY_NAME, meshFile, subentities);
		        
				//Add entity to the static geometry
				staticGeometry->addEntity(entity, position, rotation, scale);

				//Destroy entity
				this->sceneManager->destroyEntity(entity);
			}
		}

//render textures TODO
void CustomOgreMaxScene::LoadRenderTextureMaterials ( const TiXmlElement* objectElement, std::vector<OgreMax::Types::RenderTextureParameters::Material>& materials ) {
    size_t index = 0;
    const TiXmlElement* childElement = 0;
    while (childElement = OgreMax::OgreMaxUtilities::IterateChildElements(objectElement, childElement))
    {
        RenderTextureParameters::Material& material = materials[index++];
        material.name = childElement->Attribute("name");
        material.techniqueIndex = OgreMax::OgreMaxUtilities::GetIntAttribute(childElement, "technique", 0);
        material.passIndex = OgreMax::OgreMaxUtilities::GetIntAttribute(childElement, "pass", 0);
        material.textureUnitIndex = OgreMax::OgreMaxUtilities::GetIntAttribute(childElement, "textureUnit", 0);
    }
}

void CustomOgreMaxScene::LoadRenderTextures(const TiXmlElement* objectElement)
{
    //Get default pixel format
    //It's unlikely that this would vary among render windows, but just take the minimum anyway
    unsigned int bestColorDepth = 32;
    if (this->renderWindows->Start())
    {
        do
        {
            bestColorDepth = std::min(this->renderWindows->GetCurrent()->getColourDepth(), bestColorDepth);
        }while (this->renderWindows->MoveNext());
    }
    PixelFormat defaultPixelFormat = (bestColorDepth == 16) ? PF_A4R4G4B4 : PF_A8R8G8B8;

    //Read all the render textures
    const TiXmlElement* renderTextureElement = 0;
    while (renderTextureElement = OgreMax::OgreMaxUtilities::IterateChildElements(objectElement, renderTextureElement))
    {
        LoadedRenderTexture* loadedRenderTexture = new LoadedRenderTexture;
        this->loadedRenderTextures.push_back(loadedRenderTexture);

        RenderTextureParameters& renderTextureParams = loadedRenderTexture->parameters;

        renderTextureParams.name = OgreMax::OgreMaxUtilities::GetStringAttribute(renderTextureElement, "name");
        renderTextureParams.width = OgreMax::OgreMaxUtilities::GetIntAttribute(renderTextureElement, "width", 512);
        renderTextureParams.height = OgreMax::OgreMaxUtilities::GetIntAttribute(renderTextureElement, "height", 512);
        renderTextureParams.pixelFormat = OgreMax::OgreMaxUtilities::GetPixelFormatAttribute(renderTextureElement, "pixelFormat", defaultPixelFormat);
        renderTextureParams.textureType = OgreMax::OgreMaxUtilities::GetTextureTypeAttribute(renderTextureElement, "textureType", renderTextureParams.textureType);
        renderTextureParams.cameraName = OgreMax::OgreMaxUtilities::GetStringAttribute(renderTextureElement, "camera");
        renderTextureParams.scheme = OgreMax::OgreMaxUtilities::GetStringAttribute(renderTextureElement, "scheme");
        renderTextureParams.clearEveryFrame = OgreMax::OgreMaxUtilities::GetBoolAttribute(renderTextureElement, "clearEveryFrame", renderTextureParams.clearEveryFrame);
        renderTextureParams.autoUpdate = OgreMax::OgreMaxUtilities::GetBoolAttribute(renderTextureElement, "autoUpdate", renderTextureParams.autoUpdate);
        renderTextureParams.hideRenderObject = OgreMax::OgreMaxUtilities::GetBoolAttribute(renderTextureElement, "hideRenderObject", renderTextureParams.hideRenderObject);
        renderTextureParams.renderObjectName = OgreMax::OgreMaxUtilities::GetStringAttribute(renderTextureElement, "renderObjectName");
        renderTextureParams.backgroundColor = this->backgroundColor;
        renderTextureParams.resourceGroupName = this->defaultResourceGroupName;

        String elementName;
        const TiXmlElement* childElement = 0;
        while (childElement = OgreMax::OgreMaxUtilities::IterateChildElements(renderTextureElement, childElement))
        {
            elementName = childElement->Value();

            if (elementName == "backgroundColor")
                renderTextureParams.backgroundColor = OgreMax::OgreMaxUtilities::LoadColor(childElement);
            else if (elementName == "materials")
            {
                size_t materialCount = OgreMax::OgreMaxUtilities::GetElementCount(childElement, "material");
                renderTextureParams.materials.resize(materialCount);
                LoadRenderTextureMaterials(childElement, renderTextureParams.materials);
            }
            else if (elementName == "hiddenObjects")
                LoadObjectNames(childElement, "hiddenObject", renderTextureParams.hiddenObjects);
            else if (elementName == "exclusiveObjects")
                LoadObjectNames(childElement, "exclusiveObject", renderTextureParams.exclusiveObjects);
            else if (elementName == "renderPlane")
                renderTextureParams.renderPlane = OgreMax::OgreMaxUtilities::LoadPlane(childElement);
        }

        //Notify callback
        if (this->callback != 0)
            this->callback->LoadingRenderTexture(this, renderTextureParams);

        //Create the render texture
        loadedRenderTexture->renderTexture = TextureManager::getSingleton().createManual
            (
            renderTextureParams.name,
            renderTextureParams.resourceGroupName,
            renderTextureParams.textureType,
            renderTextureParams.width,
            renderTextureParams.height,
            0,
            renderTextureParams.pixelFormat,
            TU_RENDERTARGET
            );

        //Initialize all the texture's render targets
        size_t faceCount = loadedRenderTexture->renderTexture->getNumFaces();
        for (size_t faceIndex = 0; faceIndex < faceCount; faceIndex++)
        {
            RenderTarget* renderTarget = loadedRenderTexture->renderTexture->getBuffer(faceIndex)->getRenderTarget();
            renderTarget->setAutoUpdated(renderTextureParams.autoUpdate);
            renderTarget->addListener(this);
            this->renderTargets[renderTarget] = loadedRenderTexture;
        }
    }
}

void CustomOgreMaxScene::FinishLoadingRenderTextures()
{
    static const Quaternion CUBE_FACE_CAMERA_ORIENTATIONS[] =
        {
        Quaternion(0.707107, 0, -0.707107, 0),
        Quaternion(-0.707107, 0, -0.707107, 0),
        Quaternion(0.707107, 0.707107, 0, 0),
        Quaternion(0.707107, -0.707107, 0, 0),
        Quaternion(1, 0, 0, 0),
        Quaternion(0, 0, -1, 0)
        };

    for (; this->currentRenderTextureIndex < this->loadedRenderTextures.size(); this->currentRenderTextureIndex++)
    {
        LoadedRenderTexture* loadedRenderTexture = this->loadedRenderTextures[this->currentRenderTextureIndex];
        const RenderTextureParameters& renderTextureParams = loadedRenderTexture->parameters;

        //Create the scheme
        if (!renderTextureParams.scheme.empty())
            MaterialManager::getSingleton()._getSchemeIndex(renderTextureParams.scheme);

        //Get the camera
        //First try the camera callback
        if (this->callback != 0)
            loadedRenderTexture->camera = this->callback->GetRenderTextureCamera(this, renderTextureParams);

        //If there's no camera yet, get the camera from the scene
        if (loadedRenderTexture->camera == 0 &&
            !renderTextureParams.cameraName.empty() &&
            this->sceneManager->hasCamera(renderTextureParams.cameraName))
        {
            loadedRenderTexture->camera = this->sceneManager->getCamera(renderTextureParams.cameraName);
        }

        //Set up viewport and render object (either a reflection plane for 2D, or an object for 3D)
        size_t faceCount = loadedRenderTexture->renderTexture->getNumFaces();
        if (renderTextureParams.textureType == TEX_TYPE_2D)
        {
            Viewport* viewport = 0;
            if (loadedRenderTexture->camera != 0)
            {
                RenderTarget* renderTarget = loadedRenderTexture->renderTexture->getBuffer()->getRenderTarget();

                //Add viewport
                Viewport* viewport = renderTarget->addViewport(loadedRenderTexture->camera);
                viewport->setClearEveryFrame(renderTextureParams.clearEveryFrame);
                viewport->setBackgroundColour(renderTextureParams.backgroundColor);
                viewport->setMaterialScheme(renderTextureParams.scheme);
                loadedRenderTexture->viewports[0] = viewport;

                //Set up render object (reflection plane)
                if (!renderTextureParams.renderObjectName.empty())
                {
                    //Build the plane name
                    String movablePlaneName;
                    OgreMax::OgreMaxUtilities::CreateMovablePlaneName(movablePlaneName, renderTextureParams.renderObjectName);

                    //Get or create the render plane
                    MovablePlanesMap::iterator planeIterator = this->movablePlanes.find(movablePlaneName);
                    if (planeIterator != this->movablePlanes.end())
                    {
                        //Found an existing movable plane
                        loadedRenderTexture->renderPlane = planeIterator->second;
                        loadedRenderTexture->renderObjectNode = (SceneNode*)loadedRenderTexture->renderPlane->getParentNode();
                    }
                    else
                    {
                        //Create a new movable plane
                        loadedRenderTexture->renderPlane = new MovablePlane(movablePlaneName);
                        this->movablePlanes[movablePlaneName] = loadedRenderTexture->renderPlane;
                        loadedRenderTexture->renderPlane->normal = renderTextureParams.renderPlane.normal;
                        loadedRenderTexture->renderPlane->d = renderTextureParams.renderPlane.d;
                    }

                    //Configure reflection
                    for (size_t materialIndex = 0; materialIndex < renderTextureParams.materials.size(); materialIndex++)
                    {
                        const RenderTextureParameters::Material& renderTextureMaterial =
                            renderTextureParams.materials[materialIndex];

                        MaterialPtr material = MaterialManager::getSingleton().getByName(renderTextureMaterial.name);
                        if (!material.isNull())
                        {
                            if (renderTextureMaterial.techniqueIndex < material->getNumTechniques())
                            {
                                Technique* technique = material->getTechnique(renderTextureMaterial.techniqueIndex);
                                if (renderTextureMaterial.passIndex < technique->getNumPasses())
                                {
                                    Pass* pass = technique->getPass(renderTextureMaterial.passIndex);
                                    if (renderTextureMaterial.textureUnitIndex < pass->getNumTextureUnitStates())
                                    {
                                        TextureUnitState* textureUnit =
                                            pass->getTextureUnitState(renderTextureMaterial.textureUnitIndex);
                                        textureUnit->setProjectiveTexturing(true, loadedRenderTexture->camera);
                                    }
                                }
                            }
                        }
                    }
                    loadedRenderTexture->camera->enableReflection(loadedRenderTexture->renderPlane);
                    loadedRenderTexture->camera->enableCustomNearClipPlane(loadedRenderTexture->renderPlane);
                }
            }
        }
        else if (renderTextureParams.textureType == TEX_TYPE_CUBE_MAP)
        {
            //Get the render object, if any
            if (!renderTextureParams.renderObjectName.empty() && this->sceneManager->hasSceneNode(renderTextureParams.renderObjectName))
                loadedRenderTexture->renderObjectNode = GetSceneNode(renderTextureParams.renderObjectName, false);

            //Get the position from which the cube map will be rendered
            Vector3 position;
            loadedRenderTexture->GetReferencePosition(position);

            //Create a camera and viewport for each cube face
            String cameraName;
            for (size_t faceIndex = 0; faceIndex < faceCount; faceIndex++)
            {
                //Build a unique camera name
                cameraName = loadedRenderTexture->camera->getName() + "_CubeFaceCamera" + StringConverter::toString(faceIndex);

                //Create camera
                Camera* cubeFaceCamera = this->sceneManager->createCamera(cameraName);
                cubeFaceCamera->setAspectRatio(1);
                cubeFaceCamera->setFOVy(Degree(90));
                cubeFaceCamera->setPosition(position);
                cubeFaceCamera->setOrientation(CUBE_FACE_CAMERA_ORIENTATIONS[faceIndex]);

                //Use the reference camera's clip distances, if possible
                if (loadedRenderTexture->camera != 0)
                {
                    cubeFaceCamera->setNearClipDistance(loadedRenderTexture->camera->getNearClipDistance());
                    cubeFaceCamera->setFarClipDistance(loadedRenderTexture->camera->getFarClipDistance());
                }

                loadedRenderTexture->cubeFaceCameras[faceIndex] = cubeFaceCamera;

                //Add viewport
                RenderTarget* renderTarget = loadedRenderTexture->renderTexture->getBuffer(faceIndex)->getRenderTarget();
                Viewport* viewport = renderTarget->addViewport(cubeFaceCamera);
                viewport->setClearEveryFrame(renderTextureParams.clearEveryFrame);
                viewport->setBackgroundColour(renderTextureParams.backgroundColor);
                viewport->setMaterialScheme(renderTextureParams.scheme);
                loadedRenderTexture->viewports[faceIndex] = viewport;
            }
        }

        GetRenderTextureObjects(loadedRenderTexture);

        //Notify callback
        if (this->callback != 0)
           this->callback->CreatedRenderTexture(this, loadedRenderTexture);
    }
}

void CustomOgreMaxScene::GetRenderTextureObjects(OgreMax::Types::LoadedRenderTexture* loadedRenderTexture)
{
    const RenderTextureParameters& renderTextureParams = loadedRenderTexture->parameters;

    //Get hidden objects
    loadedRenderTexture->hiddenObjects.reserve(renderTextureParams.hiddenObjects.size());
    for (size_t hiddenIndex = 0; hiddenIndex < renderTextureParams.hiddenObjects.size(); hiddenIndex++)
    {
        if (this->sceneManager->hasSceneNode(renderTextureParams.hiddenObjects[hiddenIndex]))
        {
            SceneNode* node = GetSceneNode(renderTextureParams.hiddenObjects[hiddenIndex], false);
            loadedRenderTexture->hiddenObjects.push_back(node);
        }
    }
    loadedRenderTexture->hiddenObjects.Hide();

    //Get exclusive objects
    loadedRenderTexture->exclusiveObjects.reserve(renderTextureParams.exclusiveObjects.size());
    for (size_t exclusiveIndex = 0; exclusiveIndex < renderTextureParams.exclusiveObjects.size(); exclusiveIndex++)
    {
        if (this->sceneManager->hasSceneNode(renderTextureParams.exclusiveObjects[exclusiveIndex]))
        {
            SceneNode* node = GetSceneNode(renderTextureParams.exclusiveObjects[exclusiveIndex], false);
            loadedRenderTexture->exclusiveObjects.push_back(node);
        }
    }
    loadedRenderTexture->exclusiveObjects.Hide();
}



//terrain TODO
void CustomOgreMaxScene::LoadTerrain(const TiXmlElement* objectElement) {
    //Exit early if skip option is set
    if ((this->loadOptions & SKIP_TERRAIN) != 0)
        return;

    String renderQueue = OgreMax::OgreMaxUtilities::GetStringAttribute(objectElement, "renderQueue");

    String dataFile = OgreMax::OgreMaxUtilities::GetStringAttribute(objectElement, "dataFile");
    if (!dataFile.empty())
    {
        this->sceneManager->setWorldGeometry(dataFile);
        if (!renderQueue.empty())
            this->sceneManager->setWorldGeometryRenderQueue(OgreMax::OgreMaxUtilities::ParseRenderQueue(renderQueue));
    }

    //Parse child elements
    String elementName;
    const TiXmlElement* childElement = 0;
    while (childElement = OgreMax::OgreMaxUtilities::IterateChildElements(objectElement, childElement))
    {
        elementName = childElement->Value();

        if (elementName == "userDataReference")
            OgreMax::OgreMaxUtilities::LoadUserDataReference(childElement, this->terrainExtraData.userDataReference);
        else if (elementName == "userData")
            OgreMax::OgreMaxUtilities::GetChildText(childElement, this->terrainExtraData.userData);
    }

    if (this->sceneManager->hasSceneNode("Terrain"))
    {
        SceneNode* terrainNode = this->sceneManager->getSceneNode("Terrain");
        terrainNode->setOrientation(OgreMax::OgreMaxUtilities::GetOrientation(this->upAxis));
    }

    //Notify callback
    if (this->callback != 0)
        this->callback->CreatedTerrain(this, dataFile);
		
		
}

void CustomOgreMaxScene::LoadLookTarget(const TiXmlElement* objectElement, SceneNode* node, Camera* camera)
{
    LookTarget lookTarget(node, camera);

    lookTarget.nodeName = OgreMax::OgreMaxUtilities::GetStringAttribute(objectElement, "nodeName");

    String relativeTo = OgreMax::OgreMaxUtilities::GetStringAttribute(objectElement, "relativeTo");
    if (!relativeTo.empty())
        lookTarget.relativeTo = OgreMax::OgreMaxUtilities::ParseTransformSpace(relativeTo);

    //Parse child elements
    String elementName;
    const TiXmlElement* childElement = 0;
    while (childElement = OgreMax::OgreMaxUtilities::IterateChildElements(objectElement, childElement))
    {
        elementName = childElement->Value();

        if (elementName == "position")
        {
            lookTarget.position = OgreMax::OgreMaxUtilities::LoadXYZ(childElement);
            lookTarget.isPositionSet = true;
        }
        else if (elementName == "localDirection")
            lookTarget.localDirection = OgreMax::OgreMaxUtilities::LoadXYZ(childElement);
    }

    //Store look target information for later
    this->lookTargets.push_back(lookTarget);
}

void CustomOgreMaxScene::LoadTrackTarget(const TiXmlElement* objectElement, SceneNode* node, Camera* camera)
{
    TrackTarget trackTarget(node, camera);

    trackTarget.nodeName = OgreMax::OgreMaxUtilities::GetStringAttribute(objectElement, "nodeName");

    //Parse child elements
    String elementName;
    const TiXmlElement* childElement = 0;
    while (childElement = OgreMax::OgreMaxUtilities::IterateChildElements(objectElement, childElement))
    {
        elementName = childElement->Value();

        if (elementName == "offset")
            trackTarget.offset = OgreMax::OgreMaxUtilities::LoadXYZ(childElement);
        else if (elementName == "localDirection")
            trackTarget.localDirection = OgreMax::OgreMaxUtilities::LoadXYZ(childElement);
    }

    //Store track target information for later
    this->trackTargets.push_back(trackTarget);
}


*/

}