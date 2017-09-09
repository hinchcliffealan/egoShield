/********************************************************************************************
* 	 	File: 		egoShield.h 															*
*		Version:    0.1.0                                           						*
*      	Date: 		September 4th, 2017	                                    				*
*      	Author: 	Mogens Groth Nicolaisen                                					*
*                                                   										*	
*********************************************************************************************
*	(C) 2017																				*
*																							*
*	ON Development IVS																		*
*	www.on-development.com 																	*
*	administration@on-development.com 														*
*																							*
*	The code contained in this file is released under the following open source license:	*
*																							*
*			Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International			*
* 																							*
* 	The code in this file is provided without warranty of any kind - use at own risk!		*
* 	neither ON Development IVS nor the author, can be held responsible for any damage		*
* 	caused by the use of the code contained in this file ! 									*
*                                                                                           *
********************************************************************************************/
/**
*	\mainpage Arduino library for the uStepper egoShield shield
*	
*	This is the egoShield Arduino library, providing software functions for the uStepper egoShield shield.
*	
*	\par News!
*	This version of the library adds two new functions to the library:
*	moveToAngle()   - 	This makes it possible to specify a desired angle to reach, with respect to the last reset of home position
*	MoveAngle() 	- 	This makes it possible to move the motor an angle relative to its current position, without having to calculate
*						the steps required, and manually call moveSteps();
*
*	\par Features
*	The egoShield library contains the following features:
*	
*	\image html functional.png
*	
*	- State is indicated at the top left corner of the OLED display
*	- Brake mode is indicated in the top middle of the OLED display
*	- Loop mode is indicated in the top right corner of the OLED display
*	- The button functionalities are indicated by a bar in the bottom of the OLED display
* 
*	\par Installation
*	To install the egoShield library into the Arduino IDE, perform the following steps:
*
*	- Go to Sketch->Include Libraries->Manage Libraries... in the Arduino IDE
*	- Search for "egoShield", in the top right corner of the "Library Manager" window
*	- Install egoShield library 
*	
*	The library is tested with Arduino IDE 1.6.10
*
*	\par Prerequisites
*	The library requires the uStepper library:
*	<a rel="license" href="https://github.com/uStepper/uStepper">uStepper GitHub</a>
*	and the u8g2 library from Olikraus
*	<a rel="license" href="https://github.com/olikraus/u8g2">u8g2 GitHub</a>
*
*	\par Copyright
*
*	(C)2017 ON Development IVS	
*																	
*	www.on-development.com 																	
*
*	administration@on-development.com 														
*																							
*	<img alt="Creative Commons License" style="border-width:0" src="https://i.creativecommons.org/l/by-nc-sa/4.0/88x31.png" />																
*
*	The code contained in this file is released under a <a rel="license" href="http://creativecommons.org/licenses/by-nc-sa/4.0/">Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License</a>	
*																							
*	The code in this library is provided without warranty of any kind - use at own risk!		
* 	neither ON Development IVS nor the author, can be held responsible for any damage		
* 	caused by the use of the code contained in this library ! 	
*
*	\par To do list
*	- Make PID parameters available for the user
*	- Make motor speed adjustable
*	- Add comments in the .cpp
*	- clean the code in .cpp
*	- Add BT support
*
*	\par Known Bugs
*	- No known bugs
*
*	\author Mogens Groth Nicolaisen (mogens@ustepper.com)
*	\par Change Log
*	\version 0.1.0:	
*	- Initial release
*	
*/

/**
 * @file egoShield.h
 * @brief      Function prototypes and definitions for the egoShield library
 *
 *             This file contains class and function prototypes for the library,
 *             as well as necessary constants and global variables.
 *
 * @author     Mogens Groth Nicolaisen (mogens@ustepper.com)
 */

#ifndef egoShield_h
#define egoShield_h

#include "uStepper.h"
#include "U8g2lib.h"
#include "SPI.h"
#include "Arduino.h"

#define FWBT A3
#define PLBT A1
#define RECBT A2
#define BWBT A0
#define CNT 50

#define en_width 11
#define en_height 9
static unsigned char fw_bits[] = {
   0x41, 0x00, 0xc3, 0x00, 0xc7, 0x01, 0xcf, 0x03, 0xdf, 0x07, 0xcf, 0x03,
   0xc7, 0x01, 0xc3, 0x00, 0x41, 0x00 };
