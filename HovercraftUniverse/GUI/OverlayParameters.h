#ifndef _OVERLAYPARAMETERS_H
#define _OVERLAYPARAMETERS_H

#include <Hikari.h>
#include "BasicOverlay.h"
#include <OgreString.h>
#include "IResolution.h"
#include <boost/shared_ptr.hpp>

namespace HovUni {
	/**
	 * Class that contains all parameters needed to create and position an overlay.
	 *
	 * @author Nickman
	 */
	template <typename T>
	class OverlayParameters {
		private:
			/** The name for the component */
			Ogre::String mName;
			/** The name of the file that has to be loaded */
			Ogre::String mFilename;
			/** The position to place the overlay */
			Hikari::Position mPosition;
			/** The resolution of the overlay */
			boost::shared_ptr<IResolution> mResolution;
			/** The prefered zOrder, 0 to put on top */
			Ogre::ushort mZOrder;
		
		public:
			/** The class of the overlay */
			typedef T mType;

			/**
			 * Constructor for an overlay parameters object.
			 *
			 * @param name The name for the overlay.
			 * @param filename The name of the file to use.
			 * @param position The position for the overlay.
			 * @param resolution The resolution for the overlay.
			 */
			OverlayParameters(const Ogre::String& name, const Ogre::String& filename, const Hikari::Position& position, boost::shared_ptr<IResolution> resolution, Ogre::ushort zOrder = 0);

			/**
			 * Request the name of the overlay.
			 *
			 * @return The name of the overlay.
			 */
			inline Ogre::String getName() { return mName; }

			/**
			 * Request the positioning of this overlay.
			 *
			 * @return The position for this overlay.
			 */
			inline Hikari::Position getPosition() { return mPosition; }

			/**
			 * request the resolution of this overlay.
			 *
			 * @return The resolution of the overlay.
			 */
			inline boost::shared_ptr<IResolution> getResolution() { return mResolution; }

			boost::shared_ptr<typename mType> getInstancedOverlay();
	};

	template <typename T>
	OverlayParameters<T>::OverlayParameters(const Ogre::String& name, const Ogre::String& filename, const Hikari::Position& position, boost::shared_ptr<IResolution> resolution, Ogre::ushort zOrder) 
		: mName(name), mFilename(filename), mPosition(position), mResolution(resolution), mZOrder(zOrder) {
	}

	// Pre-existing bug fix (not Ogre/modernization-related, see
	// docs/porting/hikari-gui.md): as shipped, this function referenced an
	// undeclared identifier `mtype` (a typo of the class's own `mType`
	// member typedef), was missing the `<T>` template argument on its
	// out-of-class qualification, and never returned anything -- it could
	// not have compiled or worked. Confirmed unused anywhere in
	// HovercraftUniverse/** (grep for getInstancedOverlay/OverlayParameters<
	// finds only this declaration), so this was dead, broken template code.
	// Fixed to do what its name/signature/parameter list obviously intend.
	template <typename T>
	boost::shared_ptr<typename OverlayParameters<T>::mType> OverlayParameters<T>::getInstancedOverlay() {
		return boost::shared_ptr<mType>(new T(mName, mFilename, mResolution->getWidth(), mResolution->getHeight(), mPosition, mZOrder));
	}
}

#endif //_OVERLAYPARAMETERS_H