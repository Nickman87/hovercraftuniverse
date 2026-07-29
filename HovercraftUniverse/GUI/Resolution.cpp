#include "Resolution.h"

namespace HovUni {

Resolution::Resolution(int width, int height, bool relWidth, bool relHeight) 
	: IResolution(), mWidth(width), mHeight(height), mRelativeWidth(relWidth), mRelativeHeight(relHeight) {
}

int Resolution::getWidth() {
	if (!isWidthRelative()) {
		return mWidth;
	} else {
		//Is the parent resolution set?
		if (mParentResolution.get() == 0) {
			THROW(NoParentResolutionException, "A scaled resolution can only be calculated if the parent resolution is set");
		}

		return (int) ((mWidth / 100.0f) * mParentResolution->getWidth()); //Return percentage
	}
}

int Resolution::getHeight() {
	if (!isHeightRelative()) {
		return mHeight;
	} else {
		//Is the parent resolution set?
		if (mParentResolution.get() == 0) {
			THROW(NoParentResolutionException, "A scaled resolution can only be calculated if the parent resolution is set");
		}

		return (int) ((mHeight / 100.0f) * mParentResolution->getHeight()); //Return percentage
	}
}

}