#include "remotecontrol.h"

static RemoteCommand_Medium currentMedium;

static uint8  frameReceived;
static uint8  firstCapture;
static uint8  dataPosition;
static uint8  commandRunning;

static uint32 ismSmallClock;
static uint32 ismBigClock;
static uint32 ismClockTolerance;

static uint8  repeatCount;
static uint8  currentRepeatCount;

static uint32 receiveTimeout;
static uint32 sendTimeout;
static uint8  minTransitions;

static uint32 irReceiveTimeout;
static uint32 irSendTimeout;
static uint8  irRepeatCount;

static uint32 radio433MhzReceiveTimeout;
static uint32 radio433MhzSendTimeout;
static uint8  radio433MhzRepeatCount;

static uint32 radio868MhzReceiveTimeout;
static uint32 radio868MhzSendTimeout;
static uint8  radio868MhzRepeatCount;

static RemoteCommand captureCommand;
static RemoteCommand *sendCommand;

static uint32 frequencyCaptureCount;

static void irCaptureFunction(void);
static void ismCaptureFunction(void);
static void runFunction(void);
static void frequencyCaptureFunction(void);
static uint32 frequencyCaptureCalculate(uint32 transitions, RemoteCommand* remoteCommand);
static void finalizeCapture();

static void initializeVariables();
int8 initializeIrTx();
static void initializeIrRx();
static void initializeIrDetect();
static void initializeIsm433Rx();
static void initializeRadio868();

void initializeVariables()
{
    frameReceived = false;
    firstCapture = true;
    dataPosition = 0u;
    commandRunning = false;

    ismSmallClock = 300u;
    ismBigClock = 1000u;
    ismClockTolerance = 150u;

    minTransitions = 5u;

    irReceiveTimeout = 20000u;
    irSendTimeout = 50000u;
    irRepeatCount = 4u;

    radio433MhzReceiveTimeout = 9000u;
    radio433MhzSendTimeout = 10000u;
    radio433MhzRepeatCount = 5u;

    radio868MhzReceiveTimeout = 9000u;
    radio868MhzSendTimeout = 10000u;
    radio868MhzRepeatCount = 5u;
}

// Initialize IR TX
int8 initializeIrTx()
{
    Gpio_setDirection(IrTxPort, IrTxPin, Gpio_Direction_Output);  // PWM
    Pin_setMode(IrTxPort, IrTxPin, Pin_Mode_PullDown);
    
    if (Pwm_initialize(38000u, 0.5, IrTxPwm) == (int8)(-1))
    {
        return (int8)(-1);
    }
    
    return (int8)0;
}

// Initialize IR RX
void initializeIrRx()
{
    Gpio_setDirection(IrRxPort, IrRxPin, Gpio_Direction_Input); // TSOP input pin
    Pin_setMode(IrRxPort, IrRxPin, Pin_Mode_NoPullUpDown);
    Switch_initializeSwitch(IrRxEnableSwitch, 
                            IrRxEnablePort, IrRxEnablePin, 
                            IrRxEnableType, IrRxEnablePinMode, IrRxEnableOpenDrain);
    Switch_off(IrRxEnableSwitch); // Enable IR RX for testing
}

// Initialize IR DETECT
void initializeIrDetect()
{
    Gpio_setDirection(IrDetectPort, IrDetectPin, Gpio_Direction_Input);
    Pin_setMode(IrDetectPort, IrDetectPin, Pin_Mode_NoPullUpDown);
    Switch_initializeSwitch(IrDetectEnableSwitch, 
                            IrDetectEnablePort, IrDetectEnablePin,
                            IrDetectEnableType, IrDetectEnablePinMode, IrDetectEnableOpenDrain);
    Switch_off(IrDetectEnableSwitch); // Enable IR DETECT for testing
}

// Initialize 433Mhz receiver module
void initializeIsm433Rx()
{
    Gpio_setDirection(Ism433MhzRxPort, Ism433MhzRxPin, Gpio_Direction_Input);
    Pin_setMode(Ism433MhzRxPort, Ism433MhzRxPin, Pin_Mode_NoPullUpDown);
    Switch_initializeSwitch(Ism433MhzEnableSwitch,
                            Ism433MhzEnablePort, Ism433MhzEnablePin,
                            Ism433MhzEnableType, Ism433MhzEnablePinMode, Ism433MhzEnableOpenDrain);
    Switch_on(Ism433MhzEnableSwitch); // Enable ISM module for testing
}

