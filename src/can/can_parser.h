#pragma once

#include <cstdint>
#include "../platform.hpp"

// CAN frame structure (generic format for both platforms)
struct CANFrame {
  uint32_t timestamp_ms;  // For desktop playback timing
  uint16_t id;            // 11-bit CAN ID (or 29-bit extended)
  uint8_t dlc;            // Data length (0-8)
  uint8_t data[8];        // Payload data
};

#if PLATFORM_ESP32
// MCP2515 library compatibility
// Conversion helper for ESP32 MCP2515 frames
CANFrame from_mcp2515_frame(uint32_t can_id, uint8_t can_dlc, const uint8_t* can_data);
#endif

// Decoded engine parameters
struct EngineData {
  uint16_t rpm;          // Engine RPM
  uint8_t speed;         // Vehicle speed (MPH)
  uint8_t throttle;      // Throttle position (0-100%)
  int8_t coolant_temp;   // Coolant temp (Celsius)
  int8_t intake_temp;    // Intake air temp (Celsius)
  float boost_psi;       // Boost pressure (PSI, negative = vacuum)
  uint8_t knock_count;   // Knock event count
  int8_t timing_adv;     // Timing advance (degrees)
  uint8_t engine_load;   // Engine load (%)
  uint8_t maf_gs;        // Mass air flow (g/s)
  float battery_voltage; // Battery voltage (V)

  // Flags
  bool knock_detected;
  bool overboost;
};

class CANParser {
public:
  // Main decode function - works on both ESP32 and Desktop
  static void decode_frame(const CANFrame& frame, EngineData& data);

#if PLATFORM_DESKTOP
  // Desktop-only: Parse a line from the test data file
  static bool parse_line(const char* line, CANFrame& frame);
#endif
};