static unsigned char bw_bits[] = {
   0x10, 0x04, 0x18, 0x06, 0x1c, 0x07, 0x9e, 0x07, 0xdf, 0x07, 0x9e, 0x07,
   0x1c, 0x07, 0x18, 0x06, 0x10, 0x04 };
#define tt_width 10
#define tt_height 10
static unsigned char rec_bits[] = {
   0xfc, 0x00, 0xfe, 0x01, 0xff, 0x03, 0xff, 0x03, 0xff, 0x03, 0xff, 0x03,
   0xff, 0x03, 0xff, 0x03, 0xfe, 0x01, 0xfc, 0x00 };
static unsigned char stop_bits[] = {
   0xff, 0x03, 0xff, 0x03, 0xff, 0x03, 0xff, 0x03, 0xff, 0x03, 0xff, 0x03,
   0xff, 0x03, 0xff, 0x03, 0xff, 0x03, 0xff, 0x03 };
static unsigned char pse_bits[] = {
   0xcc, 0x00, 0xcc, 0x00, 0xcc, 0x00, 0xcc, 0x00, 0xcc, 0x00, 0xcc, 0x00,
   0xcc, 0x00, 0xcc, 0x00, 0xcc, 0x00, 0xcc, 0x00 };
#define play_width 6
#define play_height 11
static unsigned char play_bits[] = {
   0x01, 0x03, 0x07, 0x0f, 0x1f, 0x3f, 0x1f, 0x0f, 0x07, 0x03, 0x01 };
#define loop_width 11
#define loop_height 10
static unsigned char loop_bits[] = {
   0x04, 0x00, 0x08, 0x00, 0x9e, 0x03, 0x09, 0x04, 0x05, 0x04, 0x01, 0x05,
   0x81, 0x04, 0xce, 0x03, 0x80, 0x00, 0x00, 0x01 };
#define logo_width 90
#define logo_height 16
static unsigned char logo_bits[] = {
   0x00, 0x00, 0x7e, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x80, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0xc0, 0xff, 0x60, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0xc0, 0xc7, 0x78, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x3e, 0xcf, 0x07, 0xfc, 0xe3, 0xc3, 0x7b, 0x78, 0x0f, 0x3e, 0x9c, 0x01,
   0xdf, 0xcf, 0x3f, 0xfe, 0xf3, 0xf7, 0xff, 0xfe, 0x1f, 0x7f, 0xff, 0x03,
   0x3e, 0xcf, 0xff, 0x78, 0x38, 0xc7, 0xf3, 0x78, 0x9e, 0x73, 0xfe, 0x03,
   0x3c, 0x8f, 0xff, 0x79, 0xfc, 0xcf, 0xf3, 0x79, 0xde, 0xff, 0xbc, 0x01,
   0x1c, 0x47, 0xfe, 0x78, 0xfc, 0xcf, 0xf3, 0x78, 0xbe, 0x7f, 0x1e, 0x00,
   0x3c, 0xef, 0xf0, 0x79, 0x38, 0xc0, 0xf3, 0x79, 0xde, 0x03, 0x1e, 0x00,
   0xbe, 0xcf, 0xff, 0x78, 0x78, 0xc4, 0xf3, 0x78, 0x9e, 0x47, 0x3e, 0x00,
   0xfc, 0xdf, 0x7f, 0xf8, 0xf3, 0xc7, 0x7f, 0xf8, 0x0f, 0x7f, 0x7e, 0x00,
   0x38, 0x07, 0x3f, 0xf0, 0xe1, 0xc3, 0x3f, 0xf8, 0x07, 0x3e, 0x7f, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0xe0, 0x03, 0x7c, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0xf0, 0x07, 0xfc, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0xc0, 0x07, 0xf8, 0x00, 0x00, 0x00, 0x00 };

