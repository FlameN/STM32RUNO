/**
 ******************************************************************************
 * @file    stm32_mini_lcd.c
 * @author  Martin Thomas (not STM),
 *          based on code/information from: Sitronix ST7781 data-sheet,
 *          STM MCD Application Team and PowerCPU.com
 * @version V0.0.3
 * @date    28. Nov. 2009
 * @brief   This file contains all the functions for the LCD firmware driver.
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
 * <h2><center>&copy; COPYRIGHT 2009 M. Thomas and STMicroelectronics</center></h2>
 */

/* Includes ------------------------------------------------------------------*/
#include "stm32_mini_lcd.h"
#include "fonts.h"

/* Hardware-Connection:
 STM32        TFT-Module
 -----------------------
 PB8~15 <---> DB0~7
 PC0~7  <---> DB10~17
 PC8     ---> nCS        (low=selected, high=unselected)
 PC9     ---> RS         (low=command, high=data or parameter)
 PC10    ---> nWR        (low=write)
 PC11    ---> nRD        (low=read)
 PC12    ---> nReset     (low=in reset)
 PC13    ---> Back-Light LEDs enable (NPN)

 I80-system 16-bit interface(?)
 IM3 IM2 IM1 IM0 Interface
 0   0   1   0  16-bit
 RS /RD /WR  Read Back Selection
 0  1  /\   Write Command (DB[17:10])
 1  1  /\   Write Display Data (DB[17:10])
 1  /\  1   Read Display Data (DB[17:10])
 1  /\  1   Read Parameter or Status (DB[17:10])
 0  1  /\   Write Command (DB[17:10], DB[8:1])
 1  1  /\   Write Display Data (DB[17:10], DB[8:1])
 1  /\  1   Read Display Data (DB[17:10], DB[8:1])
 1  /\  1   Read Parameter or Status (DB[17:10], DB[8:1])
 ( /\ = raising )
 */

/** @addtogroup Utilities
 * @{
 */

/** @defgroup STM32_MINI_LCD
 * @brief This file includes the LCD driver for the STM32 MINI 2.8" TFT
 * using a "bit-bang" parallel interface. Controller is most probably
 * a Sitronix ST7783.
 * @{
 */

/** @defgroup STM32_MINI_LCD_Private_TypesDefinitions
 * @{
 */
/**
 * @}
 */

/** @defgroup STM32_MINI_LCD_Private_Defines
 * @{
 */
/**
 * @}
 */

/** @defgroup STM32_MINI_LCD_Private_Macros
 * @{
 */
#define Set_Cs  GPIO_SetBits(GPIOC,GPIO_Pin_8);
#define Clr_Cs  GPIO_ResetBits(GPIOC,GPIO_Pin_8);

#define Set_Rs  GPIO_SetBits(GPIOC,GPIO_Pin_9);
#define Clr_Rs  GPIO_ResetBits(GPIOC,GPIO_Pin_9);

#define Set_nWr GPIO_SetBits(GPIOC,GPIO_Pin_10);
#define Clr_nWr GPIO_ResetBits(GPIOC,GPIO_Pin_10);

#define Set_nRd GPIO_SetBits(GPIOC,GPIO_Pin_11);
#define Clr_nRd GPIO_ResetBits(GPIOC,GPIO_Pin_11);

#define Set_Rst GPIO_SetBits(GPIOC,GPIO_Pin_12);
#define Clr_Rst GPIO_ResetBits(GPIOC,GPIO_Pin_12);

#define Lcd_Light_ON   GPIO_SetBits(GPIOC,GPIO_Pin_13);
#define Lcd_Light_OFF  GPIO_ResetBits(GPIOC,GPIO_Pin_13);
/**
 * @}
 */

/** @defgroup STM32_MINI_LCD_Private_Variables
 * @{
 */
/* Global variables to set the written text colors */
static uint16_t TextColor = 0x0000, BackColor = 0xFFFF;

/* Global variable to keep the orientation-mode */
static LCD_OrientationMode_t orientation_mode = LCD_ORIENTATION_DEFAULT;

/**
 * @}
 */

/** @defgroup STM32_MINI_LCD_Private_FunctionPrototypes
 * @{
 */
#ifndef LCD_USE_Delay
static void delay(__IO uint32_t nCount);
#endif /* LCD_USE_Delay*/

/**
 * @}
 */

/** @defgroup STM32_MINI_LCD_Private_Functions
 * @{
 */

static void LCD_BusLinesDirection(int out) {
	GPIO_InitTypeDef GPIO_InitStructure;

	if (out) {
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	} else {
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	}
	// LSB
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8 | GPIO_Pin_9 | GPIO_Pin_10
			| GPIO_Pin_11 | GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14
			| GPIO_Pin_15;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	// MSB
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2
			| GPIO_Pin_3 | GPIO_Pin_4 | GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7;
	GPIO_Init(GPIOC, &GPIO_InitStructure);
}

