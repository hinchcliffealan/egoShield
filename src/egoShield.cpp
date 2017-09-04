/********************************************************************************************
*     File:       egoShield.cpp                                                             *
*     Version:    0.1.0                                                                     *
*     Date:       September 4th, 2017                                                       *
*     Author:     Mogens Groth Nicolaisen                                                   *
*                                                                                           * 
*********************************************************************************************
*                 egoShield class                                                           *
*                                                                                           *
* This file contains the implementation of the class methods, incorporated in the           *
* egoShield Arduino library. The library is used by instantiating an egoShield object       *
* by calling of the overloaded constructor:                                                 *
*                                                                                           *
*   example:                                                                                *
*                                                                                           *
*   egoShield ego;                                                                          *
*                                                                                           *
* The instantiation above creates an egoShield object                                       *
* after instantiation of the object, the object setup function should be called within      *
* Arduino's setup function, and the object loop function should be run within the Arduino's *
* loop function:                                                                            *
*                                                                                           *
*   example:                                                                                *
*                                                                                           *
*   egoShield ego;                                                                          *
*                                                                                           *
*   void setup()                                                                            *
*   {                                                                                       *
*     ego.setup();                                                                          *
*   }                                                                                       *
*                                                                                           *
*   void loop()                                                                             *
*   {                                                                                       *
*     ego.loop();                                                                           *
*   }                                                                                       *
*                                                                                           *
*                                                                                           *
*********************************************************************************************
* (C) 2017                                                                                  *
*                                                                                           *
* ON Development IVS                                                                        *
* www.on-development.com                                                                    *
* administration@on-development.com                                                         *
*                                                                                           *
* The code contained in this file is released under the following open source license:      *
*                                                                                           *
*     Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International               *
*                                                                                           *
*   The code in this file is provided without warranty of any kind - use at own risk!       *
*   neither ON Development IVS nor the author, can be held responsible for any damage       *
*   caused by the use of the code contained in this file !                                  *
*                                                                                           *
********************************************************************************************/
/**
 * @file egoShield.cpp
 * @brief      Class implementations for the egoShield library
 *
 *             This file contains the implementations of the classes defined in
 *             egoShield.h
 *
 * @author     Mogens Groth Nicolaisen (mogens@ustepper.com)
 */
 
#include "egoShield.h"

egoShield::egoShield(void)
{
  u8g2 = new U8G2_SSD1306_128X64_NONAME_1_4W_SW_SPI(U8G2_R0, /* clock=*/ 11, /* data=*/ 9, /* cs=*/ U8X8_PIN_NONE, /* dc=*/ 2, /* reset=*/ 10);
}

void egoShield::setup(void)
{
  stepper.setup(PID,SIXTEEN,20,10,1.0,1.0,0.006);
  stepper.encoder.setHome();//set new home position
  //Serial.begin(9600);
  pinMode(FWBT ,INPUT);
  pinMode(PLBT ,INPUT);
  pinMode(RECBT ,INPUT);
  pinMode(BWBT ,INPUT);
  digitalWrite(FWBT ,HIGH);//pull-up
  digitalWrite(PLBT ,HIGH);//pull-up
  digitalWrite(RECBT ,HIGH);//pull-up
  digitalWrite(BWBT ,HIGH);//pull-up
  setPoint = stepper.encoder.getAngleMoved();//set manual move setpoint to current position
  u8g2->begin();
  this->startPage();
  delay(2000);
  state = 'a';
}

