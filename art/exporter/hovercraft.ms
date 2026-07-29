done = false

for o in objects while done == false do (
	
	objectSettings = ogremax.GetObjectSettingsByNode(o) 
	
	if hasProperty objectSettings "userdata" do ( 
		
		if  objectSettings.userdata != undefined and objectSettings.UserDataClass == "Hovercraft" then (
			start = findString objectSettings.userdata "<OgreEntity>" + 12
			end = findString objectSettings.userdata "</OgreEntity>"
			count = end - start			
			objectSettings.userdata = replace objectSettings.userdata start count o.name
		)		
		
		if  objectSettings.userdata != undefined and objectSettings.UserDataClass == "Hovercraft" and o.children.count > 0 then (	

			for child in o.children while done == false do (
				for m in child.modifiers while done == false do(
					if m.name == "Rigid Body" then (
						
						child.name = o.name
							
						start = findString objectSettings.userdata "<Mass>" + 6
						end = findString objectSettings.userdata "</Mass>"

						if start != undefined and end != undefined then (
							count = end - start
							mass = substring objectSettings.userdata start count		
							print mass							
							m.mass =(mass as Number)							
						)
						
						/* */
						done = true
					)
				)				
			)
		)
	)
)