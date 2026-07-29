package {
	import flash.display.*;
	import flash.events.*;
	import flash.utils.Timer;
	import flash.external.ExternalInterface;
	import flash.text.*;
	import flash.display.StageAlign;
	import flash.events.TextEvent;
	
	// Class Definitie
	public class menuButton extends MovieClip{
		//Variabels
		private var mMain:MovieClip;
		private var mMainBg:MovieClip;
		
		private var mHoverEffect:MovieClip;
		private var mPressedEffect:MovieClip;
		
		private var mHoverTmr:Timer;
		private var mPressTmr:Timer;
		
		private static var mAnimSpeed:Number = 200;
		private static var mAnimSteps:Number = 20;
		private static var mStep:Number = (100 / (mAnimSpeed / mAnimSteps)) / 100;
		
		private static var mStickyPress:Boolean = false;
		private static var mTempSticky:Boolean = false;
		
		var mTextFormat:TextFormat;
		private var mText:TextField;
		
		[Embed(systemFont="Gill Sans MT", fontName="Text", mimeType="application/x-font")]
		var mTextFont:Class;
		
		// Constructor
		public function menuButton():void{
			Font.registerFont(mTextFont);
			
			mTextFormat = new TextFormat();
			mTextFormat.size = 50;
			mTextFormat.color = 0xFFFFFF;
			mTextFormat.font = "Text";
			mTextFormat.bold = true;
			mTextFormat.align = "center";
			
			mMain = defaultBorder;
			mMainBg = mainBackground;
			
			mHoverEffect = new Hover();
			mHoverEffect.alpha = 0;
			mMain.addChild(mHoverEffect);
			mHoverTmr = new Timer(mAnimSteps);

			mPressedEffect = new Pressed();
			mPressedEffect.alpha = 0;
			mMain.addChild(mPressedEffect);
			mPressTmr = new Timer(mAnimSteps);
			
			mMain.addEventListener(MouseEvent.MOUSE_OVER, onHover);
			mMain.addEventListener(MouseEvent.MOUSE_OUT, exitHover);
			mMain.addEventListener(MouseEvent.MOUSE_DOWN, onPressed);
			
			//Set the textfield
			mText = new TextField();
			mMainBg.addChild(mText);
			
			//Make external functions available
			ExternalInterface.addCallback("setText", setText);
			ExternalInterface.addCallback("stickyPress", stickyPress);
			ExternalInterface.addCallback("cancelPress", cancelPress);
		}
		
		public function setText(val:String):void {
			var screen_width:Number = stage.stageWidth;
			var screen_height:Number = stage.stageHeight;
			var padding:Number = 80;
			mText.embedFonts = true;
			mText.defaultTextFormat = mTextFormat;
			mText.width = screen_width - padding;
			mText.htmlText = val;
			mText.x = -( mText.width / 2 );// + (padding / 2);
			mText.autoSize = TextFieldAutoSize.CENTER;
			mText.y = -(mText.height / 2);
		}
		
		public function stickyPress():void {
			mStickyPress = true;
		}
		
		public function cancelPress():void {
			mStickyPress = false;
			exitPressed(null);
		}
		
		public function onHover(evt:Event):void {
			//lets start showing the hover clip
			mHoverTmr.stop();
			mHoverTmr = new Timer(mAnimSteps);
			mHoverTmr.addEventListener("timer", showHover);
			mHoverTmr.start();
		}
		
		public function exitHover(evt:Event):void {
			//lets start showing the hover clip
			mHoverTmr.stop();
			mHoverTmr = new Timer(mAnimSteps);
			mHoverTmr.addEventListener("timer", hideHover);
			mHoverTmr.start();
			
			//We might want to remove the pressed overlay to
			if (!mTempSticky) {
				if (!mStickyPress) {
					exitPressed(null);
				}
			} else {
				mTempSticky = false; //Var can only be used once
			}
		}
		
		private function showHover(evt:Event):void {
			//Increment the alpha of the hover clip
			mHoverEffect.alpha += mStep;
			
			if (mHoverEffect.alpha >= 1) {
				mHoverEffect.alpha = 1;
				mHoverTmr.stop();
			}
		}
		
		private function hideHover(evt:Event):void {
			//Increment the alpha of the hover clip
			mHoverEffect.alpha -= mStep;
			
			if (mHoverEffect.alpha <= 0) {
				mHoverEffect.alpha = 0;
				mHoverTmr.stop();
			}
		}
		
		public function onPressed(evt:MouseEvent):void {
			//let listeners know whe pressed the mouse
			ExternalInterface.call("press");
			
			mPressTmr.stop();
			mPressTmr = new Timer(mAnimSteps);
			mPressTmr.addEventListener("timer", showPressed);
			mPressTmr.start();
			
			if (mHoverEffect.alpha > 0) { //We don't want the hover effect
				mTempSticky = true;
				exitHover(null);
			}
		}
		
		public function exitPressed(evt:MouseEvent):void {
			mPressTmr.stop();
			mPressTmr = new Timer(mAnimSteps);
			mPressTmr.addEventListener("timer", hidePressed);
			mPressTmr.start();
			
			if (mHoverEffect.alpha < 1) { //We might want the hover effect...
				if (evt != null) {
					if (mMain.hitTestPoint(evt.target.mouseX, evt.target.mouseY)) {
						exitHover(null);
					}
				}
			}
		}
		
		private function showPressed(evt:Event):void {
			//Increment the alpha of the hover clip
			mPressedEffect.alpha += mStep;
			
			if (mPressedEffect.alpha >= 1) {
				mPressedEffect.alpha = 1;
				mPressTmr.stop();
			}
		}
		
		private function hidePressed(evt:Event):void {
			//Increment the alpha of the hover clip
			mPressedEffect.alpha -= mStep;
			
			if (mPressedEffect.alpha <= 0) {
				mPressedEffect.alpha = 0;
				mPressTmr.stop();
			}
		}
	}
}