void egoShield::loop(void)
{
  this->inputs();//check inputs
  switch (state)
  {
    case 'a'://if we are in stop
    this->idlePage(brakeFlag,setPoint);
    if(play == 2)//if play/stop/pause is pressed for long time, invert the brake mode
    {
      if(brakeFlag == 0)
      {
        brakeFlag = 1;
        stepper.hardStop(HARD);
      }
      else
      {
        brakeFlag = 0;
        stepper.hardStop(SOFT);
      }
    }
    else if(fw == 1)//if manual forward signal
    {
      setPoint +=5;
      stepper.moveToAngle(setPoint,brakeFlag);//move 5deg
      //Serial.println("FW");
    }
    else if(fw == 2)//if manual forward signal long
    {
      this->fastForward();
    }
    else if(bw == 1)//if manual backward signal
    {
      setPoint -=5;
      stepper.moveToAngle(setPoint,brakeFlag);
      //Serial.println("BW");
    }
    else if(bw == 2)//if manual backward signal long
    {
      this->fastBackward();
    }
    else if(rec == 2)
    {
      if(!brakeFlag)//make it possible to move stuff by hand
      {
         stepper.setup(NORMAL,SIXTEEN,10,5,1.0,0.02,0.006,0);//pause PID to allow manual movement
         stepper.softStop(SOFT);
         stepper.setMaxVelocity(1000);
         stepper.setMaxAcceleration(1500);
      }
      state = 'c';
      //Serial.println("Record mode");
    }
    else if(play == 1)//we want to play sequence when doing a short press
    {
      state = 'b';
      //Serial.println("Play mode");
    }
    break;
    
    case 'b'://if we are in play
    this->playPage(loopMode,brakeFlag,place);
    if(loopMode && place > endmove)
    {
      place=0;
    }
    else if(place > endmove)//If we are at the end move
    {
      place = 0;//reset array counter
      state = 'a';
      setPoint = pos[endmove];
      //Serial.println("Stop mode");
    }
    else
    {
      stepper.moveToAngle(pos[place],brakeFlag);
      //Serial.print("Moving to position ");//Print where we are going for convenience
      //Serial.println(place);
      place++;//increment array counter
      while(stepper.getMotorState())
      {
        this->inputs();//check inputs
        if(play == 2)//if play/stop/pause is pressed again for long time, stop
        {
          place = 0;//reset array counter
          loopMode = 0;
          state = 'a';
          setPoint = stepper.encoder.getAngleMoved();
          //Serial.println("Stop mode");
        }
        else if(rec == 1)
        {
          state = 'd';//pause
          //Serial.println("Pause mode");
        }
        else if(fw == 2)//loop mode start
        {
          loopMode = 1;
          //Serial.println("Loop mode on");
        }
        else if(bw == 2)//loop mode stop
        {
          loopMode = 0;
          //Serial.println("Loop mode off");
        }
      }
    }
    break;
    
    case 'c'://if we are in record
    this->recordPage(brakeFlag,0,place,stepper.encoder.getAngleMoved());
    if(brakeFlag)
    {
      if(fw == 1)//if manual forward signal
      {
        setPoint +=5;
        stepper.moveToAngle(setPoint,brakeFlag);//move 5deg
        //Serial.println("FW");
      }
      else if(fw == 2)//if manual forward signal long
      {
        this->fastForward();
      }
      else if(bw == 1)//if manual backward signal
      {
        setPoint -=5;
        stepper.moveToAngle(setPoint,brakeFlag);
        //Serial.println("BW");
      }
      else if(bw == 2)//if manual forward signal long
      {
        this->fastBackward();
      }
    }
    if(rec == 1)//record position
    {      
      if(record == 0)//If we were not recording before
      {
        stepper.encoder.setHome();//Set current position as home
        setPoint = 0;
        place = 0;//Reset the array counter
        record = 1;//Record flag
      }
      this->recordPage(brakeFlag,1,place,stepper.encoder.getAngleMoved());
      delay(500);
      if(record == 1)//If we have initialized recording
      {
        //Serial.print("Position ");//Print position number for convenience
        //Serial.print(place);
        //Serial.println(" recorded");
        pos[place] = stepper.encoder.getAngleMoved();//Save current position
        place++;//Increment array counter
        if(place>CNT)
        {
          place=0;
        }
      }
    }
    else if(play == 2)//stop pressed
    {
      if(!brakeFlag)//if we recorded without the feedbackloop (i.e. with movement of the application by hand)
      {
        stepper.setup(PID,SIXTEEN,10,5,1.0,0.02,0.006,0);//reinitialize PID - unpause
      }
      endmove = place-1;//set the endmove to the current position
      place = 0;//reset array counter
      record = 0;//reset record flag
      state = 'a';//stop state
      setPoint = stepper.encoder.getAngleMoved();
      //Serial.println("Stop mode");
    }
    break;
  
    case 'd'://in pause
    this->pausePage(loopMode,brakeFlag,place);
    if(play == 1)//unpause
    {
      state = 'b';
      //Serial.println("Play mode");
    }
    else if(play == 2)//stop
    {
      state = 'a';
      setPoint = stepper.encoder.getAngleMoved();
      //Serial.println("Stop mode");
    }
    break;
  }
}

