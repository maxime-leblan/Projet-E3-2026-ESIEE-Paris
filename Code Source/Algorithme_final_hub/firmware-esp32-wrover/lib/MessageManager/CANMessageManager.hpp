#pragma once

#include <Arduino.h>
#include "driver/twai.h"

// Pins used to connect to CAN bus transceiver:
#define HUB_RX_PIN 34
#define HUB_TX_PIN 25
#define ANCHOR_RX_PIN 15
#define ANCHOR_TX_PIN 16

// Interval:
#define TRANSMIT_RATE_MS 1000

#define POLLING_RATE_MS 1000

static bool driver_installed = false;

/**
 * Initialise le composant pour qu'il puisse communiquer 
 */
void initCANTransmitter();