static inline void LCD_BusLinesWrite(uint16_t data)
{
	// LSB of data to PB8-PB15, MSB of data to PC0-PC7
	u8 set, clear;
	set   = data;
	clear = ~set;
	GPIOB->BSRR = (set << 8) | (clear << 24);
	set   = data >> 8;
	clear = ~set;
	GPIOC->BSRR = set | (clear << 16);
}

static uint16_t LCD_BusLinesRead(void)
{
	uint16_t tmp;

	GPIOB->CRH = (GPIOB->CRH & 0x00000000) | 0x44444444; // Bus LSB Pins -> Input
	GPIOC->CRL = (GPIOC->CRL & 0x00000000) | 0x44444444; // Bus MSB Pins -> Input
	Clr_nRd;
	tmp = (((GPIOB->IDR)>>8)|((GPIOC->IDR)<<8));
	Set_nRd;
	GPIOB->CRH = (GPIOB->CRH & 0x00000000) | 0x33333333; // Bus LSB Pins -> Output
	GPIOC->CRL = (GPIOC->CRL & 0x00000000) | 0x33333333; // Bus MSB Pins -> Output

	return tmp;
}

static uint16_t LCD_ReadControllerID(void) {
	uint16_t tmp, tmp1, tmp2;

	// LCD_WriteReg(0x0000, 0x0001); //start oscillation
	// _delay_(5);

	LCD_BusLinesDirection(0);

	GPIO_ResetBits(GPIOC,LCD_CTRL_nCS_PIN);
	GPIO_SetBits(GPIOC,LCD_CTRL_RS_PIN);
	GPIO_ResetBits(GPIOC,LCD_CTRL_nRD_PIN);

	tmp1 = GPIO_ReadInputData(GPIOB);
	tmp2 = GPIO_ReadInputData(GPIOC);
	tmp = (tmp1 >> 8) | (tmp2 << 8);

	GPIO_SetBits(GPIOC,LCD_CTRL_nRD_PIN);
	GPIO_SetBits(GPIOC,LCD_CTRL_nCS_PIN);

	LCD_BusLinesDirection(1);

	return tmp;
}

/**
 * @brief  Setups the LCD.
 * @param  None
 * @retval LCD ID
 */
uint16_t LCD_Setup(void) {
	uint16_t id;

	LCD_InterfaceConfig();
	LCD_BackLight(ENABLE);

	LCD_BusLinesWrite(0xffff);
	Set_Rst;
	Set_nWr;
	Set_Cs;
	Set_Rs;
	Set_nRd;

	/* Reset Controller  ------------------------------------------------------*/
	LCD_HardwareReset();
	LCD_WriteReg(0x0000, 0x0001); //start oscillation
	_delay_(5);

	/* Get Device ID Code ------------------------------------------------------*/
	id = LCD_ReadControllerID();

	if (id == 0x7783) {

		LCD_WriteReg(0x00FF, 0x0001); // FAh/FEh Enable: FXEN
		LCD_WriteReg(0x00F3, 0x0008); // not documented
		LCD_WriteReg(0x0001, 0x0100); // Driver Output Control: SS
		LCD_WriteReg(0x0002, 0x0700); // Driver Wave Control:
		// LCD_WriteReg(0x0003, 0x1030); // Entry Mode BGR=1,AM=0,ID1/0=1/1 // LS: 1028
		LCD_WriteReg(0x0003, LCD_ENTRY_MODE_DEFAULT);
		LCD_WriteReg(0x0004, 0x0000); // Resize Control
		LCD_WriteReg(0x0008, 0x0302); // Display Control 2
		LCD_WriteReg(0x0009, 0x0000); // Display Control 3
		LCD_WriteReg(0x000A, 0x0000); // Display Control 4
		LCD_WriteReg(0x0010, 0x0790); // Power Control 1 //PA:0x0790
		LCD_WriteReg(0x0011, 0x0005); // Power Control 2
		LCD_WriteReg(0x0012, 0x0000); // Power Control 3
		LCD_WriteReg(0x0013, 0x0000); // Power Control 4
		_delay_(10);
		LCD_WriteReg(0x0010, 0x12B0); // Power Control 1
		_delay_(5);
		LCD_WriteReg(0x0011, 0x0007); // Power Control 2
		_delay_(5);
		LCD_WriteReg(0x0012, 0x008B); // Power Control 3
		_delay_(5);
		LCD_WriteReg(0x0013, 0x1700); // Power Control 4
		_delay_(5);
		LCD_WriteReg(0x0029, 0x0022); // VCOMH Control

		// Gamma Controls 0x0030 = Gamma Control 1 ~ 0x003D = Gamma Control 10
		LCD_WriteReg(0x0030, 0x0000);
		LCD_WriteReg(0x0031, 0x0707);
		LCD_WriteReg(0x0032, 0x0505);
		LCD_WriteReg(0x0035, 0x0107);
		LCD_WriteReg(0x0036, 0x0008);
		LCD_WriteReg(0x0037, 0x0000);
		LCD_WriteReg(0x0038, 0x0202);
		LCD_WriteReg(0x0039, 0x0106);
		LCD_WriteReg(0x003C, 0x0202);
		LCD_WriteReg(0x003D, 0x0408);
		_delay_(5);

		/* Set GRAM area -----------------------------------------------------------*/
		LCD_WriteReg(0x0050, 0x0000); /* Horizontal GRAM Start Address */
		LCD_WriteReg(0x0051, 240-1);  /* Horizontal GRAM End Address */
		LCD_WriteReg(0x0052, 0x0000); /* Vertical GRAM Start Address */
		LCD_WriteReg(0x0053, 320-1);  /* Vertical GRAM End Address */
		LCD_WriteReg(0x0060, (1<<15) | (0x27<<8)); /* Gate Scan Line GS=1, NL=0x27->320 lines */
		LCD_WriteReg(0x0061, 0x0001); /* NDL=0,VLE=0,REV=1 */
		/* Partial Display Control -------------------------------------------------*/
		LCD_WriteReg(0x0080, 0x0000);
		LCD_WriteReg(0x0081, 0x0000);
		LCD_WriteReg(0x0082, 0x0000);
		LCD_WriteReg(0x0083, 0x0000);
		LCD_WriteReg(0x0084, 0x0000);
		LCD_WriteReg(0x0085, 0x0000);
		/* Panel Control -----------------------------------------------------------*/
		LCD_WriteReg(0x0090, 0x0033); // Panel Interface Control 1
		LCD_WriteReg(0x0092, 0x0000); // Panel Interface Control 2
		LCD_WriteReg(0x002B, 0x000B); // Frame Rate and Color Control
		LCD_WriteReg(0x0007, 0x0133); // Display Control 1

		_delay_(5);
		/* 50ms Display on stabling time */

		LCD_SetOrientation(LCD_ORIENTATION_DEFAULT);


		// LCD_WindowModeDisable();
	}
	Set_Cs;

	return id;
}

