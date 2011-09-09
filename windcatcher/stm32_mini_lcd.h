/**
  ******************************************************************************
  * @file    stm32_mini_lcd.h
  * @author  Martin Thomas (not STM), based on code from
  *          MCD Application Team and PowerCPU.com
  * @version V0.0.3
  * @date    28. Nov. 2009
  * @brief   This file contains all the functions prototypes for the LCD firmware driver.
  ******************************************************************************
  * @copy
  *
  * THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING USERS
  * WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
  * TIME. AS A RESULT, THE CREATOR OF THIS CODE SHALL NOT BE HELD LIABLE FOR ANY
  * DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
  * FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE
  * CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
  *
  * <h2><center>&copy; COPYRIGHT 2009 M. Thomas, STMicroelectronics</center></h2>
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef STM32_MINI_LCD_H
#define STM32_MINI_LCD_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f10x.h"

/** @addtogroup Utilities
  * @{
  */

/** @addtogroup STM32_MINI_LCD
  * @{
  */


/** @defgroup STM32_MINI_LCD_Exported_Types
  * @{
  */
/**
  * @}
  */



/** @defgroup STM32_MINI_LCD_Exported_Constants
  * @{
  */

/**
 * @brief Uncomment the line below if you want to use LCD_DrawBMP function to
 *        display a bitmap picture on the LCD. This function assumes that the bitmap
 *        file is loaded in the SPI Flash (mounted on STM3210C-EVAL board), however
 *        user can tailor it according to his application hardware requirement.
 */
/*#define USE_LCD_DrawBMP*/

/**
 * @brief Uncomment the line below if you want to use user defined Delay function
 *        (for precise timing), otherwise default _delay_ function defined within
 *         this driver is used (less precise timing).
 */
#define LCD_USE_Delay

#ifdef LCD_USE_Delay
#include "timebase.h"

  #define _delay_(n__)  timebase_delay_ms(n__*10)  /* !< delay n__*10ms  */
#else
  #define _delay_     delay      /* !< Default _delay_ function with less precise timing */
#endif

/**
 * @brief  LCD orientation and limits
 */

typedef enum {
	LCD_PORTRAIT_TOP_DOWN,
	LCD_PORTRAIT_BOTTOM_UP,
	LCD_LANDSCAPE_TOP_DOWN,
	LCD_LANDSCAPE_BOTTOM_UP
} LCD_OrientationMode_t;

#define LCD_WIDTH_HW         240
#define LCD_HEIGHT_HW        320

#define LCD_PORTRAIT           0
#define LCD_LANDSCAPE          1
// #define LCD_ORIENTATION        LCD_PORTRAIT
#define LCD_ORIENTATION        LCD_LANDSCAPE

#if LCD_ORIENTATION == LCD_PORTRAIT
#define LCD_WIDTH              240
#define LCD_HEIGHT             320
#define LCD_CHARS_PER_LINE      10
#define LCD_CHAR_LINES          13
#define LCD_ENTRY_MODE_DEFAULT 0x1030 /* Entry Mode BGR=1,AM=0,ID1/0=1/1 */
#define LCD_ORIENTATION_DEFAULT LCD_PORTRAIT_TOP_DOWN
#else
#define LCD_WIDTH              320
#define LCD_HEIGHT             240
#define LCD_CHARS_PER_LINE      15
#define LCD_CHAR_LINES          10
#define LCD_ENTRY_MODE_DEFAULT 0x1018 /* Entry Mode BGR=1,AM=1,ID1/0=1/0 */
#define LCD_ORIENTATION_DEFAULT LCD_LANDSCAPE_TOP_DOWN
#endif

/**
  * @brief  LCD Control pins
  */
#define LCD_CTRL_nCS_PIN       GPIO_Pin_8
#define LCD_CTRL_RS_PIN        GPIO_Pin_9
#define LCD_CTRL_nWR_PIN       GPIO_Pin_10
#define LCD_CTRL_nRD_PIN       GPIO_Pin_11
#define LCD_CTRL_RST_PIN       GPIO_Pin_12
#define LCD_CTRL_BL_PIN        GPIO_Pin_13
#define LCD_CTRL_GPIO_PORT     GPIOC
#define LCD_CTRL_GPIO_CLK      RCC_APB2Periph_GPIOC