// Initialize 433MHz module
void RemoteControl_initializeRadio433()
{
    Gpio_Pair selPair;
    Gpio_Pair dataPair;
    
    Pin_setFunction(Radio433MhzSckPort, Radio433MhzSckPin, Radio433MhzSckFunction);
    Pin_setMode(Radio433MhzSckPort, Radio433MhzSckPin, Pin_Mode_NoPullUpDown);
    Pin_setFunction(Radio433MhzMisoPort, Radio433MhzMisoPin, Radio433MhzMisoFunction);
    Pin_setMode(Radio433MhzMisoPort, Radio433MhzMisoPin, Pin_Mode_NoPullUpDown);
    Pin_setFunction(Radio433MhzMosiPort, Radio433MhzMosiPin, Radio433MhzMosiFunction);
    Pin_setMode(Radio433MhzMosiPort, Radio433MhzMosiPin, Pin_Mode_NoPullUpDown);
    Gpio_setDirection(Radio433MhzDclkPort, Radio433MhzDclkPin, Gpio_Direction_Input);
    Gpio_setDirection(Radio433MhzIntPort, Radio433MhzIntPin, Gpio_Direction_Input);
    Gpio_setDirection(Radio433MhzIrqPort, Radio433MhzIrqPin, Gpio_Direction_Input);
    selPair.port = Radio433MhzSelPort;
    selPair.pin = Radio433MhzSelPin;
    dataPair.port = Radio433MhzRxPort;
    dataPair.pin = Radio433MhzRxPin;
    Rfm12_initialize(Radio433MhzRfm12, Radio433MhzSsp, selPair, dataPair);
    Rfm12_disablePower(Radio433MhzRfm12);
}

void RemoteControl_deinitializeRadio433()
{
     Pin_setFunction(Radio433MhzSckPort, Radio433MhzSckPin, Pin_Function_Primary);
     Pin_setFunction(Radio433MhzMisoPort, Radio433MhzMisoPin, Pin_Function_Primary);
     Pin_setFunction(Radio433MhzMosiPort, Radio433MhzMosiPin, Pin_Function_Primary);
}

// Initialize 868MHz module
void initializeRadio868()
{
    Gpio_Pair selPair;
    Gpio_Pair dataPair;
    
    Pin_setFunction(Radio868MhzSckPort, Radio868MhzSckPin, Radio868MhzSckFunction);
    Pin_setMode(Radio868MhzSckPort, Radio868MhzSckPin, Pin_Mode_NoPullUpDown);
    Pin_setFunction(Radio868MhzMisoPort, Radio868MhzMisoPin, Radio868MhzMisoFunction);
    Pin_setMode(Radio868MhzMisoPort, Radio868MhzMisoPin, Pin_Mode_NoPullUpDown);
    Pin_setFunction(Radio868MhzMosiPort, Radio868MhzMosiPin, Radio868MhzMisoFunction);
    Pin_setMode(Radio868MhzMosiPort, Radio868MhzMosiPin, Pin_Mode_NoPullUpDown);
    Gpio_setDirection(Radio868MhzDclkPort, Radio868MhzDclkPin, Gpio_Direction_Input);
    Gpio_setDirection(Radio868MhzIntPort, Radio868MhzIntPin, Gpio_Direction_Input);
    Gpio_setDirection(Radio868MhzIrqPort, Radio868MhzIrqPin, Gpio_Direction_Input);
    selPair.port = Radio868MhzSelPort;
    selPair.pin = Radio868MhzSelPin;
    dataPair.port = Radio868MhzRxPort;
    dataPair.pin = Radio868MhzRxPin;
    Rfm12_initialize(Radio868MhzRfm12, Radio868MhzSsp, selPair, dataPair);
    Rfm12_disablePower(Radio868MhzRfm12);
}

int8 RemoteControl_initialize(void)
{   
    initializeVariables();
    
    if (initializeIrTx() == (int8)(-1))
    {
        return (int8)(-1);
    }
    initializeIrRx();
    initializeIrDetect();
    initializeIsm433Rx();
    RemoteControl_initializeRadio433();
    initializeRadio868();
    
    if (Timer_initialize(Timer3, 1000u, 1E9) == (int8)(-1)) // Timer used for time measuring
    {
        return (int8)(-1);
    }
    
    return (int8)(0);
}