/**
 * @brief  Initializes the LCD.
 * @param  None
 * @retval None
 */
uint16_t LCD_Init(void) {
	/* Setups the LCD */
	return LCD_Setup();
}

/**
 * @brief  Sets the Text color.
 * @param  Color: specifies the Text color code RGB(5-6-5).
 * @retval None
 */
void LCD_SetTextColor(uint16_t Color) {
	TextColor = Color;
}

/**
 * @brief  Returns the Text color.
 * @param  None
 * @retval current TextColor
 */
uint16_t LCD_GetTextColor(void) {
	return TextColor;
}

/**
 * @brief  Sets the Background color.
 * @param  Color: specifies the Background color code RGB(5-6-5).
 * @retval None
 */
void LCD_SetBackColor(uint16_t Color) {
	BackColor = Color;
}

/**
 * @brief Sets the LCD orientation
 * @param m: orientation mode
 * @retval none
 */
void LCD_SetOrientation(LCD_OrientationMode_t m)
{
	uint16_t em;

	switch (m) {
	case LCD_PORTRAIT_TOP_DOWN:
		em = 0x1030;
		break;
	case LCD_PORTRAIT_BOTTOM_UP:
		em = 0x1010;
		break;
	case LCD_LANDSCAPE_TOP_DOWN:
		em = 0x1018;
		break;
	case LCD_LANDSCAPE_BOTTOM_UP:
		em = 0x1008;
		break;
	default:
		em = 0x0130;
		break;
	}
	LCD_WriteReg(0x0003, em);  /* Entry Mode */
	orientation_mode = m;
	LCD_SetCursor(0x00, 0x00);
}

/**
 * @brief Get current LCD orientation
 * @param none
 * @retval orientation
 */
LCD_OrientationMode_t LCD_GetOrientation(void)
{
	return orientation_mode;
}

/**
 * @brief Get width in current LCD orientation
 * @param none
 * @retval width
 */
uint16_t LCD_GetWidth(void)
{
	switch (orientation_mode) {
	case LCD_LANDSCAPE_TOP_DOWN:
	case LCD_LANDSCAPE_BOTTOM_UP:
		return LCD_HEIGHT_HW;
		break;
	case LCD_PORTRAIT_TOP_DOWN:
	case LCD_PORTRAIT_BOTTOM_UP:
	default:
		return LCD_WIDTH_HW;
	}
}

/**
 * @brief Get height in current LCD orientation
 * @param none
 * @retval height
 */
