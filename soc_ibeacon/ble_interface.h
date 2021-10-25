/*
 * ble_interface.h
 *
 *  Created on: Oct 10, 2021
 *      Author: mmoliassa
 */
#include "stdint.h"
#include "sl_bluetooth.h"

#ifndef BLE_INTERFACE_H_
#define BLE_INTERFACE_H_

#define UUID_BYTE_SIZE 16
#define DEVICE_ID_SIZE 6

 typedef struct iBeaconAdvertisement iBeaconAdvertisement;

 static iBeaconAdvertisement advertisement_packet;

 struct iBeaconAdvertisement {
   uint8_t flags_length;             // Length of the Flags field.
   uint8_t flags_type;               // Type of the Flags field.
   uint8_t flags;                    // Flags field.
   uint8_t manufacturer_data_length; // Length of the Manufacturer Data field.
   uint8_t manufacturer_data_type;   // Type of the Manufacturer Data field.
   uint8_t company_id[2];            // Company ID field.
   uint8_t beacon_type[2];           // Beacon Type field.
   uint8_t uuid[UUID_BYTE_SIZE];     // 80-bit Universally Unique Identifier (UUID). The UUID is an identifier for the company using the beacon.
   //uint8_t mac_addr[DEVICE_ID_SIZE]; // 6-byte device specific Bluetooth system ID.
   uint8_t major_num[2];             // Beacon major number. Used to group related beacons.
   uint8_t minor_num[2];             // Beacon minor number. Used to specify individual beacons within a group.
   uint8_t tx_power;                 // The Beacon's measured RSSI at 1 meter distance in dBm. See the iBeacon specification for measurement guidelines.
 };

 // Function to create and return BLE advertisement packet with relevant occupancy data
 void set_advertisement_packet(uint8_t isOccupied);

 // Get unique Bluetooth device ID
 void get_device_BLE_address(uint8_t* device_id);

 void set_advertisement_packet(uint8_t isOccupied);

 void stop_BLE_advertising();

 void start_BLE_advertising(int16_t duration);

 void init_BLE_advertising();

 void send_data(char * data);

 // Get the UUID set for all occupancy monitoring devices
 uint8_t* get_device_UUID();

#endif /* BLE_INTERFACE_H_ */