uint8 RemoteControl_isCaptureFinished(void)
{
    if (frameReceived == true)  // frame was received
    {
        finalizeCapture();      // time for finalizing calculations
        return true;
    }
    else
    {
        return false;
    }
}

RemoteCommand* RemoteControl_command(void)
{
    if (frameReceived == true)
        return &captureCommand;
    else
        return NULL;
}

void RemoteControl_startCapture(RemoteCommand_Medium medium)
{    
    RemoteCommand_initialize(&captureCommand);
    frameReceived = false;
    firstCapture = true;
    dataPosition = 0u;
    
    currentMedium = medium;
    
    if (medium == RemoteCommand_Medium_Ir)
    {
        receiveTimeout = irReceiveTimeout;
        
        Switch_on(IrRxEnableSwitch);        // Turn on IR receiver
        Switch_on(IrDetectEnableSwitch);    // Turn on IR frequency detector
        Timer_delayMs(10u);                 // Let the power settle
        Gpio_enableInterrupt(IrRxPort, 
                             IrRxPin, 
                             Gpio_Interrupt_FallingAndRisingEdge, 
                             &irCaptureFunction);
        Gpio_enableInterrupt(IrDetectPort,
                             IrDetectPin,
                             Gpio_Interrupt_FallingAndRisingEdge,
                             &frequencyCaptureFunction);
    }
    else if (medium == RemoteCommand_Medium_433Mhz)
    {
        receiveTimeout = radio433MhzReceiveTimeout;
#if 0
        Rfm12_prepareOokReceiving(Radio433MhzRfm12, Rfm12_FrequencyBand433Mhz, 433.92, 9600u);
        Timer_delayMs(10u);                 // Let the power settle
        Gpio_enableInterrupt(Radio433MhzRxPort, 
                             Radio433MhzRxPin, 
                             Gpio_Interrupt_FallingAndRisingEdge, 
                             &ismCaptureFunction);
#else
        Switch_on(Ism433MhzEnableSwitch);   // Turn on the ISM receiver module
        Timer_delayMs(10u);                 // Let the power settle
        Gpio_enableInterrupt(Ism433MhzRxPort, 
                             Ism433MhzRxPin, 
                             Gpio_Interrupt_FallingAndRisingEdge, 
                             &ismCaptureFunction);
#endif
    }
    else if (medium == RemoteCommand_Medium_868Mhz)
    {
        receiveTimeout = radio868MhzReceiveTimeout;
        
        Rfm12_prepareOokReceiving(Radio868MhzRfm12, Rfm12_FrequencyBand868Mhz, 868.0, 4200u);
        Gpio_enableInterrupt(Radio868MhzRxPort, 
                             Radio868MhzRxPin, 
                             Gpio_Interrupt_FallingAndRisingEdge, 
                             &ismCaptureFunction);
    }
    else
    {
        return;
    }
    
    Timer_connectFunction(Timer3, NULL);
    Timer_setIntervalUs(Timer3, 1E6);
    Timer_reset(Timer3);
}

void RemoteControl_stopCapture(void)
{
    Timer_stop(Timer3); // Stop the time timer
    
    if (currentMedium == RemoteCommand_Medium_Ir)
    {
        Gpio_disableInterrupt(IrRxPort, IrRxPin);
        Gpio_disableInterrupt(IrDetectPort, IrDetectPin);
        Switch_off(IrRxEnableSwitch);               // Disable IR receiver module
        Switch_off(IrDetectEnableSwitch);           // Disable IR frequency detector
    }
    else if (currentMedium == RemoteCommand_Medium_433Mhz)
    {
        Gpio_disableInterrupt(Ism433MhzRxPort, Ism433MhzRxPort);
#if 0
        Rfm12_ookClear(Radio433MhzRfm12);                    // workaround
        Rfm12_disablePower(Radio433MhzRfm12);                // Disable SRD receiver
#else
        Switch_off(Ism433MhzEnableSwitch);          // Disable ISM receiver
#endif
    }
    else if (currentMedium == RemoteCommand_Medium_868Mhz)
    {
        Gpio_disableInterrupt(Radio868MhzRxPort, Radio868MhzRxPin);
        Rfm12_ookClear(Radio868MhzRfm12);                    // workaround
        Rfm12_disablePower(Radio868MhzRfm12);                // Disable SRD receiver
    }
    else
    { // should never be reached
    }
    
    Led_clear(Led1);    // clear the indicator led
}

