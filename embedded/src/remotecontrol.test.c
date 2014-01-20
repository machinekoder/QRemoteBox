/*
 *  Simple example of a CUnit unit test.
 *
 *  This program (crudely) demonstrates a very simple "black box"
 *  test of the standard library functions fprintf() and fread().
 *  It uses suite initialization and cleanup functions to open
 *  and close a common temporary file used by the test functions.
 *  The test functions then write to and read from the temporary
 *  file in the course of testing the library functions.
 *
 *  The 2 test functions are added to a single CUnit suite, and
 *  then run using the CUnit Basic interface.  The output of the
 *  program (on CUnit version 2.0-2) is:
 *
 *           CUnit : A Unit testing framework for C.
 *           http://cunit.sourceforge.net/
 *
 *       Suite: Suite_1
 *         Test: test of fprintf() ... passed
 *         Test: test of fread() ... passed
 *
 *       --Run Summary: Type      Total     Ran  Passed  Failed
 *                      suites        1       1     n/a       0
 *                      tests         2       2       2       0
 *                      asserts       5       5       5       0
 */

//#include <stdio.h>
//#include <string.h>

#include <types.h>

int8 pwmInitializeReturn = 0;
int8 rfm12InitializeReturn = 0;
int8 timerInitializeReturn = 0;
uint32 timerCounterValueReturn = 0u;
int8 timerDelayMsReturn = 0;

/* Function stubs */
#define Gpio_setDirection(x,y,z) pseudoCall()
#define Pin_setMode(x,y,z) pseudoCall()
#define Pwm_initialize(x,y,z) pwmInitializeReturn
#define Switch_initializeSwitch(x,y,z,u,v,w) pseudoCall()
#define Switch_off(x) pseudoCall()
#define Switch_on(x) pseudoCall()
#define Pin_setFunction(x,y,z) pseudoCall()
#define Rfm12_initialize(x,y,z,u) rfm12InitializeReturn
#define Rfm12_disablePower(y) pseudoCall()
#define Timer_initialize(x,y,z) timerInitializeReturn
#define Remotecontrol_startCapture() pseudoCall()
#define RemoteCommand_initialize(x) pseudoCall()
#define Timer_delayMs(x) timerDelayMsReturn
#define Gpio_enableInterrupt(x,y,z,u) pseudoCall()
#define Rfm12_prepareOokReceiving(x,y,z,u) pseudoCall()
#define Pwm_toggle(x) pseudoCall()
#define Rfm12_ookToggle(x) pseudoCall()
#define Led_toggle(x) pseudoCall()
#define Timer_setIntervalUs(x,y) pseudoCall()
#define Timer_connectFunction(x,y) pseudoCall()
#define Timer_reset(x) pseudoCall()
#define Timer_stop(x) pseudoCall()
#define Gpio_disableInterrupt(x,y) pseudoCall()
#define Rfm12_ookClear(x) pseudoCall()
#define Led_clear(x) pseudoCall()
#define Timer_counterValue(x) timerCounterValueReturn
#define Rfm12_prepareOokSending(x,y,z,u) pseudoCall()
#define Timer_start(x) pseudoCall()
#define Pwm_stop(x) pseudoCall()

void printfData(char* bla, ...)
{
}

/** The pseudoCall is used to get some nice code coverage stats */
void pseudoCall(void)
{
}

#include "remotecontrol.c"
#include <CUnit/Basic.h>

/* The suite initialization function.
 * Opens the temporary file used by the tests.
 * Returns zero on success, non-zero otherwise.
 */
int init_suite1(void)
{
   /*if (NULL == (temp_file = fopen("temp.txt", "w+"))) {
      return -1;
   }
   else {
      return 0;
   }*/
   RemoteControl_initialize();
   return 0;
}

/* The suite cleanup function.
 * Closes the temporary file used by the tests.
 * Returns zero on success, non-zero otherwise.
 */
int clean_suite1(void)
{
   /*if (0 != fclose(temp_file)) {
      return -1;
   }
   else {
      temp_file = NULL;
      return 0;
   }*/
   return 0;
}

void testFrequencyCaptureCalculate(void)
{
    uint32 result;
    frequencyCaptureCount = 720u;
    RemoteCommand remoteCommand;
    
    remoteCommand.length = 9u;
    remoteCommand.data[0u] = 1000u;
    remoteCommand.data[1u] = 300u;
    remoteCommand.data[2u] = 300u;
    remoteCommand.data[3u] = 1000u;
    remoteCommand.data[4u] = 1000u;
    remoteCommand.data[5u] = 300u;
    remoteCommand.data[6u] = 1000u;
    remoteCommand.data[7u] = 300u;
    remoteCommand.data[8u] = 300u;
    
    result = frequencyCaptureCalculate(frequencyCaptureCount, &remoteCommand);
    
    CU_ASSERT(result == 100u);
}

