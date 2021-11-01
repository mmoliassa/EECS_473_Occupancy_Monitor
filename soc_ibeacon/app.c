/***************************************************************************//**
 * @file
 * @brief Core application logic.
 *******************************************************************************
 * # License
 * <b>Copyright 2020 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************
 *
 * SPDX-License-Identifier: Zlib
 *
 * The licensor of this software is Silicon Laboratories Inc.
 *
 * This software is provided 'as-is', without any express or implied
 * warranty. In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software
 *    in a product, an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 *
 ******************************************************************************/

#include "sl_bluetooth.h"
#include "app_assert.h"
#include "gatt_db.h"
#include "app.h"
#include "ble_interface.h"
#include "em_emu.h"
#include "em_gpio.h"
#include "em_cmu.h"

#include "em_device.h"
#include "em_chip.h"
#include "em_rtc.h"
#include "em_burtc.h"
#include "em_rmu.h"
#include "em_letimer.h"

// Macros.
#define UINT16_TO_BYTES(n)            ((uint8_t) (n)), ((uint8_t)((n) >> 8))
#define UINT16_TO_BYTE0(n)            ((uint8_t) (n))
#define UINT16_TO_BYTE1(n)            ((uint8_t) ((n) >> 8))

#define LED_GPIO_PIN 0
#define LED_GPIO_PORT gpioPortB

#define BUTTON_PIN 1
#define BUTTON_PORT gpioPortB

//#define SENSOR_INPUT_PORT gpioPortA
//#define SENSOR_INPUT_PIN 7
#define SENSOR_INPUT_PORT gpioPortB
#define SENSOR_INPUT_PIN 1


#define DEBUG_OUTPUT_PORT gpioPortA
#define DEBUG_OUTPUT_PIN 8

#define BURTC_IRQ_PERIOD  1000

#define TIME_DISABLE_GPIO 5 // (T1)
#define TIMEOUT_OCCUPIED 15  // (T2)
#define TIMEOUT_CLEAR_POSCOUNT 30  // (T3)

#define SENSOR_INPUT_MASK 1 << SENSOR_INPUT_PIN

#define NUM_ADVERTISING_PACKETS 2

uint8_t isOccupied = 0;
uint8_t state = 0;
uint8_t poscount = 0;
uint8_t test = 0;
uint8_t GPIO_flag = 0;
uint8_t BURTC_flag = 0;
uint8_t em3_flag = 0;
uint8_t counter = 0;


/* NOTE: Not always consistent on sending the 1 after an unoccupied send. Need to figure out what is happening there
 * might be because the event queue doesn't get it in time before going back into em3
 *
 *
 *
 *
 *  */


void GPIO_SENSOR_IRQ(void);

// The advertising set handle allocated from Bluetooth stack.
//static uint8_t advertising_set_handle = 0xff;

/**************************************************************************//**
 * Set up a custom advertisement package according to iBeacon specifications.
 * The advertisement package is 30 bytes long.
 * See the iBeacon specification for further details.
 *****************************************************************************/
//static void setup_beacon_advertising(void);

/**************************************************************************//**
 * Application Init.
 *****************************************************************************/

// TO DO: ADD DECREMENT OF POSCOUNT OVER TIME

// TO DO: ADD Counter


void enterEM3(void) {

  //GPIO_PinOutSet(LED_GPIO_PORT, LED_GPIO_PIN);
  EMU_EnterEM3(false);
  //GPIO_PinOutClear(LED_GPIO_PORT, LED_GPIO_PIN);

}

void GPIO_SENSOR_IRQ(void) {
  counter = 0;
  // Clear the pin number bit
  //GPIO_PinOutSet(DEBUG_OUTPUT_PORT, DEBUG_OUTPUT_PIN);
  //GPIO_PinOutToggle(LED_GPIO_PORT, LED_GPIO_PIN);
  uint32_t interruptMask = GPIO_IntGet();
  counter++;
  poscount += 1;
  counter++;
  //GPIO_PinOutClear(DEBUG_OUTPUT_PORT, DEBUG_OUTPUT_PIN);

  if(poscount >= 5){
    GPIO_PinOutSet(DEBUG_OUTPUT_PORT, DEBUG_OUTPUT_PIN);
    state = 1;
    isOccupied = 1;

    //enable and start
    BURTC_CounterReset();
    BURTC_CompareSet(0, TIME_DISABLE_GPIO * BURTC_IRQ_PERIOD);
    em3_flag = 1;
    set_advertisement_packet(isOccupied);
    start_BLE_advertising(5);
  }
  counter++;
  //disable GPIO interrupts
  GPIO_IntClear(interruptMask);
  counter++;
  if(poscount >= 5){
      GPIO_IntDisable(SENSOR_INPUT_MASK);
  }
  counter++;

}

void GPIO_ODD_IRQHandler(void)
{
  GPIO_SENSOR_IRQ();
  counter = 10;
//  if(GPIO_flag == 0){
      //EMU_EnterEM2(true);
//  }
//  else{
//      GPIO_flag = 0;
//  }
}