void egoShield::inputs(void)
{
  fw = this->buttonState(FWBT,0);
  play = this->buttonState(PLBT,1);
  rec = this->buttonState(RECBT,2);
  bw = this->buttonState(BWBT,3);
}

uint8_t egoShield::buttonState(uint8_t button, uint8_t nmbr)
{
  uint32_t count = 0;
  uint8_t push = 0;
  if(digitalRead(button)==0)
  {
    while(digitalRead(button)==0 && longPushFlag[nmbr] == 0)
    {
      count++;
      if(count>=5000)//short press
      {
        push = 1;
      }
      if(count>=100000)//long press
      {
        push = 2;
        longPushFlag[nmbr] = 1;
      }
    }
  }
  else//no press
  {
    push = 0;
    longPushFlag[nmbr] = 0;
  }
  return push;
}

void egoShield::fastForward(void)
{
  setPoint +=10;
  stepper.moveToAngle(setPoint,brakeFlag);
  while(digitalRead(FWBT)==0)
  {
    setPoint +=10;
    stepper.moveToAngle(setPoint,brakeFlag);
    this->idlePage(brakeFlag,stepper.encoder.getAngleMoved());
  }    
  while(stepper.getMotorState())
  {
    delay(1);
  }
  stepper.hardStop(brakeFlag);
}

void egoShield::fastBackward(void)
{
  setPoint -=10;
  stepper.moveToAngle(setPoint,brakeFlag);
  while(digitalRead(BWBT)==0)
  {
    setPoint -=10;
    stepper.moveToAngle(setPoint,brakeFlag);
    this->idlePage(brakeFlag,stepper.encoder.getAngleMoved());
  }  
  while(stepper.getMotorState())
  {
    delay(1);
  }
  stepper.hardStop(brakeFlag);
}

void egoShield::startPage(void)
{
  u8g2->firstPage();
  do {
    u8g2->drawXBM(19, 20, logo_width, logo_height, logo_bits);
  } while ( u8g2->nextPage() );
}

void egoShield::idlePage(bool brakeMode, float pos)
{
  char buf[16];
  String sBuf;
  
  u8g2->firstPage();
  do {
    u8g2->drawBox(1, 1, 128, 12);
    u8g2->drawBox(1, 48, 128, 68);
    u8g2->setFontMode(0);
    u8g2->setDrawColor(0);
    u8g2->setFontDirection(0);
    u8g2->setFont(u8g2_font_6x10_tf);
    
    //Bottom bar
    u8g2->drawXBM(5, 51, en_width, en_height, bw_bits);
    u8g2->drawXBM(112, 51, en_width, en_height, fw_bits);
    u8g2->drawXBM(32, 50, play_width, play_height, play_bits);
    u8g2->drawXBM(43, 51, tt_width, tt_height, stop_bits);
    u8g2->drawXBM(71, 51, tt_width, tt_height, rec_bits);
    u8g2->drawXBM(85, 51, tt_width, tt_height, pse_bits);

    //Mode
    u8g2->drawStr(2,10,"Idle");
    if(brakeMode)
    {
      u8g2->drawStr(45,10,"Brake");
    }
    else
    {
      u8g2->drawStr(45,10,"No Brake");
    }
    u8g2->setFontMode(1);
    u8g2->setDrawColor(1);
    sBuf = "Encoder: ";
    sBuf += (int32_t)pos;
    sBuf += (char)176;
    sBuf.toCharArray(buf, 16);
    u8g2->drawStr(2,35,buf);
  } while ( u8g2->nextPage() );  
}

