package {
	import flash.display.*;
	import flash.events.*;
	import flash.utils.Timer;
	import flash.external.ExternalInterface;
	import flash.text.*;
	import flash.display.StageAlign;
	import flash.events.TextEvent;
	
	// Class Definitie
	public class Chatbox extends MovieClip {
		private var	mChatDisplay:Array = new Array();
		private var mCaretVisible:Boolean = false;
		private var mCaretTmr:Timer = new Timer(350);
		private var textFilter:GlowFilter;
		private var mFocusTmr:Timer = new Timer(mAnimSteps);
		
		//Font
		[Embed(systemFont="Gill Sans MT", fontName="Chat", mimeType="application/x-font")]
		var mChatFont:Class;
		
		public function Chatbox():void{
			stage.align = StageAlign.TOP_LEFT;
			stage.scaleMode = StageScaleMode.NO_SCALE;
			
			Font.registerFont(mChatFont);
			
			//Fix font sizes
			mChatTextFormat = new TextFormat();
			mChatTextFormat.size = 14;
			mChatTextFormat.color = 0x000000;
			mChatTextFormat.font = "Chat";
			mChatTextFormat.leading = 2;
			mChatTextFormat.bold = true;
			
			
			//Fix display array
			mChatDisplay = new Array(6);
			
			//Set test data
			mChatDisplay[0] = "<b><font color=\"#FFFF00\">KeejOow LOST the race!</font></b>";
			mChatDisplay[1] = "<b>Nickman: </b>Kristof, zijt ge nu weeral verloren... NOOB!";
			mChatDisplay[2] = "<b>PJ: </b>Hij kon de physics ni aan he...";
			mChatDisplay[3] = "<b>Tiger: </b>Ik wist niet dat we begonnen waren?";
			mChatDisplay[4] = "<b>EagleEye: </b>Zelfs mijn scripts hebben KeejOow verslagen!";
			mChatDisplay[5] = "<b>Norfolk: </b>Ik heb wel veel lag, hoe komt dat 0_o?";
			
			stage.addEventListener(Event.RESIZE, rePaint);
		 	rePaint(null);
			
			mRefresher = new Timer(400);
			mRefresher.addEventListener("timer", rePaint);
			mRefresher.start();
		}
	  
		private function rePaint (e:Event):void {
			//Remove all
			if (mChatField != null) {
				mBackGr.removeChild(mChatField);
			}
			if (mChatInput != null) {
				mChatInputBg.removeChild(mChatInput);
			}
			if (mChatInputBg != null) {
				mBackGr.removeChild(mChatInputBg);
			}
			if (mBackGr != null) {
				removeChild(mBackGr);
			}
			
			// get new stage dimensions
			//var screen_width:Number = stage.stageWidth;
			//var screen_height:Number = stage.stageHeight;
			var screen_width:Number = stage.stageWidth;
			var screen_height:Number = stage.stageHeight;
			
			var padding:Number = 15;
			
			////////////////////////////////
			// Background
			////////////////////////////////
			var backGr_width:Number = screen_width - (2 * padding);
			var backGr_height:Number = screen_height - (2 * padding);
			mBackGr = new Sprite();
			mBackGr.graphics.lineStyle(0, 0x000000, 0);
			mBackGr.graphics.beginFill(0x000000, 0.4);
			mBackGr.graphics.drawRoundRect(padding, padding, backGr_width, backGr_height, 15, 15);
			mBackGr.graphics.endFill();
			addChild(mBackGr);
			
			////////////////////////////////
			// Text
			////////////////////////////////
			var chatField_width:Number = backGr_width - 2 * padding;
			var chatField_height:Number = 120;
			mChatField = new TextField();
			mChatField.defaultTextFormat = mChatTextFormat;
			mChatField.embedFonts = true;
			mChatField.height = chatField_height;
			mChatField.width = chatField_width;
			mChatField.multiline = true;
			mChatField.selectable = false;
			mChatField.text = String(screen_width) + String(" x ") + String(screen_height);
			mChatField.appendText(String("\n") + String(mBackGr.x) + String(":") + String(mBackGr.y));
			mChatField.x = (screen_width / 2) - (chatField_width / 2);
			mChatField.y = padding * 1.5;
			mBackGr.addChild(mChatField);
			
			////////////////////////////////
			// Input Bg
			////////////////////////////////
			mChatInputBg = new Sprite();
			mChatInputBg.graphics.lineStyle(0, 0x000000, 0);
			mChatInputBg.graphics.beginFill(0x000000, 0.4);
			mChatInputBg.graphics.drawRoundRect(mChatField.x - 5,  mChatField.y + chatField_height + padding - 20, chatField_width + 10, 1.5 * padding, 10, 10);
			mChatInputBg.graphics.endFill();
			mBackGr.addChild(mChatInputBg);
			
			////////////////////////////////
			// Input
			////////////////////////////////
			mChatInput = new TextField();
			mChatInput.defaultTextFormat = mChatTextFormat;
			mChatInput.embedFonts = true;
			mChatInput.type = TextFieldType.INPUT;
			mChatInput.width = chatField_width;
			mChatInput.height = 1.5 * padding;
			mChatInput.border = false;
			mChatInput.x = mChatField.x;
			mChatInput.y = mChatField.y + chatField_height + padding - 20;
			mChatInput.addEventListener(TextEvent.TEXT_INPUT,textInputHandler);
			mChatInputBg.addChild(mChatInput);
			
			mChatInput.text = String(screen_width) + "x" + String(screen_height);
			
			updateDisplay();
		}
		
		private function updateDisplay():void {
			var chatContent:String = "";
			
			for (var i:Number = 0; i < 6; ++i){
				if (i != 0) {
					chatContent += "<br>";
				}
				chatContent += mChatDisplay[i];
			}
			
			mChatField.styleSheet = new StyleSheet();
			mChatField.text = chatContent;
			mChatField.wordWrap = true;
			//trace("htmlText: " + mChatField.htmlText);
		}
		
		private function textInputHandler(event:TextEvent):void {
           //mChatInput.text = event.text;
        }
	}
}