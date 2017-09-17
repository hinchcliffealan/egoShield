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

void egoShield::setup(uint16_t acc, uint16_t vel, uint8_t uStep, uint16_t fTol, uint16_t fHys, float P, float I, float D, bool config, float res)//mode? timelapse or the other mode, maybe a res deg to mm? brake mode?
{
  this->acceleration = acc;
  this->velocity = vel;
  this->microStepping = uStep;
  this->faultTolerance = fTol;
  this->faultHysteresis = fHys;
  this->pTerm = P;
  this->iTerm = I;
  this->dTerm = D;
  this->resolution = res;


  brakeFlag = 1;
  

  stepper.setup(PID,this->microStepping,this->faultTolerance,this->faultHysteresis,this->pTerm,this->iTerm,this->dTerm,1);
  stepper.encoder.setHome();
  pidFlag = 1;//enable PID
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
  u8g2->begin();//start display
  this->startPage();//show startpage
  delay(2000);//for 2 seconds
  state = 'a';//start in idle
  if(config)
  {
    #define TIMELAPSES
  }
}

void egoShield::loop(void)
{
  this->inputs();//check buttons
  switch (state)
  {
    case 'a'://if we are in idle
    idleMode();
    break;
    
    case 'b'://if we are in play
    #ifdef TIMELAPSES
    state = 'a';
    break;
    #endif
    playMode();
    break;
    
    case 'c'://if we are in record
    #ifdef TIMELAPSES
    timeMode();
    break;
    #endif
    recordMode();
    break;
  
    case 'd'://in pause
    pauseMode();
    break;
  }
}

void egoShield::idleMode(void)
{
  this->idlePage(pidFlag,stepper.encoder.getAngleMoved());
  if(play == 2)//if play/stop/pause is pressed for long time, invert the pid mode
  {
    if(pidFlag == 0)
    {
      pidFlag = 1;
      stepper.setup(PID,this->microStepping,this->faultTolerance,this->faultHysteresis,this->pTerm,this->iTerm,this->dTerm,0);//pause PID to allow manual movement
      this->idlePage(pidFlag,stepper.encoder.getAngleMoved());
    }
    else
    {
      pidFlag = 0;
      stepper.setup(NORMAL,this->microStepping,this->faultTolerance,this->faultHysteresis,this->pTerm,this->iTerm,this->dTerm,0);//pause PID to allow manual movement
      stepper.hardStop(SOFT);
      this->idlePage(pidFlag,stepper.encoder.getAngleMoved());
    }
  }
  else if(fw == 1 || fw == 2)//if manual forward signal
  {
    this->manForward();
  }
  else if(bw == 1 || bw == 2)//if manual backward signal
  {
    this->manBackward();
  }
  else if(rec == 2)
  {
    state = 'c';
  }
  else if(play == 1)//we want to play sequence when doing a short press
  {
    state = 'b';
  }
}