uint16_t LCD_GetHeight(void)
{
	switch (orientation_mode) {
	case LCD_LANDSCAPE_TOP_DOWN:
	case LCD_LANDSCAPE_BOTTOM_UP:
		return LCD_WIDTH_HW;
		break;
	case LCD_PORTRAIT_TOP_DOWN:
	case LCD_PORTRAIT_BOTTOM_UP:
	default:
		return LCD_HEIGHT_HW;
	}
}


/**
 * @brief  Clears the selected line.
 * @param  Line: the Line to be cleared.
 *   This parameter can be one of the following values:
 *     @arg Linex: where x can be 0..12
 * @retval None
 */
void LCD_ClearLine(uint8_t Line)
{
	LCD_DisplayStringLine(Line, "               ");
}

/**
 * @brief  Clears the hole LCD.
 * @param  Color: the color of the background.
 * @retval None
 */
void LCD_Clear(uint16_t Color)
{
	uint32_t index;

	LCD_SetCursor(0x0000, 0x0000);
	LCD_WriteRAM_Prepare(); /* Prepare to write GRAM */
	for (index = 0; index < (240 * 320L); index++) {
		LCD_WriteRAM(Color);
	}
	Set_Cs;

}

/**
 * @brief  Sets the cursor position.
 * @param  Xpos: specifies the X position.
 * @param  Ypos: specifies the Y position.
 * @retval None
 */
void LCD_SetCursor(uint16_t Xpos, uint16_t Ypos)
{
	uint16_t he, ve, al, ah;

	switch (orientation_mode) {
	case LCD_PORTRAIT_TOP_DOWN:
		he = LCD_WIDTH_HW-1;
		ve = LCD_HEIGHT_HW-1;
		al = Xpos;
		ah = Ypos;
		break;
	case LCD_PORTRAIT_BOTTOM_UP:
		he = LCD_WIDTH_HW-1;
		ve = LCD_HEIGHT_HW-1-Ypos;
		al = Xpos;
		ah = LCD_HEIGHT_HW-1-Ypos;
		break;
	case LCD_LANDSCAPE_TOP_DOWN:
		he = LCD_WIDTH_HW-1;
		ve = LCD_HEIGHT_HW-1-Xpos;
		al = Ypos;
		ah = Xpos;
		break;
	case LCD_LANDSCAPE_BOTTOM_UP:
		he = LCD_WIDTH_HW-1-Ypos;
		ve = LCD_HEIGHT_HW-1-Xpos;
		al = Ypos;
		ah = Xpos;
		break;
	default:
		he = LCD_WIDTH_HW-1;
		ve = LCD_HEIGHT_HW-1;
		al = Xpos;
		ah = Ypos;
		break;
	}
	LCD_WriteReg(0x0051, he);
	LCD_WriteReg(0x0053, ve);
	LCD_WriteReg(0x0020, al);
	LCD_WriteReg(0x0021, ah);
}

/**
 * @brief  Draws a Pixel on LCD.
 * @param  x: x-coordinate
 * @param  y: y-coordinate
 * @param  Color: color
 * @retval None
 */
void LCD_DrawPixel(uint16_t x, uint16_t y, uint16_t Color)
{
	LCD_SetCursor(x, y);
	LCD_WriteRAM_Prepare();
	LCD_WriteRAM(Color);
	Set_Cs;
}

/**
 * @brief  Reads color of Pixel on LCD
 * @param  x: x-coordinate
 * @param  y: y-coordinate
 * @retval Color
 */
uint16_t LCD_GetPixel(uint16_t x, uint16_t y)
{
	uint16_t c;

	LCD_SetCursor(x, y);
	LCD_WriteRAM_Prepare();
	LCD_BusLinesRead();     /* dummy read */
	c = LCD_BusLinesRead(); /* "real" read */
	Set_Cs;

	return c;
}

/**
 * @brief  Draws a character on LCD.
 * @param  Xpos: the Line where to display the character shape.
 * @param  Ypos: start column address.
 * @param  c: pointer to the character data.
 * @retval None
 */
void LCD_DrawChar(uint16_t Xpos, uint16_t Ypos, const uint16_t *c)
{
	uint32_t index = 0, i = 0;
	uint8_t Xaddress = 0;

	Xaddress = Xpos;

	LCD_SetCursor(Xaddress, Ypos);

	for (index = 0; index < 24; index++) {
		LCD_WriteRAM_Prepare(); /* Prepare to write GRAM */
		for (i = 0; i < 16; i++) {
			if ((c[index] & (1 << i)) == 0x00) {
				LCD_WriteRAM(BackColor);
			} else {
				LCD_WriteRAM(TextColor);
			}
		}
		Ypos++;
		LCD_SetCursor(Xaddress, Ypos);
	}
	Set_Cs;
}

/**
 * @brief  Displays one character (16dots width, 24dots height).
 * @param  Line: the Line where to display the character shape .
 *   This parameter can be one of the following values:
 *     @arg Linex: where x can be 0..9
 * @param  Column: start column address.
 * @param  Ascii: character ascii code, must be between 0x20 and 0x7E.
 * @retval None
 */
