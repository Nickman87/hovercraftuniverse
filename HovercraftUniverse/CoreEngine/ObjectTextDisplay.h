#ifndef OBJECTTEXTDISPLAY_H_
#define OBJECTTEXTDISPLAY_H_

#include <OgreMovableObject.h>
#include <OgreCamera.h>
#include <OgreOverlayManager.h>
#include <OgreOverlayContainer.h>
#include <OgreOverlayElement.h>
#include <OgreLogManager.h>


namespace HovUni {

	/**
	*	Ogre Entity Text Overlay.
	*	@author Dirk Delahaye, 17/04/2010
	*/
	class ObjectTextDisplay {
	protected:
		/** The Ogre Movable object this text overlay will be linked to */
		const Ogre::MovableObject* m_p;

		/** The Ogre Camera, used to get view matrix */
		const Ogre::Camera* m_c;

		/** If set to true, this overlay will be enabled and the text will be shown*/
		bool m_enabled;

		/** The Ogre Overlay, shared by all ObjectTextDisplays */
		Ogre::Overlay* m_pOverlay;

		/** The Ogre Overlay element containing the text, unique to this ObjectTextDisplay */
		Ogre::OverlayElement* m_pText;

		/** The Ogre Overlay Container that holds the Overlay Element, unique to this ObjectTextDisplay */
		Ogre::OverlayContainer* m_pContainer;

		/** The actual text to be bound to m_pText */
		Ogre::String m_text;

		/** The unique overlay name */
		Ogre::String mOverlayName;
	public:
		/**
		*	Constructor.
		*	@param	p	The Ogre MovableObject that the text overlay should be following.
		*	@param	c	The Ogre Camera, used for the view matrix to calculate the position of the overlay.
		*/
		ObjectTextDisplay(const Ogre::MovableObject* p, const Ogre::Camera* c);

		/**
		*	Destructor.
		*/
		virtual ~ObjectTextDisplay();

		/**
		*	Enables or disables showing the text overlay.
		*	@param	enable	Iff true, the text overlay will be shown.
		*/
		void enable(bool enable);

		/**
		*	Set the actual text for the overlay.
		*	@param	text	The text for the overlay
		*/
		void setText(const Ogre::String& text);

		/**
		*	Updates the position of the text overlay, relative to the Ogre::MovableObject.
		*	This must be called for _every_ update of the MovableObject.
		*/
		void update();
	};
}

#endif //OBJECTTEXTDISPLAY_H_
