package {
	import flash.display.*;
	import flash.events.*;
	import flash.utils.Timer;
	import flash.external.ExternalInterface;
	import flash.text.*;
	import flash.display.StageAlign;
	import flash.events.TextEvent;
	import flash.filters.*;
	
	// Class Definitie
	public class ServerMenu extends MovieClip{
		//Variabels
		private var mInput:TextField;
		private var mIPMain:MovieClip;
		
		private var mNOk:MovieClip;
		private var mNCancel:MovieClip;
		
		private var mGOk:MovieClip;
		private var mGCancel:MovieClip;
		
		private static var mAnimSpeed:Number = 150;
		private static var mAnimSteps:Number = 15;
		private static var mStep:Number = (100 / (mAnimSpeed / mAnimSteps)) / 100;
		
		private var mOkTmr:Timer = new Timer(mAnimSteps);
		private var mCancelTmr:Timer = new Timer(mAnimSteps);
		private var mFocusTmr:Timer = new Timer(mAnimSteps);
		private var mCaretTmr:Timer = new Timer(350);
		
		private var mCaretVisible:Boolean;
		
		private var textFilter:GlowFilter;
		
		// Constructor
		public function ServerMenu():void{
			//Store some data
			mIPMain = mIPbg;
			mNOk = mOk;
			mNCancel = mCancel;
			
			mCaretVisible = false;
			
			textFilter = new GlowFilter(0x3483E1, 0.0, 7.0, 7.0, 4, BitmapFilterQuality.HIGH);
			var myFilters:Array = mIPMain.filters;
			myFilters.push(textFilter);
			mIPMain.filters = myFilters;
			
			//Create textField
			mInput = mInputInstance;
			
			//Create glow buttons
			mGOk = new OkG();
			mGOk.alpha = 0;
			mNOk.addChild(mGOk);
			mGCancel = new CancelG();
			mGCancel.alpha = 0;
			mNCancel.addChild(mGCancel);
			
			//Add listeners
			mNOk.addEventListener(MouseEvent.MOUSE_OVER, onOkHover);
			mNOk.addEventListener(MouseEvent.MOUSE_OUT, exitOkHover);
			mNOk.addEventListener(MouseEvent.MOUSE_UP, onOk);
			mNCancel.addEventListener(MouseEvent.MOUSE_OVER, onCancelHover);
			mNCancel.addEventListener(MouseEvent.MOUSE_OUT, exitCancelHover);
			mNCancel.addEventListener(MouseEvent.MOUSE_UP, onCancel);
			
			mInput.addEventListener(MouseEvent.MOUSE_OVER, onFocus);
			mInput.addEventListener(MouseEvent.MOUSE_OUT, onDefocus);
			mInput.addEventListener(MouseEvent.MOUSE_UP, textFocus);  
			
			mCaretTmr.addEventListener("timer", toggleCaret);
			
			mInput.text=" "; //a SPACE or temp "foo" text, but NOT empty !
			mInput.stage.focus = mInput;
			mInput.setSelection(mInput.length,mInput.length);
			mInput.text = "";
			textFocus(null);
		}
		
		public function	textCheckReturn(evt:KeyboardEvent) {
			//a key was pressed, check if it was Enter => charCode 13.
			if(evt.charCode == 13){
				//ok, enter was pressed.
				onOk(null);
			}
		}
		
		public function toggleCaret(evt:Event) {
			if (!mCaretVisible) {
				//Show the caret
				if (mInput.text == "") {
					mInput.text = "|";
				} else {
					mInput.appendText("|");
				}
			} else {
				//Hide the carret
				var tmp:String = mInput.text;
				mInput.text = tmp.substr(0, tmp.length - 1);
			}
			
			
			mCaretVisible = !mCaretVisible;
		}
		
		public function textFocus(evt:Event):void {
			stage.addEventListener( KeyboardEvent.KEY_DOWN, textCheckReturn);
			mCaretTmr.start();
		}
		
		public function onFocus(evt:Event):void {
			mFocusTmr.stop();
			mFocusTmr = new Timer(mAnimSteps);
			mFocusTmr.addEventListener("timer", showFocusHover);
			mFocusTmr.start();
		}
		
		public function onDefocus(evt:Event):void {
			mFocusTmr.stop();
			mFocusTmr = new Timer(mAnimSteps);
			mFocusTmr.addEventListener("timer", hideFocusHover);
			mFocusTmr.start();
		}
		
		private function showFocusHover(evt:Event):void {
			//Increment the alpha of the hover clip
			textFilter.alpha += (mStep * 0.7);
			
			if (textFilter.alpha >= 0.7) {
				textFilter.alpha = 0.7;
				mFocusTmr.stop();
			}
			
			var myFilters:Array = new Array();
			myFilters.push(textFilter);
			mIPMain.filters = myFilters;
		}
		
		private function hideFocusHover(evt:Event):void {
			//Increment the alpha of the hover clip
			textFilter.alpha -= (mStep * 0.7);
			
			if (textFilter.alpha <= 0) {
				textFilter.alpha = 0;
				mFocusTmr.stop();
			}
			
			var myFilters:Array = new Array();
			myFilters.push(textFilter);
			mIPMain.filters = myFilters;
		}
		
		public function onOk(evt:Event):void {
			if (mCaretVisible) {
				var tmp:String = mInput.text;
				ExternalInterface.call("onOk", tmp.substr(0, tmp.length - 1));
			} else {
				ExternalInterface.call("onOk", mInput.text);
			}
		}
		
		public function onCancel(evt:Event):void {
			ExternalInterface.call("onCancel");
		}
		
		public function onOkHover(evt:MouseEvent):void {
			//lets start showing the hover clip
			mOkTmr.stop();
			mOkTmr = new Timer(mAnimSteps);
			mOkTmr.addEventListener("timer", showOkHover);
			mOkTmr.start();
		}
		
		public function onCancelHover(evt:MouseEvent):void {
			//lets start showing the hover clip
			mCancelTmr.stop();
			mCancelTmr = new Timer(mAnimSteps);
			mCancelTmr.addEventListener("timer", showCancelHover);
			mCancelTmr.start();
		}
		
		public function exitOkHover(evt:MouseEvent):void {
			//lets start removing the hover clip
			mOkTmr.stop();
			mOkTmr = new Timer(mAnimSteps);
			mOkTmr.addEventListener("timer", hideOkHover);
			mOkTmr.start();
		}
		
		public function exitCancelHover(evt:MouseEvent):void {
			//lets start removing the hover clip
			mCancelTmr.stop();
			mCancelTmr = new Timer(mAnimSteps);
			mCancelTmr.addEventListener("timer", hideCancelHover);
			mCancelTmr.start();
		}
		
		private function showOkHover(evt:Event):void {
			//Increment the alpha of the hover clip
			mGOk.alpha += mStep;
			
			if (mGOk.alpha >= 1) {
				mGOk.alpha = 1;
				mOkTmr.stop();
			}
		}
		
		private function hideOkHover(evt:Event):void {
			//Increment the alpha of the hover clip
			mGOk.alpha -= mStep;
			
			if (mGOk.alpha <= 0) {
				mGOk.alpha = 0;
				mOkTmr.stop();
			}
		}
		
		private function showCancelHover(evt:Event):void {
			//Increment the alpha of the hover clip
			mGCancel.alpha += mStep;
			
			if (mGCancel.alpha >= 1) {
				mGCancel.alpha = 1;
				mCancelTmr.stop();
			}
		}
		
		private function hideCancelHover(evt:Event):void {
			//Increment the alpha of the hover clip
			mGCancel.alpha -= mStep;
			
			if (mGCancel.alpha <= 0) {
				mGCancel.alpha = 0;
				mCancelTmr.stop();
			}
		}
	}
}