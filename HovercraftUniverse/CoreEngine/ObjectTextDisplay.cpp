#include "ObjectTextDisplay.h"

namespace HovUni {

	ObjectTextDisplay::ObjectTextDisplay(const Ogre::MovableObject* p, const Ogre::Camera* c) {
		m_p = p;
		m_c = c;
		m_enabled = false;
		m_text = "";

		// create an overlay that we can use for later
		m_pOverlay = Ogre::OverlayManager::getSingleton().getByName("TextOverlay" + p->getName());
		if (m_pOverlay == 0) {
			m_pOverlay = Ogre::OverlayManager::getSingleton().create("TextOverlay" + p->getName());
		}

		Ogre::OverlayElement* overlayElement = Ogre::OverlayManager::getSingleton().createOverlayElement("Panel", "TextOverlayContainer" + p->getName());

		m_pContainer = static_cast<Ogre::OverlayContainer*>(overlayElement);

		m_pOverlay->add2D(m_pContainer);
		mOverlayName = "EntityLabelText" + p->getName();
		//Ogre::LogManager::getSingleton().getDefaultLog()->stream() << "Creating text area: " << mOverlayName;

		m_pText = Ogre::OverlayManager::getSingleton().createOverlayElement("TextArea", mOverlayName);
		m_pText->setDimensions(1.0, 1.0);
		m_pText->setMetricsMode(Ogre::GMM_PIXELS);
		m_pText->setPosition(0, 0);

		m_pText->setParameter("font_name", "StarWars");
		m_pText->setParameter("char_height", "12");
		m_pText->setParameter("horz_align", "center");
		m_pText->setColour(Ogre::ColourValue(1.0, 1.0, 1.0));

		m_pContainer->addChild(m_pText);
		m_pOverlay->show();
	}

	ObjectTextDisplay::~ObjectTextDisplay() {
		// overlay cleanup -- Ogre would clean this up at app exit but if your app 
		// tends to create and delete these objects often it's a good idea to do it here.
		m_pOverlay->hide();
		Ogre::OverlayManager *overlayManager = Ogre::OverlayManager::getSingletonPtr();
		m_pContainer->removeChild(mOverlayName);
		m_pOverlay->remove2D(m_pContainer);
		overlayManager->destroyOverlayElement(m_pText);
		overlayManager->destroyOverlayElement(m_pContainer);
		//overlayManager->destroy(m_pOverlay);
	}

	void ObjectTextDisplay::enable(bool enable) {
		m_enabled = enable;
		if (enable) {
			m_pOverlay->show();
		} else {
			m_pOverlay->hide();
		}
	}

	void ObjectTextDisplay::setText(const Ogre::String& text) {
		m_text = text;
		m_pText->setCaption(m_text);
	}

	void ObjectTextDisplay::update()  {
		if (!m_enabled) {
			return;
		}

		//Project camera_to_object onto the forward vector
		//Check if the object is in front of our camera
		//Make sure the overlay is shown
		m_pOverlay->show();

		// get the projection of the object's AABB into screen space
		//AABB = axis-aligned bounding box
		const Ogre::AxisAlignedBox& bbox = m_p->getWorldBoundingBox(true);
		Ogre::Matrix4 mat = m_c->getViewMatrix();
		const Ogre::Vector3* corners = bbox.getAllCorners();

		float min_x = 1.0f;
		float max_x = 0.0f;
		float min_y = 1.0f;
		float max_y = 0.0f;
		float min_z = 1.0f;
		float max_z = 0.0f;

		// expand the screen-space bounding-box so that it completely encloses 
		// the object's AABB
		for (int i=0; i<8; i++) {
			Ogre::Vector3 corner = corners[i];

			// multiply the AABB corner vertex by the view matrix to 
			// get a camera-space vertex
			corner = mat * corner;

			if (corner.z > 0) {
				//Object is (at least partly) BEHIND the camera. do not draw overlay.
				m_pContainer->hide();
				return;
			}

			// make 2D relative/normalized coords from the view-space vertex
			// by dividing out the Z (depth) factor -- this is an approximation
			float x = corner.x / corner.z + 0.5f;
			float y = corner.y / corner.z + 0.5f;

			if (x < min_x) {
				min_x = x;
			}
			if (x > max_x) {
				max_x = x;
			}
			if (y < min_y) {
				min_y = y;
			}
			if (y > max_y) {
				max_y = y;
			}
		}

		// we now have relative screen-space coords for the object's bounding box; here
		// we need to center the text above the BB on the top edge. The line that defines
		// this top edge is (min_x, min_y) to (max_x, min_y)
		//std::cout << min_x << ", " << min_y << " || " << max_x << ", " << max_y << std::endl;
		if ((min_x < 0) || (min_x > 1) || (min_y < 0) || (min_y > 1) || (max_x < 0) || (max_x > 1) || (max_y < 0) || (max_y > 1)) {
			//m_pContainer->setEnabled(false);
			m_pContainer->hide();
		} else {
			//m_pContainer->setEnabled(true);			
			m_pContainer->show();
			m_pContainer->setPosition(1-max_x, min_y);  // Edited by alberts: This code works for me
			m_pContainer->setDimensions(max_x - min_x, 0.1f); // 0.1, just "because"
		}
	}


}