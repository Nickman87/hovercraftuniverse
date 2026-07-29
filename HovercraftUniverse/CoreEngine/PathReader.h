#ifndef PATHREADER_H_
#define PATHREADER_H_

#include <vector>
#include <string>

namespace HovUni {
	/**
	*	Parser for the Path files.
	*	@author Dirk Delahaye, 06/04/2010
	*/
	class PathReader {
	public:
		/**
		*	Parses a path file.
		*	Format of the path file:
		*	
		*	X	Y	Z	R
		*	X	Y	Z	R
		*	...
		*
		*	All values are floating point values. 
		*	The path radius of line X is applied to pathline [X X+1].
		*	The very last radius is ignored.
		*	@param	filename	The path file name
		*	@return	
		*		A twodimensional vector[nr of lines in file][4]
		*/
		static std::vector<std::vector<float>> parsePath(const std::string& filename);
	};
}

#endif //PATHREADER_H_