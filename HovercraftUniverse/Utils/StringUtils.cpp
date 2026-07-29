#include "StringUtils.h"
#include <string>

namespace HovUni {
	std::string StringUtils::htmlSpecialChars(const std::string& subject) {
		std::string result = subject;
		const int size = 5;
		char characters[size] = {'&', '<', '>', '\"', '\''};
		std::string replaceVals[size] = {"&amp;", "&lt;", "&gt;", "&quot;", "&#039;"};

		for (int i = 0; i < size; i++) {
			int position = result.find(characters[i]);
			while (position != std::string::npos)  {
				result.replace(position, 1, replaceVals[i]);
				position = result.find(characters[i], position + 1);
			}
		}
		return result;
	}
}
