#ifndef STRINGUTILS_H_
#define STRINGUTILS_H_

#include <string>

namespace HovUni {

	/**
	*	String utilities.
	*	@author Dirk Delahaye, 06/04/2010
	*/
	class StringUtils {
	public:
		/** 
		* Certain characters have special significance in HTML, and should be represented by HTML entities if they are to preserve their meanings. This function returns a string with some of these conversions made; the translations made are those most useful for everyday web programming. If you require all HTML character entities to be translated, use htmlentities() instead.
		* This function is useful in preventing user-supplied text from containing HTML markup, such as in a message board or guest book application.
		* The translations performed are:
		*	<ul>
		*     <li> '&' (ampersand) becomes '&amp;' </li>
		*     <li> '"' (double quote) becomes '&quot;' when ENT_NOQUOTES is not set. </li>
		*     <li> ''' (single quote) becomes '&#039;' only when ENT_QUOTES is set. </li>
		*     <li> '<' (less than) becomes '&lt;' </li>
		*     <li> '>' (greater than) becomes '&gt;' </li>
		*	</ul>
		*/
		static std::string htmlSpecialChars(const std::string& subject);
	};
}
#endif //STRINGUTILS_H_
