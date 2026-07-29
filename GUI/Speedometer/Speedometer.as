package {
	import flash.display.*;
	import flash.events.*;
	import flash.utils.Timer;
	import flash.external.ExternalInterface;
	
	// Class Definitie
	public class Speedometer extends MovieClip{
		//Variabels
		var schermContainer:MovieClip = new MovieClip;
		static public var MAX_SPEED:Number = 110;
		static public var MIN_SPEED:Number = 0;
		
		static public var MAX_ANGLE:Number = -87;
		static public var MIN_ANGLE:Number = -33;
		
		static public var MAX_ROTATION:Number = 306;
		
		private var pointer:MovieClip;
		private var zero_angle:Number = -33;
		private var currSpeed:Number = 0;
		
		private var mToSpeed:Number;
		private var mStep:Number;
		
		private var mSpeedTimer:Timer;
		
		
		// Constructor
		public function Speedometer():void{
			addChild(schermContainer);												// Container voor schermrand
			pointer = moveable_pointer;
			zero_angle = pointer.rotation;
			
			mSpeedTimer = new Timer(250);
			mSpeedTimer.addEventListener("timer", speedIteration);
			
			ExternalInterface.addCallback("setSpeed", setSpeed);
		}
		
		public function setToRotation(rot:Number) {
			pointer.rotation = MIN_ANGLE + rot;
		}
		
		function setSpeed(speed:Number) {
			if (speed > MAX_SPEED) {
				speed = MAX_SPEED;
			}
			if (speed < MIN_SPEED) {
				speed = MIN_SPEED;
			}
			
			currSpeed = speed;
			
			var part:Number = speed/MAX_SPEED;
			var currRotation:Number = part*306;
			
			if (currRotation > MAX_ROTATION) {
				currRotation = MAX_ROTATION;
			}
			
			if (currRotation < 0) {
				currRotation = 0;
			}
			
			setToRotation(currRotation);
		}
		
		function moveToSpeed(speed:Number, step:Number) {
			mToSpeed = speed;
			mStep = step;
			
			if (!mSpeedTimer.running) {
				mSpeedTimer.start();
			}
		}
		
		function speedIteration(eventArgs:TimerEvent) {
			var speed:Number;
			
			if (currSpeed < mToSpeed) {
				speed = currSpeed + mStep;
				
				if (speed > mToSpeed) {
					speed = mToSpeed;
				}
				
				setSpeed(speed);
			} else if (currSpeed > mToSpeed) {
				speed = currSpeed - mStep;
				
				if (speed < mToSpeed) {
					speed = mToSpeed;
				}
				
				setSpeed(speed);
			} else {
				mSpeedTimer.stop();
			}
		}
	}
}