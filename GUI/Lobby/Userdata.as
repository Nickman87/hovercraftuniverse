package {
	//Userdata container
	class Userdata {
		public var ID:int;
		public var Username:String;
		public var Charactername:String;
		public var Carname:String;
		
		public function Userdata(id:int, username:String, charactername:String, carname:String):void {
			ID = id;
			Username = username;
			Charactername = charactername;
			Carname = carname;
		}
	}
}