void finalizeCapture()
{
    captureCommand.length = dataPosition;
    captureCommand.medium = currentMedium;
    
    if (currentMedium == RemoteCommand_Medium_Ir)
    {
        captureCommand.frequency = frequencyCaptureCalculate(frequencyCaptureCount, &captureCommand);
    }
}

void irCaptureFunction(void)
{
    static uint32 timeDiff;

    timeDiff = Timer_counterValue(Timer3);
        
    if (timeDiff >= receiveTimeout)     // Detected a timeout => frameReceived
    {
        if ((dataPosition > minTransitions) 
            && ((dataPosition % 2u) == 1u))
        {
            RemoteControl_stopCapture();
            frameReceived = true;
            return;
        }
        else
        {
            dataPosition = 0u;
            frequencyCaptureCount = 0u;
            firstCapture = false;
        }
    }
    else if (firstCapture == false)
    {
        if (dataPosition < REMOTE_COMMAND_MAX_TRANSITIONS)
        {
            captureCommand.data[dataPosition] = timeDiff;
            dataPosition++;
        }
        else
        {
            firstCapture = true;
        }
    }
    
    Timer_reset(Timer3);    // Reset the timer
    Led_toggle(Led1);       // visual feedback
}

void frequencyCaptureFunction(void)
{
    frequencyCaptureCount++;
}

uint32 frequencyCaptureCalculate(uint32 transitionCount, RemoteCommand *remoteCommand)
{
    uint32 frequency;
    uint32 timeSum;
    uint8  i;
    
    timeSum = 0u;
    
    for (i = 0u; i < remoteCommand->length; i += 2u)
    {
        timeSum += (uint32)(remoteCommand->data[i]);
    }
    
    frequency = (uint32)((transitionCount*10000u)/(timeSum))*100u; // calculating the frequency in Hz
    
    return frequency;
}

void ismCaptureFunction(void)
{
    static uint32 timeDiff;
    static uint8  verificationCount;
    
    timeDiff = Timer_counterValue(Timer3);
    
    if (timeDiff >= receiveTimeout)     // Detected a timeout => frameReceived
    {
        if ((dataPosition > minTransitions) 
            && ((dataPosition % 2u) == 1u))
        {
            RemoteControl_stopCapture();
            frameReceived = true;
            return;
        }
        else
        {
            dataPosition = 0u;
            firstCapture = false;
            verificationCount = 0b00u;
        }
    }
    else if (firstCapture == false)
    {
        if ((timeDiff > (ismSmallClock - ismClockTolerance))
            && (timeDiff < (ismSmallClock + ismClockTolerance)))
        {
            verificationCount ^= 0b01u;
        }
        else if ((timeDiff > (ismBigClock - ismClockTolerance))
                && (timeDiff < (ismBigClock + ismClockTolerance)))
        {
            verificationCount ^= 0b10u;
        }
        else
        {
        }
        
        if (verificationCount == 0b11u)
        {
            verificationCount = 0b00u;
        }
        else if (verificationCount == 0b00u) // if this is true either no clock was detected or two times the same
        {
            firstCapture = true;
        }
        
        if (dataPosition >= REMOTE_COMMAND_MAX_TRANSITIONS)
        {
            firstCapture = true;
        }
        
        if (firstCapture == false)
        {
            captureCommand.data[dataPosition] = timeDiff;
            dataPosition++;
            Led_toggle(Led1);       // visual feedback
        }
        else
        {
            Led_clear(Led1);       // visual feedback
        }
    }
        
    Timer_reset(Timer3);    // Reset the timer
}

void RemoteControl_runCommand(RemoteCommand* command)
{
    sendCommand = command;
    currentMedium = command->medium;
    dataPosition = 0u;
    commandRunning = true;     // Set the variable which indicates that a command is running
    currentRepeatCount = 0u;
    
    if (currentMedium == RemoteCommand_Medium_Ir)
    {
        sendTimeout = irSendTimeout;
        repeatCount = irRepeatCount;
    }
    else if (currentMedium == RemoteCommand_Medium_433Mhz)
    {
        sendTimeout = radio433MhzSendTimeout;
        repeatCount = radio433MhzRepeatCount;
        
        Rfm12_prepareOokSending(Radio433MhzRfm12, Rfm12_FrequencyBand433Mhz, 433.92, 4200u);
    }
    else if (currentMedium == RemoteCommand_Medium_868Mhz)
    {
        sendTimeout = radio868MhzSendTimeout;
        repeatCount = radio868MhzRepeatCount;
        
        Rfm12_prepareOokSending(Radio868MhzRfm12, Rfm12_FrequencyBand868Mhz, 868.0, 4200u);
    }
    else
    {
        return; // should never be reached
    }
    
    Timer_connectFunction(Timer3, &runFunction);
    Timer_setIntervalUs(Timer3, 3000u);  // let the devices 3ms settle
    Timer_start(Timer3);
}

