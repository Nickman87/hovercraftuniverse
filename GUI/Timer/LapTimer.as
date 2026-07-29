package {
	import flash.display.*;
	import flash.events.*;
	import flash.utils.Timer;
	import flash.external.ExternalInterface;
	
	// Class Definitie
	public class LapTimer extends MovieClip{
		var decimal1:MovieClip;
		var unit1:MovieClip;
			var sep1:MovieClip;
		var decimal2:MovieClip;
		var unit2:MovieClip;
			var sep2:MovieClip;
		var decimal3:MovieClip;
		var unit3:MovieClip;
		
		var minutes:int;
		var seconds:int;
		var hundredseconds:int;
		
		var startTime:Number;
		
		var CHAR_WIDTH:Number = 60;
		var CHAR_HEIGHT:Number = 80;
		
		var X_OFFSET:Number = 20 + (CHAR_WIDTH / 2.0);
		var Y_OFFSET:Number = 20 + (CHAR_HEIGHT / 2.0);
		
		private var iter:Timer;
		
		
		// Constructor
		public function LapTimer():void{
			minutes = 0;
			seconds = 0;
			hundredseconds = 0;
			
			sep1 = new Colon();
			sep2 = new Colon();

			addChild(sep1);
			addChild(sep2);
			
			ExternalInterface.addCallback("start", startRunning);
			ExternalInterface.addCallback("stop", stopRunning);
			ExternalInterface.addCallback("setTime", setTime);
			
			iterate(null);
		}
		
		public function startRunning():void {
			var currTime:Date = new Date();
			startTime = currTime.time;
			//Start iterating
			iter = new Timer(10);
			iter.addEventListener("timer", iterate);
			iter.start();
		}
		
		public function stopRunning():void {
			iter.stop();
		}
		
		public function setTime(min:int, sec:int, hundred:int):void {
			minutes = min;
			seconds = sec;
			hundredseconds = hundred;
			
			repaint();
		}
		
		public function iterate(evt:Event) {
			//Determine values
			var currTime:Number = new Date().time;
			var diff = currTime - startTime;
			
			minutes = diff / (1000 * 60);
			seconds = (diff - minutes * (1000 * 60)) / 1000;
			hundredseconds = (diff - seconds * 1000 - minutes * (60 * 1000)) / 10;
			
			repaint();
		}
		
		function repaint():void {
			//Remove all the digits
			makeClean();
			
			//Calculate the new visualisations
			decimal1 = getDigitClip(getDigit(minutes, 10));
			unit1 = getDigitClip(getDigit(minutes, 1));
			decimal2 = getDigitClip(getDigit(seconds, 10));
			unit2 = getDigitClip(getDigit(seconds, 1));
			decimal3 = getDigitClip(getDigit(hundredseconds, 10));
			unit3 = getDigitClip(getDigit(hundredseconds, 1));
			
			//Re-insert the digits
			insertNew();
			
			setOffsets();
		}
		
		function makeClean():void {
			if ( (decimal1 != null) && (this.contains(decimal1)) ) {
				removeChild(decimal1);
			}
			if ( (unit1 != null) && (this.contains(unit1)) ) {
				removeChild(unit1);
			}
			if ( (decimal2 != null) && (this.contains(decimal2)) ) {
				removeChild(decimal2);
			}
			if ( (unit2 != null) && (this.contains(unit2)) ) {
				removeChild(unit2);
			}
			if ( (decimal3 != null) && (this.contains(decimal3)) ) {
				removeChild(decimal3);
			}
			if ( (unit3 != null) && (this.contains(unit3)) ) {
				removeChild(unit3);
			}
		}
		
		function insertNew():void {
			addChild(decimal1);
			addChild(unit1);
			addChild(decimal2);
			addChild(unit2);
			addChild(decimal3);
			addChild(unit3);
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
		
		function setOffsets():void {
			if (getDigit(minutes, 10) == 0) {
				decimal1.visible = false;
			} else {
				decimal1.x = X_OFFSET;
				decimal1.y = Y_OFFSET;
			}
			
			unit1.x = X_OFFSET + CHAR_WIDTH;
			unit1.y = Y_OFFSET;
			
			sep1.x = X_OFFSET + 2 * CHAR_WIDTH;
			sep1.y = Y_OFFSET;
			
			decimal2.x = X_OFFSET + 3 * CHAR_WIDTH;
			decimal2.y = Y_OFFSET;
			unit2.x = X_OFFSET + 4 * CHAR_WIDTH;
			unit2.y = Y_OFFSET;
			
			sep2.x = X_OFFSET + 5 * CHAR_WIDTH;
			sep2.y = Y_OFFSET;
			
			decimal3.x = X_OFFSET + 6 * CHAR_WIDTH;
			decimal3.y = Y_OFFSET;
			unit3.x = X_OFFSET + 7 * CHAR_WIDTH;
			unit3.y = Y_OFFSET;
		}
		
	}
}