package {
	import flash.display.*;
	import flash.events.*;
	import flash.utils.Timer;
	import flash.external.ExternalInterface;
	import flash.text.*;
	import flash.filters.*;
	
	// Class Definitie
	public class raceresults extends MovieClip {
		private var mTimeLeft:Number;
		private var mLastTime:Number;
		private var tmr:Timer = new Timer(1000.0 / 24.0);
		
		private var mCurrentVisual:MovieClip;
		private var mSecondDigit:MovieClip;
		
		var textFormat:TextFormat;

		// Constructor
		public function raceresults():void {
			mCurrentVisual = null;
			
			ExternalInterface.addCallback("addPosition", addPosition);
			textFormat = new TextFormat();
			textFormat.letterSpacing = 5;
			
			hideAll();
			//startCountdown(20000);
		}
		
		public function addPosition(pos:int, username:String, time:Number):void {
			var timeString:String = buildTime(time);
			if (pos == 1) {
				showPos1(username, timeString);
			} else if (pos == 2) {
				showPos2(username, timeString);
			} else if (pos == 3) {
				showPos3(username, timeString);
			} else if (pos == 4) {
				showPos4(username, timeString);
			} else if (pos == 5) {
				showPos5(username, timeString);
			}
		}
		
		public function buildTime(time:Number):String {
			var minutes:int = time / (1000 * 60);
			var seconds:int = (time - minutes * (1000 * 60)) / 1000;
			var hundredseconds:int = (time - seconds * 1000 - minutes * (60 * 1000)) / 10;
			
			var ret:String = String(minutes) + ":";
			if (seconds < 10) {
				ret += "0";
			}
			ret += String(seconds) + ":";
			if (hundredseconds < 10) {
				ret += "0";
			}
			ret += String(hundredseconds);
			
			trace(String(minutes));
			trace(String(seconds));
			trace(String(hundredseconds));
			
			return ret;
		}
		
		public function hideAll() {
			Pos1.visible = false;
			Pos1_Name.visible = false;
			Pos1_Time.visible = false;
			Pos2.visible = false;
			Pos2_Name.visible = false;
			Pos2_Time.visible = false;
			Pos3.visible = false;
			Pos3_Name.visible = false;
			Pos3_Time.visible = false;
			Pos4.visible = false;
			Pos4_Name.visible = false;
			Pos4_Time.visible = false;
			Pos5.visible = false;
			Pos5_Name.visible = false;
			Pos5_Time.visible = false;
		}
		
		public function showPos1(username:String, time:String) {
			Pos1_Name.text = username;
			Pos1_Time.text = time;
			Pos1.visible = true;
			Pos1_Name.visible = true;
			Pos1_Time.visible = true;
			Pos1_Name.setTextFormat(textFormat);
			Pos1_Time.setTextFormat(textFormat);
		}
		
		public function showPos2(username:String, time:String) {
			Pos2_Name.text = username;
			Pos2_Time.text = time;
			Pos2.visible = true;
			Pos2_Name.visible = true;
			Pos2_Time.visible = true;
			Pos2_Name.setTextFormat(textFormat);
			Pos2_Time.setTextFormat(textFormat);
		}
		
		public function showPos3(username:String, time:String) {
			Pos3_Name.text = username;
			Pos3_Time.text = time;
			Pos3.visible = true;
			Pos3_Name.visible = true;
			Pos3_Time.visible = true;
			Pos3_Name.setTextFormat(textFormat);
			Pos3_Time.setTextFormat(textFormat);
		}
		
		public function showPos4(username:String, time:String) {
			Pos4_Name.text = username;
			Pos4_Time.text = time;
			Pos4.visible = true;
			Pos4_Name.visible = true;
			Pos4_Time.visible = true;
			Pos4_Name.setTextFormat(textFormat);
			Pos4_Time.setTextFormat(textFormat);
		}
		
		public function showPos5(username:String, time:String) {
			Pos5_Name.text = username;
			Pos5_Time.text = time;
			Pos5.visible = true;
			Pos5_Name.visible = true;
			Pos5_Time.visible = true;
			Pos5_Name.setTextFormat(textFormat);
			Pos5_Time.setTextFormat(textFormat);
		}
	}
}
