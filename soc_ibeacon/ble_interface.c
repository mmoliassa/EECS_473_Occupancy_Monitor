/*
 * ble_interface.h
 *
 *  Created on: Oct 10, 2021
 *      Author: mmoliassa
 */

#include "ble_interface.h"
#include "app_assert.h"

#define UINT16_TO_BYTES_U(n)            (uint8_t)((n) >> 8)
#define UINT16_TO_BYTES_L(n)            ((uint8_t) (n))

#define COMPANY_ID 0x4C00
#define IBEACON_ID 0x0215
#define MANUFACTURER_DATA 0xFF
#define LE_DISCOVERABLE 0x06
#define ADVERTISING_DURATION 500

static uint8_t advertising_set_handle = 0xff;

uint8_t UUID[] = {0x78, 0xE3, 0xB3,
                  0x74, 0xA0, 0x79,
                  0x40, 0x11, 0xA9,
                  0x99, 0x62, 0xDA,
                  0x95, 0x80, 0x9B, 0xBA};

//78e3b374-a079-4011-a999-62da95809bba

//to do: change to set isoccupied only
 void init_advertisement_packet(uint8_t isOccupied){

   // Standard iBeacon flag values
   advertisement_packet.flags_length = 2;                          // Length of flag section
   advertisement_packet.flags_type = 0x01;                            // Default flag type
   advertisement_packet.flags = LE_DISCOVERABLE;                      // Low Energy General Discoverable Mode, BR/EDR not supported
   advertisement_packet.manufacturer_data_length = 26;                // Length of the manufacturer-specified data (UUID, beacon type, company)
   advertisement_packet.manufacturer_data_type = MANUFACTURER_DATA;   // Data type is manufacturer-specified data

   advertisement_packet.company_id[0] = UINT16_TO_BYTES_U(COMPANY_ID);  // Company ID
   advertisement_packet.company_id[1] = UINT16_TO_BYTES_L(COMPANY_ID);

   advertisement_packet.beacon_type[0] = UINT16_TO_BYTES_U(IBEACON_ID); // IBeacon type ID
   advertisement_packet.beacon_type[1] = UINT16_TO_BYTES_L(IBEACON_ID);


   // Set UUID bytes
   for(int i = 0; i < UUID_BYTE_SIZE; ++i){
       advertisement_packet.uuid[i] = UUID[i];
   }

   // Get chip unique bluetooth id (similar to mac address)
 /*
   uint8_t device_id[DEVICE_ID_SIZE];
   get_device_BLE_address(device_id);

   for(int i = 0; i < DEVICE_ID_SIZE; ++i){
       advertisement_packet.mac_addr[i] = device_id[i];
   }
*/
   uint8_t device_id[DEVICE_ID_SIZE];
   get_device_BLE_address(device_id);

   for(int i = 0; i < 2; ++i){
       advertisement_packet.major_num[i] = device_id[i];
   }
   for(int i = 2; i<4; ++i){
       advertisement_packet.minor_num[i-2] = device_id[i];
   }

   // Set whether occupied or not
   advertisement_packet.minor_num[1] &= 0b11111110;
   advertisement_packet.minor_num[1] |= isOccupied;

   advertisement_packet.tx_power = 0xD7;

 }

 void get_device_BLE_address(uint8_t* device_id){
   sl_status_t status;
   bd_addr bluetoothAddress;
   uint8_t address_type;

   status = sl_bt_system_get_identity_address(&bluetoothAddress, &address_type);
   app_assert_status(status);

   // Error checking
   if(address_type == 1){

   }

   device_id[0] = bluetoothAddress.addr[0];
   device_id[1] = bluetoothAddress.addr[1];
   device_id[2] = bluetoothAddress.addr[2];
   device_id[3] = bluetoothAddress.addr[3];
   device_id[4] = bluetoothAddress.addr[4];
   device_id[5] = bluetoothAddress.addr[5];

 }

 void set_advertisement_packet(uint8_t isOccupied){
      advertisement_packet.minor_num[1] &= 0b11111110;
      advertisement_packet.minor_num[1] |= isOccupied;
 }

 uint8_t* get_device_UUID(){
   return UUID;
 }


 void stop_BLE_advertising(){
   sl_bt_advertiser_stop(advertising_set_handle);
 }

 // Duration in 10ms multiples
 void start_BLE_advertising(int16_t duration){

   sl_status_t sc;

   // Set advertising parameters. 100ms advertisement interval.
   sc = sl_bt_advertiser_set_timing(
     advertising_set_handle,
     160,     // min. adv. interval (milliseconds * 1.6)
     160,     // max. adv. interval (milliseconds * 1.6)
     0,  // adv. duration
     duration);      // max. num. adv. events
   app_assert_status(sc);


   sc = sl_bt_advertiser_set_data(advertising_set_handle,
                                         0,
                                         sizeof(advertisement_packet),
                                         (uint8_t *)(&advertisement_packet));

   app_assert_status(sc);


   // Start advertising in user mode and disable connections.
   sc = sl_bt_advertiser_start(
     advertising_set_handle,
     sl_bt_advertiser_user_data,
     sl_bt_advertiser_non_connectable);
   app_assert_status(sc);
 }

 void init_BLE_advertising(){
   sl_status_t sc;

   init_advertisement_packet(0);

   sc = sl_bt_advertiser_create_set(&advertising_set_handle);
   app_assert_status(sc);

   sc = sl_bt_advertiser_set_data(advertising_set_handle,
                                      0,
                                      sizeof(advertisement_packet),
                                      (uint8_t *)(&advertisement_packet));

   app_assert_status(sc);

 }

 void send_data(char * data){

 }