void RemoteControl_stopCommand(void)
{
    Led_clear(Led1);
    
    if (currentMedium == RemoteCommand_Medium_Ir)
    {
        Pwm_stop(IrTxPwm);
    }
    else if (currentMedium == RemoteCommand_Medium_433Mhz)
    {
        Rfm12_ookClear(Radio433MhzRfm12);
    }
    else if (currentMedium == RemoteCommand_Medium_868Mhz)
    {
        Rfm12_ookClear(Radio868MhzRfm12);
    }
    else
    {   // should never be reached
    }
    
    currentRepeatCount++;
    
    if (currentRepeatCount < repeatCount)  // Repeat the command a few times
    {
        dataPosition = 0u;
        Timer_setIntervalUs(Timer3, sendTimeout);    // Timeout between commands
    }
    else
    {
        Timer_stop(Timer3);
        commandRunning = false;
        
        if (currentMedium == RemoteCommand_Medium_433Mhz)   // enable power saving
        {
            Rfm12_disablePower(Radio433MhzRfm12);
        }
        else if (currentMedium == RemoteCommand_Medium_868Mhz)
        {
            Rfm12_disablePower(Radio868MhzRfm12);
        }
        else
        {
        }
    }
}

void runFunction(void)
{
    uint16 timeout;
    
    if (dataPosition >= sendCommand->length)
    {
        RemoteControl_stopCommand();
        return;
    }
    
    timeout = sendCommand->data[dataPosition];
    
    if (currentMedium == RemoteCommand_Medium_Ir)
    {
        Pwm_toggle(IrTxPwm);
    }
    else if (currentMedium == RemoteCommand_Medium_433Mhz)
    {
        static uint8 outputEnabled;
        
        if (dataPosition == 0u)
        {
            outputEnabled = true;
        }
        
        if (outputEnabled == true)
        {
            timeout += 160u;
            outputEnabled = false;
        }
        else
        {
            timeout -= 160u;
            outputEnabled = true;
        }
        
        Rfm12_ookToggle(Radio433MhzRfm12);
    }
    else if (currentMedium == RemoteCommand_Medium_868Mhz)
    {
        Rfm12_ookToggle(Radio868MhzRfm12);
    }
    else
    {   // should never be reached
    }
    
    dataPosition++;
    Led_toggle(Led1);
    Timer_setIntervalUs(Timer3, (uint32)timeout);
}

uint8 RemoteControl_isCommandRunning(void )
{
    return commandRunning;
}

void RemoteControl_setReceiveTimeout(RemoteCommand_Medium medium, uint32 timeout)
{
    if (medium == RemoteCommand_Medium_Ir)
    {
        irReceiveTimeout = timeout;
    }
    else if (medium == RemoteCommand_Medium_433Mhz)
    {
        radio433MhzReceiveTimeout = timeout;
    }
    else if (medium == RemoteCommand_Medium_868Mhz)
    {
        radio868MhzReceiveTimeout = timeout;
    }
}

void RemoteControl_setSendTimeout(RemoteCommand_Medium medium, uint32 timeout)
{
    if (medium == RemoteCommand_Medium_Ir)
    {
        irSendTimeout = timeout;
    }
    else if (medium == RemoteCommand_Medium_433Mhz)
    {
        radio433MhzSendTimeout = timeout;
    }
    else if (medium == RemoteCommand_Medium_868Mhz)
    {
        radio868MhzSendTimeout = timeout;
    }
}

void RemoteControl_setRepeatCount(RemoteCommand_Medium medium, uint8 count)
{
    if (medium == RemoteCommand_Medium_Ir)
    {
        irRepeatCount = count;
    }
    else if (medium == RemoteCommand_Medium_433Mhz)
    {
        radio433MhzRepeatCount = count;
    }
    else if (medium == RemoteCommand_Medium_868Mhz)
    {
        radio868MhzRepeatCount = count;
    }
}
