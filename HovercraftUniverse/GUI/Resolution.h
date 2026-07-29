#ifndef _RESOLUTION_H
#define _RESOLUTION_H

#include "IResolution.h"

namespace HovUni {
	/**
	 * Class that represents a basic resolution. the resolution can either contain fixed
	 * values (in number of pixels) or relative values (in percentages [0 - 100]).
	 *
	 * @author Nickman
	 */
	class Resolution : public IResolution {
		private:
			/** The width */
			int mWidth;

			/** The height */
			int mHeight;

			/** Mark the width as relative */
			bool mRelativeWidth;

			/** Mark the height as relative */
			bool mRelativeHeight;

		public:
			/**
			 * Constructor for a fixed resolution. The height and width of this resolution
			 * are fixed. Don't forget to set the parent resolution if you use relative values!
			 *
			 * @param width The width of the resolution
			 * @param height The height of the resolution
			 * @param relWidth Boolean to mark the width as relative (default: false)
			 * @param relHeight Boolean to mark the height as relative (default: false)
			 */
			Resolution(int width, int height, bool relWidth = false, bool relHeight = false);

			/**
			 * Ask if the defined width is relative or absolute
			 *
			 * @return True if the width is relative, false otherwise
			 */
			virtual inline bool isWidthRelative() { return mRelativeWidth; }

			/**
			 * Request the width of this resolution
			 *
			 * @return The width of this resolution
			 * @throws NoParentResolutionException If this resolution is dependent
			 *		on a parent resolution (relative width) and the parent has not
			 *		been set.
			 */
			virtual inline int getWidth();

			/**
			 * Ask if the defined height is relative or absolute
			 *
			 * @return True if the height is relative, false otherwise
			 */
			virtual inline bool isHeightRelative() { return mRelativeHeight; }
			
			/**
			 * Request the height of this resolution
			 *
			 * @return The height of this resolution
			 * @throws NoParentResolutionException If this resolution is dependent
			 *		on a parent resolution (relative height) and the parent has not
			 *		been set.
			 */
			virtual inline int getHeight();
	};
}

#endif //_RESOLUTION_H