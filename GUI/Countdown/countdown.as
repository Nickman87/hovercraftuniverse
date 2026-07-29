package {
	import flash.display.*;
	import flash.events.*;
	import flash.utils.Timer;
	import flash.external.ExternalInterface;
	import flash.text.*;
	import flash.filters.*;
	
	// Class Definitie
	public class countdown extends MovieClip {
		private var mTimeLeft:Number;
		private var mLastTime:Number;
		private var tmr:Timer = new Timer(1000.0 / 24.0);
		
		private var mCurrentVisual:MovieClip;
		private var mSecondDigit:MovieClip;

		// Constructor
		public function countdown():void {
			tmr.addEventListener("timer", onFrame);
			mCurrentVisual = null;
			
			ExternalInterface.addCallback("start", startCountdown);
			ExternalInterface.addCallback("resync", startCountdown);
			
			//startCountdown(20000);
		}
		
		public function startCountdown(timeToGo:Number):void {
			mTimeLeft = timeToGo + 1000;
			mLastTime = new Date().time;
			tmr.start();
		}
		
		public function resync(timeToGo:Number):void {
			mTimeLeft = timeToGo + 1000;
			mLastTime = new Date().time;
		}
		
		public function onFrame(evt:Event) {
			var currTime:Number = new Date().time;
			var diff:Number = currTime - mLastTime;
			
			mLastTime = currTime;
			mTimeLeft -= diff;
			
			var seconds:int = mTimeLeft / 1000;
			//var milliSeconds:Number = diff - (seconds * 1000);
			//var partLeft:Number = milliSeconds / 1000.0;
			
			if ( (mCurrentVisual != null) && (this.contains(mCurrentVisual)) ) {
				removeChild(mCurrentVisual);
			}
			
			if ( (mSecondDigit != null) && (this.contains(mSecondDigit)) ) {
				removeChild(mSecondDigit);
			}
			
			if (seconds >= 10) {
				//Double digits
				trace(seconds);
				mCurrentVisual = getDigitClip(getDigit(seconds, 10));
				mSecondDigit = getDigitClip(getDigit(seconds, 1));
				mCurrentVisual.x = 60;
				mCurrentVisual.y = 50;
				mSecondDigit.x = 140;
				mSecondDigit.y = 50;
				addChild(mSecondDigit);
			} else if (seconds >= 1) {
				mCurrentVisual = getDigitClip(seconds);
				mCurrentVisual.x = 100;
				mCurrentVisual.y = 50;
			} else {
				//Show GO!
				mCurrentVisual = new GO();
				tmr.stop();
				mCurrentVisual.x = 100;
				mCurrentVisual.y = 50;
			}
			
			addChild(mCurrentVisual);
		}
		
		function getDigit(numb:Number, digit:int) {
			var res:int;
			
			if (numb < digit) {
				return 0;
			}
			
			for (var i:int=1; i <= digit; i = i * 10){
				res = numb % 10;
				numb = Math.floor(numb / 10);
			}
			
			return res;
		}
		
		function getDigitClip(val:int):MovieClip {
			trace("getting digit: "+String(val));
			switch (val) {
				case 0:
					return new Zero();
					break;
				case 1:
					return new One();
					break;
				case 2:
					return new Two();
					break;
				case 3:
					return new Three();
					break;
				case 4:
					return new Four();
					break;
				case 5:
					return new Five();
					break;
				case 6:
					return new Six();
					break;
				case 7:
					return new Seven();
					break;
				case 8:
					return new Eight();
					break;
				case 9:
					return new Nine();
					break;
				default:
					return new Dot();
			}
		}
	}
	
}