void egoShield::playMode(void)
{
  this->playPage(loopMode,pidFlag,place);
  while(play != 1)
  {
    if(fw == 1 && this->velocity <= 9900 && this->acceleration <= 19900)//increase speed
      {
        this->velocity+=100;
        this->acceleration+=100;
        this->playPage(loopMode,pidFlag,place);
      }
      else if(bw == 1 && this->velocity >= 200 && this->acceleration >= 200)//decrease speed
      {
        this->velocity-=100;
        this->acceleration-=100;
        this->playPage(loopMode,pidFlag,place);
      }
  }  
  stepper.setMaxVelocity(this->velocity);
  stepper.setMaxAcceleration(this->acceleration);
  if(loopMode && place > endmove)
  {
    place=0;
  }
  else if(place > endmove)//If we are at the end move
  {
    place = 0;//reset array counter
    state = 'a';
    setPoint = pos[endmove];
  }
  else
  {
    stepper.moveToAngle(pos[place],brakeFlag);  
    while(stepper.getMotorState())
    {
      this->inputs();//check inputs
      if(play == 2)//if play/stop/pause is pressed again for long time, stop
      {
        place = 0;//reset array counter
        loopMode = 0;
        state = 'a';
        setPoint = stepper.encoder.getAngleMoved();
      }
      else if(rec == 1)
      {
        state = 'd';//pause
      }
      else if(fw == 1 && this->velocity <= 9900 && this->acceleration <= 19900)//increase speed
      {
        this->velocity+=100;
        this->acceleration+=100;
        this->playPage(loopMode,pidFlag,place);
      }
      else if(fw == 2)//loop mode start
      {
        loopMode = 1;
        this->playPage(loopMode,pidFlag,place);
      }
      else if(bw == 1 && this->velocity >= 200 && this->acceleration >= 200)//decrease speed
      {
        this->velocity-=100;
        this->acceleration-=100;
        this->playPage(loopMode,pidFlag,place);
      }
      else if(bw == 2)//loop mode stop
      {
        loopMode = 0;
        this->playPage(loopMode,pidFlag,place);
      }
    }
    place++;//increment array counter
  }  
  stepper.setMaxVelocity(1000);
  stepper.setMaxAcceleration(1500);
}

