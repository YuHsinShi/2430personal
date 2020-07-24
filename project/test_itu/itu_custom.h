/** @file
 * ITE Custom UI header file.
 *
 * @author Jim Tan
 * @version 1.0
 * @date 2013
 * @copyright ITE Tech. Inc. All Rights Reserved.
 */
/** @defgroup itu ITE Custom UI
 *  @{
 */
#ifndef ITE_ITU_CUSTOM_H
#define ITE_ITU_CUSTOM_H

#include "ite/itu.h"

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * Custom text widget definition. This is used for demo custom widget ability.
 */
typedef struct
{
    ITUWidget widget;               ///< Base widget definition.
    ITUColor bgColor;               ///< Background color.
    int fontHeight;                 ///< Font height.
    int fontWidth;                  ///< Font width.
    char* string;                   ///< String of the text.
    ITUStringSet* stringSet;        ///< The string set definition.
} ITUCustomText;

/**
 * Initializes the custom text widget.
 *
 * @param text The custom text widget to initialize.
 */
void ituCustomTextInit(ITUCustomText* text);

/**
 * Loads the custom text widget. This is called by scene manager.
 *
 * @param widget The custom text widget to load.
 * @param base The address in the scene file buffer.
 */
void ituCustomTextLoad(ITUWidget* widget, uint32_t base);

/**
 * Clones the custom text widget.
 *
 * @param widget The custom text widget to clone.
 * @param cloned Retrieved cloned custom text widget.
 * @return true if clone is success, false otherwise.
 */
bool ituCustomTextClone(ITUWidget* widget, ITUWidget** cloned);

/**
 * Updates the custom text widget by specified event.
 *
 * @param widget The custom text widget to update.
 * @param ev The event to notify.
 * @param arg1 The event related argument #1.
 * @param arg2 The event related argument #2.
 * @param arg3 The event related argument #3.
 * @return true if the text widget is modified and need to be redraw, false if no need to be redraw.
 */
bool ituCustomTextUpdate(ITUWidget* widget, ITUEvent ev, int arg1, int arg2, int arg3);

/**
 * Draws the custom text widget to the specified surface.
 *
 * @param widget The text widget to draw.
 * @param dest The surface to draw to.
 * @param x The x coordinate of destination surface, in pixels.
 * @param y The y coordinate of destination surface, in pixels.
 * @param alpha the alpha value to do the constant alphablending to the surface.
 */
void ituCustomTextDraw(ITUWidget* widget, ITUSurface* dest, int x, int y, uint8_t alpha);

/**
 * Do the specified action. This is triggered by other widget's event.
 *
 * @param widget The custom text widget to do the action.
 * @param action The action to do.
 * @param param The parameter of action.
 */
void ituCustomTextOnAction(ITUWidget* widget, ITUActionType action, char* param);

/**
 * Sets font width of the custom text widget.
 *
 * @param custom text Pointer referring to the text widget.
 * @param width The font width to set.
 */
void ituCustomTextSetFontWidth(ITUCustomText* text, int width);

/**
 * Sets font height of the custom text widget.
 *
 * @param text Pointer referring to the custom text widget.
 * @param height The font height to set.
 */
void ituCustomTextSetFontHeight(ITUCustomText* text, int height);

/**
 * Sets font size of the custom text widget.
 *
 * @param text Pointer referring to the custom text widget.
 * @param size The font size to set.
 */
void ituCustomTextSetFontSize(ITUCustomText* text, int size);

/**
 * Sets string of the custom text widget. This is the implementation of custom text widget.
 *
 * @param text Pointer referring to the custom text widget.
 * @param string The string to set.
 */
void ituCustomTextSetStringImpl(ITUCustomText* text, char* string);

/**
 * Gets string of the custom text widget. This is the implementation of custom text widget.
 *
 * @param text Pointer referring to the custom text widget.
 * @return The string.
 */
char* ituCustomTextGetStringImpl(ITUCustomText* text);

/**
 * Sets background color of the custom text widget.
 *
 * @param text Pointer referring to the custom text widget.
 * @param alpha The alpha value.
 * @param red The red value.
 * @param green The green value.
 * @param blue The blue value.
 */
void ituCustomTextSetBackColor(ITUCustomText* text, uint8_t alpha, uint8_t red, uint8_t green, uint8_t blue);

/**
 * Resizes the custom text widget to fit the length of text.
 *
 * @param text Pointer referring to the custom text widget.
 */
void ituCustomTextResize(ITUCustomText* text);

/**
 * Sets string of the custom text widget.
 *
 * @param text Pointer referring to the custom text widget.
 * @param string The string to set.
 */
#define ituCustomTextSetString(text, string)  ituCustomTextSetStringImpl((ITUCustomText*)(text), (char*)(string))

/**
 * Gets string of the custom text widget.
 *
 * @param text Pointer referring to the custom text widget.
 * @return The string.
 */
#define ituCustomTextGetString(text)  ituCustomTextGetStringImpl((ITUCustomText*)(text))

/** @} */ // end of itu_widget_custom_text

#ifdef __cplusplus
}
#endif

#endif // ITE_ITU_CUSTOM_H
/** @} */ // end of custom itu
