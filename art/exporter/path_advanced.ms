macroscript Track category: "Hovercraft Universe Tools"
(
	local static
	rollout StaticRollout "Static" width:216 height:184
	(
		fn objectFilter obj = iskindof obj GeometryClass
		
		pickbutton btn1 "PickButton" pos:[40,32] width:88 height:32 filter:objectFilter
		button btn2 "Update" pos:[40,144] width:88 height:32
		edittext edtmass "Mass" pos:[8,88] width:120 height:16
		editText edt4 "Object" pos:[8,8] width:120 height:16
				
		on btn1 picked obj do
		(
			edt4.text = obj.name
		)
		on btn2 pressed do
		(
			s = getNodeByName edt4.text 
			
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
			s.modifiers[#Rigid_Body].mass = (edtmass.text as float)
			
			dummyname = s.name + "_ent"	
				
			/* delete dummy if already there */
			olddum = getNodeByName dummyname
			if olddum != undefined do
				delete olddum	
				
			dum = Dummyhelper pos:(s.pos) iconSize:10
			/* make our dummy slightly bigger than object */
			sc = length (s.max - s.min) / 9.95
			scale dum [sc,sc,sc]							
			dum.name = dummyname
			dum.parent = s

			objectSettingsStatic = ogremax.GetObjectSettingsByNode(s) 
			objectSettingsDummy = ogremax.GetObjectSettingsByNode(dum) 
			
			objectSettingsDummy.type = "External"
			objectSettingsDummy.UserDataClass = "StaticBody"
			objectSettingsDummy.UserData = "<StaticBody><OgreEntity>" + s.name + "</OgreEntity><ProcessInterval>" + "-1" + "</ProcessInterval></StaticBody>"
		
			objectSettingsStatic.UserDataClass = "OgreEntity"
			objectSettingsStatic.UserData = "<OgreEntity><GameEntity>" + dum.name + "</GameEntity></OgreEntity>"
		)
	)
	
	
	local floater
	rollout TrackRollout "Path" width:216 height:168
	(
		fn shapeFilter obj = iskindof obj Shape
		fn objectFilter obj = iskindof obj GeometryClass	
				
		pickbutton btnPickTerrain "..." pos:[176,80] width:32 height:16 filter:objectFilter
		pickbutton btnPickShape "..." pos:[176,32] width:32 height:16 filter:shapeFilter
		button btnOK "OK" pos:[152,144] width:56 height:16
		edittext txtShape "Shape" pos:[8,8] width:200 height:16 enabled:false
		spinner spnSubdivisions "Subdivisions" pos:[24,120] width:184 height:16 range:[0,100,10] type:#integer
		button btnCancel "Cancel" pos:[96,144] width:48 height:16
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
		on btnCancel pressed do
		(
			closeRolloutFloater floater
		)
	)
	
	on Execute do 
	(
			floater  = NewRolloutFloater "Track Creator" 500 200
			addRollout TrackRollout floater
			addRollout StaticRollout floater
	)
)