#define LCD_NCS_GPIO_PORT      LCD_CTRL_GPIO_PORT
#define LCD_NCS_PIN            LCD_CTRL_nCS_PIN

/**
  * @brief  LCD SPI Interface pins
  */
#define LCD_SPI_SCK_PIN        GPIO_Pin_10
#define LCD_SPI_MISO_PIN       GPIO_Pin_11
#define LCD_SPI_MOSI_PIN       GPIO_Pin_12
#define LCD_SPI_GPIO_PORT      GPIOC
#define LCD_SPI_GPIO_CLK       RCC_APB2Periph_GPIOC
#define LCD_SPI                SPI3
#define LCD_SPI_CLK            RCC_APB1Periph_SPI3

/**
  * @brief  LCD Registers
  */
#define R0             0x00
#define R1             0x01
#define R2             0x02
#define R3             0x03
#define R4             0x04
#define R5             0x05
#define R6             0x06
#define R7             0x07
#define R8             0x08
#define R9             0x09
#define R10            0x0A
#define R12            0x0C
#define R13            0x0D
#define R14            0x0E
#define R15            0x0F
#define R16            0x10
#define R17            0x11
#define R18            0x12
#define R19            0x13
#define R20            0x14
#define R21            0x15
#define R22            0x16
#define R23            0x17
#define R24            0x18
#define R25            0x19
#define R26            0x1A
#define R27            0x1B
#define R28            0x1C
#define R29            0x1D
#define R30            0x1E
#define R31            0x1F
#define R32            0x20
#define R33            0x21
#define R34            0x22
#define R36            0x24
#define R37            0x25
#define R40            0x28
#define R41            0x29
#define R43            0x2B
#define R45            0x2D
#define R48            0x30
#define R49            0x31
#define R50            0x32
#define R51            0x33
#define R52            0x34
#define R53            0x35
#define R54            0x36
#define R55            0x37
#define R56            0x38
#define R57            0x39
#define R59            0x3B
#define R60            0x3C
#define R61            0x3D
#define R62            0x3E
#define R63            0x3F
#define R64            0x40
#define R65            0x41
#define R66            0x42
#define R67            0x43
#define R68            0x44
#define R69            0x45
#define R70            0x46
#define R71            0x47
#define R72            0x48
#define R73            0x49
#define R74            0x4A
#define R75            0x4B
#define R76            0x4C
#define R77            0x4D
#define R78            0x4E
#define R79            0x4F
#define R80            0x50
#define R81            0x51
#define R82            0x52
#define R83            0x53
#define R96            0x60
#define R97            0x61
#define R106           0x6A
#define R118           0x76
#define R128           0x80
#define R129           0x81
#define R130           0x82
#define R131           0x83
#define R132           0x84
#define R133           0x85
#define R134           0x86
#define R135           0x87
#define R136           0x88
#define R137           0x89
#define R139           0x8B
#define R140           0x8C
#define R141           0x8D
#define R143           0x8F
#define R144           0x90
#define R145           0x91
#define R146           0x92
#define R147           0x93
#define R148           0x94
#define R149           0x95
#define R150           0x96
#define R151           0x97
#define R152           0x98
#define R153           0x99
#define R154           0x9A
#define R157           0x9D
#define R192           0xC0
#define R193           0xC1
#define R229           0xE5

/**
  * @brief  LCD color
  */
#define White          0xFFFF
#define Black          0x0000
#define Grey           0xF7DE
#define Blue           0x001F
#define Blue2          0x051F
#define Red            0xF800
#define Magenta        0xF81F
#define Green          0x07E0
#define Cyan           0x7FFF
#define Yellow         0xFFE0

#define LCD_CHAR_HEIGHT 24
#define Line0          0
#define Line1          24
#define Line2          48
#define Line3          72
#define Line4          96
#define Line5          120
#define Line6          144
#define Line7          168
#define Line8          192
#define Line9          216
#define Line10         (Line9+LCD_CHAR_HEIGHT)
#define Line11         (Line10+LCD_CHAR_HEIGHT)
#define Line12         (Line11+LCD_CHAR_HEIGHT)
#define LineLast       ((LCD_CHAR_LINES-1)*LCD_CHAR_HEIGHT)



