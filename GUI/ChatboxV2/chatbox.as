package {
	import flash.display.*;
	import flash.events.*;
	import flash.utils.Timer;
	import flash.external.ExternalInterface;
	import fl.controls.ComboBox;
	import flash.text.*;
	import flash.filters.*;
	
	// Class Definitie
	public class chatbox extends MovieClip {
		//Variabels
		var myFont:TextFormat = new TextFormat();
		
		//GLOBAL
		private static var mAnimSpeed:Number = 150;
		private static var mAnimSteps:Number = 15;
		private static var mStep:Number = (100 / (mAnimSpeed / mAnimSteps)) / 100;
		
		//CHAT
		private var	mChatDisplay:Array = new Array();
		private var mCaretVisible:Boolean = false;
		private var mCaretTmr:Timer = new Timer(350);
		private var textFilter:GlowFilter;
		private var mFocusTmr:Timer = new Timer(mAnimSteps);
		
		//Font
		[Embed(systemFont="Gill Sans MT", fontName="mFont", mimeType="application/x-font")]
		var mFont:Class;
		
		public function chatbox():void{
			stage.align = StageAlign.TOP_LEFT;
			stage.scaleMode = StageScaleMode.NO_SCALE;
			
			Font.registerFont(mFont);
			
			//Setup chat
			textFilter = new GlowFilter(0x3483E1, 0.0, 7.0, 7.0, 4, BitmapFilterQuality.HIGH);
			
			mChatOutput.wordWrap = true;
			mChatOutput.styleSheet = new StyleSheet();
			
			mChatInput.addEventListener(MouseEvent.MOUSE_OVER, onFocus);
			mChatInput.addEventListener(MouseEvent.MOUSE_OUT, onDefocus);
			mChatInput.addEventListener(MouseEvent.MOUSE_UP, inputFocus); 
			mCaretTmr.addEventListener("timer", toggleCaret);
			
			//Set callbacks
			ExternalInterface.addCallback("addText", addText);
			ExternalInterface.addCallback("addAction", addAction);
			ExternalInterface.addCallback("giveFocus", giveFocus);
			ExternalInterface.addCallback("removeFocus", removeFocus);
			
			
			mChatInput.addEventListener(FocusEvent.FOCUS_IN, dispFocus);
			mChatInput.addEventListener(FocusEvent.FOCUS_OUT, dispDefocus);
		}
		
		public function dispFocus(evt:Event) {
			//addAction("OMG: got focus");
		}
		
		public function dispDefocus(evt:Event) {
			//addAction("OMG: lost focus");
		}
		
		public function giveFocus():void {
			//addAction("forcing focus");
			var empty:Boolean = (mChatInput.text == "");
			
			if (empty) {
				mChatInput.text=" "; //a SPACE or temp "foo" text, but NOT empty !
			}
			mChatInput.stage.focus = mChatInput;
			mChatInput.setSelection(mChatInput.length,mChatInput.length);
			if (empty) {
				mChatInput.text = "";
			}
			onFocus(null);
			inputFocus(null);
		}
		
		public function removeFocus():void {
			//addAction("forcing defocus");
			inputDeFocus(null);
			onDefocus(null);
			mChatInput.stage.focus = null;
		}
		
		public function inputFocus(evt:Event):void {
			//addAction("input is focused");
			stage.addEventListener(KeyboardEvent.KEY_DOWN, textCheckReturn);
			mCaretTmr.start();
		}
		
		public function inputDeFocus(evt:Event):void {
			//addAction("input is defocused");
			stage.removeEventListener(KeyboardEvent.KEY_DOWN, textCheckReturn);
			mCaretTmr.stop();
			if (mCaretVisible) {
				toggleCaret(null);
			}
		}
		
		public function	textCheckReturn(evt:KeyboardEvent) {
			//a key was pressed, check if it was Enter => charCode 13.
			////addAction("Character code: \""+String(evt.charCode)+"\"");
			if(String(evt.charCode) == "13"){
				//ok, enter was pressed.
				onEnter(null);
			}
		}
		
		public function toggleCaret(evt:Event) {
			if (!mCaretVisible) {
				//Show the caret
				if (mChatInput.text == "") {
					mChatInput.text = "|";
				} else {
					mChatInput.appendText("|");
				}
			} else {
				//Hide the carret
				var tmp:String = mChatInput.text;
				mChatInput.text = tmp.substr(0, tmp.length - 1);
			}
			
			mCaretVisible = !mCaretVisible;
		}
		
		public function onEnter(evt:Event):void {
			//Send input to the application
			var stripped:String = mChatInput.text;
			if (mCaretVisible) {
				stripped = stripped.substr(0, stripped.length - 1);
			}
			if (stripped != "") {
				ExternalInterface.call("chatInput", stripped);
			}
			
			mChatInput.text = "";
		}
		
		public function addText(username:String, msg:String):void {
			mChatDisplay.push("<b><font color=\"#326FC0\">"+username+": </font></b>"+msg);
			
			while (mChatDisplay.length > 10) {
				mChatDisplay.shift(); //Remove an element
			}
			
			redrawChat();
		}
		
		public function addAction(msg:String):void {
			mChatDisplay.push("<b><font color=\"#FFFF00\">&#8226; "+msg+"</font></b>");
			
			while (mChatDisplay.length > 10) {
				mChatDisplay.shift(); //Remove an element
			}
			
			redrawChat();
		}
		
		private function redrawChat():void {
			var chatContent:String = "";
			
			for (var i:Number = 0; i < mChatDisplay.length; ++i){
				if (i != 0) {
					chatContent += "<br>";
				}
				chatContent += mChatDisplay[i];
			}
			
			mChatOutput.styleSheet = new StyleSheet();
			mChatOutput.text = chatContent;
			mChatOutput.wordWrap = true;
		}
		
		public function onFocus(evt:Event):void {
			//addAction("Mouse focus");
			mFocusTmr.stop();
			mFocusTmr = new Timer(mAnimSteps);
			mFocusTmr.addEventListener("timer", showFocusHover);
			mFocusTmr.start();
		}
		
		public function onDefocus(evt:Event):void {
			//addAction("Mouse defocus");
			mFocusTmr.stop();
			mFocusTmr = new Timer(mAnimSteps);
			mFocusTmr.addEventListener("timer", hideFocusHover);
			mFocusTmr.start();
		}
		
		private function showFocusHover(evt:Event):void {
			//Increment the alpha of the hover clip
			textFilter.alpha += (mStep * 0.7);
			
			if (textFilter.alpha >= 0.7) {
				//addAction("showing hover");
				textFilter.alpha = 0.7;
				mFocusTmr.stop();
			}
			
			var myFilters:Array = new Array();
			myFilters.push(textFilter);
			mChatInputBorder.filters = myFilters;
		}
		
		private function hideFocusHover(evt:Event):void {
			//Increment the alpha of the hover clip
			textFilter.alpha -= (mStep * 0.7);
			
			if (textFilter.alpha <= 0) {
				//addAction("hiding hover");
				textFilter.alpha = 0;
				mFocusTmr.stop();
			}
			
			var myFilters:Array = new Array();
			myFilters.push(textFilter);
			mChatInputBorder.filters = myFilters;
		}
	}
}