#ifndef _IRESOLUTION_H
#define _IRESOLUTION_H

#include <Exception.h>
#include <boost/shared_ptr.hpp>

namespace HovUni {
	/**
	 * Interface for any type of resolution.
	 *
	 * @author Nickman
	 */
	class IResolution {
		protected:
			/** A parent resolution that this resolution might be linked to */
			boost::shared_ptr<IResolution> mParentResolution;

		public:
			/**
			 * Constructor to initialise the parent resolution to null.
			 */
			IResolution() : mParentResolution() {}

			/**
			 * Ask if the defined width is relative or absolute
			 *
			 * @return True if the width is relative, false otherwise
			 */
			virtual inline bool isWidthRelative() = 0;

			/**
			 * Request the width of this resolution
			 *
			 * @return The width of this resolution
			 * @throws NoParentResolutionException If this resolution is dependent
			 *		on a parent resolution (relative width) and the parent has not
			 *		been set.
			 */
			virtual inline int getWidth() = 0;

			/**
			 * Ask if the defined height is relative or absolute
			 *
			 * @return True if the height is relative, false otherwise
			 */
			virtual inline bool isHeightRelative() = 0;
			
			/**
			 * Request the height of this resolution
			 *
			 * @return The height of this resolution
			 * @throws NoParentResolutionException If this resolution is dependent
			 *		on a parent resolution (relative width) and the parent has not
			 *		been set.
			 */
			virtual inline int getHeight() = 0;

			/**
			 * Set the parent resolution for this resolution
			 *
			 * @param parent The parent resolution
			 */
			void setParentResolution(boost::shared_ptr<IResolution> parent) { mParentResolution = parent; }
	};
}

#endif //_IRESOLUTION_H