void LCD_DisplayChar(uint16_t Line, uint16_t Column, uint8_t Ascii)
{
	Ascii -= 32;
	// LCD_DrawChar(Line, Column, &ASCII_Table[Ascii * 24]);
	LCD_DrawChar(Column, Line, &ASCII_Table[Ascii * 24]);
}

/**
 * @brief  Displays a maximum of 15 char on the LCD.
 * @param  Line: the Line where to display the character shape .
 *   This parameter can be one of the following values:
 *     @arg Linex: where x can be 0..9
 * @param  *ptr: pointer to string to display on LCD.
 * @retval None
 */
void LCD_DisplayStringLine(uint16_t Line, /*uint8_t*/const char *ptr)
{
	uint32_t i = 0;
	uint16_t refcolumn = 0;
	/* Send the string character by character on lCD */
	while ((*ptr != 0) & (i < 15)) {
		/* Display one character on LCD */
		LCD_DisplayChar(Line, refcolumn, *ptr);
		/* Increment the column position by 16 */
		refcolumn += 16;
		/* Point on the next character */
		ptr++;
		/* Increment the character counter */
		i++;
	}
}

/**
 * @brief  Sets a display window
 * @param  Xpos: specifies the X buttom left position.
 * @param  Ypos: specifies the Y buttom left position.
 * @param  Height: display window height.
 * @param  Width: display window width.
 * @retval None
 */
void LCD_SetDisplayWindow(uint16_t Xpos, uint16_t Ypos, uint8_t Height,
		uint16_t Width)
{
	/* Horizontal GRAM Start Address */
	if (Xpos >= Height) {
		LCD_WriteReg(R80, (Xpos - Height + 1));
	} else {
		LCD_WriteReg(R80, 0);
	}
	/* Horizontal GRAM End Address */
	LCD_WriteReg(R81, Xpos);
	/* Vertical GRAM Start Address */
	if (Ypos >= Width) {
		LCD_WriteReg(R82, (Ypos - Width + 1));
	} else {
		LCD_WriteReg(R82, 0);
	}
	/* Vertical GRAM End Address */
	LCD_WriteReg(R83, Ypos);
	LCD_SetCursor(Xpos, Ypos);
}

/**
 * @brief  Disables LCD Window mode.
 * @param  None
 * @retval None
 */
void LCD_WindowModeDisable(void)
{
	LCD_SetDisplayWindow(239, 0x13F, 240, 320);
	LCD_WriteReg(R3, LCD_ENTRY_MODE_DEFAULT); // was 0x1028
}

/**
 * @brief  Displays a line.
 * @param  Xpos: specifies the X position.
 * @param  Ypos: specifies the Y position.
 * @param  Length: line length.
 * @param  Direction: line direction.
 *   This parameter can be one of the following values: LCD_LINE_VERTICAL or Horizontal.
 * @retval None
 */
void LCD_DrawLine(uint16_t Xpos, uint16_t Ypos, uint16_t Length,
		uint8_t Direction)
{
	uint32_t i = 0;

	LCD_SetCursor(Xpos, Ypos);
	if (Direction == LCD_LINE_HORIZONTAL) {
		LCD_WriteRAM_Prepare(); /* Prepare to write GRAM */
		for (i = 0; i < Length; i++) {
			LCD_WriteRAM(TextColor);
		}
	} else {
		for (i = 0; i < Length; i++) {
			LCD_WriteRAMWord(TextColor);
			Ypos++;
			LCD_SetCursor(Xpos, Ypos);
		}
	}
	LCD_CtrlLinesWrite(LCD_NCS_GPIO_PORT,LCD_NCS_PIN, Bit_SET);
}

/**
 * @brief  Displays a rectangle.
 * @param  Xpos: specifies the X position.
 * @param  Ypos: specifies the Y position.
 * @param  Height: display rectangle height.
 * @param  Width: display rectangle width.
 * @retval None
 */
void LCD_DrawRect(uint16_t Xpos, uint16_t Ypos, uint16_t Height, uint16_t Width)
{
	LCD_DrawLine(Xpos, Ypos, Width, LCD_LINE_HORIZONTAL);
	LCD_DrawLine(Xpos, Ypos, Height, LCD_LINE_VERTICAL);
	LCD_DrawLine((Xpos + Width - 1), Ypos, Height, LCD_LINE_VERTICAL);
	LCD_DrawLine(Xpos, (Ypos + Height - 1), Width, LCD_LINE_HORIZONTAL);
}

/**
 * @brief  Displays a box (filled rectangle).
 * @param  Xpos: specifies the X position.
 * @param  Ypos: specifies the Y position.
 * @param  Height: display rectangle height.
 * @param  Width: display rectangle width.
 * @retval None
 */