void GPIO_init(void)
{
  CMU_ClockEnable(cmuClock_GPIO, true);

  // Configure Port A8 as GPIO Output
  GPIO_PinModeSet(DEBUG_OUTPUT_PORT, DEBUG_OUTPUT_PIN, gpioModePushPull, 0);

  GPIO_PinModeSet(LED_GPIO_PORT, LED_GPIO_PIN, gpioModePushPull, 0);

  // Configure Port A7 as input and enable interrupt
  GPIO_PinModeSet(SENSOR_INPUT_PORT, SENSOR_INPUT_PIN, gpioModeInput, 0);
  GPIO_ExtIntConfig(SENSOR_INPUT_PORT, SENSOR_INPUT_PIN, SENSOR_INPUT_PIN, true, false, true);

  // Enable ODD interrupt to catch button press that changes slew rate
  NVIC_ClearPendingIRQ(GPIO_ODD_IRQn);
  NVIC_EnableIRQ(GPIO_ODD_IRQn);


}
/**************************************************************************//**
 * @brief BURTC Interrupt Handler clears the flag
 *****************************************************************************/
void BURTC_IRQHandler(void)
{
  //GPIO_PinOutToggle(LED_GPIO_PORT, LED_GPIO_PIN);
  /* Clear interrupt source */
  GPIO_PinOutSet(DEBUG_OUTPUT_PORT, DEBUG_OUTPUT_PIN);
  BURTC_IntClear(BURTC_IF_COMP);

  GPIO_IntEnable(SENSOR_INPUT_MASK);
  BURTC_CounterReset();

  if(state == 1) {
    state = 2;
    BURTC_CompareSet(0, TIMEOUT_OCCUPIED * BURTC_IRQ_PERIOD);
  }
  else if (state == 2) {
    state = 0;
    poscount = 0;
    BURTC_CompareSet(0, TIMEOUT_CLEAR_POSCOUNT * BURTC_IRQ_PERIOD);
    //send bluetooth
    isOccupied = 0;
    em3_flag = 1;
    set_advertisement_packet(isOccupied);
    start_BLE_advertising(5);
  }
  else if (state == 0) {
    poscount = 0;
  }

  GPIO_PinOutClear(DEBUG_OUTPUT_PORT, DEBUG_OUTPUT_PIN);
  //if(BURTC_flag == 0) {
  //    enterEM3();
      //EMU_EnterEM3(true);
  //}
  //else {
  //    BURTC_flag = 0;
  //}
}

/**************************************************************************//**
 * @brief  Setup BURTC
 * Using LFRCO clock source and enabling interrupt on COMP0 match
 *****************************************************************************/
void setupBurtc(void)
{
  CMU_ClockSelectSet(cmuClock_EM4GRPACLK, cmuSelect_ULFRCO);
  CMU_ClockEnable(cmuClock_BURTC, true);
  CMU_ClockEnable(cmuClock_BURAM, true);

  BURTC_Init_TypeDef burtcInitvar = BURTC_INIT_DEFAULT;
  burtcInitvar.clkDiv       = burtcClkDiv_1; /* Clock prescaler, set to 128 so one tick is 3.9 ms */
  burtcInitvar.compare0Top  = true; /* Clear counter on compare match */
  burtcInitvar.em4comp      = true;
  burtcInitvar.em4overflow  = true;

  /* Initialize BURTC */
  BURTC_Init(&burtcInitvar);

  BURTC_CounterReset();
  BURTC_CompareSet(0, BURTC_IRQ_PERIOD * TIMEOUT_CLEAR_POSCOUNT);

  BURTC_IntClear(BURTC_IF_COMP);
  BURTC_IntEnable(BURTC_IF_COMP);    // compare match
  NVIC_ClearPendingIRQ(BURTC_IRQn);
  NVIC_EnableIRQ(BURTC_IRQn);
  BURTC_Enable(true);

}


SL_WEAK void app_init(void)
{
  setupBurtc();
  GPIO_init();
  //EMU_EnterEM2(false);
  enterEM3();
  GPIO_PinOutClear(LED_GPIO_PORT, LED_GPIO_PIN);
}

/**************************************************************************//**
 * Application Process Action.
 *****************************************************************************/
SL_WEAK void app_process_action(void)
{

  /////////////////////////////////////////////////////////////////////////////
  // Put your additional application code here!                              //
  // This is called infinitely.                                              //
  // Do not call blocking functions from here!                               //
  /////////////////////////////////////////////////////////////////////////////
}

/**************************************************************************//**
 * Bluetooth stack event handler.
 * This overrides the dummy weak implementation.
 *
 * @param[in] evt Event coming from the Bluetooth stack.
 *****************************************************************************/
void sl_bt_on_event(sl_bt_msg_t *evt)
{
  sl_status_t sc;
  int16_t ret_power_min, ret_power_max;
  switch (SL_BT_MSG_ID(evt->header)) {
    // -------------------------------
    // This event indicates the device has started and the radio is ready.
    // Do not call any stack command before receiving this boot event!
    case sl_bt_evt_system_boot_id:
      // Set 0 dBm maximum Transmit Power.
      sc = sl_bt_system_set_tx_power(SL_BT_CONFIG_MIN_TX_POWER, 0,
                                     &ret_power_min, &ret_power_max);
      app_assert_status(sc);
      (void)ret_power_min;
      (void)ret_power_max;
      // Initialize iBeacon ADV data.
      init_BLE_advertising();

      break;

    case sl_bt_evt_system_external_signal_id:
      break;

    case sl_bt_evt_advertiser_timeout_id:
      //GPIO_PinOutClear(DEBUG_OUTPUT_PORT, DEBUG_OUTPUT_PIN);
      em3_flag = 0;
      //enterEM3();
      //EMU_EnterEM2(false);
      break;

    ///////////////////////////////////////////////////////////////////////////
    // Add additional event handlers here as your application requires!      //
    ///////////////////////////////////////////////////////////////////////////

    // -------------------------------
    // Default event handler.
    default:
      break;
  }
}
