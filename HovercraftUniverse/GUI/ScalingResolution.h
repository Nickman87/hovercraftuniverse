#ifndef _SCALINGRESOLUTION_H
#define _SCALINGRESOLUTION_H

#include "IResolution.h"

namespace HovUni {
	/**
	 * Class that represents a scaling resolution. This resolution will scale
	 * according to any parent resolution.
	 *
	 * @author Nickman
	 */
	class ScalingResolution : public IResolution {
		private:
			/** The minimum resolution we want */
			boost::shared_ptr<IResolution> mMinResolution;
			/** The maximum resolution we want */
			boost::shared_ptr<IResolution> mMaxResolution;

			/** The maximum screen resolution at which we want the minimum resolution */
			boost::shared_ptr<IResolution> mMinScreenResolution;
			/** The minimum screen resolution at which we want the maximum resolution */
			boost::shared_ptr<IResolution> mMaxScreenResolution;

			/** Boolean to mark whether we want to preserve the aspect ratio or not */
			bool mPreserverAspectRatio;

		public:
			/**
			 * Constructor for a scaling resolution.
			 * 
			 * @param minResolution The minimum resolution we want to have.
			 * @param minScreenResolution The minimum resolution at which we want to start using the
			 *		minResolution values.
			 * @parem maxResolution The maximum resolution we want to have.
			 * @param maxScreenResolution The maximum resolution at which we want to start using the
			 *		maxResolution values.
			 * @param arPreserving Boolean to toggle aspect ratio preservation on or off (true by default)
			 */
			ScalingResolution(boost::shared_ptr<IResolution> minResolution, boost::shared_ptr<IResolution> minScreenResolution, boost::shared_ptr<IResolution> maxResolution, boost::shared_ptr<IResolution> maxScreenResolution, bool arPreserving = true);

			/**
			 * Constructor for a scaling resolution without a minimum and maximum resolution.
			 *
			 * @param resolution The resolution that the object should have at the given
			 *		screen resolution
			 * @param screenResolution The screen resolution that is linked to the given
			 *		resolution and will be used to scale the result.
			 * @param arPreserving Boolean to toggle aspect ratio preservation on or off (true by default)
			 */
			ScalingResolution(boost::shared_ptr<IResolution> resolution, boost::shared_ptr<IResolution> screenResolution, bool arPreserving = true);

			/**
			 * Will check if the minimum (or only) resolution width is relative. And thus if
			 * the resulting width is dependent on the parent resolution.
			 *
			 * @param return True if we have a relative width, false otherwise.
			 */
			virtual inline bool isWidthRelative() { return mMinResolution->isWidthRelative(); }

			/**
			 * Get the width of this resultion, scaled to the parent resolution.
			 * Make sure the parent resolution is set, otherwise this call will throw
			 * a NoParentResolutionException exception.
			 *
			 * @return The scaled width of this resolution
			 * @throws NoParentResolutionException will be thrown if the parent resolution
			 *		was not set.
			 */
			virtual int getWidth();

			/**
			 * Will check if the minimum (or only) resolution height is relative. And thus if
			 * the resulting height is dependent on the parent resolution.
			 *
			 * @param return True if we have a relative height, false otherwise.
			 */
			virtual inline bool isHeightRelative() { return mMinResolution->isHeightRelative(); }
			
			/**
			 * Get the height of this resultion, scaled to the parent resolution.
			 * Make sure the parent resolution is set, otherwise this call will throw
			 * a NoParentResolutionException exception.
			 *
			 * @return The scaled height of this resolution
			 * @throws NoParentResolutionException will be thrown if the parent resolution
			 *		was not set.
			 */
			virtual int getHeight();

		private:
			/**
			 * Calculate the actual dimension we want to use for either width of height. The maxSize
			 * and maxScreen values are optional. When not provided, the function will scale the
			 * minSize value by using the minScreen and parentSize values.
			 *
			 * @param minSize The minimum size we want at the minScreen resolution.
			 * @param minScree The minimum screen resolution at which we start to use minSize.
			 * @param relSize Boolean to mark whether we are working with relative values.
			 * @param parentSize The real size that is available.
			 * @param maxSize The maximum size we want at the maxScreen resolution (Optional).
			 * @param maxScree The maximum screen resolution at which we start to use maxSize (Optional).
			 * @return The calculated dimension
			 */
			int calculateDimension(int minSize, int minScreen, bool relSize, int parentSize, int maxSize = -1, int maxScreen = -1);
			
			/**
			 * Calculate a scaling factor from the minimum and maximum value and the current value we are at.
			 *
			 * @param min The minimum value.
			 * @param max The maximum value.
			 * @param curr The current vlaue.
			 * @param normalize Normalize the scale between 0 and 1, default = true.
			 * @return The calculated scale
			 */
			float calculateScale(int min, int max, int curr, bool normalize = true);

			/**
			 * Scale a value between min and max according to the given scale.
			 *
			 * @param minValue The minimum value we can get.
			 * @param maxValue The maximum value we can het.
			 * @param scale The scale we want to use [0-1].
			 * @return The scaled value
			 */
			float scaleValue(float minValue, float maxValue, float scale);

			/**
			 * Request a scale that preserves the aspect ratio of the resolution
			 *
			 * @return A scale that should be used to scale both the widht and height
			 *		to preserve the aspect ratio of the resolution.
			 */
			float getARScale();
	};
}

#endif //_SCALINGRESOLUTION_H