void LCD_DrawBox(uint16_t Xpos, uint16_t Ypos, uint16_t Height, uint16_t Width)
{
	int32_t CurY;

	for (CurY = Ypos; CurY < (Ypos + Height); CurY++) {
		LCD_DrawLine(Xpos, CurY, Width, LCD_LINE_HORIZONTAL);
	}
}

/**
 * @brief  Displays a circle.
 * @param  Xpos: specifies the X position.
 * @param  Ypos: specifies the Y position.
 * @param  Radius
 * @retval None
 */
void LCD_DrawCircle(uint16_t Xpos, uint16_t Ypos, uint16_t Radius)
{
	int32_t  D;    /* Decision Variable */
	uint32_t CurX; /* Current X Value */
	uint32_t CurY; /* Current Y Value */

	D = 3 - (Radius << 1);
	CurX = 0;
	CurY = Radius;

	while (CurX <= CurY) {
		LCD_SetCursor(Xpos + CurX, Ypos + CurY);
		LCD_WriteRAMWord(TextColor);
		LCD_SetCursor(Xpos + CurX, Ypos - CurY);
		LCD_WriteRAMWord(TextColor);
		LCD_SetCursor(Xpos - CurX, Ypos + CurY);
		LCD_WriteRAMWord(TextColor);
		LCD_SetCursor(Xpos - CurX, Ypos - CurY);
		LCD_WriteRAMWord(TextColor);
		LCD_SetCursor(Xpos + CurY, Ypos + CurX);
		LCD_WriteRAMWord(TextColor);
		LCD_SetCursor(Xpos + CurY, Ypos - CurX);
		LCD_WriteRAMWord(TextColor);
		LCD_SetCursor(Xpos - CurY, Ypos + CurX);
		LCD_WriteRAMWord(TextColor);
		LCD_SetCursor(Xpos - CurY, Ypos - CurX);
		LCD_WriteRAMWord(TextColor);
		if (D < 0) {
			D += (CurX << 2) + 6;
		} else {
			D += ((CurX - CurY) << 2) + 10;
			CurY--;
		}
		CurX++;
	}
	LCD_CtrlLinesWrite(LCD_NCS_GPIO_PORT,LCD_NCS_PIN, Bit_SET);
}

/**
 * @brief  Displays a monocolor picture.
 * @param  Pict: pointer to the picture array.
 * @retval None
 */
void LCD_DrawMonoPict(const uint32_t *Pict)
{
	uint32_t index = 0, i = 0;
	LCD_SetCursor(0, 319);
	LCD_WriteRAM_Prepare(); /* Prepare to write GRAM */
	for (index = 0; index < 2400; index++) {
		for (i = 0; i < 32; i++) {
			if ((Pict[index] & (1 << i)) == 0x00) {
				LCD_WriteRAM(BackColor);
			} else {
				LCD_WriteRAM(TextColor);
			}
		}
	}
	LCD_CtrlLinesWrite(LCD_NCS_GPIO_PORT,LCD_NCS_PIN, Bit_SET);
}

#ifdef USE_LCD_DrawBMP
/**
 * @brief  Displays a bitmap picture loaded in the SPI Flash.
 * @param  BmpAddress: Bmp picture address in the SPI Flash.
 * @retval None
 */
//void LCD_DrawBMP(uint32_t BmpAddress)
//{
//  uint32_t i = 0, size = 0;
//
//  /* Read bitmap size */
//  SPI_FLASH_BufferRead((uint8_t*)&size, BmpAddress + 2, 4);
//
//  /* get bitmap data address offset */
//  SPI_FLASH_BufferRead((uint8_t*)&i, BmpAddress + 10, 4);
//
//  size = (size - i)/2;
//
//  SPI_FLASH_StartReadSequence(BmpAddress + i);
//
//  /* Disable SPI1  */
//  SPI_Cmd(SPI1, DISABLE);
//  /* SPI in 16-bit mode */
//  SPI_DataSizeConfig(SPI1, SPI_DataSize_16b);
//
//  /* Enable SPI1  */
//  SPI_Cmd(SPI1, ENABLE);
//
//  /* Set GRAM write direction and BGR = 1 */
//  /* I/D=00 (LCD_LINE_HORIZONTAL : decrement, LCD_LINE_VERTICAL : decrement) */
//  /* AM=1 (address is updated in vertical writing direction) */
//  LCD_WriteReg(R3, 0x1008);
//
//  LCD_WriteRAM_Prepare(); /* Prepare to write GRAM */
//
//  /* Read bitmap data from SPI Flash and send them to LCD */
//  for(i = 0; i < size; i++)
//  {
//    LCD_WriteRAM(__REV_HalfWord(SPI_FLASH_SendHalfWord(0xA5A5)));
//  }
//
//  LCD_CtrlLinesWrite(LCD_NCS_GPIO_PORT, LCD_NCS_PIN, Bit_SET);
//
//  /* Deselect the FLASH: Chip Select high */
//  SPI_FLASH_CS_HIGH();
//
//  /* Disable SPI1  */
//  SPI_Cmd(SPI1, DISABLE);
//  /* SPI in 8-bit mode */
//  SPI_DataSizeConfig(SPI1, SPI_DataSize_8b);
//
//  /* Enable SPI1  */
//  SPI_Cmd(SPI1, ENABLE);
//
//  /* Set GRAM write direction and BGR = 1 */
//  /* I/D = 01 (LCD_LINE_HORIZONTAL : increment, LCD_LINE_VERTICAL : decrement) */
//  /* AM = 1 (address is updated in vertical writing direction) */
//  LCD_WriteReg(R3, 0x1018);
//}


