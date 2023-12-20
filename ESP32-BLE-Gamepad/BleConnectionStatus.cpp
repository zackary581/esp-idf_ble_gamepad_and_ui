#include "BleConnectionStatus.h"
#include "NimBLEDevice.h"
#include "esp_sleep.h"

/* Time to sleep between advertisements */
static uint32_t sleepSeconds = 20;

BleConnectionStatus::BleConnectionStatus(void)
{
}
/*
void BleConnectionStatus::onConnect(NimBLEServer *pServer, ble_gap_conn_desc *desc)
{
    pServer->updateConnParams(desc->conn_handle, 6, 7, 0, 600);
    this->connected = true;
}

void BleConnectionStatus::onDisconnect(NimBLEServer *pServer)
{
    this->connected = false;
}
*/

///*
void BleConnectionStatus::onConnect(NimBLEServer *pServer, NimBLEConnInfo &connInfo)
{
    printf("Client connected:: %s\n", connInfo.getAddress().toString().c_str());
    this->connected = true;
}

void BleConnectionStatus::onDisconnect(NimBLEServer *pServer, NimBLEConnInfo &connInfo, int reason)
{
    printf("Client disconnected - sleeping for %" PRIu32 " seconds\n", sleepSeconds);
    this->connected = false;
    esp_deep_sleep_start();
}
//*/