sceneSettings = ogremax.GetSceneSettings() 

basefilename = ""
ready = false


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


if  ready == true then (

	done = false
	for o in objects while done == false do (
		
		name = toLower o.name
		
		if (findString name "Path" != undefined) and (classof o == SplineShape or classof o == Line) then 
		(

			filename = basefilename + ".path" 

			file = createFile filename
			
			for s = 1 to (numSplines o) do (
				
				for k = 1 to (numKnots o s) do
				(
					knot = getKnotPoint o s k
					
					format "% % % 1\n" knot.x knot.y knot.z to: file				

				)--end k loop
			)--end s loop
			
			close file
			
			/* set path settings in scene */
			
			start = findString sceneSettings.userdata "<PathFileName>"
			end = findString sceneSettings.userdata "</PathFileName>"
			start += 14
			count = end - start	
			sceneSettings.userdata = replace sceneSettings.userdata start count filename	
			
			
			done = true;
		)
	)
)

if ( ready == false or done == false ) do (
	messageBox "No path could be exported"
)