void egoShield::recordMode(void)
{
  this->recordPage(pidFlag,0,place,stepper.encoder.getAngleMoved());
  if(fw == 1 || fw == 2)//if manual forward signal
  {
    this->manForward();
  }
  else if(bw == 1|| bw == 2)//if manual backward signal
  {
    this->manBackward();
  }
  else if(rec == 1)//record position
  {      
    if(record == 0)//If we were not recording before
    {
      stepper.encoder.setHome();//Set current position as home
      setPoint = 0;
      place = 0;//Reset the array counter
      record = 1;//Record flag
    }
    this->recordPage(pidFlag,1,place,stepper.encoder.getAngleMoved());
    delay(500);
    if(record == 1)//If we have initialized recording
    {
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
    endmove = place-1;//set the endmove to the current position
    place = 0;//reset array counter
    record = 0;//reset record flag
    state = 'a';//stop state
    setPoint = stepper.encoder.getAngleMoved();
  }  
}

void egoShield::pauseMode(void)
{
  this->pausePage(loopMode,pidFlag,place);
  if(play == 1)//unpause
  {
    state = 'b';
  }
  else if(play == 2)//stop
  {
    state = 'a';
    setPoint = stepper.encoder.getAngleMoved();
  }  
}

void egoShield::timeMode(void)
{  
  uint8_t step = 0;
  this->timePage(step,pidFlag);
  while(step == 0)//first put in how long to move at every step in mm
  {
    this->inputs();//check buttons
    this->timePage(step,pidFlag);
    if(fw == 1 && stepSize < 65000)
    {
      stepSize=stepSize+5;
    }
    else if(bw == 1 && stepSize > 10)
    {
      stepSize=stepSize-5;
    }
    else if(rec == 1)
    {
      step = 1;
    }
  }
  while(step == 1)//next put in how long the intervals between moves are in milliseconds
  {
    this->inputs();//check buttons
    this->timePage(step,pidFlag);
    if(fw == 1 && interval < 65000)
    {
      interval=interval+25;
    }
    else if(bw == 1 && interval > 50)
    {
      interval=interval-25;
    }
    else if(rec == 1)
    {
      step = 2;
    }
  }
  while(step == 2)//ready to play
  {
    this->inputs();//check buttons
    this->timePage(step,pidFlag);   
    if(play == 2)
    {
      state = 'a';
      step = 0;
      break;
    }
    else if(play == 1)
    {
      step = 3;
    }
  }
  while(step == 3 && !endOfRail)//playing until the end
  {
    this->inputs();//check buttons
    this->timePage(step,pidFlag);    
    if(play == 2)
    {
      state = 'a';
      step = 0;
      break;
    }
    else
    {
      setPoint = stepper.encoder.getAngleMoved();
      setPoint +=(stepSize);
      stepper.moveToAngle(setPoint,brakeFlag);
      while(stepper.getMotorState())
      {
        this->timePage(step,pidFlag);
      }
      digitalWrite(OPTO, LOW);   // sets the LED in the opto on triggering the camera
      delay(200);              // waits for a 200 milli seconds to allow camera to realise trigger has been fired
      digitalWrite(OPTO, HIGH);  // sets the LED in the opto off releases the camera trigger
      delay(interval);
    }
  }

  setPoint =setPoint-10;//back off a little
  stepper.moveToAngle(setPoint,brakeFlag); 
  state = 'a';//idle state

  //first put in how long to move at every step (mark line)
  //then press rec, to save this (increment a counter)
  //next put in how long the intervals between moves are (mark line)
  //press rec to save it 
  //start timelapse with settings, and end when end of rail is reached (show ready, and press play to start)
  //stop when reaching end of rail

  //if pid error above some threshold, give signal
 
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
  uint16_t count = 0;
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
      if(count>=65500)//long press
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

void egoShield::manForward(void)
{
  setPoint = stepper.encoder.getAngleMoved();
  setPoint +=5;
  stepper.moveToAngle(setPoint,brakeFlag);
  while(digitalRead(FWBT)==0 || stepper.getMotorState())
  {
    if(digitalRead(FWBT)==0)//fast forward
    {
      setPoint +=10;
      stepper.moveToAngle(setPoint,brakeFlag);
    }
    if(state == 'a')
    {
      this->idlePage(pidFlag,stepper.encoder.getAngleMoved());
    }
    else
    {
      this->recordPage(pidFlag,0,place,stepper.encoder.getAngleMoved());
    }
  }    
}

void egoShield::manBackward(void)
{
  setPoint = stepper.encoder.getAngleMoved();
  setPoint -=5;
  stepper.moveToAngle(setPoint,0);
  while(digitalRead(BWBT)==0 || stepper.getMotorState())
  {
    if(digitalRead(BWBT)==0)//fast backward
    {
      setPoint -=10;
      stepper.moveToAngle(setPoint,brakeFlag);
    }
    if(state == 'a')
    {
      this->idlePage(pidFlag,stepper.encoder.getAngleMoved());
    }
    else
    {
      this->recordPage(pidFlag,0,place,stepper.encoder.getAngleMoved());
    }
  }  
}

void egoShield::startPage(void)
{
  u8g2->firstPage();
  do {
    u8g2->drawXBM(19, 20, logo_width, logo_height, logo_bits);
  } while ( u8g2->nextPage() );
}

void egoShield::idlePage(bool pidMode, float pos)
{
  char buf[18];
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
    if(pidMode)
    {
      u8g2->drawStr(45,10,"PID ON");
    }
    else
    {
      u8g2->drawStr(45,10,"PID OFF");
    }
    u8g2->setFontMode(1);
    u8g2->setDrawColor(1);
    sBuf = "Encoder: ";
    sBuf += (int32_t)pos;
    sBuf += (char)176;
    sBuf.toCharArray(buf, 18);
    u8g2->drawStr(2,35,buf);
  } while ( u8g2->nextPage() );  
}

void egoShield::recordPage(bool pidMode, bool recorded, uint8_t index, float pos)
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
    
    u8g2->drawXBM(5, 51, en_width, en_height, bw_bits);
    u8g2->drawXBM(112, 51, en_width, en_height, fw_bits);
    u8g2->drawXBM(38, 51, tt_width, tt_height, stop_bits);
    u8g2->drawXBM(76, 51, tt_width, tt_height, rec_bits);

    //Mode
    u8g2->drawStr(2,10,"Record");
    if(pidMode)
    {
      u8g2->drawStr(45,10,"PID ON");
    }
    else
    {
      u8g2->drawStr(45,10,"PID OFF");
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
    sBuf.toCharArray(buf, 22);
    u8g2->drawStr(2,35,buf);
    }
  } while ( u8g2->nextPage() );  
}

