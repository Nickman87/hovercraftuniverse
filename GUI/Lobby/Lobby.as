package {
	import flash.display.*;
	import flash.events.*;
	import flash.utils.Timer;
	import flash.external.ExternalInterface;
	import fl.controls.ComboBox;
	import fl.data.DataProvider;
	import flash.text.*;
	import flash.filters.*;
	import Userdata;
	
	import de.polygonal.ds.HashMap;
	import flash.Boot;
	
	// Class Definitie
	public class Lobby extends MovieClip{
		//Variabels
		var myFont:TextFormat = new TextFormat();
		
		//GLOBAL
		private static var mAnimSpeed:Number = 150;
		private static var mAnimSteps:Number = 15;
		private static var mStep:Number = (100 / (mAnimSpeed / mAnimSteps)) / 100;
		
		//USERDATA
		private var userdata:HashMap = new HashMap();
		private var userSort:Vector.<int> = new Vector.<int>();
		private var adminMark:MovieClip = new Star();
		
		//CHAT
		private var	mChatDisplay:Array = new Array();
		private var mCaretVisible:Boolean = false;
		private var mCaretTmr:Timer = new Timer(350);
		private var textFilter:GlowFilter;
		private var mFocusTmr:Timer = new Timer(mAnimSteps);
		
		//SETTINGS
		/*
			CLIENT BOXES:
			mTypeClient
			mTrackClient
			mLapsClient
			mPlayersClient
			mBotsClient
			
			ADMIN BOXES:
			mGameTypeBox
			mTrackBox
			mLaps
			mPlayers
			mBots
		*/
		private var mAdmin:Boolean;
		
		//Font
		[Embed(systemFont="Gill Sans MT", fontName="mFont", mimeType="application/x-font")]
		var mFont:Class;
		
		// Constructor
		public function Lobby():void{
			Font.registerFont(mFont);
			new flash.Boot(this);
			
			myFont.font = "mFont";
			myFont.color = 0xFFFFFF;
			myFont.size = 20;

			mGameTypeBox.addItem({data:"tmp", label:"Race"});
			mGameTypeBox.addItem({data:"tmp", label:"Sprint"});
			mGameTypeBox.addItem({data:"tmp", label:"Drag"});
			mGameTypeBox.textField.setStyle("textFormat", myFont);
			mGameTypeBox.textField.setStyle("embedFonts", true);
			mGameTypeBox.dropdown.setRendererStyle("textFormat", myFont);
			mGameTypeBox.dropdown.setRendererStyle("embedFonts", true);
			mGameTypeBox.dropdown.rowHeight = 40;
			
			mTrackBox.textField.setStyle("textFormat", myFont);
			mTrackBox.dropdown.setRendererStyle("textFormat", myFont);
			mTrackBox.dropdown.rowHeight = 40;
			mTrackBox.textField.setStyle("embedFonts", true);
			
			mLaps.setStyle("textFormat", myFont);
			mLaps.setStyle("embedFonts", true);
			
			mPlayers.setStyle("textFormat", myFont);
			mPlayers.setStyle("embedFonts", true);
			
			//Setup Settings
			setAdmin(true);
			
			mHovercrafts.addEventListener(Event.CHANGE, hovercraftToggle);
			
			mTrackBox.addEventListener(Event.CHANGE, trackToggle);
			mTrackBox.textField.setStyle("textFormat", myFont);
			mTrackBox.textField.setStyle("embedFonts", true);
			mTrackBox.dropdown.setRendererStyle("textFormat", myFont);
			mTrackBox.dropdown.setRendererStyle("embedFonts", true);
			mTrackBox.dropdown.rowHeight = 40;
			
			mBots.addItem({label:"Yes", data:"Yes"});
			mBots.addItem({label:"No", data:"No"});
			mBots.addEventListener(Event.CHANGE, botsToggle);
			
			mBots.textField.setStyle("textFormat", myFont);
			mBots.textField.setStyle("embedFonts", true);
			mBots.dropdown.setRendererStyle("textFormat", myFont);
			mBots.dropdown.setRendererStyle("embedFonts", true);
			mBots.dropdown.rowHeight = 40;
			setFillBots(false); //Default
			
			mPlayers.minimum = 1;
			mPlayers.maximum = 255;
			mPlayers.value = 6;
			mPlayers.addEventListener(Event.CHANGE, playersChange);
			
			ExternalInterface.addCallback("setAdmin", setAdmin);
			ExternalInterface.addCallback("setFillBots", setFillBots);
			ExternalInterface.addCallback("setPlayerMax", setPlayerMax);
			
			//Trackbox
			ExternalInterface.addCallback("clearMaps", clearMaps);
			ExternalInterface.addCallback("addMap", addMap);
			ExternalInterface.addCallback("setMap", setMap);
			
			//Hovercraft box
			ExternalInterface.addCallback("clearHovercrafts", clearHovercrafts);
			ExternalInterface.addCallback("addHovercraft", addHovercraft);
			ExternalInterface.addCallback("setHovercraft", setHovercraft);

			
			//Setup user list
			//for (var i:int = 1; i < 14; ++i) {
			//	if (i == 7) {
			//		deleteUser(3);
			//	}
			//	pushUser(i, "Player"+String(i), "Character"+String(i), "Car"+String(i));
			//}
			//editUser(9, "Nickman", "Ownz0r", "Coolmobile");
			
			//Setup chat
			textFilter = new GlowFilter(0x3483E1, 0.0, 7.0, 7.0, 4, BitmapFilterQuality.HIGH);
			
			mChatOutput.mouseWheelEnabled = true;
			mChatOutput.addEventListener(Event.CHANGE, scrollToBot);
			mChatOutput.wordWrap = true;
			mChatOutput.styleSheet = new StyleSheet();
			
			addAction("Welcome in the lobby");
			
			mChatInput.addEventListener(MouseEvent.MOUSE_OVER, onFocus);
			mChatInput.addEventListener(MouseEvent.MOUSE_OUT, onDefocus);
			//mChatInput.addEventListener(MouseEvent.MOUSE_OUT, inputDeFocus); //DONT DO THIS! :)
			mChatInput.addEventListener(MouseEvent.MOUSE_UP, inputFocus); 
			mCaretTmr.addEventListener("timer", toggleCaret);
			
			mSendButton.addEventListener(MouseEvent.CLICK, onEnter);
			
			/*
			for (var i:int = 0; i < 10; ++i) {
				pushUser(i, "naam"+String(i), "charac"+String(i), "car"+String(i));
			}
			markAdmin(9);
			*/
			
			//Set callbacks
			ExternalInterface.addCallback("addText", addText);
			ExternalInterface.addCallback("addAction", addAction);
			
			ExternalInterface.addCallback("addUser", pushUser);
			ExternalInterface.addCallback("editUser", editUser);
			ExternalInterface.addCallback("deleteUser", deleteUser);
			ExternalInterface.addCallback("markAdmin", markAdmin);
		}
		
		//////////////////////////////////////
		//				SETTINGS			//
		//////////////////////////////////////
		
		public function clearHovercrafts():void {
			mHovercrafts.removeAll();
		}
		
		public function addHovercraft(id:int, val:String) {
			mHovercrafts.addItem({label:val, data:id});
			mHovercrafts.textField.setStyle("textFormat", myFont);
			mHovercrafts.textField.setStyle("embedFonts", true);
			mHovercrafts.dropdown.setRendererStyle("textFormat", myFont);
			mHovercrafts.dropdown.setRendererStyle("embedFonts", true);
			mHovercrafts.dropdown.rowHeight = 40;
		}
		
		public function setHovercraft(id:int, val:String) {
			mHovercrafts.selectedIndex = findItemIndex(mHovercrafts, id.toString());
			mHovercrafts.drawNow();
		}
		
		public function hovercraftToggle(evt:Event) {
			//Notify the listener
			var val:int = ComboBox(evt.target).selectedItem.data; 
			
			ExternalInterface.call("hovercraftChange", val);
		}
		
		public function clearMaps():void {
			mTrackBox.removeAll();
		}
		
		public function addMap(id:int, val:String):void {
			mTrackBox.addItem({label:val, data:id});
			mTrackBox.textField.setStyle("textFormat", myFont);
			mTrackBox.textField.setStyle("embedFonts", true);
			mTrackBox.dropdown.setRendererStyle("textFormat", myFont);
			mTrackBox.dropdown.setRendererStyle("embedFonts", true);
			mTrackBox.dropdown.rowHeight = 40;
		}
		
		public function setMap(id:int, val:String):void {
			//addAction("Setting map to "+String(id)+" | "+String(val));
			
			mTrackClient.text = val;
			mTrackBox.selectedIndex = findItemIndex(mTrackBox, id.toString());
			mTrackBox.drawNow();
			//addAction("Index "+findItemIndex(mTrackBox, id.toString()));
		}
		
		public function trackToggle(evt:Event):void {
			//Notify the listener
			var val:int = ComboBox(evt.target).selectedItem.data; 
			
			ExternalInterface.call("mapChange", val);
			
			//Update the client view
			mTrackClient.text = ComboBox(evt.target).selectedItem.label;
		}
		
		public function setAdmin(admin:Boolean):void {
			//CLIENT BOXES
			mTypeClient.visible = !admin;
			mTrackClient.visible = !admin;
			mLapsClient.visible = !admin;
			mPlayersClient.visible = !admin;
			mBotsClient.visible = !admin;
			//ADMIN BOXES
			mGameTypeBox.visible = admin;
			mTrackBox.visible = admin;
			mLaps.visible = admin;
			mPlayers.visible = admin;
			mBots.visible = admin;
			
			mAdmin = admin;
		}
		
		public function setFillBots(val:Boolean):void {
			if (val) {
				mBots.selectedIndex = this.findItemIndex(mBots, "Yes");
				mBotsClient.text = "Yes";
			} else {
				mBots.selectedIndex = this.findItemIndex(mBots, "No");
				mBotsClient.text = "No";
			}
		}

		private function findItemIndex (element:ComboBox, dataString:String):int {
			var index:int = 0;
			for (var i = 0; i < element.length; i++) {
				if (element.getItemAt(i).data.toString() == dataString) {
					index = i;
					break;
				} else {
				}
			}
			return index;
		}
		
		public function botsToggle(evt:Event):void {
			//Notify the listener
			var fillBots = ComboBox(evt.target).selectedItem.data; 
			
			if (fillBots == "Yes") {
				ExternalInterface.call("botsValue", true);
			} else {
				ExternalInterface.call("botsValue", false);
			}
			
			//Update the client view
			mBotsClient.text = fillBots;
		}
		
		public function playersChange(evt:Event):void {
			var val:Number = NumericStepper(evt.target).value;
			mPlayersClient.text = String(val);
			ExternalInterface.call("playerMax", val);
		}
		
		public function setPlayerMax(val:int):void {
			mPlayers.value = val;
			mPlayersClient.text = String(val);
		}
		
		//////////////////////////////////////
		//				Users				//
		//////////////////////////////////////
		
		private function clearData():void {
			mPlayerNames.text = "";
			mCharacters.text = "";
			mCars.text = "";
		}
		
		public function markAdmin(id:int):void {
			//Find the row of the admin
			if (userdata.contains(id)) {
				var rowheight:Number = mPlayerNames.textHeight / userSort.length;
				var	row = userSort.indexOf(id);
				
				adminMark.x = 50;
				adminMark.y = 100 + (rowheight / 2) + (row * (rowheight - 0.5));
				
				if (!this.contains(adminMark)) {
					this.addChild(adminMark);
				}
			}
		}
		
		public function pushUser(id:int, username:String, charactername:String, carname:String):void {
			userdata.set(id, new Userdata(id, username, charactername, carname));
			//A new user, add it to the sort list
			userSort.push(id);
			
			addAction("New user: "+username);
			
			redrawUsers();
		}
		
		public function editUser(id:int, username:String, charactername:String, carname:String):void {
			var currUser:Userdata = userdata.get(id) as Userdata;
			
			if (currUser != null) {
				currUser.Username = username;
				currUser.Charactername = charactername;
				currUser.Carname = carname;
				//Delete the old one
				userdata.removeIfExists(id);
				//Push the new one
				userdata.set(id, currUser);
			}
			redrawUsers();
		}
		
		public function deleteUser(id:int) {
			 if (userdata.removeIfExists(id)) {
				 //Remove him from the sorting
				 userSort.splice(userSort.indexOf(id), 1);
			 }
			 redrawUsers();
		}
		
		public function redrawUsers() {
			//Make it empty
			clearData();
			
			var prefix:String = "";
			//Loop over the userdata we have and put it in the right columns
			
			for (var i:int = 0; i < userSort.length; i++) {
				var currUser:Userdata = userdata.get(userSort[i]) as Userdata;
				
				if (currUser != null) {
					//Put the correct info in the correct windows
					mPlayerNames.appendText(prefix.concat(currUser.Username));
					mCharacters.appendText(prefix.concat(currUser.Charactername));
					mCars.appendText(prefix.concat(currUser.Carname));
					
					if (prefix == "") {
						prefix = "\n";
					}
				} else {
					//this index seems invallid, delete it from the sortList
					
				}
			}
		}
		
		//////////////////////////////////////
		//				CHAT				//
		//////////////////////////////////////
		public function inputFocus(evt:Event):void {
			stage.addEventListener(KeyboardEvent.KEY_DOWN, textCheckReturn);
			mCaretTmr.start();
		}
		
		public function inputDeFocus(evt:Event):void {
			stage.removeEventListener(KeyboardEvent.KEY_DOWN, textCheckReturn);
		}
		
		public function	textCheckReturn(evt:KeyboardEvent) {
			//a key was pressed, check if it was Enter => charCode 13.
			//addAction("Character code: \""+String(evt.charCode)+"\"");
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
			
			if (stripped == "adm") {
				//setFillBots(true);
			}
			
			mChatInput.text = "";
		}
		
		public function addText(username:String, msg:String):void {
			mChatDisplay.push("<b><font color=\"#326FC0\">"+username+": </font></b>"+msg);
			redrawChat();
		}
		
		public function addAction(msg:String):void {
			mChatDisplay.push("<b><font color=\"#FFFF00\">&#8226; "+msg+"</font></b>");
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
			//trace("need to scroll to" + String(mChatOutput.maxScrollV));
			scrollToBot(null);
			//trace("htmlText: " + mChatField.htmlText);
		}
		
		public function scrollToBot(evt:Event):void {
			//trace("scrolling to" + String(mChatOutput.maxScrollV));
			mChatOutput.scrollV = mChatOutput.maxScrollV;
			mChatScroller.maxScrollPosition = mChatOutput.maxScrollV;
			mChatScroller.scrollPosition = mChatOutput.maxScrollV;
			//trace("scrolled to: "+ String(mChatOutput.scrollV) + "|" + String(mChatScroller.scrollPosition) + " | " + String(mChatScroller.maxScrollPosition));
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
			mChatInputBorder.filters = myFilters;
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
			mChatInputBorder.filters = myFilters;
		}
	}
}