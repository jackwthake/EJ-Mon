#include "can_parser.h"
#include <cstring>
#include <cstdio>

CANFrame from_mcp2515_frame(uint32_t can_id, uint8_t can_dlc, const uint8_t* can_data) {
  CANFrame frame;
  frame.timestamp_ms = 0;  // ESP32 can use millis() or FreeRTOS tick
  frame.id = can_id & 0x7FF;  // Mask to 11-bit (or use full 29-bit if needed)
  frame.dlc = can_dlc;
  memcpy(frame.data, can_data, 8);
  return frame;
}

void CANParser::decode_frame(const CANFrame& frame, EngineData& data) {
  switch (frame.id) {
    case 0x140:  // RPM
      // RPM: Big-endian 16-bit value at bytes 1-2 (already in RPM)
      data.rpm = ((uint16_t)frame.data[1] << 8) | frame.data[2];
      break;

    case 0x141:  // Throttle
      data.throttle = (frame.data[1] * 100) / 255;
      break;

    case 0x142:  // Temperatures
      data.coolant_temp = frame.data[0] - 40;
      data.intake_temp = frame.data[1] - 40;
      break;

    case 0x143:  // Boost
      data.boost_psi = ((int16_t)frame.data[0] - 100) * 0.145f;
      break;

    case 0x144:  // Knock & Timing
      data.knock_count = frame.data[0] + frame.data[1];
      data.timing_adv = (int8_t)frame.data[2] - 128;
      break;
    default: break;
  }
}
