#ifndef INPUTMANAGER_H_
#define INPUTMANAGER_H_

//Include OIS but prevent double link errors!
#define OIS_DYNAMIC_LIB
#include <OIS.h>

#include <OgreRenderWindow.h>
#include <string>
#include "KeyManager.h"

namespace HovUni {

/**
 * The input manager to capture and process the input.
 *
 * @author Kristof Overdulve / adjusted from the Ogre Wiki
 */
class InputManager : public OIS::KeyListener, public OIS::MouseListener, public OIS::JoyStickListener {
protected:

	/** Mouse input object */
	OIS::Mouse * mMouse;

	/** Keyboard input object */
    OIS::Keyboard * mKeyboard;

	/** Input system */
    OIS::InputManager *mInputSystem;

	/** The joysticks available */
    std::vector<OIS::JoyStick *> mJoysticks;

	/** The key listeners registered */
	std::map<std::string, OIS::KeyListener *> mKeyListeners;

	/** The mouse listeners registered */
    std::map<std::string, OIS::MouseListener *> mMouseListeners;

	/** The joystick listeners registered */
    std::map<std::string, OIS::JoyStickListener *> mJoystickListeners;

	/** The key manager */
	KeyManager * mKeyManager;

	/** The singleton object */
    static InputManager * mInputManager;

public:
	/**
	 * Destructor.
	 */
    virtual ~InputManager( void );

	/**
	 * Initialize the Ogre render window.
	 *
	 * @param renderWindow the window in which rendering occurs
	 */
    void initialise(Ogre::RenderWindow * renderWindow);

	/**
	 * Indicates that the input devices should be captured.
	 */
    void capture(void);

	/**
	 * Adds a key listener to the instance with the given name.
	 *
	 * @param keyListener the key listener to add
	 * @param instanceName the instance that is listening to the keys
	 */
    void addKeyListener(OIS::KeyListener * keyListener, const std::string & instanceName);

	/**
	 * Adds a mouse listener to the instance with the given name.
	 *
	 * @param mouseListener the mouse listener to add
	 * @param instanceName the instance that is listening to the mouse
	 */
    void addMouseListener(OIS::MouseListener * mouseListener, const std::string & instanceName );

	/**
	 * Adds a joystick listener to the instance with the given name.
	 *
	 * @param joystickListener the joystick listener to add
	 * @param instanceName the instance that is listening to the joystick
	 */
    void addJoystickListener(OIS::JoyStickListener * joystickListener, const std::string & instanceName);

	/**
	 * Removes the key listener for the given instance.
	 *
	 * @param instanceName the instance that will stop listening to keys
	 */
    void removeKeyListener(const std::string & instanceName);

	/**
	 * Removes the mouse listener for the given instance.
	 *
	 * @param instanceName the instance that will stop listening to the mouse
	 */
    void removeMouseListener(const std::string & instanceName);

	/**
	 * Removes the joystick listener for the given instance.
	 *
	 * @param instanceName the instance that will stop listening to the joystick
	 */
    void removeJoystickListener(const std::string & instanceName);

	/**
	 * Removes the given key listener.
	 *
	 * @param keyListener the listener to remove
	 */
    void removeKeyListener(OIS::KeyListener * keyListener);

	/**
	 * Removes the given mouse listener.
	 *
	 * @param mouseListener the listener to remove
	 */
    void removeMouseListener(OIS::MouseListener * mouseListener);

	/**
	 * Removes the given joystick listener.
	 *
	 * @param joystickListener the listener to remove
	 */
    void removeJoystickListener(OIS::JoyStickListener * joystickListener);

	/**
	 * Removes all the listeners.
	 */
    void removeAllListeners(void);

	/**
	 * Removes all the key listeners.
	 */
    void removeAllKeyListeners(void);

	/**
	 * Removes all the mouse listeners.
	 */
    void removeAllMouseListeners(void);

	/**
	 * Removes all the joystick listeners.
	 */
    void removeAllJoystickListeners(void);

	/**
	 * Sets the windows extents.
	 *
	 * @param width the width of the window
	 * @param height the height of the window
	 */
    void setWindowExtents(int width, int height);

