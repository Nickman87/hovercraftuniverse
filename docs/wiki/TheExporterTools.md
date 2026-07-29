---
title: TheExporterTools
---

#Info about the exporter tools

# Introduction

This page will contain most info about the exporter tools.
Later this page might be slit over many more pages but than this page can be considered the starting point.

# Details

### The terrain shader Tool

The shader tool is a tool that allows an artist to paint the texture of the asteroids.
It is based on the vertex paint tools in combination with alpha splatting.

Info about the GUI:

check: Checks if the selected object has a valid state (modifier stack etc) so that the tool can work on it.

create: should be done at least once for the asteroid, it will create the materials in the library, it doesn't do anything but this. This will only work if an object is selected.
(Will be removed later on)

update: will update the the actual alpha maps of the model. This should be called when the artist wants an update of the model in the viewport and in ogre!

copy ogre: copies the ogre material to given slot in the material editor

copy dx:  copies the dx material to given slot in the material editor

show color: shows vertex color material in the viewport

show ogre: shows ogre material (this will probably be just black)

show dx: shows direct x shader, this is what most artists want since it gives a view of how it will look in the engine

Usage:

  - Use a poly mesh.
  - Aply one or two vertex paint modifiers (one if 3 textures are fine, two if you need 6)
  - Paint the mesh, mind that red, green and blue are the main colors for the textures
  - Open the tool
  - Press create once while the object is selected
  - Copy the directx map to a material slot in the material editor
  - Fill in the required maps
  - Press update
  - Do some painting and updating till happy

