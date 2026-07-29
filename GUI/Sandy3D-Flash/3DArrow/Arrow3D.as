package
{
   import flash.display.Sprite; 
   import flash.events.*;
   import flash.ui.*;
   import sandy.core.Scene3D;
   import sandy.core.data.*;
   import sandy.core.scenegraph.*;
   import sandy.materials.*;
   import sandy.materials.attributes.*;
   import sandy.primitive.*;
   import flash.external.ExternalInterface;
   
   import flash.display.StageAlign;
   import flash.display.StageScaleMode;

   public class Arrow3D extends Sprite {
	   
      private var scene:Scene3D;
	  private var camera:Camera3D;
	  private var tg:TransformGroup;
	  private var myCone:Cone;
	  private var myHedra:Hedra;
	  
	  private var mBase:Box;
	  private var mTop:Cylinder;
	  
	  private var currAngle:Number;

      public function Arrow3D ()
      { 
		 // We create the camera
		 camera = new Camera3D( 150, 100 );
		 camera.x = 0;
		 camera.y = -100;
		 camera.z = -230
		 camera.lookAt(0,0,0);
		 
		 currAngle = 0;
		 
		 //stage.scaleMode = StageScaleMode.NO_SCALE;
		 stage.align = StageAlign.TOP_LEFT;
		 
		 // We create the "group" that is the tree of all the visible objects
         var root:Group = createScene();
		 
		 // We create a Scene and we add the camera and the objects tree 
	     scene = new Scene3D( "scene", this, camera, root );
		 
		 // Listen to the heart beat and render the scene
		 addEventListener( Event.ENTER_FRAME, enterFrameHandler );
		 //stage.addEventListener(KeyboardEvent.KEY_DOWN, keyPressed);
		 stage.addEventListener (Event.RESIZE, onResize);
		 onResize (null);
		 
		 scene.light.setDirection(0.0, 100.0, 100.0);
		 
		 ExternalInterface.addCallback("setAngle", setAngle);
      }
	  
	  private function onResize (e:Event):void {
		// get new stage dimensions
		var w:Number = stage.stageWidth;
		var h:Number = stage.stageHeight;
		// update the viewport
		scene.camera.viewport.width = w;
		scene.camera.viewport.height = h;
		// get plane distance to camera
		//var d:Number = plane.getPosition("camera").getNorm ();
		// focus camera on wz = d for 1:1 scale
		//scene.camera.fov = 2 * Math.atan2 (h / 2, d) * (180 / Math.PI);
		// re-render the scene
		scene.render();
	}

      // Create the scene graph based on the root Group of the scene
	private function createScene():Group {
		// Create the root Group
		var g:Group = new Group();
		
		// first we will create a simple coordinate system
		var myXLine:Line3D = new Line3D( "x-coord", new Point3D(-50, 0, 0), new Point3D( 50, 0, 0 ));
		var myYLine:Line3D = new Line3D( "y-coord", new Point3D(0, -50, 0), new Point3D( 0, 50, 0 ));
		var myZLine:Line3D = new Line3D( "z-coord", new Point3D(0, 0, -50), new Point3D( 0, 0, 50 ));
		
		// We need to create a transformGroup
		tg = new TransformGroup('myGroup');
		
		var baseHeight:Number = 70;
		var topHeight:Number = 60;
		var thickness:Number = 25;
		
		mBase = new Box('Base', 40, baseHeight, thickness);
		mTop = new Cylinder('Top', topHeight, thickness, 3, 1, topHeight);
		mTop.rotateAxis(1.0, 0.0, 0.0, 90);
		mTop.rotateAxis(0.0, 0.0, 1.0, 90);
		
		mTop.y = (baseHeight / 2) + (topHeight / 2);
		
		var materialAttr:MaterialAttributes = new MaterialAttributes( 
				new LightAttributes( true, 0.1),
				new OutlineAttributes( 2, 0x000000, 1.0 ) );
		
		var material:Material = new ColorMaterial( 0x3478F7, 1.0, materialAttr );
		material.lightingEnable = true;
		var app:Appearance = new Appearance( material );
		
		mBase.appearance = app;
		mTop.appearance = app;
		
		//mBase.enableBackFaceCulling = false;
		//mTop.enableBackFaceCulling = false;
		
		tg.addChild(mBase);
		tg.addChild(mTop);
		
		tg.rotateAxis(1.0, 0.0, 0.0, 100);
		//tg.rotateAxis(0.0, 1.0, 0.0, 45);
		
		// we now add all the object to the root group:
		g.addChild(tg);
		
		return g;
	}
	  
	public function setAngle(angle:Number):void {
		moveAngle(angle);
	}
	  
	public function moveAngle(angle:Number):void {
		//First undo the rotation
		tg.rotateAxis(0.0, 1.0, 0.0, -currAngle);
		
		//Go to the new angle
		currAngle = angle;
		tg.rotateAxis(0.0, 1.0, 0.0, currAngle);
	}

	// The Event.ENTER_FRAME event handler tells the world to render
	private function enterFrameHandler( event : Event ) : void {
		//setAngle(currAngle + 1);
		scene.render();
	}
}
}
