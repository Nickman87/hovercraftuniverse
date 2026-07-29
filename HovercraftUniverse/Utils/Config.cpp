#include "Config.h"

#include "Exception.h"
#include <boost/algorithm/string/trim.hpp>

namespace HovUni {

	Config::Config() {

	}

	Config::~Config() {
		//...
	}

	void Config::loadFile(std::string filename) {
		bool success = mReader.Load(filename, true);
		std::cout << "Config Loading file " << filename <<"."<< std::endl;
		if (!success) {
			THROW(ConfigException,"Could not read configuration file at " + filename + ".");
		}
		mFilename = filename;
		std::cout << "Config new Filename " << mFilename << std::endl;
	}

	void Config::saveFile(std::string filename) {
		bool success = mReader.Save(filename);
		if (!success) {
			THROW(ConfigException,"Could not save to config file " + filename + ".");
		}
	}

	void Config::saveFile() {
		bool success = mReader.Save(mFilename);
		if (!success) {
			THROW(ConfigException,"Could not save to default config file " + mFilename + ".");
		}
	}

	const Config::keymap& Config::getKeys(const std::string& section) const {
		return mReader.GetSection(section)->GetKeys();
	}

	std::string Config::getStringValue(const std::string& section, const std::string& field, const std::string& defaultValue) {
		std::string value = mReader.GetKeyValue(section, field);
		boost::algorithm::trim(value);
		if (value == "") {
			value = defaultValue;
			mReader.SetKeyValue(section, field, defaultValue);
		}
		return value;
	}

	void Config::putStringValue(const std::string& section, const std::string& field, const std::string& value) {
		mReader.SetKeyValue(section,field,value);
	}
}