	/** 
	 * Returns the mouse object.
	 *
	 * @return the mouse object
	 */
    OIS::Mouse * getMouse(void);

	/**
	 * Returns the keyboard object.
	 *
	 * @return the keyboard object
	 */
    OIS::Keyboard * getKeyboard(void);

	/**
	 * Returns the joystick object.
	 *
	 * @return the joystick object
	 */
    OIS::JoyStick * getJoystick(unsigned int index);

	/**
	 * Returns the number of joysticks.
	 *
	 * @return the number of joysticks
	 */
    int getNumOfJoysticks(void);

	/**
	 * Returns the input manager singleton.
	 *
	 * @return the singleton pointer
	 */
    static InputManager * getSingletonPtr(void);

	/**
	 * Returns the key manager.
	 *
	 * @return the key manager.
	 */
	KeyManager * getKeyManager(void);

	/**
	 * Set the location of the controls ini file
	 *
	 * @param	file	Location of Controls.ini
	 */
	void setControlsFile(std::string file);

	/**
	 * Get the location of the controls ini file
	 *
	 * @return	file	Location of Controls.ini
	 */
	std::string getControlsFile();

	/**
	 * Move the mouse to a given location
	 *
	 * @param x The x coordinate
	 * @param y The y coordinate
	 */
	void moveMouseTo(int x, int y);

private:

	/**
	 * Cannot construct this object directly.
	 */
    InputManager(void);

	/**
	 * Assignment operator.
	 *
	 * @param input manager to assign
	 * @return the new input manager
	 */
    InputManager & operator= (const InputManager&);

	/**
	 * Notifies all the listeners that the given key was pressed.
	 *
	 * @param e the key event that will be propagated to all listeners
	 * @return whether succesful
	 */
    bool keyPressed(const OIS::KeyEvent &e);

	/**
	 * Notifies all the listeners that the given key was released.
	 *
	 * @param e the key event that will be propagated to all listeners
	 * @return whether succesful
	 */
    bool keyReleased(const OIS::KeyEvent &e);

	/**
	 * Notifies all the listeners that the mouse has moved.
	 *
	 * @param e the mouse event that will be propagated to all listeners
	 * @return whether succesful
	 */
    bool mouseMoved(const OIS::MouseEvent &e);

	/**
	 * Notifies all the listeners that a mouse button has been pressed.
	 *
	 * @param e the mouse event that will be propagated to all listeners
	 * @return whether succesful
	 */
	bool mousePressed(const OIS::MouseEvent & e, OIS::MouseButtonID id);

	/**
	 * Notifies all the listeners that a mouse button has been released.
	 *
	 * @param e the mouse event that will be propagated to all listeners
	 * @return whether succesful
	 */
    bool mouseReleased(const OIS::MouseEvent & e, OIS::MouseButtonID id);

	/**
	 * Notifies all the listeners that the Point of View of the joystick has moved.
	 *
	 * @param e the joystick event that will be propagated to all listeners
	 * @return whether succesful
	 */
    bool povMoved(const OIS::JoyStickEvent & e, int pov);

	/**
	 * Notifies all the listeners that the axis of the joystick has moved.
	 *
	 * @param e the joystick event that will be propagated to all listeners
	 * @return whether succesful
	 */
    bool axisMoved(const OIS::JoyStickEvent & e, int axis);

	/**
	 * Notifies all the listeners that the slides of the joystick has moved.
	 *
	 * @param e the joystick event that will be propagated to all listeners
	 * @return whether succesful
	 */
    bool sliderMoved(const OIS::JoyStickEvent & e, int sliderID);

	/**
	 * Notifies all the listeners that a button of the joystick has been pressed.
	 *
	 * @param e the joystick event that will be propagated to all listeners
	 * @return whether succesful
	 */
    bool buttonPressed(const OIS::JoyStickEvent & e, int button);

	/**
	 * Notifies all the listeners that a button of the joystick has been released.
	 *
	 * @param e the joystick event that will be propagated to all listeners
	 * @return whether succesful
	 */
    bool buttonReleased(const OIS::JoyStickEvent & e, int button);
};

}

#endif