/**
 * @brief  Displays a bitmap picture loaded in the SPI Flash.
 * @param  BmpAddress: Bmp picture address in the SPI Flash.
 * @retval None
 */
void LCD_DrawBMP(const uint16_t *BmpAddress)
{
	uint32_t i = 0, size = 0;
	/* Read bitmap size */
	size = BmpAddress[1] | (BmpAddress[2] << 16);
	/* get bitmap data address offset */
	i = BmpAddress[5] | (BmpAddress[6] << 16);
	size = (size - i)/2;
	BmpAddress += i/2;
	/* Set GRAM write direction and BGR = 1 */
	/* I/D=00 (LCD_LINE_HORIZONTAL : decrement, LCD_LINE_VERTICAL : decrement) */
	/* AM=1 (address is updated in vertical writing direction) */
	LCD_WriteReg(R3, 0x1008);
	LCD_WriteRAM_Prepare(); /* Prepare to write GRAM */
	/* Read bitmap data from SPI Flash and send them to LCD */
	for(i = 0; i < size; i++)
	{
		LCD_WriteRAM(BmpAddress[i]);
	}
	LCD_CtrlLinesWrite(LCD_NCS_GPIO_PORT, LCD_NCS_PIN, Bit_SET);
	/* Set GRAM write direction and BGR = 1 */
	/* I/D = 01 (LCD_LINE_HORIZONTAL : increment, LCD_LINE_VERTICAL : decrement) */
	/* AM = 1 (address is updated in vertical writing direction) */
	/// LCD_WriteReg(R3, 0x1018);
	LCD_WriteReg(0x0003, 0x1030);
}
#endif


/**
 * @brief  Prepare to write to the LCD RAM.
 * @param  None
 * @retval None
 */
void LCD_WriteRAM_Prepare(void) {
	Clr_Cs;
	Clr_Rs;
	Set_nRd;
	LCD_BusLinesWrite(0x0022); /* Select GRAM Reg */
	Clr_nWr;
	Set_nWr;
	Set_Rs;
}

/**
 * @brief  Writes 1 word to the LCD RAM.
 * @param  RGB_Code: the pixel color in RGB mode (5-6-5).
 * @retval None
 */
void LCD_WriteRAMWord(uint16_t RGB_Code) {
	LCD_WriteRAM_Prepare();
	LCD_WriteRAM(RGB_Code);
}

/**
 * @brief  Writes to the selected LCD register.
 * @param  LCD_Reg: address of the selected register.
 * @param  LCD_RegValue: value to write to the selected register.
 * @retval None
 */
void LCD_WriteReg(uint8_t LCD_Reg, uint16_t LCD_RegValue) {
	Clr_Cs;
	Clr_Rs;
	Set_nRd;
	LCD_BusLinesWrite(LCD_Reg);
	Clr_nWr;
	Set_nWr;
	Set_Rs;
	LCD_BusLinesWrite(LCD_RegValue);
	Clr_nWr;
	Set_nWr;
	Set_Cs;
}

/**
 * @brief  Writes to the LCD RAM.
 * @param  RGB_Code: the pixel color in RGB mode (5-6-5).
 * @retval None
 */
void LCD_WriteRAM(uint16_t RGB_Code)
{
	LCD_BusLinesWrite(RGB_Code);
	Clr_nWr;
	Set_nWr;
	// Set_Cs;
}


/**
 * @brief Release LCD by setting /CS
 * @param none
 * @retval none
 */
void LCD_Release(void)
{
	Set_Cs;
}

/**
 * @brief  Power on the LCD.
 * @param  None
 * @retval None
 */
void LCD_PowerOn(void)
{
	/* Power On sequence ---------------------------------------------------------*/
	LCD_WriteReg(0x0010, 0x0790); // Power Control 1 //PA:0x0790
	LCD_WriteReg(0x0011, 0x0005); // Power Control 2
	LCD_WriteReg(0x0012, 0x0000); // Power Control 3
	LCD_WriteReg(0x0013, 0x0000); // Power Control 4
	_delay_(20);
	LCD_WriteReg(0x0010, 0x12B0); // Power Control 1
	LCD_WriteReg(0x0011, 0x0007); // Power Control 2
	_delay_(5);
	LCD_WriteReg(0x0012, 0x008B); // Power Control 3
	_delay_(5);
	LCD_WriteReg(0x0013, 0x1700); // Power Control 4
	LCD_WriteReg(0x0029, 0x0022); // VCOMH Control
	_delay_(5);
}

