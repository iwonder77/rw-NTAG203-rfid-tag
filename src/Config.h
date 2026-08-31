#pragma once
/**
 * Config.h
 *
 * Centralized configuration constants used by the Wall Battery (Arduino
 * Leonardo) system firmware
 *
 * - make sure to `#include "Config.h"` then use config::XXXXX throughout the
 * projects for no magic numbers
 * - units are encoded in the name (e.g. _MS, _US)
 */

#include <Arduino.h>

namespace config {

static constexpr uint8_t WS1850S_I2C_ADDR = 0x28;
static constexpr uint32_t I2C_CLOCK_SPEED = 100000; // 100kHz standard mode

} // namespace config
