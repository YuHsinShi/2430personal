#ifndef ITH_LCD_H
#define ITH_LCD_H

/** @addtogroup ith ITE Hardware Library
 *  @{
 */
/** @addtogroup ith_lcd LCD
 *  @{
 */

#ifdef __cplusplus
extern "C" {
#endif

typedef enum
{
    ITH_LCD_RGB565   = 0,    ///< RGB565 format
    ITH_LCD_ARGB1555 = 1,    ///< ARGB1555 format
    ITH_LCD_ARGB4444 = 2,    ///< ARGB4444 format
    ITH_LCD_ARGB8888 = 3,    ///< ARGB8888 format
    ITH_LCD_YUV      = 4     ///< YUV format
} ITHLcdFormat;

/**
 * Resets LCD module.
 */
void ithLcdReset(void);

/**
 * Loads the first part of LCD initial script.
 *
 * @param script the script array.
 * @param count the item count in the script array.
 */
void ithLcdLoadScriptFirst(const uint32_t *script, unsigned int count);

/**
 * Runs the next part of LCD initial script.
 */
void ithLcdLoadScriptNext(void);

/**
 * Enables LCD module.
 */
void ithLcdEnable(void);

/**
 * Disables LCD module.
 */
void ithLcdDisable(void);

/**
 * Sets LCD base address A.
 *
 * @param addr the address to set.
 */
void ithLcdSetBaseAddrA(uint32_t addr);

/**
 * Gets LCD base address A.
 *
 * @return the address A.
 */
static inline uint32_t ithLcdGetBaseAddrA(void)
{
    // this function must be an inline function, because lcd_clear.c will use it.
    // don't modify it.
    return (ithReadRegA(ITH_LCD_BASE + ITH_LCD_BASEA_REG) & ITH_LCD_BASEA_MASK);
}

/**
 * Sets LCD base address A.
 *
 * @param addr the address to set.
 */
void ithLcdSetBaseAddrB(uint32_t addr);

/**
 * Gets LCD base address B.
 *
 * @return the address B.
 */
uint32_t ithLcdGetBaseAddrB(void);

/**
 * Sets LCD base address C.
 *
 * @param addr the address to set.
 */
void ithLcdSetBaseAddrC(uint32_t addr);

/**
 * Gets LCD base address C.
 *
 * @return the address C.
 */
uint32_t ithLcdGetBaseAddrC(void);

/**
 * Enables hardware flip.
 */
void ithLcdEnableHwFlip(void);

/**
 * Disables hardware flip.
 */
void ithLcdDisableHwFlip(void);

/**
 * Enables video flip.
 */
void ithLcdEnableVideoFlip(void);

/**
 * Disables video flip.
 */
void ithLcdDisableVideoFlip(void);

/**
 * Gets current LCD format.
 *
 * @return the LCD format.
 */
ITHLcdFormat ithLcdGetFormat(void);

/**
 * Gets current LCD width.
 *
 * @return the LCD width.
 */
static inline unsigned int ithLcdGetWidth(void)
{
    return (ithReadRegA(ITH_LCD_BASE + ITH_LCD_WIDTH_REG) & ITH_LCD_WIDTH_MASK) >> ITH_LCD_WIDTH_BIT;
}

/**
 * Sets current LCD width.
 *
 * @param width the LCD width.
 */
void ithLcdSetWidth(uint32_t width);

/**
 * Gets current LCD height.
 *
 * @return the LCD height.
 */
static inline unsigned int ithLcdGetHeight(void)
{
    // this function must be an inline function, because lcd_clear.c will use it.
    // don't modify it.
    return (ithReadRegA(ITH_LCD_BASE + ITH_LCD_HEIGHT_REG) & ITH_LCD_HEIGHT_MASK) >> ITH_LCD_HEIGHT_BIT;
}

/**
 * Sets current LCD height.
 *
 * @param width the LCD height.
 */
void ithLcdSetHeight(uint32_t height);

/**
 * Gets current LCD pitch.
 *
 * @return the LCD pitch.
 */
static inline unsigned int ithLcdGetPitch(void)
{
    // this function must be an inline function, because lcd_clear.c will use it.
    // don't modify it.
    return (ithReadRegA(ITH_LCD_BASE + ITH_LCD_PITCH_REG) & ITH_LCD_PITCH_MASK) >> ITH_LCD_PITCH_BIT;
}

/**
 * Sets current LCD pitch.
 *
 * @param width the LCD pitch.
 */
void ithLcdSetPitch(uint32_t pitch);

/**
 * Gets current LCD scanline on X coordinate.
 *
 * @return the LCD scanline on X coordinate.
 */
unsigned int ithLcdGetXCounter(void);

/**
 * Gets current LCD scanline on Y coordinate.
 *
 * @return the LCD scanline on Y coordinate.
 */
unsigned int ithLcdGetYCounter(void);

/**
 * Synchronize fire LCD.
 */
void ithLcdSyncFire(void);

/**
 * Is LCD synchronize fire or not.
 *
 * @return Is LCD synchronize fire or not.
 */
bool ithLcdIsSyncFired(void);

/**
 * Is LCD enabled or not.
 *
 * @return Is LCD enabled or not.
 */
bool ithLcdIsEnabled(void);

/**
 * Gets current LCD flip counter.
 *
 * @return the flip counter. 0 is A, 1 is B, 2 is C.
 */
unsigned int ithLcdGetFlip(void);

unsigned int ithLcdGetMaxLcdBufCount(void);

/**
 * Software flip the LCD.
 *
 * @param index the index to flip. 0 is A, 1 is B, 2 is C.
 */
void ithLcdSwFlip(unsigned int index);
// LCD Cursor

/**
 * Cursor control definition
 */
typedef enum
{
    ITH_LCD_CURSOR_ALPHABLEND_ENABLE = 0,     ///< Enable hardware cursor alpha blending function
    ITH_LCD_CURSOR_DEFDEST_ENABLE,            ///< Enable hardware cursor default destination data (RGB565)
    ITH_LCD_CURSOR_INVDEST_ENABLE             ///< Inverse hardware cursor re-define dstination color when hardware cursor is "11"
} ITHLcdCursorCtrl;

/**
 * Cursor color definition
 */
typedef enum
{
    ITH_LCD_CURSOR_DEF_COLOR = 0,       ///< Hardware cursor default color (RGB565)
    ITH_LCD_CURSOR_FG_COLOR,            ///< Hardware cursor foreground color (RGB565)
    ITH_LCD_CURSOR_BG_COLOR             ///< Hardware cursor background color (RGB565)
} ITHLcdCursorColor;

/**
 * Enables hardware cursor
 */
void ithLcdCursorEnable(void);

/**
 * Disables hardware cursor
 */
void ithLcdCursorDisable(void);

/**
 * Enables specified cursor controls.
 *
 * @param ctrl the controls to enable.
 */
void ithLcdCursorCtrlEnable(ITHLcdCursorCtrl ctrl);

/**
 * Disables specified cursor controls.
 *
 * @param ctrl the controls to disable.
 */
void ithLcdCursorCtrlDisable(ITHLcdCursorCtrl ctrl);

/**
 * Sets the width of cursor.
 *
 * @param width the width.
 */
void ithLcdCursorSetWidth(unsigned int width);

/**
 * Sets the height of cursor.
 *
 * @param height the height.
 */
void ithLcdCursorSetHeight(unsigned int height);

/**
 * Sets the pitch of cursor.
 *
 * @param pitch the pitch.
 */
void ithLcdCursorSetPitch(unsigned int pitch);

/**
 * Sets the X coordinate of cursor.
 *
 * @param x the X coordinate.
 */
void ithLcdCursorSetX(unsigned int x);

/**
 * Sets the Y coordinate of cursor.
 *
 * @param y the Y coordinate.
 */
void ithLcdCursorSetY(unsigned int y);

/**
 * Sets base address of cursor.
 *
 * @param addr the base address to set.
 */
void ithLcdCursorSetBaseAddr(uint32_t addr);

/**
 * Sets the color of cursor.
 *
 * @param color the color type to set.
 * @param value the color value to set.
 */
void ithLcdCursorSetColor(ITHLcdCursorColor color, uint16_t value);

/**
 * Sets the color weight of cursor.
 *
 * @param color the color type to set.
 * @param value the weight value to set.
 */
void ithLcdCursorSetColorWeight(ITHLcdCursorColor color, uint8_t value);

/**
 * Updates the cursor.
 */
void ithLcdCursorUpdate(void);

/**
 * Is cursor updated or not.
 *
 * @return Is cursor updated or not.
 */
bool ithLcdCursorIsUpdateDone(void);

// LCD Interrupt
/**
 * LCD interupt controls definition.
 */
typedef enum
{
    ITH_LCD_INTR_ENABLE = 0,   ///< Enable LCD Interrupt
    ITH_LCD_INTR_OUTPUT2,      ///< Interrupt from output2 (9070)
    ITH_LCD_INTR_FIELDMODE2,   ///< Output2 Interrupt on field mode (9070)
    ITH_LCD_INTR_OUTPUT1,      ///< Interrupt from output1 (9070)
    ITH_LCD_INTR_FIELDMODE1,   ///< output1 Interrupt on field mode (9070/9850)
} ITHLcdIntrCtrl;

/**
 * Enables specified interrupt controls.
 *
 * @param ctrl the controls to enable.
 */
void ithLcdIntrCtrlEnable(ITHLcdIntrCtrl ctrl);

/**
 * Disables specified interrupt controls.
 *
 * @param ctrl the controls to disable.
 */
void ithLcdIntrCtrlDisable(ITHLcdIntrCtrl ctrl);

/**
 * Enables LCD interrupt
 */
void ithLcdIntrEnable(void);

/**
 * Disables LCD interrupt
 */
void ithLcdIntrDisable(void);

/**
 * Clears LCD interrupt
 */
void ithLcdIntrClear(void);

/**
 * Sets the first scanline to interrupt.
 *
 * @param line the scanline to interrupt.
 */
void ithLcdIntrSetScanLine1(unsigned int line);

/**
 * Sets the second scanline to interrupt.
 *
 * @param line the scanline to interrupt.
 */
void ithLcdIntrSetScanLine2(unsigned int line);

typedef enum
{
    ITH_LCD_ROT0   = 0,  ///< No Rotation
    ITH_LCD_FLIP   = 1,  ///< Flip
    ITH_LCD_MIRROR = 2,  ///< Mirror
    ITH_LCD_ROT180 = 3,  ///< Rotation 180
} ITHLcdRotMode;

typedef enum
{
    ITH_LCD_Horizontal = 0,  ///< Horizontal
    ITH_LCD_Vertical   = 1,  ///< Vertical
} ITHLcdScanType;

typedef enum
{
    ITH_LCD_RGB  = 0, ///< RGB panel
    ITH_LCD_MIPI = 1, ///< MIPI panel
    ITH_LCD_LVDS = 2, ///< LVDS panel
} ITHLcdPanelType;

/**
 * LCD rotation display Mode
 *
 * @param type the LCD panel scan type.
 * @param mode the rotation mode.
 */
void ithLcdSetRotMode(ITHLcdScanType type, ITHLcdRotMode mode);

/**
 * LCD get panel Type
 *
 * @return type of LCD panel.
 */
ITHLcdPanelType ithLcdGetPanelType(void);

#ifndef _WIN32
/**
* Init IT6122 for MIPI to LVDS.
*/
bool ithLcdInitIT6122(void);

/**
* Dump IT6122 Registers.
*/
void ithLcdDumpRegIT6122(void);

/**
* Init IT6151 for MIPI to EDP.
*/
void ithLcdInitIT6151(void);

/**
* Dump IT6151 Registers.
*/
void ithLcdDumpRegIT6151(void);

/**
* Init IT6151 check ESD task.
*/
void ithLcdInitCheckTaskIT6151(void);
#endif  //#ifndef _WIN32

#ifdef __cplusplus
}
#endif

#endif // ITH_LCD_H
/** @} */ // end of ith_lcd
/** @} */ // end of ith