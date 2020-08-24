/** @file
 * Template scene definition.
 *
 */
/** @defgroup template
 *  @{
 */
#ifndef SCENE_H
#define SCENE_H

#include "ite/itu.h"

#ifdef __cplusplus
extern "C" {
#endif

/** @defgroup template_scene
 *  @{
 */

#define MS_PER_FRAME    17  ///< Drawing delay per frame


typedef enum
{
	EVENT_CUSTOM_SCREENSAVER = ITU_EVENT_CUSTOM,    ///< Ready to enter screensaver mode. Custom0 event on GUI Designer.
	EVENT_CUSTOM_SD_INSERTED,                       ///< #1: SD card inserted.
	EVENT_CUSTOM_SD_REMOVED,                        ///< #2: SD card removed.
	EVENT_CUSTOM_USB_INSERTED,                      ///< #3: USB drive inserted.
	EVENT_CUSTOM_USB_REMOVED,                       ///< #4: USB drive removed.
	EVENT_CUSTOM_KEY0,                              ///< #5: Key #0 pressed.
	EVENT_CUSTOM_KEY1,                              ///< #6: Key #1 pressed.
	EVENT_CUSTOM_KEY2,                              ///< #7: Key #2 pressed.
	EVENT_CUSTOM_KEY3,                              ///< #8: Key #3 pressed.
	EVENT_CUSTOM_UART                               ///< #9: UART message.

} CustomEvent;

/**
 * Global instance variable of scene.
 */
extern ITUScene theScene;

// scene
/**
 * Initializes scene module.
 */
void SceneInit(void);

/**
 * Exits scene module.
 */
void SceneExit(void);

/**
 * Loads ITU file.
 */
void SceneLoad(void);

/**
 * Runs the main loop to receive events, update and draw scene.
 *
 * @return The QuitValue.
 */
int SceneRun(void);

/** @} */ // end of template_scene

#ifdef __cplusplus
}
#endif

#endif /* SCENE_H */

/** @} */ // end of template