class egoShield
{
public:
	/**
	* @brief      Constructor of egoShield class
	*
	*             This is the constructor of the egoShield class. No arguments are present in the constructor.
	*/
	egoShield(void);
	/**
	* @brief      	Initializes buttons, OLED, uStepper and BT-module.
	*
	* @param[in]  	acc takes in the maximum acceleration in play mode.
	*
	* @param[in]  	vel takes in the maximum velocity in play mode.
	*/
	void setup(uint16_t acc, uint16_t vel);
	/**
	* @brief      	Contains the main logic of the shield functionality, e.g. transition between states (idle, play, record and pause).
	*/	
	void loop(void);
	/**
	* @brief      	Creates an uStepper instance.
	*/
	uStepper stepper;
private:
	/**Creates an u8g2 (OLED) instance */
	U8G2_SSD1306_128X64_NONAME_1_4W_SW_SPI *u8g2;
	/** This variable holds the step number in the recorded sequence */
	uint8_t place;
	/** This variable holds the final step number in the recorded sequence */
	uint8_t endmove;
	/** This array holds the encoder value at the recorded positions */
	float pos[CNT];
	/** This variable indicates if motor brakes are enabled */
	bool brakeFlag;
	/** This variable indicates whether we are recording */
	bool record;
	/** This variable indicates whether we are in loop mode during playback */
	bool loopMode;
	/** This array indicates whether one of the buttons have experienced a long duration push */
	bool longPushFlag[4];
	/** This variable holds the current state of the program, which tells whether the program is in idle, play, record or pause mode */
	char state;
	/** This variable holds the current state of the record button, which tells whether no, short or long push has been detected */
	uint8_t rec;
	/** This variable holds the current state of the play button, which tells whether no, short or long push has been detected */
	uint8_t play;
	/** This variable holds the current state of the forward button, which tells whether no, short or long push has been detected */
	uint8_t fw;
	/** This variable holds the current state of the backward button, which tells whether no, short or long push has been detected */
	uint8_t bw;
	/** This variable holds the current set-point to the PID, either from manual control or during playback of the sequence */
	float setPoint;

	uint16_t acceleration;
	uint16_t velocity;

	/**
	* @brief      	Reads the four buttons and writes their value; no push, short push or long push, to global variables.
	*/
	void inputs(void);
	/**
	* @brief      	Returns the button state of the appropriate button.
	*
	* @param[in]  	button is set to either of the four available buttons.
	*
	* @param[in]  	nmbr is used for indexing in the longPushFlag array.
	*
	* @return     0	- no push detected.
	* @return     1	- short push detected.
	* @return     2	- long push detected.
	*
	*/
	uint8_t buttonState(uint8_t button, uint8_t nmbr);
	/**
	* @brief      	Holds the idle logic; page to show, what buttons to enable etc.
	*/
	void idleMode(void);
	/**
	* @brief      	Holds the play logic, showing play page and running the recorded sequence.
	*/
	void playMode(void);
	/**
	* @brief      	Holds the record logic, showing the record page and recording positions from user input.
	*/
	void recordMode(void);
	/**
	* @brief      	Holds the pause logic, showing the pause page and pausing the playing of a sequence.
	*/
	void pauseMode(void);
	/**
	* @brief      	Holds the fast forward logic for driving the stepper motor manually with the pushbuttons.
	*/
	void fastForward(void);
	/**
	* @brief      	Holds the fast backward logic for driving the stepper motor manually with the pushbuttons.
	*/
	void fastBackward(void);
	/**
	* @brief      	Holds the code for the start page of the OLED.
	*/
	void startPage(void);
	/**
	* @brief      	Holds the code for the idle page of the OLED.
	*
	* @param[in]  	brakeMode tells if the display should show brakes or no brakes.
	*
	* @param[in]  	pos is the encoder position to be displayed.
	*/
	void idlePage(bool brakeMode, float pos);
	/**
	* @brief      	Holds the code for the record page of the OLED.
	*
	* @param[in]  	brakeMode tells if the display should show brakes or no brakes.
	* 
	* @param[in]  	recorded tells if a step has been recorded.
	*
	* @param[in]  	index tells which step we are at.
	*
	* @param[in]  	pos is the encoder position to be displayed.
	*/
	void recordPage(bool brakeMode, bool recorded, uint8_t index, float pos);
	/**
	* @brief      	Holds the code for the play page of the OLED.
	*
	* @param[in]  	loopMode tells if the display should show loop symbol.
	*
	* @param[in]  	brakeMode tells if the display should show brakes or no brakes.
	*
	* @param[in]  	index tells which step we are at.
	*/
	void playPage(bool loopMode, bool brakeMode, uint8_t index);
	/**
	* @brief      	Holds the code for the pause page of the OLED.
	*
	* @param[in]  	loopMode tells if the display should show loop symbol.
	*
	* @param[in]  	brakeMode tells if the display should show brakes or no brakes.
	*
	* @param[in]  	index tells which step we are at.
	*/
	void pausePage(bool loopMode, bool brakeMode, uint8_t index);
};
#endif