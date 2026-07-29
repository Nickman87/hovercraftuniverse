macroscript Asteroid category: "Hovercraft Universe Tools"
(

local shaderfx_path = pathConfig.getCurrentProjectFolder()  + "\\shaderfx\\"
local terrain_layer1_shader = shaderfx_path  + "terrain1.fx"
local terrain_layer2_shader = shaderfx_path  + "terrain2.fx"
local colormapoutputpath = (getDir #image) + "\\asteroid"

print terrain_layer1_shader
print terrain_layer2_shader

struct AsteroidInfo (
	/** Indices of the paint modifiers in the object modifier stack */
	vertexpaintidices = #(0,0),
	
	/** Number of paint layers, this is 0, 1 or 2 */
	vertexpaintlayers = 0,
	
	/** The Asteroid object **/
	asteroid = undefined,
	
	/** The ids of the color, viewport and ogre materials */
	color_material_id = undefined,	
	ogre_material_id = undefined,
	viewport_material_id = undefined,	
	
	terrain_layers = #("","","","","",""),
	
	fn createAlphaMap size outputfile = 
	(
		print "createAlphaMap"
		
		if color_material_id == undefined then (
			messageBox "No color material found! Use create firts" title:"Error"			
		)
		else (		
			asteroid.material = currentMaterialLibrary[color_material_id]
			
			/* clean all previous */
			asteroid.iNodeBakeProperties.removeAllBakeElements() 
			
			be1 = diffusemap() --instance of the bake element class
			be1.outputSzX = be1.outputSzY = size --set the size of the baked map	
			be1.fileType = outputfile --specify the full file path, name and type:
			be1.fileName = filenameFromPath be1.fileType 
			be1.filterOn = true --enable filtering 
			be1.shadowsOn = false --disable shadows
			be1.lightingOn = false --disable lighting
			be1.enabled = true --enable baking
			
			asteroid.INodeBakeProperties.addBakeElement be1 --add first element
			asteroid.INodeBakeProperties.bakeEnabled = true --enabling baking
			asteroid.INodeBakeProperties.bakeChannel = 1 --channel to bake
			asteroid.INodeBakeProperties.nDilations = 1 --expand the texture a bit

			--Call the renderer to bake both elements:   
			render rendertype:#bakeSelected vfb:off progressBar:true outputSize:[size,size]
		)
	)
	
	,
	
	fn clean = 
	(
		/* clean materials */
		if color_material_id != undefined and color_material_id <=  currentMaterialLibrary.count then (
			currentMaterialLibrary[color_material_id] = standardMaterial ()
		)
		
		if viewport_material_id != undefined and viewport_material_id <=  currentMaterialLibrary.count  then (
			currentMaterialLibrary[viewport_material_id] = standardMaterial ()
		)
		
		/* clean properties */
		setUserPropBuffer asteroid ""		
	)
	
	,
	
	fn setTerrainLayer index filename =
	(
		terrain_layers[index] = filename
		
		prop = "layer_" + (index as string)
			
		setUserProp asteroid  prop filename
	)
	
	,	
	
	fn updateTextures = 
	(
		if viewport_material_id != undefined then (
			
			dxmat = currentMaterialLibrary[viewport_material_id]
			
			if dxmat != undefined then (
			
				showProperties dxmat
				
				if vertexpaintlayers > 1 then (	
					tmp = openBitMap (colormapoutputpath + "\\" + asteroid.name + "1_alpha.png")	
			/*		if tmp != undefined then 
						dxmat.AlphaMap0_TS = tmp	
					
					tmp = openBitMap terrain_layers[1]
					if tmp != undefined then 
						dxmat.Layer0_TS = tmp

					tmp = openBitMap terrain_layers[2]
					if tmp != undefined then 
						dxmat.Layer1_TS = tmp
							
					tmp = openBitMap terrain_layers[3]
					if tmp != undefined then 
						dxmat.Layer2_TS = tmp	*/
				)
				
				if vertexpaintlayers == 2 then (
					tmp = openBitMap (colormapoutputpath + "\\" + asteroid.name + "2_alpha.png")
			/*		if tmp != undefined then 
						dxmat.AlphaMap1_TS = tmp
					
					tmp = openBitMap terrain_layers[4]
					if tmp != undefined then 
						dxmat.Layer4_TS = tmp	
					
					tmp = openBitMap terrain_layers[5]
					if tmp != undefined then 
						dxmat.Layer5_TS = tmp	
									
					tmp = openBitMap terrain_layers[6]
					if tmp != undefined then 
						dxmat.Layer5_TS = tmp	*/
				)	
			)
		)
		
		if ogre_material_id == undefined then (
		)
	)
	
	,
		
	/** 
	 * Initialise the Asteroid info struct 
	 * @param obj, the asteroid object
	 */
	fn create obj = 
	(
		print "create"
		
		asteroid = obj
		
		/* check if one or two vertex paint layers */
		for i = 1 to obj.modifiers.count do (
			if obj.modifiers[i].name == "VertexPaint"  then (	
				vertexpaintlayers += 1
				vertexpaintidices[vertexpaintlayers] = i
			)
		)
		
		/** Read saved properties */
		color_material_id = getUserProp obj "color"
		ogre_material_id =  getUserProp obj "ogre"
		viewport_material_id = getUserProp  obj "viewport"		
		
		for i = 1 to 6 do 
		(			
			prop = "layer_" + (i as string)			
			filename = getUserProp asteroid  prop
			if filename != undefined  then			
				terrain_layers[i] = filename 			
		)
		
		/** Crete if nessessary **/
		if color_material_id == undefined then (
			/** Create Global material */
			vertexcolormap = VertexColor()
			colormat = standardMaterial diffusemap: vertexcolormap
			append currentMaterialLibrary colormat
			color_material_id = findItem currentMaterialLibrary colormat
			setUserProp asteroid "color" color_material_id
		)
		
		if viewport_material_id == undefined then (
			/** Create DirectX material (viewport) */
			effectfile = terrain_layer1_shader
			if vertexpaintlayers == 2 then
				effectfile = terrain_layer2_shader
			
			dxmat = DxMaterial effectfile:effectfile

			append currentMaterialLibrary dxmat
			viewport_material_id = findItem currentMaterialLibrary dxmat
			setUserProp asteroid "viewport" viewport_material_id
		)
		
		done = false
		/* find unwrap */
		for i = 1 to obj.modifiers.count while done == false do (

			if obj.modifiers[i].name == "Unwrap UVW"  then (	
				obj.modifiers[i].unwrap2.selectByMatID 1
				obj.modifiers[i].setFlattenAngle 60
				obj.modifiers[i].flattenMapNoParams()
				done = true					
			)		
		)
	)
	
	,
	
	fn getDxMaterial = 
	(
		if viewport_material_id == undefined then (
			return undefined			
		)
		else (
			return currentMaterialLibrary[viewport_material_id]
		)
	)
	
	,
	
	/** Set current material as color material */
	fn showColor = (
		if color_material_id == undefined then (
			messageBox "No color material found! Use create firts" title:"Error"			
		)
		else (
			asteroid.material = currentMaterialLibrary[color_material_id]
		)
	)
	
	,
	
	/** Set current material as viewport material */
	fn showViewport = (
		if viewport_material_id == undefined then (
			messageBox "No viewport material found! Use create firts" title:"Error"			
		)
		else (
			asteroid.material = currentMaterialLibrary[viewport_material_id]
		)
	)
	
	,
	
	fn updateAsteroid size = 
	(		
		print "updateAsteroid"
		
		if color_material_id == undefined or viewport_material_id == undefined or ogre_material_id == undefined then (
			messageBox "Not all required materials found! Use create firts" title:"Error"			
		)
		else (		
			
			index = -1
			
			for i = 1 to vertexpaintlayers do (	
				/** current filename **/
				filename =  (colormapoutputpath + "\\" + asteroid.name + (i as string) + "_alpha.png") 
				
				/** disable all but the current vertex color layer */
				for j = 1 to vertexpaintlayers do (
					index = vertexpaintidices[j]
					asteroid.modifiers[index].enabled = false
				)
				
				index = vertexpaintidices[i]
				asteroid.modifiers[index].enabled = true		

				/** create the alpha map **/
				createAlphaMap size filename
			)
			
			updateTextures()
		)
	)

)


rollout AsteroidTool "Asteroid Tool" width:184 height:464
(
	
	GroupBox grp2 "Settings" pos:[8,8] width:168 height:200
	edittext size "size" pos:[24,32] width:136 height:16 text:"512"
	GroupBox Actions "GroupBox" pos:[8,216] width:168 height:136
	button update_asteroid "Update" pos:[16,240] width:72 height:24
	button cpyogre "Copy Ogre" pos:[16,272] width:72 height:24
	spinner OgreMat "" pos:[96,272] width:72 height:16 range:[1,24,1] type:#integer
	button cpydx "Copy DX" pos:[16,312] width:72 height:24
	spinner dxmat "" pos:[96,312] width:72 height:16 range:[1,24,1] type:#integer
	GroupBox grp3 "Visual" pos:[8,360] width:168 height:80
	button color_asteroid "Show Color" pos:[24,376] width:64 height:24
	button showOgre "Show Ogre" pos:[96,376] width:64 height:24
	button showDX "Show DX" pos:[96,408] width:64 height:24
	mapButton layer1 "Layer1" pos:[16,88] width:72 height:32	
	mapButton layer2 "Layer2" pos:[96,88] width:72 height:32
	mapButton layer3 "Layer3" pos:[16,128] width:72 height:32
	mapButton layer4 "Layer4" pos:[96,128] width:72 height:32
	mapButton layer5 "Layer5" pos:[16,168] width:72 height:32	
	mapButton layer6 "Layer6" pos:[96,168] width:72 height:32
	button clean "Clean" pos:[96,240] width:72 height:24
	editText output "output" pos:[24,56] width:136 height:16 text:colormapoutputpath
	
	on update_asteroid pressed do
	(	
		sel = getCurrentSelection()
		if sel.count != 1 then (
			messageBox "Select one object" title:"Error"
			return false
		)
		
		sizenr = size.text as number
		if sizenr != 128 and sizenr != 256 and  sizenr != 512 and sizenr != 1024 and sizenr != 2048 then (
			messageBox "Size should be 128, 256, 512, 1024 or 2048" title:"Error"
			return false
		) 
		
		asteroid = AsteroidInfo()
		asteroid.create (sel[1])			
		asteroid.updateAsteroid (sizenr)
	)
	on cpyogre pressed do
	(
		sel = getCurrentSelection()		
		if sel.count != 1 then (
			messageBox "Select one object" title:"Error"	
			return false
		)
		
		asteroid = AsteroidInfo()
		asteroid.create (sel[1])	
		
		mat = asteroid.getOgreMaterial()
		if mat != undefined then (
			setMeditMaterial OgreMat.value mat
		)
		else (
			messageBox "No material found, use create firts!" title:"Error"
		)
		
	)
	on cpydx pressed do
	(
		sel = getCurrentSelection()	
		if sel.count != 1 then (
			messageBox "Select one object" title:"Error"	
			return false
		)
		
		asteroid = AsteroidInfo()
		asteroid.create (sel[1])	
		
		mat = asteroid.getDxMaterial()
		if mat != undefined then (
			setMeditMaterial DXMat.value mat
		)
		else (
			messageBox "No material found, use create firts!" title:"Error"
		)
	)
	on color_asteroid pressed do
	(
		sel = getCurrentSelection()		
		if sel.count != 1 then (
			messageBox "Select one object" title:"Error"	
			return false
		)
		
		asteroid = AsteroidInfo()
		asteroid.create (sel[1])		
		asteroid.showColor()
	)
	on showOgre pressed do
	(
		sel = getCurrentSelection()		
		if sel.count != 1 then (
			messageBox "Select one object" title:"Error"	
			return false
		)		
		
		asteroid = AsteroidInfo()
		asteroid.create (sel[1])	
		asteroid.showOgre()
	)
	on showDX pressed do
	(
		sel = getCurrentSelection()		
		if sel.count != 1 then (
			messageBox "Select one object" title:"Error"	
			return false
		)
		
		asteroid = AsteroidInfo()
		asteroid.create (sel[1])	
		asteroid.showViewport()
	)
	on layer1 picked texmap do
	(
		sel = getCurrentSelection()		
		if sel.count != 1 then (
			messageBox "Select one object" title:"Error"	
			return false
		)		
		
		asteroid = AsteroidInfo()
		asteroid.create (sel[1])	
		asteroid.setTerrainLayer 1 texmap.fileName
		layer1.text=texmap.fileName
	)
	on layer2 picked texmap do
	(
		sel = getCurrentSelection()		
		if sel.count != 1 then (
			messageBox "Select one object" title:"Error"	
			return false
		)		
		
		asteroid = AsteroidInfo()
		asteroid.create (sel[1])	
		asteroid.setTerrainLayer 2 texmap.fileName
		layer2.text=texmap.fileName
	)
	on layer3 picked texmap do
	(
		sel = getCurrentSelection()		
		if sel.count != 1 then (
			messageBox "Select one object" title:"Error"	
			return false
		)		
		
		asteroid = AsteroidInfo()
		asteroid.create (sel[1])	
		asteroid.setTerrainLayer 3 texmap.fileName
		layer3.text=texmap.fileName
	)
	on layer4 picked texmap do
	(
		sel = getCurrentSelection()		
		if sel.count != 1 then (
			messageBox "Select one object" title:"Error"	
			return false
		)		
		
		asteroid = AsteroidInfo()
		asteroid.create (sel[1])	
		asteroid.setTerrainLayer 4 texmap.fileName
		layer4.text=texmap.fileName
	)
	on layer5 picked texmap do
	(
		sel = getCurrentSelection()		
		if sel.count != 1 then (
			messageBox "Select one object" title:"Error"	
			return false
		)		
		
		asteroid = AsteroidInfo()
		asteroid.create (sel[1])	
		asteroid.setTerrainLayer 5 texmap.fileName
		layer5.text=texmap.fileName
	)
	on layer6 picked texmap do
	(
		sel = getCurrentSelection()		
		if sel.count != 1 then (
			messageBox "Select one object" title:"Error"	
			return false
		)		
		
		asteroid = AsteroidInfo()
		asteroid.create (sel[1])	
		asteroid.setTerrainLayer 6 texmap.fileName
		layer6.text=texmap.fileName
	)
	on clean pressed do
	(
		sel = getCurrentSelection()		
		if sel.count != 1 then (
			messageBox "Select one object" title:"Error"	
			return false
		)		
		
		asteroid = AsteroidInfo()
		asteroid.create (sel[1])	
		asteroid.clean()
	)
	on output entered text do
	(
		colormapoutputpath = text
	)
)

	on Execute do 
	(
		rof=newrolloutfloater "Asteroid Tools" 200 500
		addrollout AsteroidTool rof
	)

)


