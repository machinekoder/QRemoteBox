/**
 * @file remotecontrol.h
 * @author Alexander Rössler
 * @brief Implements various function for IR, ISM and SRD capturing and playing
 * @date 10-12-2012
 */
#pragma once

#include <pwm.h>
#include <timer.h>
#include <gpio.h>
#include <pincon.h>
#include <circularbuffer.h>
#include <rfm12.h>
#include <led.h>
#include <switch.h>

#include "remotecommand.h"

static const uint32 IrTimeout = 30000u;  // Time that should pass until a timeout (end of frame) occurs
static const uint32 IrWaitTimeout = 15000u;  // Time that should pass until a capture event will be aborted
static const uint8  IrRxPort = 0u;
static const uint8  IrRxPin = 2u;
static const uint8  IrTxPort = 1u;
static const uint8  IrTxPin = 18u;
static const Pwm    IrTxPwm = Pwm0;
static const uint8  IrDetectPort = 0u;
static const uint8  IrDetectPin = 3u;
static const uint8          IrRxEnablePort = 1u;
static const uint8          IrRxEnablePin = 1u;
static const Switch_Type    IrRxEnableType      = Switch_Type_LowActive;
static const Pin_Mode       IrRxEnablePinMode   = Pin_Mode_NoPullUpDown;
static const Pin_OpenDrain  IrRxEnableOpenDrain = Pin_OpenDrain_Enable;
static const Switch         IrRxEnableSwitch    = Switch1;
static const uint8          IrDetectEnablePort = 1u;
static const uint8          IrDetectEnablePin = 0u;
static const Switch_Type    IrDetectEnableType  = Switch_Type_LowActive;
static const Pin_Mode       IrDetectEnablePinMode = Pin_Mode_NoPullUpDown;
static const Pin_OpenDrain  IrDetectEnableOpenDrain = Pin_OpenDrain_Enable;
static const Switch         IrDetectEnableSwitch = Switch2;

static const uint8  Ism433MhzRxPort = 0u;
static const uint8  Ism433MhzRxPin = 22u;
static const uint8  Ism433MhzEnablePort = 1u;
static const uint8  Ism433MhzEnablePin = 19u;
static const Switch_Type    Ism433MhzEnableType = Switch_Type_LowActive;
static const Pin_Mode       Ism433MhzEnablePinMode = Pin_Mode_NoPullUpDown;
static const Pin_OpenDrain  Ism433MhzEnableOpenDrain = Pin_OpenDrain_Enable;
static const Switch         Ism433MhzEnableSwitch = Switch3;

static const uint32 Radio433MhzTimeout = 9000u;
static const uint32 Radio433MhzWaitTimeout = 15000u;
static const uint8  Radio433MhzRxPort = 0u;
static const uint8  Radio433MhzRxPin = 10u;
static const uint8  Radio433MhzSelPort = 1u;
static const uint8  Radio433MhzSelPin = 28u;
static const uint8  Radio433MhzDclkPort = 0u;
static const uint8  Radio433MhzDclkPin = 1u;
static const uint8  Radio433MhzIntPort = 2u;
static const uint8  Radio433MhzIntPin = 0u;
static const uint8  Radio433MhzIrqPort = 0u;
static const uint8  Radio433MhzIrqPin = 11u;
static const uint8  Radio433MhzSckPort = 0u;
static const uint8  Radio433MhzSckPin = 15u;
static const Pin_Function Radio433MhzSckFunction = Pin_Function_SecondAlternate;
static const uint8  Radio433MhzMisoPort = 0u;
static const uint8  Radio433MhzMisoPin = 17u;
static const Pin_Function Radio433MhzMisoFunction = Pin_Function_SecondAlternate;
static const uint8  Radio433MhzMosiPort = 0u;
static const uint8  Radio433MhzMosiPin = 18u;
static const Pin_Function Radio433MhzMosiFunction = Pin_Function_SecondAlternate;
static const Ssp    Radio433MhzSsp = Ssp0;
static const Rfm12  Radio433MhzRfm12 = Rfm12_0;

static const uint32 Radio868MhzTimeout = 9000u;
static const uint32 Radio868MhzWaitTimeout = 15000u;
static const uint8  Radio868MhzRxPort = 0u;
static const uint8  Radio868MhzRxPin = 25u;
static const uint8  Radio868MhzSelPort = 0u;
static const uint8  Radio868MhzSelPin = 6u;
static const uint8  Radio868MhzDclkPort = 2u;
static const uint8  Radio868MhzDclkPin = 5u;
static const uint8  Radio868MhzIntPort = 2u;
static const uint8  Radio868MhzIntPin = 4u;
static const uint8  Radio868MhzIrqPort = 0u;
static const uint8  Radio868MhzIrqPin = 26u;
static const uint8  Radio868MhzSckPort = 0u;
static const uint8  Radio868MhzSckPin = 7u;
static const Pin_Function Radio868MhzSckFunction = Pin_Function_SecondAlternate;
static const uint8  Radio868MhzMisoPort = 0u;
static const uint8  Radio868MhzMisoPin = 8u;
static const Pin_Function Radio868MhzMisoFunction = Pin_Function_SecondAlternate;
static const uint8  Radio868MhzMosiPort = 0u;
static const uint8  Radio868MhzMosiPin = 9u;
static const Pin_Function Radio868MhzMosiFunction = Pin_Function_SecondAlternate;
static const Ssp    Radio868MhzSsp = Ssp1;
static const Rfm12  Radio868MhzRfm12 = Rfm12_1;

void RemoteControl_initializeRadio433();
void RemoteControl_deinitializeRadio433();
int8 RemoteControl_initialize(void);

void RemoteControl_startCapture(RemoteCommand_Medium medium);
void RemoteControl_stopCapture(void);
uint8 RemoteControl_isCaptureFinished(void);
RemoteCommand* RemoteControl_command(void);

void RemoteControl_runCommand(RemoteCommand *command);
void RemoteControl_stopCommand(void);
uint8 RemoteControl_isCommandRunning(void);

void RemoteControl_setReceiveTimeout(RemoteCommand_Medium medium, uint32 timeout);
void RemoteControl_setSendTimeout(RemoteCommand_Medium medium, uint32 timeout);
void RemoteControl_setRepeatCount(RemoteCommand_Medium medium, uint8 count);
