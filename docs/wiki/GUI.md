---
title: GUI
---

# Content
  1. Libraries
  1. Usage
  1. Components

# Libraries
  - Hikari
  - Awesomium - Navi

## Hikari
A Flash embedding layer for Ogre3D. Just create flash interface components, and load them in the program. Communication between C++ and Actionscript is provided if needed.<br />
[Homepage](http://code.google.com/p/hikari-library/)

## Awesomium - Navi
Awesomium is a wrapper for google chrome. It can display web pages inside any application.
NaviLibrary is a library specifically for Ogre3D that allows developers to create and interact with web-content as an overlay or material in a 3D scene. It is powered by and depends upon the Awesomium library.<br />
[Awesomium](http://www.khrona.com/products/awesomium/) - [Navi](http://princeofcode.com/navilibrary.php#download) (old version, update should come end of february)

# Usage
Mostly Hikari will be used for visual interface components, when Navi gets updated to the newest Awesomium, HTML pages in combination with Javascript can be used to create more simple, content oriented interface objects.

# Components
  - HUD
  - Menu

## HUD
The interface that is displayed during the game itself ([visual guide](http://uhasseltaacgua.googlecode.com/files/hudoverview.zip).
It will be possible to make changes to the layout of the HUD by changing the HUD configuration file. The standard config file is as follows (for now):
```
<?xml version="1.0" standalone=no>
<!-- 
	Possible relative positions:
		Left, TopLeft, TopCenter, TopRight, Right,
		BottomRight, BottomCenter, BottomLeft, Center
	For absolute positioning:
		left and top distance is needed, can be percentage
		
	Percentages used in width/height will only be calculated relatively
	in respect to other objects when using relative positioning.
	ex.:
		Chat window -> BottomLeft - width 100%
		Speedometer -> BottomRight - width 120
		==> Chat windo width = screenWidth - 120 - Padding
-->
<HUD mediaPath="..\..\Media\GUI">
	<element type="speedometer" file="speedometer.swf">
		<size>
			<min width="40" height="40" />
			<max width="120" height="120" />
		</size>
		<position>
			<relative val="BottomRight" />
		</position>
	</element>
	
	<element type="position" file="position.swf">
		<size>
			<min width="114" height="40" />
			<max width="340" height="120" />
		</size>
		<position>
			<relative val="TopLeft" />
		</position>
	</element>
	
<!--
	This might require some extra attention
-->
<!--
	<element type="positionBar" file="positionBar.swf">
		<size>
			<min width="114" height="40" />
			<max width="340" height="120" />
		</size>
		<position>
			<relative val="TopLeft" />
		</position>
	</element>
-->	

	<element type="timer" file="timer.swf">
		<size>
			<min width="114" height="40" />
			<max width="340" height="120" />
		</size>
		<position>
			<relative val="TopRight" />
		</position>
	</element>
	
	<element type="direction" file="direction.swf">
		<size>
			<min width="40" height="40" />
			<max width="120" height="120" />
		</size>
		<position>
			<relative val="TopCenter" />
		</position>
	</element>
	
	<element type="chat" file="chat.swf">
		<size>
			<min width="100%" height="80" />
			<max width="100%" height="80" />
		</size>
		<position>
			<relative val="BottomLeft" />
		</position>
	</element>
	
	<element type="lapTime" file="lapTime.swf">
		<size>
			<min width="120" height="40" />
			<max width="300" height="100" />
		</size>
		<position>
			<relative val="Center" />
		</position>
	</element>
</HUD>
```

## Menu
The menu's that will be displayed at the beginning of the game. Here you will be able to choose different actions like:
  - Single Player game
  - Multiplayer game
  - Options
  - Exit

The final menu design has yet to be decided, as will the content of the menu(s).