void testIrCapture(void)
{
    uint8 i;
    
    RemoteControl_startCapture(RemoteCommand_Medium_Ir);
    
    CU_ASSERT(frameReceived == 0u);
    CU_ASSERT(firstCapture == true);
    CU_ASSERT(dataPosition == 0u);
    
    timerCounterValueReturn = 2000000u;     // 2s delay
    irCaptureFunction();                    // first transition received
    CU_ASSERT(dataPosition == 0u);   // position should not have changed
    CU_ASSERT(firstCapture == false);  
    
    for (i = 0u; i < 9u; i++)  // 9 transitions -> should be correct
    {
        timerCounterValueReturn = 250u; // 250µs delay
        irCaptureFunction();
        CU_ASSERT(dataPosition == (i+1u));   // position moved on
        CU_ASSERT(captureCommand.data[i] == timerCounterValueReturn);
    }
    
    timerCounterValueReturn = 100000u;  // 100ms delay -> timeout
    irCaptureFunction();
    CU_ASSERT(frameReceived == true);
    CU_ASSERT(dataPosition == 9u);
    
    // lets try a bad case
    RemoteControl_startCapture(RemoteCommand_Medium_Ir);    // we do not need to test this again
    
    timerCounterValueReturn = 2000000u;     // 2s delay
    irCaptureFunction();                    // first transition received
    CU_ASSERT(dataPosition == 0u);   // position should not have changed
    
    for (i = 0u; i < 10u; i++)  // 10 transitions -> something went wrong
    {
        timerCounterValueReturn = 250u; // 250µs delay
        irCaptureFunction();
        CU_ASSERT(dataPosition == (i+1u));   // position moved on
        CU_ASSERT(captureCommand.data[i] == timerCounterValueReturn);
    }
    
    timerCounterValueReturn = 100000u;  // 100ms delay -> timeout
    irCaptureFunction();
    CU_ASSERT(frameReceived == false);
    CU_ASSERT(dataPosition == 0u);   // the position should be returned to 0
    
    for (i = 0u; i < REMOTE_COMMAND_MAX_TRANSITIONS; i++)  // trigger a max transitions situation
    {
        timerCounterValueReturn = 250u; // 250µs delay
        irCaptureFunction();
        CU_ASSERT(dataPosition == (i+1u));   // position moved on
        CU_ASSERT(captureCommand.data[i] == timerCounterValueReturn);
    }
    
    timerCounterValueReturn = 250u; // 250µs delay
    irCaptureFunction();
    CU_ASSERT(firstCapture == true);   // we should be at 0 again
    
    timerCounterValueReturn = 100000u;  // 100ms delay -> timeout
    irCaptureFunction();
    CU_ASSERT(frameReceived == false);
    CU_ASSERT(dataPosition == 0u);   // the position should be returned to 0
}

