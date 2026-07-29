for o in objects do (
	
	ogrekeyword = "<OgreEntity>"
	ogreendkeyword = "</OgreEntity>"	
	
	
	objectSettings = ogremax.GetObjectSettingsByNode(o) 
	
	if hasProperty objectSettings "userdata" do ( 
			
		position = undefined
		end = undefined
				
		if  objectSettings.userdata != undefined  then (			
			position = findString objectSettings.userdata ogrekeyword
			end = findString objectSettings.userdata ogreendkeyword
		)
		
		/*break()*/
		
		/** SET OGRE-ENTITY ON GAME-ENTITIES **/
			
		/** if OGRE-ENTITY tag found but not a Ogre-Entity itself **/
		if position != undefined AND position != 1 then (			
			/** Replace the name with the parent name or with "" **/
			start = position + ogrekeyword.count
			count = end - start	
			if o.parent != undefined then (
				objectSettings.userdata = replace objectSettings.userdata start count o.parent.name
			)
			else (
				objectSettings.userdata = replace objectSettings.userdata start count ""
			)				
		)
				
		/** SET GAME-ENTITIES ON OGRE-ENTITY **/		
		done = false
		
		for child in o.children while  done == false do (
					
			childObjectSettings = ogremax.GetObjectSettingsByNode(child) 	
		
			
			
			/** The child should have userdata **/
			if hasProperty childObjectSettings "userdata" AND childObjectSettings.userdata != undefined do (
				
				/** The child should have the OGRE-ENTITY tag but it should not be an OGRE-ENTITY itself **/
				test = findString childObjectSettings.userdata ogrekeyword
				if test != undefined AND test != 1 do (			
						
					/** Update the tag **/					
					if objectSettings.userdata == undefined OR  objectSettings.userdata == "" then (
						/** Add tag if no tags on object **/
						objectSettings.UserDataClass = "OgreEntity"
						objectSettings.userdata = ogrekeyword + "<GameEntity>"
						objectSettings.userdata += child.name
						objectSettings.userdata += "</GameEntity>" + ogreendkeyword
					)
					else (
						
						gamekeyword = "<GameEntity>"
						gameendkeyword = "</GameEntity>"	
						
						position = findString objectSettings.userdata gamekeyword
						end = findString objectSettings.userdata gameendkeyword
						
						
						if position != undefined and end != undefined then (
							/** Replace content if tag existed **/
							print objectSettings.userdata
							
							start = position + gamekeyword.count
							count = end - start	
							objectSettings.userdata = replace objectSettings.userdata start count child.name		

							print  objectSettings.userdata	
							
						)
						else (							
							/** Add tag as extra tag on object **/
							
							/** look for last "</" **/
							p1 = 0
							p2 = undefined						
							while p1 != undefined do (
								p2 = p1
								p1 = findString objectSettings.userdata "</"							
							)
													
							newtag = ogrekeyword + "<GameEntity>" + child.name + "</GameEntity>" + ogreendkeyword	
							objectSettings.userdata = replace objectSettings.userdata p2 0 newtag		
						)						
					)
					
					/** SET  UP PIVOT POSITION **/
					o.pivot = child.pivot
					
					done = true
				)
			)					
		)		
	)
)
