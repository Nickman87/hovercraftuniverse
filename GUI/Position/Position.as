package {
	import flash.display.*;
	import flash.events.*;
	import flash.utils.Timer;
	import flash.external.ExternalInterface;
	
	// Class Definitie
	public class Position extends MovieClip{
		var decimal1:MovieClip;
		var unit1:MovieClip;
		
		var slash:MovieClip;
		
		var decimal2:MovieClip;
		var unit2:MovieClip;
		
		var position:int;
		var total:int;
		
		var CHAR_WIDTH:Number = 60;
		var CHAR_HEIGHT:Number = 80;
		
		var X_OFFSET:Number = 20 + (CHAR_WIDTH / 2.0);
		var Y_OFFSET:Number = 20 + (CHAR_HEIGHT / 2.0);
		
		
		// Constructor
		public function Position():void{
			position = 0;
			total = 0;
			
			slash = new Slash();

			addChild(slash);
			
			ExternalInterface.addCallback("setTotal", setTotal);
			ExternalInterface.addCallback("setPosition", setPosition);
			recalculate();
			//iterate(null);
		}
		
		public function setTotal(val:int) {
			total = val;
			recalculate();
		}
		
		public function setPosition(val:int) {
			position = val;
			recalculate();
		}
		
		public function iterate(evt:Event) {
			position += 1;
			total += 1;
			
			if (position > 20) {
				position = 0;
			}
			
			if (total > 20) {
				total = 0;
			}
			
			recalculate();
		}
		
		function recalculate():void {
			visualizePosition();
			visualizeTotal();
			
			setOffsets();
		}
		
		function visualizePosition():void {
			//Remove the old ones
			if ( (decimal1 != null) && (this.contains(decimal1)) ) {
				removeChild(decimal1);
			}
			if ( (unit1 != null) && (this.contains(unit1)) ) {
				removeChild(unit1);
			}
			
			//Create the new ones
			decimal1 = getDigitClip(getDigit(position, 10));
			unit1 = getDigitClip(getDigit(position, 1));
			
			//Add the new ones
			addChild(decimal1);
			addChild(unit1);
		}
		
		function visualizeTotal():void {
			//Remove the old ones
			if ( (decimal2 != null) && (this.contains(decimal2)) ) {
				removeChild(decimal2);
			}
			if ( (unit2 != null) && (this.contains(unit2)) ) {
				removeChild(unit2);
			}
			
			//Create the new ones
			decimal2 = getDigitClip(getDigit(total, 10));
			unit2 = getDigitClip(getDigit(total, 1));
			
			//Add the new ones
			addChild(decimal2);
			addChild(unit2);
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
			if (getDigit(position, 10) == 0) {
				decimal1.visible = false;
			} else {
				decimal1.x = X_OFFSET;
				decimal1.y = Y_OFFSET;
			}
			
			unit1.x = X_OFFSET + CHAR_WIDTH;
			unit1.y = Y_OFFSET;
			
			slash.x = X_OFFSET + 2 * CHAR_WIDTH;
			slash.y = Y_OFFSET;
			
			if (getDigit(total, 10) == 0) {
				decimal2.visible = false;
				
				unit2.x = X_OFFSET + 3 * CHAR_WIDTH;
				unit2.y = Y_OFFSET;
			} else {
				decimal2.x = X_OFFSET + 3 * CHAR_WIDTH;
				decimal2.y = Y_OFFSET;
				
				unit2.x = X_OFFSET + 4 * CHAR_WIDTH;
				unit2.y = Y_OFFSET;
			}
		}
		
	}
}