void testIsmCapture(void)
{
    uint8 i;
    uint32_t smallClock;
    uint32_t bigClock;
    
    smallClock = 300u;          // 300µs
    bigClock = 1000u;           // 1000µs
    
    receiveTimeout = 10000u;    // 10ms
    ismSmallClock = smallClock;
    ismBigClock = bigClock;
    ismClockTolerance = 100u;
    
    RemoteControl_startCapture(RemoteCommand_Medium_433Mhz);
    
    CU_ASSERT(frameReceived == 0u);
    CU_ASSERT(firstCapture == 1u);
    CU_ASSERT(dataPosition == 0u);
    
    timerCounterValueReturn = 2000000u;     // 2s delay
    ismCaptureFunction();                   // first transition received
    CU_ASSERT(dataPosition == 0u);       // position should not have changed
    
    timerCounterValueReturn = smallClock;   // small clock
    ismCaptureFunction();                   
    CU_ASSERT(dataPosition == 1u);       // position should have upadated
    
    timerCounterValueReturn = bigClock;     // big clock
    ismCaptureFunction();                   
    CU_ASSERT(dataPosition == 2u);       // position should have upadated
    
    timerCounterValueReturn = bigClock;     // big clock
    ismCaptureFunction();                   
    CU_ASSERT(dataPosition == 3u);       // position should have upadated
    
    timerCounterValueReturn = bigClock;     // big clock
    ismCaptureFunction();                   
    CU_ASSERT(dataPosition == 0u);       // position should be reset
    
    timerCounterValueReturn = smallClock;   // small clock
    ismCaptureFunction();                   
    CU_ASSERT(dataPosition == 0u);       // position stays at zero
    
    
    timerCounterValueReturn = receiveTimeout * 2u; // timeout -> we should be able to receive something again
    ismCaptureFunction();                   
    CU_ASSERT(dataPosition == 0u);       // position stays at zero
    
    timerCounterValueReturn = bigClock;     // small clock
    ismCaptureFunction();                   
    CU_ASSERT(dataPosition == 1u);       // position should have upadated
    
    timerCounterValueReturn = smallClock;   // big clock
    ismCaptureFunction();                   
    CU_ASSERT(dataPosition == 2u);       // position should have upadated

    timerCounterValueReturn = smallClock * 2u; // some random clock
    ismCaptureFunction();                   
    CU_ASSERT(dataPosition == 0u);       // positions should be reset to zero
    
    
    timerCounterValueReturn = receiveTimeout * 2u; // timeout -> we should be able to receive something again
    ismCaptureFunction();                   
    CU_ASSERT(dataPosition == 0u);      // position stays at zero
    
    for (i = 0u; i < minTransitions; i++)  // trigger a max transitions situation
    {
        if ((i % 2u) == 1u)
        {
            timerCounterValueReturn = bigClock;   // small clock
        }
        else
        {
            timerCounterValueReturn = smallClock; // big clock
        }
        
        ismCaptureFunction();
        CU_ASSERT(dataPosition == (i+1u));   // position moved on
    }
    
    timerCounterValueReturn = receiveTimeout * 2u; // timeout -> we should be able to receive something again
    ismCaptureFunction();                   
    CU_ASSERT(dataPosition == 0u);       // position stays at zero
    CU_ASSERT(frameReceived == false);      // we should not have received a frame
    
    for (i = 0u; i < minTransitions + 2u; i++)    // trigger a max transitions situation
    {
        if ((i % 2u) == 1u)
        {
            timerCounterValueReturn = bigClock;   // small clock
        }
        else
        {
            timerCounterValueReturn = smallClock; // big clock
        }
        
        ismCaptureFunction();
        CU_ASSERT(dataPosition == (i+1u));    // position moved on
    }
    
    timerCounterValueReturn = receiveTimeout * 2u; // timeout -> we should be able to receive something again
    ismCaptureFunction();                   
    CU_ASSERT(frameReceived == true);            // we should have received a frame

    
    RemoteControl_startCapture(RemoteCommand_Medium_433Mhz);
   
    timerCounterValueReturn = receiveTimeout * 2u; // timeout -> we should be able to receive something again
    ismCaptureFunction();                   
    CU_ASSERT(dataPosition == 0u);            // position stays at zero
    
    for (i = 0u; i < REMOTE_COMMAND_MAX_TRANSITIONS; i++)  // trigger a max transitions situation
    {
        if ((i % 2u) == 1u)
        {
            timerCounterValueReturn = smallClock;   // small clock
        }
        else
        {
            timerCounterValueReturn = bigClock;     // big clock
        }
        
        ismCaptureFunction();
        CU_ASSERT(dataPosition == (i+1u));       // position moved on
    }
    
    timerCounterValueReturn = bigClock;   // small clock
    ismCaptureFunction();                   
    CU_ASSERT(dataPosition == 0u);     // position should have upadated
}

/* The main() function for setting up and running the tests.
 * Returns a CUE_SUCCESS on successful running, another
 * CUnit error code on failure.
 */
int main()
{
   CU_pSuite pSuite = NULL;

   /* initialize the CUnit test registry */
   if (CUE_SUCCESS != CU_initialize_registry())
      return CU_get_error();

   /* add a suite to the registry */
   pSuite = CU_add_suite("Suite_1", init_suite1, clean_suite1);
   if (NULL == pSuite) {
      CU_cleanup_registry();
      return CU_get_error();
   }

   /* add the tests to the suite */
   if ((CU_add_test(pSuite, "test of frequencyCaptureCalculate()", testFrequencyCaptureCalculate) == NULL)
       || (CU_add_test(pSuite, "test IR capturing", testIrCapture) == NULL)
       || (CU_add_test(pSuite, "test ISM capturing", testIsmCapture) == NULL))
   {
      CU_cleanup_registry();
      return CU_get_error();
   }

   /* Run all tests using the CUnit Basic interface */
   CU_basic_set_mode(CU_BRM_VERBOSE);
   CU_basic_run_tests();
   CU_cleanup_registry();
   return CU_get_error();
}