#define LCD_LINE_HORIZONTAL     0x00
#define LCD_LINE_VERTICAL       0x01
/**
  * @}
  */

/** @defgroup STM32_MINI_LCD_Exported_Macros
  * @{
  */
/**
  * @}
  */



/** @defgroup STM32_MINI_LCD_Exported_Functions
  * @{
  */
uint16_t LCD_Setup(void);
uint16_t LCD_Init(void);
void LCD_SetTextColor(uint16_t Color);
uint16_t LCD_GetTextColor(void);
void LCD_SetBackColor(uint16_t Color);
void LCD_ClearLine(uint8_t Line);
void LCD_Clear(uint16_t Color);
void LCD_SetCursor(uint16_t Xpos, uint16_t Ypos);
void LCD_DrawChar(uint16_t Xpos, uint16_t Ypos, const uint16_t *c);
void LCD_DisplayChar(uint16_t Line, uint16_t Column, uint8_t Ascii);
void LCD_DisplayStringLine(uint16_t Line, /*uint8_t*/ const char *ptr);
void LCD_SetDisplayWindow(uint16_t Xpos, uint16_t Ypos, uint8_t Height, uint16_t Width);
void LCD_WindowModeDisable(void);
void LCD_DrawLine(uint16_t Xpos, uint16_t Ypos, uint16_t Length, uint8_t Direction);
void LCD_DrawRect(uint16_t Xpos, uint16_t Ypos, uint16_t Height, uint16_t Width);
void LCD_DrawBox(uint16_t Xpos, uint16_t Ypos, uint16_t Height, uint16_t Width);
void LCD_DrawCircle(uint16_t Xpos, uint16_t Ypos, uint16_t Radius);
void LCD_DrawMonoPict(const uint32_t *Pict);
// #define USE_LCD_DrawBMP
#ifdef USE_LCD_DrawBMP
//void LCD_DrawBMP(uint32_t BmpAddress);
void LCD_DrawBMP(const uint16_t *BmpAddress);
#endif

void LCD_DrawPixel(uint16_t x, uint16_t y, uint16_t Color);
uint16_t LCD_GetPixel(uint16_t x, uint16_t y);

void LCD_nCS_StartByte(uint8_t Start_Byte);
void LCD_WriteRegIndex(uint8_t LCD_Reg);
void LCD_WriteReg(uint8_t LCD_Reg, uint16_t LCD_RegValue);
void LCD_WriteRAM_Prepare(void);
void LCD_WriteRAMWord(uint16_t RGB_Code);
uint16_t LCD_ReadReg(uint8_t LCD_Reg);
void LCD_WriteRAM(uint16_t RGB_Code);
void LCD_Release(void);

void LCD_PowerOn(void);
void LCD_PowerOff(void);
void LCD_DisplayOn(void);
void LCD_DisplayOff(void);

void LCD_CtrlLinesConfig(void);
void LCD_InterfaceConfig(void);
void LCD_CtrlLinesWrite(GPIO_TypeDef* GPIOx, uint16_t CtrlPins, BitAction BitVal);

void LCD_BackLight(BitAction ba);
void LCD_HardwareReset(void);

static inline uint16_t LCD_RGB2Pixel565(uint8_t r, uint8_t g, uint8_t b)
{
	/* RGB2PIXEL565 from a Macro in Greg Haerr's Nano-X, MPL license */
	return ((((r) & 0xf8) << 8) | (((g) & 0xfc) << 3) | (((b) & 0xf8) >> 3));
}

void LCD_SetOrientation(LCD_OrientationMode_t m);
LCD_OrientationMode_t LCD_GetOrientation(void);
uint16_t LCD_GetWidth(void);
uint16_t LCD_GetHeight(void);


#ifdef __cplusplus
}
#endif

#endif /* STM32_MINI_LCD_H */
/**
  * @}
  */


/**
  * @}
  */

/**
  * @}
  */
/******************* (C) COPYRIGHT 2009 mth, STMicroelectronics *****END OF FILE****/
