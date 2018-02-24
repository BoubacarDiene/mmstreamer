//////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                              //
//              Copyright © 2016, 2017 Boubacar DIENE                                           //
//                                                                                              //
//              This file is part of mmstreamer project.                                        //
//                                                                                              //
//              mmstreamer is free software: you can redistribute it and/or modify              //
//              it under the terms of the GNU General Public License as published by            //
//              the Free Software Foundation, either version 3 of the License, or               //
//              (at your option) any later version.                                             //
//                                                                                              //
//              mmstreamer is distributed in the hope that it will be useful,                   //
//              but WITHOUT ANY WARRANTY; without even the implied warranty of                  //
//              MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the                   //
//              GNU General Public License for more details.                                    //
//                                                                                              //
//              You should have received a copy of the GNU General Public License               //
//              along with mmstreamer.  If not, see <http://www.gnu.org/licenses/>.             //
//                                                                                              //
//////////////////////////////////////////////////////////////////////////////////////////////////

/*!
* \file Controller.h
* \author Boubacar DIENE
* \version 1.0
*
* \brief mmstreamer engine can currently be controlled in two different ways:
*        - By user using graphics elements on the UI (See Graphics.xml)
*        - By an external library without any user action
*
* Such libraries are listed in Main.xml and are simply called "controllers". Controller have
* to implement this header (Controller.h)
*
* Following 4 functions are REQUIRED in each controller:
* - controller_init_f
* - controller_uninit_f
* - controller_on_command_cb
* - controller_on_event_cb
*
* \see controller_init_f
* \see controller_uninit_f
* \see controller_on_command_cb
* \see controller_on_event_cb
*
* \see https://github.com/BoubacarDiene/mmstreamer
* \see https://github.com/BoubacarDiene/mmcontroller
*/

#ifndef __CONTROLLER_H__
#define __CONTROLLER_H__

