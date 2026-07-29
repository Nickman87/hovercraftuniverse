#ifndef CONFIG_H_
#define CONFIG_H_

#include <string>
#include "inifile.h"
#include <sstream>

namespace HovUni {

	/**
	 * This class wraps the INI functionality to read/write key-value pairs to file.
	 *
	 * @author Dirk Delahaye, 26/03/2010
	 */
	class Config {
	private:

		/**
		*	The INI File object.
		*/
		CIniFileA mReader;

		/**
		*	The default filename to save to
		*/
		std::string mFilename;

		
	public:
		/** The type of a key map */
		typedef KeyMapA keymap;

		/**
		*	Constructor. Use Config::getSingletonPtr().
		*/
		Config();

		/**
		*	Destructor.
		*/
		~Config();

		/**
		*	Loads an INI File. 
		*	NOTE: The contents will be merged into this object.
		*			If you do not wish to do this, create a new Config object instead.
		*	@param	filename
		*	@throws ConfigException
		*/
		void loadFile(std::string filename);

		/**
		*	Saves the internal representation to a file.
		*	@param	filename	The config filename.
		*	@throws ConfigException
		*/
		void saveFile(std::string filename);

		/**
		*	Saves the internal representation to the same file it's been loaded from.
		*	@throws ConfigException
		*/
		void saveFile();

		/**
		*	Returns the value for the given section and field, but converted to
		*	a given type.
		*	@param	section	The section name
		*	@param	field	The key, or field name.
		*	@param	defaultValue	If no value / empty value was found, this value will be returned
		*							Also, this default value will be SET into the config automatically.
		*	@return	The value, as the given type.
		*/
		template <typename T>
		T getValue(const std::string& section, const std::string& field, const T defaultValue);

		/**
		*	Put the value for the given section and field, but converted to string from a given type.
		*	@param	section	The section name
		*	@param	field	The key, or field name
		*	@param	value	The value
		*/
		template <typename T>
		void putValue(const std::string& section, const std::string& field, const T value);

		/**
		 * Get the available keys for a certain section
		 *
		 * @param section The section name
		 */
		const keymap& getKeys(const std::string& section) const;

	private:
		/**
		*	Returns the value for the given (section, field, value) tuple.
		*	@param	section			The section name
		*	@param	field			The key, or field name.
		*	@param	defaultValue	If no value / empty value was found, this value will be returned.
		*							Also, this default value will be SET into the config automatically.
		*	@return	The value, as a string. NOTE! This string is whitespace-trimmed.
		*/
		std::string getStringValue(const std::string& section, const std::string& field, const std::string& defaultValue);

		/**
		*	Put the value in the config as a (section, field, value) tuple.
		*	@param	section	The section name
		*	@param	field	The key, or field name.
		*	@param	value	The value, as a string. NOTE! This string will be whitespace-trimmed.
		*/
		void putStringValue(const std::string& section, const std::string& field, const std::string& value);
	};

	template <typename T>
	T Config::getValue(const std::string& section, const std::string& field, const T defaultValue) {
		std::ostringstream ss;
		ss << defaultValue;
		std::string defaultStringValue = ss.str();
		std::string stringValue = getStringValue(section, field, defaultStringValue);
		std::istringstream buffer(stringValue);
		T result;
		buffer >> result;
		return result;
	}

	template <typename T>
	void Config::putValue(const std::string& section, const std::string& field, const T value) {
		std::ostringstream ss;
		ss << value;
		std::string stringValue = ss.str();
		putStringValue(section, field, stringValue);
	}


	template <>
	inline std::string Config::getValue(const std::string& section, const std::string& field, const std::string defaultValue) {
		return getStringValue(section, field, defaultValue);
	}

	template <>
	inline void Config::putValue(const std::string& section, const std::string& field, const std::string value) {
		putStringValue(section, field, value);
	}
}

#endif //CONFIG_H_
