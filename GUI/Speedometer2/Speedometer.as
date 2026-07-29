package {
	import flash.display.*;
	import flash.events.*;
	import flash.utils.Timer;
	import flash.external.ExternalInterface;
	
	// Class Definitie
	public class Speedometer extends MovieClip{
		//Variabels
		var schermContainer:MovieClip = new MovieClip;
		static public var MAX_SPEED:Number = 100;
		static public var MIN_SPEED:Number = 0;
		
		static public var MAX_ANGLE:Number = 61;
		static public var MIN_ANGLE:Number = -77.2;
		
		static public var MAX_ROTATION:Number = MAX_ANGLE - MIN_ANGLE;
		
		private var pointer:MovieClip;
		private var zero_angle:Number = -77.2;
		private var currSpeed:Number = 0;
		
		private var mToSpeed:Number;
		private var mStep:Number;
		
		private var mSpeedTimer:Timer;
		
		private var iter:Timer;
		
		
		// Constructor
		public function Speedometer():void{
			addChild(schermContainer);												// Container voor schermrand
			pointer = moveable_pointer;
			zero_angle = pointer.rotation;
			
			mSpeedTimer = new Timer(250);
			mSpeedTimer.addEventListener("timer", speedIteration);
			/*
			iter = new Timer(30);
			iter.addEventListener("timer", iterate);
			iter.start();
			*/
			ExternalInterface.addCallback("setSpeed", setSpeed);
			ExternalInterface.addCallback("setBoost", setBoost);
			
			setSpeed(0);
			setBoost(0);
			
			//////////////////////
			//	BoostMeter
			//////////////////////
			boostFill.mask = boostMask;
			boostReflexion.mask = reflexionMask;
		}
		
		public function iterate(evt:Event) {
			currSpeed += 1;
			
			if (currSpeed > MAX_SPEED) {
				currSpeed = 0;
			}
			
			setSpeed(currSpeed);
			setBoost(100 - currSpeed);
		}
		
		function setBoost(boost:Number) {
			if (boost < 0) {
				boost = 0;
			}
			
			if (boost > 100) {
				boost = 100;
			}
			
			var rot:Number = (boost / 100.0) * 101;
			
			boostMask.rotation = 1 + rot;
			reflexionMask.rotation = 1 + rot;
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
			var currRotation:Number = part*MAX_ROTATION;
			
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