void LCD_PowerOff(void)
{
	// LCD_WriteReg(0x0007, 0x0000); // display OFF
	LCD_WriteReg(0x0010, 0x0000); // SAP, BT[3:0], APE, AP, DSTB, SLP
	LCD_WriteReg(0x0011, 0x0000); // DC1[2:0], DC0[2:0], VC[2:0]
	LCD_WriteReg(0x0012, 0x0000); // VREG1OUT voltage
	LCD_WriteReg(0x0013, 0x0000); // VDV[4:0] for VCOM amplitude
	_delay_(20); // Dis-charge capacitor power voltage
	LCD_WriteReg(0x0010, 0x0002); // SAP, BT[3:0], APE, AP, DSTB, SLP
}

/**
 * @brief  Enables the Display.
 * @param  None
 * @retval None
 */
void LCD_DisplayOn(void)
{
	/* Display On */
	// LCD_WriteReg(R7, 0x0173); /* 262K color and display ON */
	LCD_WriteReg(0x0007, 0x0133);
	// _delay_(5); /* 50ms Display on stabling time */
}

/**
 * @brief  Disables the Display.
 * @param  None
 * @retval None
 */
void LCD_DisplayOff(void)
{
	/* Display Off */
	LCD_WriteReg(R7, 0x0);
}

/**
 * @brief  Sets or reset LCD control lines.
 * @param  GPIOx: where x can be B or D to select the GPIO peripheral.
 * @param  CtrlPins: the Control line. This parameter can be:
 *     @arg LCD_NCS_PIN: Chip Select pin
 * @param  BitVal: specifies the value to be written to the selected bit.
 *   This parameter can be:
 *     @arg Bit_RESET: to clear the port pin
 *     @arg Bit_SET: to set the port pin
 * @retval None
 */
void LCD_CtrlLinesWrite(GPIO_TypeDef* GPIOx, uint16_t CtrlPins,
		BitAction BitVal)
{
	/* Set or Reset the control lines */
	if (BitVal == Bit_SET) {
		GPIO_SetBits(GPIOx, CtrlPins);
	} else {
		GPIO_ResetBits(GPIOx, CtrlPins);
	}
}

/**
 * @brief  Switch LCD Back-Light (no PWM)
 * @param  0=off, !=0 on
 * @retval None
 */
void LCD_BackLight(BitAction ba)
{
	LCD_CtrlLinesWrite(LCD_CTRL_GPIO_PORT,LCD_CTRL_BL_PIN, ba);
}

/**
 * @brief  Configures the LCD_SPI interface.
 * @param  None
 * @retval None
 */
void LCD_InterfaceConfig(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB
			| RCC_APB2Periph_GPIOC, ENABLE);

	// Bus LSB
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8 | GPIO_Pin_9 | GPIO_Pin_10
			| GPIO_Pin_11 | GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14
			| GPIO_Pin_15;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	// BUS MSB
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2
			| GPIO_Pin_3 | GPIO_Pin_4 | GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7;
	GPIO_Init(GPIOC, &GPIO_InitStructure);
	// Control Lines
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8 | GPIO_Pin_9 | GPIO_Pin_10
			| GPIO_Pin_11 | GPIO_Pin_12;
	GPIO_Init(GPIOC, &GPIO_InitStructure);
	// Back-Light
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13;
	GPIO_Init(GPIOC, &GPIO_InitStructure);
}

void LCD_HardwareReset(void)
{
	LCD_CtrlLinesWrite(LCD_CTRL_GPIO_PORT,LCD_CTRL_RST_PIN, Bit_SET);
	LCD_CtrlLinesWrite(LCD_CTRL_GPIO_PORT,LCD_CTRL_RST_PIN, Bit_RESET);
	_delay_(5);
	LCD_CtrlLinesWrite(LCD_CTRL_GPIO_PORT,LCD_CTRL_RST_PIN, Bit_SET);
	_delay_(5);
}

#ifndef LCD_USE_Delay
/**
 * @brief  Inserts a delay time.
 * @param  nCount: specifies the delay time length.
 * @retval None
 */
static void delay(__IO uint32_t nCount)
{
	volatile uint32_t index = 0;
	for(index = (100000 * nCount); index != 0; index--)
	{
	}
}
#endif /* LCD_USE_Delay*/
/**
 * @}
 */

/**
 * @}
 */

/**
 * @}
 */

/******************* (C) COPYRIGHT 2009 STMicroelectronics *****END OF FILE****/