void egoShield::playPage(bool loopMode, bool pidMode, uint8_t index)
{
  char buf[5];//char array buffer
    
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
    if(pidMode)
    {
      u8g2->drawStr(45,10,"PID ON");
    }
    else
    {
      u8g2->drawStr(45,10,"PID OFF");
    }
    u8g2->setFontMode(1);
    u8g2->setDrawColor(1);
    u8g2->drawStr(2,25,"Moving to pos");
    String(index).toCharArray(buf, 5);
    u8g2->drawStr(90,25,buf);
    u8g2->drawStr(2,40,"Speed:");
    String(this->velocity).toCharArray(buf, 5);
    u8g2->drawStr(60,40,buf);
  } while ( u8g2->nextPage() );  
}

void egoShield::pausePage(bool loopMode, bool pidMode, uint8_t index)
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
    if(pidMode)
    {
      u8g2->drawStr(45,10,"PID ON");
    }
    else
    {
      u8g2->drawStr(45,10,"PID OFF");
    }
    u8g2->setFontMode(1);
    u8g2->setDrawColor(1);
    u8g2->drawStr(2,35,"Paused at pos");
    String(index).toCharArray(buf, 3);
    u8g2->drawStr(90,35,buf);
  } while ( u8g2->nextPage() );  
}


void egoShield::timePage(uint8_t step, bool pidMode)
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
        if(step == 0)//we are waiting for the distance interval to be put in
    {
      u8g2->drawXBM(5, 51, en_width, en_height, bw_bits);
      u8g2->drawXBM(112, 51, en_width, en_height, fw_bits);
      u8g2->drawXBM(76, 51, tt_width, tt_height, rec_bits);
      u8g2->setFontMode(1);
      u8g2->setDrawColor(1);
      u8g2->drawStr(110,24,"<-");
      u8g2->setFontMode(0);
      u8g2->setDrawColor(0);
    }
    else if(step == 1)//we are waiting for the time interval to be put in
    {
      u8g2->drawXBM(5, 51, en_width, en_height, bw_bits);
      u8g2->drawXBM(112, 51, en_width, en_height, fw_bits);
      u8g2->drawXBM(76, 51, tt_width, tt_height, rec_bits);
      u8g2->setFontMode(1);
      u8g2->setDrawColor(1);
      u8g2->drawStr(110,34,"<-");
      u8g2->setFontMode(0);
      u8g2->setDrawColor(0);
    }
    else if(step == 2)//we are waiting for play to be issued
    {
      u8g2->drawXBM(32, 50, play_width, play_height, play_bits);
      u8g2->drawXBM(38, 51, tt_width, tt_height, stop_bits);
    }
    else if(step == 3)//we are playing sequence until end of rail
    {
      u8g2->drawXBM(38, 51, tt_width, tt_height, stop_bits);
    }

    u8g2->drawStr(2,10,"Time");
    if(pidMode)
    {
      u8g2->drawStr(45,10,"PID ON");
    }
    else
    {
      u8g2->drawStr(45,10,"PID OFF");
    }
    u8g2->setFontMode(1);
    u8g2->setDrawColor(1);
    sBuf = "Stepsize:  ";
    sBuf += (uint32_t)(stepSize*resolution);
    sBuf += " mm";
    sBuf.toCharArray(buf, 22);
    u8g2->drawStr(2,24,buf);
    sBuf = "Interval:  ";
    sBuf += interval*0.001;
    sBuf += " s";
    sBuf.toCharArray(buf, 22);
    u8g2->drawStr(2,34,buf); 
    sBuf = "Encoder:   ";
    sBuf += (int32_t)(stepper.encoder.getAngleMoved()*resolution);
    sBuf += " mm";
    sBuf.toCharArray(buf, 22);
    u8g2->drawStr(2,44,buf);
  } while ( u8g2->nextPage() );  
}