#ifdef __cplusplus
extern "C" {
#endif

/* -------------------------------------------------------------------------------------------- */
/* ////////////////////////////////////////// MACROS ////////////////////////////////////////// */
/* -------------------------------------------------------------------------------------------- */

/*!
 * \def CONTROLLER_FORMAT_INTEGER
 * \brief Expected format when sending commands that require an integer (Cf. see section)
 *        to mmstreamer engine
 *
 * Format is "extension" where extension can have the following values (However, only BMP
 * is currently supported):
 * - 0 => BMP
 * - 1 => PNG
 * - 2 => JPEG
 *
 * \see CONTROLLER_COMMAND_TAKE_SCREENSHOT
 */
#define CONTROLLER_FORMAT_INTEGER "%u"

/*!
 * \def CONTROLLER_FORMAT_STRING
 * \brief Expected format when sending commands that require a string (Cf. see section)
 *        to mmstreamer engine
 *
 * Format is "string" where value of "string" depends on command to send:
 * - "newLanguage" (Cf. Strings.xml)
 *       + CONTROLLER_COMMAND_CHANGE_LANGUAGE
 * - "nameOfGraphicsElement" (Cf. Graphics.xml)
 *       + CONTROLLER_COMMAND_SAVE_VIDEO_ELEMENT
 *       + CONTROLLER_COMMAND_HIDE_ELEMENT
 *       + CONTROLLER_COMMAND_SHOW_ELEMENT
 *       + CONTROLLER_COMMAND_SET_FOCUS
 *       + CONTROLLER_COMMAND_SET_CLICKABLE
 *       + CONTROLLER_COMMAND_SET_NOT_CLICKABLE
 * - "nameOfGraphicsGroup" (Cf. Graphics.xml)
 *       + CONTROLLER_COMMAND_HIDE_GROUP
 *       + CONTROLLER_COMMAND_SHOW_GROUP
 * - "nameOfVideoDevice" (Cf. Videos.xml)
 *       + CONTROLLER_COMMAND_STOP_VIDEO
 *       + CONTROLLER_COMMAND_START_VIDEO
 * - "nameOfServer" (Cf. Servers.xml)
 *       + CONTROLLER_COMMAND_STOP_SERVER
 *       + CONTROLLER_COMMAND_START_SERVER
 *       + CONTROLLER_COMMAND_SUSPEND_SERVER
 *       + CONTROLLER_COMMAND_RESUME_SERVER
 * - "nameOfClient" (Cf. Clients.xml)
 *       + CONTROLLER_COMMAND_STOP_CLIENT
 *       + CONTROLLER_COMMAND_START_CLIENT
 *
 * \see CONTROLLER_COMMAND_TAKE_SCREENSHOT
 * \see CONTROLLER_COMMAND_SAVE_VIDEO_ELEMENT
 * \see CONTROLLER_COMMAND_HIDE_ELEMENT
 * \see CONTROLLER_COMMAND_SHOW_ELEMENT
 * \see CONTROLLER_COMMAND_SET_FOCUS
 * \see CONTROLLER_COMMAND_SET_CLICKABLE
 * \see CONTROLLER_COMMAND_SET_NOT_CLICKABLE
 * \see CONTROLLER_COMMAND_HIDE_GROUP
 * \see CONTROLLER_COMMAND_SHOW_GROUP
 * \see CONTROLLER_COMMAND_STOP_VIDEO
 * \see CONTROLLER_COMMAND_START_VIDEO
 * \see CONTROLLER_COMMAND_STOP_SERVER
 * \see CONTROLLER_COMMAND_START_SERVER
 * \see CONTROLLER_COMMAND_SUSPEND_SERVER
 * \see CONTROLLER_COMMAND_RESUME_SERVER
 * \see CONTROLLER_COMMAND_STOP_CLIENT
 * \see CONTROLLER_COMMAND_START_CLIENT
 */
#define CONTROLLER_FORMAT_STRING "%s"

/*!
 * \def CONTROLLER_FORMAT_TEXT
 * \brief Expected format when sending commands that require text format (Cf. see section)
 *        to mmstreamer engine
 *
 * Format is "elementName;stringId;fontId;fontSize;colorId" where "elementName" is the name
 * of the graphics element whose characteristics are being updated (Cf. Graphics.xml)
 *
 * For "stringId": Cf. Strings.xml / "fontId": Cf. Fonts.xml / "colorId": Cf. Colors.xml
 *
 * \see CONTROLLER_COMMAND_UPDATE_TEXT
 */
#define CONTROLLER_FORMAT_TEXT "%s;%u;%u;%u;%u"

/*!
 * \def CONTROLLER_FORMAT_IMAGE
 * \brief Expected format when sending commands that require image format (Cf. see section)
 *        to mmstreamer engine
 *
 * Format is "elementName;imageId;hidenColorId" where "elementName" is the name of
 * the graphics element whose characteristics are being updated (Cf. Graphics.xml)
 *
 * For "imageId": Cf. Images.xml / "hidenColorId": Cf. Colors.xml
 *
 * \see CONTROLLER_COMMAND_UPDATE_IMAGE
 */
#define CONTROLLER_FORMAT_IMAGE "%s;%u;%d"

/*!
 * \def CONTROLLER_FORMAT_NAV
 * \brief Expected format when sending commands that require nav format (Cf. see section)
 *        to mmstreamer engine
 *
 * Format is "elementName;left;up;right;down" where "elementName" is the name of
 * the graphics element whose characteristics are being updated (Cf. Graphics.xml)
 *
 * left : Name of graphics element to move to when MOVE_LEFT request is received
 * up   : Name of graphics element to move to when MOVE_UP request is received
 * right: Name of graphics element to move to when MOVE_RIGHT request is received
 * down : Name of graphics element to move to when MOVE_DOWN request is received
 *
 * \see CONTROLLER_COMMAND_UPDATE_NAV
 * \see controller_gfx_e
 */
#define CONTROLLER_FORMAT_NAV "%s;%s;%s;%s;%s"

/*!
 * \def CONTROLLER_FORMAT_GFX
 * \brief Expected format when sending commands that require gfx format (Cf. see section)
 *        to mmstreamer engine
 *
 * Format is "type;x;y" where "type" is of type controller_gfx_e and "x;y" represents
 * the position of the graphics element which is concerned by the request. Please, have
 * a look to Graphics.xml for more details about "x;y"
 *
 * \note Currently, "x;y" is ignored except for type=CONTROLLER_GFX_CLICK
 *
 * \see CONTROLLER_COMMAND_SEND_GFX_EVENT
 * \see controller_gfx_e
 */
#define CONTROLLER_FORMAT_GFX "%u;%u;%u"

/* -------------------------------------------------------------------------------------------- */
/* ////////////////////////////////////////// TYPES /////////////////////////////////////////// */
/* -------------------------------------------------------------------------------------------- */

/*!
 * \enum controller_error_e
 * \brief List of errors a controller can return
 */
enum controller_error_e {
    CONTROLLER_ERROR_NONE,   /**< No error occurred */
    CONTROLLER_ERROR_INIT,   /**< Initializing controller has failed */
    CONTROLLER_ERROR_UNINIT, /**< Uninitializing controller has failed */
    CONTROLLER_ERROR_PARAMS, /**< Bad params are provided */
    CONTROLLER_ERROR_UNKNOWN /**< All other errors */
};

/*!
 * \enum controller_command_e
 * \brief List of commands that can be sent to mmstreamer engine
 *
 * \see controller_gfx_e
 * \see controller_command_s
 * \see sendToEngine
 */
enum controller_command_e {
    CONTROLLER_COMMAND_CLOSE_APPLICATION,  /**< No data expected */
    CONTROLLER_COMMAND_CHANGE_LANGUAGE,    /**< See CONTROLLER_FORMAT_STRING */

    CONTROLLER_COMMAND_SAVE_VIDEO_ELEMENT, /**< See CONTROLLER_FORMAT_STRING */
    CONTROLLER_COMMAND_TAKE_SCREENSHOT,    /**< See CONTROLLER_FORMAT_INTEGER */

    CONTROLLER_COMMAND_HIDE_ELEMENT,       /**< See CONTROLLER_FORMAT_STRING */
    CONTROLLER_COMMAND_SHOW_ELEMENT,       /**< See CONTROLLER_FORMAT_STRING */
    CONTROLLER_COMMAND_SET_FOCUS,          /**< See CONTROLLER_FORMAT_STRING */
    CONTROLLER_COMMAND_HIDE_GROUP,         /**< See CONTROLLER_FORMAT_STRING */
    CONTROLLER_COMMAND_SHOW_GROUP,         /**< See CONTROLLER_FORMAT_STRING */
    CONTROLLER_COMMAND_SET_CLICKABLE,      /**< See CONTROLLER_FORMAT_STRING */
    CONTROLLER_COMMAND_SET_NOT_CLICKABLE,  /**< See CONTROLLER_FORMAT_STRING */

    CONTROLLER_COMMAND_STOP_GRAPHICS,      /**< No data expected */
    CONTROLLER_COMMAND_START_GRAPHICS,     /**< No data expected */

    CONTROLLER_COMMAND_STOP_VIDEO,         /**< See CONTROLLER_FORMAT_STRING */
    CONTROLLER_COMMAND_START_VIDEO,        /**< See CONTROLLER_FORMAT_STRING */

    CONTROLLER_COMMAND_STOP_SERVER,        /**< See CONTROLLER_FORMAT_STRING */
    CONTROLLER_COMMAND_START_SERVER,       /**< See CONTROLLER_FORMAT_STRING */
    CONTROLLER_COMMAND_SUSPEND_SERVER,     /**< See CONTROLLER_FORMAT_STRING */
    CONTROLLER_COMMAND_RESUME_SERVER,      /**< See CONTROLLER_FORMAT_STRING */

    CONTROLLER_COMMAND_STOP_CLIENT,        /**< See CONTROLLER_FORMAT_STRING */
    CONTROLLER_COMMAND_START_CLIENT,       /**< See CONTROLLER_FORMAT_STRING */

    CONTROLLER_COMMAND_UPDATE_TEXT,        /**< See CONTROLLER_FORMAT_TEXT */
    CONTROLLER_COMMAND_UPDATE_IMAGE,       /**< See CONTROLLER_FORMAT_IMAGE */
    CONTROLLER_COMMAND_UPDATE_NAV,         /**< See CONTROLLER_FORMAT_NAV */

    CONTROLLER_COMMAND_SEND_GFX_EVENT      /**< See CONTROLLER_FORMAT_GFX */
};

/*!
 * \enum controller_gfx_e
 * \brief Used when sending CONTROLLER_COMMAND_SEND_GFX_EVENT command to mmstreamer
 *        engine
 *
 * All CONTROLLER_GFX_MOVE_XXX actions are performed based on the graphics element
 * that currently has focus so <x;y> is ignored.
 * For CONTROLLER_GFX_CLICK, <x;y> is required because it helps to find where on the
 * UI a click has to be simulated
 *
 * \note Graphics module has to be active for this to be useful.
 *
 * \see controller_command_e
 * \see CONTROLLER_FORMAT_GFX
 */
enum controller_gfx_e {
    CONTROLLER_GFX_MOVE_LEFT,  /**< Move to element at left of focused graphics element */
    CONTROLLER_GFX_MOVE_UP,    /**< Move to element above focused graphics element */
    CONTROLLER_GFX_MOVE_RIGHT, /**< Move to element right to focused graphics element */
    CONTROLLER_GFX_MOVE_DOWN,  /**< Move to element below focused graphics element */
    CONTROLLER_GFX_CLICK       /**< Perform a click on graphics element at point <x;y> */
};

/*!
 * \enum controller_event_e
 * \brief Events your controller can subscribe to so as to get notified when
 *        they occur
 *
 * \see registerEvents
 * \see unregisterEvents
 */
enum controller_event_e {
    CONTROLLER_EVENT_STOPPED   = 1 << 0, /**< Component (server, client, ...) stopped */
    CONTROLLER_EVENT_STARTED   = 1 << 1, /**< Component (video, graphics, ...) started */
    CONTROLLER_EVENT_SUSPENDED = 1 << 2, /**< Server is suspended */
    CONTROLLER_EVENT_CLICKED   = 1 << 3, /**< A click occurred on a graphics element */

    CONTROLLER_EVENT_ALL       = 0xFF
};

/*!
 * \struct controller_event_s
 * \brief Used to notify your controller when event with "id" occurred
 *
 * According to "id", "name" can represent the name of the module whose
 * state has changed (STOPPED, STARTED, SUSPENDED) or the one of the graphics
 * element on which the user has clicked
 *
 * \note See xml config files fr more details about modules and graphics
 *       elements defined name
 */
struct controller_event_s {
    enum controller_event_e id;
    char                    *name;
};

/*!
 * \struct controller_command_s
 * \brief Used by your controller to send "data" to mmstreamer engine
 * \note The format of "data" depends on "id"
 *
 * \see controller_command_e
 * \see sendToEngine
 */
struct controller_command_s {
    enum controller_command_e id;
    char                      *data;
};

/*!
 * \struct controller_library_s
 * \brief Used to send "data" to controller named "valueOf(name)"
 * \note "data" can contain anything you want and can also be dynamically
 *       allocated but then "actionDoneCb" should be provided
 * \warning Obviously, the receiver must be able to know how to decode "data"
 *
 * \see sendToLibrary
 */
struct controller_library_s {
    char *name;
    void *data;
};

/*!
 * \struct controller_functions_s
 * \brief A set of functions + "enginePrivateData" provided by mmstreamer engine
 *        to your controller when initializing it
 */
struct controller_functions_s {
    /*!
     * Register events your controller is interested in
     *
     * <u>Example</u>: Subscribe to EVENT_STOPPED and EVENT_SUSPENDED
     * \code
     * int eventsMask = (CONTROLLER_EVENT_STOPPED | CONTROLLER_EVENT_SUSPENDED);
     * fcts->registerEvents(enginePrivateData, eventsMask);
     * \endcode
     *
     * \see controller_event_e
     */
    void (*registerEvents)(void *enginePrivateData, int eventsMask);

    /*!
     * Unregister all events contained in eventsMask
     *
     * <u>Example</u>: Unsubscribe from all previously registered events
     * \code
     * int eventsMask = CONTROLLER_EVENT_ALL;
     * fcts->unregisterEvents(enginePrivateData, eventsMask);
     * \endcode
     *
     * \see controller_event_e
     */
    void (*unregisterEvents)(void *enginePrivateData, int eventsMask);

    /*!
     * Send a command to mmstreamer engine
     *
     * A callback can be provided so as to get notified once the action is
     * performed. That could be useful in case some final actions are required
     * such as freeing resources, sending another command that is dependent on
     * the currently being handled, ...
     *
     * <u>Example</u>: Close application
     * \code
     * struct controller_command_s command = {
     *     .id   = CONTROLLER_COMMAND_CLOSE_APPLICATION,
     *     .data = NULL
     * };
     * fcts->sendToEngine(enginePrivateData, &command, NULL);
     * \endcode
     *
     * \see controller_command_s
     */
    void (*sendToEngine)(void *enginePrivateData,
                         struct controller_command_s *command,
                         void (*actionDoneCb)(struct controller_command_s *command));

    /*!
     * Talk to another controller through mmstreamer engine
     *
     * A callback can be provided so as to get notified once the action is
     * performed. That could be useful in case some final actions are required
     * such as freeing resources, sending another command that is dependent on
     * the currently being handled, ...
     *
     * <u>Example</u>: Send some data to controller named "controller2"
     * \code
     * void actionDoneCb(struct controller_library_s *library)
     * {
     *     knownByController2_t *ptr = (knownByController2_t*)(library->data);
     *     free(ptr);
     * }
     *
     * knownByController2_t *ptr = calloc(1, sizeof(knownByController2_t));
     * // Fill in ptr ...
     *
     * struct controller_library_s library = {
     *     .name = "controller2",
     *     .data = ptr
     * };
     * fcts->sendToLibrary(enginePrivateData, &library, actionDoneCb);
     * \endcode
     *
     * \see controller_library_s
     */
    void (*sendToLibrary)(void *enginePrivateData,
                          struct controller_library_s *library,
                          void (*actionDoneCb)(struct controller_library_s *library));

    /*!
     * mmstreamer engine's private data to be provided back when calling above
     * functions
     */
    void *enginePrivateData;
};

/* -------------------------------------------------------------------------------------------- */
/* /////////////////////////////////////// MAIN CONTEXT /////////////////////////////////////// */
/* -------------------------------------------------------------------------------------------- */

/*!
 * \struct controller_s
 * \brief It can be used to store some private data that might be useful to
 *        your controller
 *
 * controller_s has to be defined in your controller's source code. On init,
 * just fill in the given "object" with data you want to get back when being
 * notified.
 *
 * Initialized by a call to:
 *     enum controller_error_e (*init)(struct controller_s **obj,
 *                                     const struct controller_functions_s * const fcts)
 *
 * Uninitialized by a call to:
 *     enum controller_error_e (*uninit)(struct controller_s **obj) 
 */
struct controller_s;

/* -------------------------------------------------------------------------------------------- */
/* /////////////////////////// MUST BE IMPLEMENTED BY EACH CONTROLLER ///////////////////////// */
/* -------------------------------------------------------------------------------------------- */

/*!
 * \fn enum controller_error_e (*controller_init_f)(struct controller_s **obj,
 *                                            const struct controller_functions_s * const fcts)
 * \brief It's the first called function
 *
 * Once a controller is loaded by mmstreamer engine, this latter uses "init" to make controller
 * initialize its private data (controller_s) and store "fcts" that allows it to send requests
 * to engine
 */
typedef enum controller_error_e (*controller_init_f)(struct controller_s **obj,
                                               const struct controller_functions_s * const fcts);

/*!
 * \fn enum controller_error_e (*controller_uninit_f)(struct controller_s **obj)
 * \brief It's the last called function
 *
 * When mmstreamer engine is stopping, it has to unload all previously loaded controllers so
 * "uninit" is first called to make controllers stop properly (release resources, ...)
 */
typedef enum controller_error_e (*controller_uninit_f)(struct controller_s **obj);

/*!
 * \fn void (*controller_on_command_cb)(struct controller_s *obj, const void * const data)
 * \brief It's used by mmstreamer engine to send you commands from other controllers
 */
typedef void (*controller_on_command_cb)(struct controller_s *obj, const void * const data);

/*!
 * \fn void (*controller_on_event_cb)(struct controller_s *obj,
                                       const struct controller_event_s * const event)
 * \brief It's used by mmstreamer engine to send you notifications about modules' state
*         (video stopped, server suspended, ...) and clicks on a graphics element in UI
 */
typedef void (*controller_on_event_cb)(struct controller_s *obj,
                                       const struct controller_event_s * const event);

#ifdef __cplusplus
}
#endif

#endif //__CONTROLLER_H__