void egoShield::recordPage(bool brakeMode, bool recorded, uint8_t index, float pos)
{
  char buf[22];//char array buffer
  String sBuf;
    
    u8g2->firstPage();
  do {
    u8g2->drawBox(1, 1, 128, 12);
    u8g2->drawBox(1, 48, 128, 68);
    u8g2->setFontMode(0);
    u8g2->setDrawColor(0);
    u8g2->setFontDirection(0);
    u8g2->setFont(u8g2_font_6x10_tf);
    
    //Bottom bar
    if(brakeMode)
    {
      u8g2->drawXBM(5, 51, en_width, en_height, bw_bits);
      u8g2->drawXBM(112, 51, en_width, en_height, fw_bits);
    }
    u8g2->drawXBM(38, 51, tt_width, tt_height, stop_bits);
    u8g2->drawXBM(76, 51, tt_width, tt_height, rec_bits);

    //Mode
    u8g2->drawStr(2,10,"Record");
    if(brakeMode)
    {
      u8g2->drawStr(45,10,"Brake");
    }
    else
    {
      u8g2->drawStr(45,10,"No Brake");
    }
    u8g2->setFontMode(1);
    u8g2->setDrawColor(1);
    if(recorded)
    {
      sBuf = "Position ";
      sBuf += index;
      sBuf += " recorded";
      sBuf.toCharArray(buf, 22);
      u8g2->drawStr(2,35,buf);
    }
    else
    {
    sBuf = "Encoder: ";
    sBuf += (int32_t)pos;
    sBuf += (char)176;
    sBuf.toCharArray(buf, 16);
    u8g2->drawStr(2,35,buf);
    }
  } while ( u8g2->nextPage() );  
}

void egoShield::playPage(bool loopMode, bool brakeMode, uint8_t index)
{
  char buf[3];//char array buffer
    
    u8g2->firstPage();
  do {
    u8g2->drawBox(1, 1, 128, 12);
    u8g2->drawBox(1, 48, 128, 68);
    u8g2->setFontMode(0);
    u8g2->setDrawColor(0);
    u8g2->setFontDirection(0);
    u8g2->setFont(u8g2_font_6x10_tf);

    if(loopMode)
    {
      u8g2->drawXBM(110, 2, loop_width, loop_height, loop_bits);
    }
    
    //Bottom bar
    u8g2->drawXBM(5, 51, en_width, en_height, bw_bits);
    u8g2->drawXBM(112, 51, en_width, en_height, fw_bits);
    u8g2->drawXBM(32, 50, play_width, play_height, play_bits);
    u8g2->drawXBM(43, 51, tt_width, tt_height, stop_bits);
    u8g2->drawXBM(77, 51, tt_width, tt_height, pse_bits);

    //Mode
    u8g2->drawStr(2,10,"Play");
    if(brakeMode)
    {
      u8g2->drawStr(45,10,"Brake");
    }
    else
    {
      u8g2->drawStr(45,10,"No Brake");
    }
    u8g2->setFontMode(1);
    u8g2->setDrawColor(1);
    u8g2->drawStr(2,35,"Moving to pos");
    String(index).toCharArray(buf, 3);
    u8g2->drawStr(90,35,buf);
  } while ( u8g2->nextPage() );  
}

void egoShield::pausePage(bool loopMode, bool brakeMode, uint8_t index)
{
  char buf[3];//char array buffer
    
    u8g2->firstPage();
  do {
    u8g2->drawBox(1, 1, 128, 12);
    u8g2->drawBox(1, 48, 128, 68);
    u8g2->setFontMode(0);
    u8g2->setDrawColor(0);
    u8g2->setFontDirection(0);
    u8g2->setFont(u8g2_font_6x10_tf);
    
    if(loopMode)
    {
      u8g2->drawXBM(110, 2, loop_width, loop_height, loop_bits);
    }
    
    //Bottom bar
    u8g2->drawXBM(32, 50, play_width, play_height, play_bits);
    u8g2->drawXBM(43, 51, tt_width, tt_height, stop_bits);

    //Mode
    u8g2->drawStr(2,10,"Pause");
    if(brakeMode)
    {
      u8g2->drawStr(45,10,"Brake");
    }
    else
    {
      u8g2->drawStr(45,10,"No Brake");
    }
    u8g2->setFontMode(1);
    u8g2->setDrawColor(1);
    u8g2->drawStr(2,35,"Paused at pos");
    String(index).toCharArray(buf, 3);
    u8g2->drawStr(90,35,buf);
  } while ( u8g2->nextPage() );  
}