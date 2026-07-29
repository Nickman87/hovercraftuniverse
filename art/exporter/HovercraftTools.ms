macroscript WorldEditing category: "Hovercraft Universe Tools"
(
	local floater
	
	local GameTypes = #("Start", "Finish", "CheckPoint", "Boost", "StaticBody","Asteroid")
	local GameTypesOptions = #(false, false, true, true, false,true)
	local GameEntitySpeedLevels = #("NONE","10fps","20fps","30fps","60fps");
	local GameEntitySpeedLevelsSpeed = #(-1,1.0/10.0,1.0/20.0,1.0/30.0,1.0/60.0);
	
	fn createObjectSettingsIfNotFound obj = (
		objectSettings = ogremax.GetObjectSettingsByNode(obj) 

		if objectSettings.userdata == undefined do (
			objectSettings.userdata = ""
		)		
	)

	/**
	  *	Get the value of a tag in the xml
	  * @param xml
	  * @param tag, the tag
	  * @return value, undefined if tag nog found
	  */	
	fn getTagValue xml tag = (
		taglength = tag.count
		begintag = "<" + tag +  ">"
		endtag = "</" + tag + ">"
		
		startpos = (findString xml begintag) 	/*begin of the value*/
		endpos = (findString xml endtag) 		/*end of the value*/
		
		if startpos != undefined and endpos != undefined then (
			startpos += taglength + 2 /** the <> and taglength */
			count = endpos - startpos
			/** replace userdata with new version */
			if count != 0 then (
				return substring xml startpos count
			)
			else (
				return ""
			)
		)
		return undefined
	)
	
	/**
	  *	Update the value of a tag in the userdata of given object
	  * @param obj, the object
	  * @param tag, the tag
	  * @param val, the new value
	  * @return true if success, false if tag nog found
	  */
	fn updateTagValue obj tag val = (
		print "updateTagValue" 
		
		taglength = tag.count
		objectSettings = ogremax.GetObjectSettingsByNode(obj) 
		begintag = "<" + tag +  ">"
		endtag = "</" + tag + ">"

		startpos = (findString objectSettings.userdata begintag) 	/*begin of the value*/
		endpos = (findString objectSettings.userdata endtag) 		/*end of the value*/
			
		if startpos != undefined and endpos != undefined then (
			startpos += taglength + 2 /** the <> and taglength */
			count = endpos - startpos
			/** replace userdata with new version */
			if count != 0 do
				objectSettings.userdata = replace objectSettings.userdata startpos count ""	/*rem value*/			
			
			objectSettings.userdata = substituteString  objectSettings.userdata endtag (val+endtag) /* add value*/
			
			return true
		) 
		else (
			return false
		)
	) 
	
	/**
	  *	Check if object has given parent tag,
	  * @param obj, the object
	  * @param tag, parent tag
	  * @return true if parent tag found, false if not
	  */
	fn hasParentTag obj tag = (	
		print "hasParentTag" 
		
		objectSettings = ogremax.GetObjectSettingsByNode(obj) 
		begintag = "<" + tag +  ">"
		
		pos = findString objectSettings.userdata begintag

		return (pos != undefined)
	)	
	
	/**
	  *	Add parent tag, removes all current user data
	  * @param obj, the object
	  * @param tag, parent tag
	  * @param val, the new value put undefined for empty parent tag
	  */
	fn addParentTag obj tag val = (	
		print "addParentTag" 
		
		objectSettings = ogremax.GetObjectSettingsByNode(obj) 
		if val != undefined then
			objectSettings.userdata = "<" + tag + ">" + val + "</" + tag + ">"
		else
			objectSettings.userdata = "<" + tag + "></" + tag + ">"		
	)
	
	/**
	  *	Add a new tag with given value in given paren tag in the userdata of given object
	  * @param obj, the object
	  * @param parenttag, parent tag
	  * @param tag, the tag
	  * @param val, the new value
	  * @return true if success, false if parent tag nog found
	  */
	fn addTag obj parenttag tag val = (
		print "addTag" 
		
		objectSettings = ogremax.GetObjectSettingsByNode(obj) 
		endtag = "</" + parenttag + ">"
		newtag = "<" + tag + ">" + val + "</" + tag +">"		

		endpos = (findString objectSettings.userdata endtag) 	/*end of the value*/	
		
		if endpos != undefined then (
			/** replace userdata with new version */
			objectSettings.userdata = substituteString  objectSettings.userdata endtag (newtag+endtag)
					
			return true
		) 
		else (
			return false
		)
	)
	
		
	fn addOrUpdateTagValue obj parenttag tag val = (
		result = (updateTagValue obj tag val)
		if not result then
			return addTag obj parenttag tag val 
		else
			return true
	)
	
	
	rollout Track "General track settings" width:176 height:160
	(
		edittext edt1 "TrackName" pos:[8,8] width:160 height:16
		button btn1 "Update" pos:[112,136] width:56 height:16

		on btn1 pressed do
		(
				sceneSettings = ogremax.GetSceneSettings() 			
				edt1.text = getTagValue sceneSettings.userdata "name"			
				sceneSettings.MeshPath = edt1.text + "/models"
				sceneSettings.ModelPath = sceneSettings.MeshPath
				sceneSettings.BitmapPath = "materials/textures"
				sceneSettings.MaterialPath = edt1.text + "/materials/scripts"
			)
	)
	
	rollout RaceObjectsRollout "Race Objects" width:216 height:269
	(
		
		fn objectFilter obj = iskindof obj GeometryClass		
		pickbutton pickBtn "..." pos:[176,32] width:32 height:16 filter:objectFilter
		button btn2 "Update" pos:[160,240] width:52 height:16
		edittext edtobjectname "Object" pos:[8,8] width:200 height:16
		dropdownList ddl "Game Object" pos:[8,56] width:200 height:40
		checkbox chkReset "Recreate Entity" pos:[8,192] width:200 height:16
		dropdownList cbx1 "Entity Update Speed" pos:[8,112] width:200 height:40
		checkbox chkentoptions "See Entity Options (if it has any)" pos:[8,216] width:200 height:16 checked:true
		checkbox chkpivot "Fix Pivot, centers and orients pivot" pos:[8,168] width:200 height:16 checked:true
		
		on RaceObjectsRollout open do
		(
			ddl.items = GameTypes
			cbx1.items = GameEntitySpeedLevels
		)
		on pickBtn picked obj do
		(
			edtobjectname.text = obj.name
		)
		on btn2 pressed do
		(
			s = getNodeByName edtobjectname.text 
			
			if s == undefined then (
				print "SELECT FIRST"
			)
			else (		
		
				if chkpivot.enabled do (				
					s.pivot = (s.max + s.min)/2
				)
				
				while s.modifiers[#Shape] != undefined do (
					m = s.modifiers[#Shape] 
					deleteModifier  s m
				)
				
				while s.modifiers[#Rigid_Body] != undefined do (
					m = s.modifiers[#Rigid_Body] 
					deleteModifier  s m
				)
							
				addModifier s (Havok_Shape()) 
				s.modifiers[#Shape].shapeType = 5
				addModifier s (Havok_Rigid_Body())
				s.modifiers[#Rigid_Body].mass = 0
				
				dummyname = s.name + "_ent"	
							
					
				/* delete possible dummy if reset set */			
				if chkReset.checked do (				
					olddum = getNodeByName dummyname
					if olddum != undefined do
						delete olddum
				)
				
				/** create dum if not found */
				dum = getNodeByName dummyname
				if dum == undefined do (			
					dum = Dummyhelper pos:(s.pos) iconSize:1
					dum.name = dummyname					
					tmp = s.transform 					
					s.transform = matrix3 1	
					sc =  (s.max - s.min)
					scale dum sc
					s.transform = tmp
					dum.rotation = s.rotation
					dum.pos = s.pos
					
					dum.parent = s
				)
				
				updatespeed = GameEntitySpeedLevelsSpeed[cbx1.selection]				
				type = ddl.selected
						
				/** update the tags */
							
				/** DUMMY */
				print "DUMMY"
				createObjectSettingsIfNotFound dum
				
				objectSettingsDummy = ogremax.GetObjectSettingsByNode(dum) 			
				objectSettingsDummy.type = "External"
				objectSettingsDummy.UserDataClass = type
				
				if not (hasParentTag dum type) do (
					addParentTag dum type undefined
				)
			
				if not (addOrUpdateTagValue dum type "OgreEntity" s.name)  do (
					print "ERR: addOrUpdateTagValue dum type OgreEntity s.name"
				)
				
				if not (addOrUpdateTagValue dum type "ProcessInterval" (updatespeed as String) )  do (
					print "ERR: addOrUpdateTagValue dum type ProcessInterval (updatespeed as String)"
				)	
				
				/** OBJECT  */
				print "OBJECT"
				createObjectSettingsIfNotFound s
				objectSettingsObject = ogremax.GetObjectSettingsByNode(s) 			
				objectSettingsObject.UserDataClass = "OgreEntity"
				
				if not (hasParentTag s "OgreEntity") do (
					addParentTag s "OgreEntity" undefined 		
				)
			
				if not (addOrUpdateTagValue s "OgreEntity" "GameEntity" dum.name)  do (
					/*ERROR*/
					print "ERR: addOrUpdateTagValue s OgreEntity GameEntity dum.name"
				)
				
				if GameTypesOptions[ddl.selection] and chkentoptions.checked do (
					tmp = getCurrentSelection()
					clearSelection()
					select dum
					OgreMaxGlobalUtilities.OgreMax.ObjectSettings ()
					clearSelection
					select tmp
				)
			)
		)
	)
	
	
	rollout TrackRollout "Path" width:216 height:168
	(
		fn shapeFilter obj = iskindof obj Shape
		fn objectFilter obj = iskindof obj GeometryClass	
				
		pickbutton btnPickTerrain "..." pos:[176,80] width:32 height:16 filter:objectFilter
		pickbutton btnPickShape "..." pos:[176,32] width:32 height:16 filter:shapeFilter
		button btnOK "OK" pos:[152,144] width:56 height:16
		edittext txtShape "Shape" pos:[8,8] width:200 height:16 enabled:false
		spinner spnSubdivisions "Subdivisions" pos:[24,120] width:184 height:16 range:[0,100,10] type:#integer
		editText txtTerrain "Terrain" pos:[8,56] width:200 height:16 enabled:false
			
		on btnPickShape picked obj do
		(
				txtShape.text = obj.name
		)
		
		on btnPickTerrain picked obj do
		(
				txtTerrain.text = obj.name
		)
		
		on btnOK pressed do
		(
			nrOfElements = spnSubdivisions.value - 1
			s = getNodeByName txtShape.text
			t = getNodeByName txtTerrain.text
			
			sceneSettings = ogremax.GetSceneSettings() 

			basefilename = ""
			if sceneSettings.userdata != undefined then (
				start = findString sceneSettings.userdata "<Name>"
				end = findString sceneSettings.userdata "</Name>"
				
				if start != undefined and end != undefined then (
					start += 6
					count = end - start
					basefilename = substring sceneSettings.userdata start count
					ready = true		
				)
			)			
			
			filename = basefilename + ".path" 

			file = createFile filename
					
			for i = 0 to nrOfElements do
			(
				p = lengthInterp s 1 ((1.0 * i) / nrOfElements)
				
				/*dir = pathTangent s ((1.0 * i) / nrOfElements)
				rotZ = atan2 dir.y dir.x
				rot = eulerAngles 0 0 -rotZ	*/		
				
				
				dir = t.position - p				
				landscapeRay = ray p dir
				result = intersectRay t landscapeRay
				
				format "% % % 1\n" p.x p.y p.z to: file	
			)
			
			close file
			
			/* set path settings in scene */
			
			start = findString sceneSettings.userdata "<PathFileName>"
			end = findString sceneSettings.userdata "</PathFileName>"
			start += 14
			count = end - start	
			sceneSettings.userdata = replace sceneSettings.userdata start count filename	
			
	

		)
	)
	
	on Execute do 
	(
			floater  = NewRolloutFloater "Track Creator" 232 500
			addRollout Track floater
			addRollout RaceObjectsRollout floater
			addRollout TrackRollout floater			
	)
)