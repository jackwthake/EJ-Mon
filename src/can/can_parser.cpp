#include "can_parser.h"
#include <cstring>
#include <cstdio>

#if PLATFORM_ESP32
CANFrame from_mcp2515_frame(uint32_t can_id, uint8_t can_dlc, const uint8_t* can_data) {
  CANFrame frame;
  frame.timestamp_ms = 0;  // ESP32 can use millis() or FreeRTOS tick
  frame.id = can_id & 0x7FF;  // Mask to 11-bit (or use full 29-bit if needed)
  frame.dlc = can_dlc;
  memcpy(frame.data, can_data, 8);
  return frame;
}
#endif

void CANParser::decode_frame(const CANFrame& frame, EngineData& data) {
  switch (frame.id) {
    case 0x140:  // RPM & Speed
      // RPM: Big-endian 16-bit value at bytes 1-2 (already in RPM)
      data.rpm = ((uint16_t)frame.data[1] << 8) | frame.data[2];
      data.speed = frame.data[4];
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
      data.overboost = (data.boost_psi > 18.0f);
      break;

    case 0x144:  // Knock & Timing
      data.knock_count = frame.data[0] + frame.data[1];
      data.knock_detected = (data.knock_count > 0);
      data.timing_adv = (int8_t)frame.data[2] - 128;
      break;

    case 0x360:  // Load & MAF
      data.engine_load = frame.data[0];
      data.maf_gs = frame.data[1];
      break;
  }
}

#if PLATFORM_DESKTOP
bool CANParser::parse_line(const char* line, CANFrame& frame) {
  if (line[0] == '#' || line[0] == '\0' || line[0] == '\n') {
    return false;  // Skip comments and empty lines
  }

  unsigned int ts, id, dlc;
  unsigned int d[8];

  int parsed = sscanf(line, "%u,%x,%u,%x,%x,%x,%x,%x,%x,%x,%x",
                     &ts, &id, &dlc,
                     &d[0], &d[1], &d[2], &d[3],
                     &d[4], &d[5], &d[6], &d[7]);

  if (parsed < 3) return false;

  frame.timestamp_ms = ts;
  frame.id = id;
  frame.dlc = dlc;

  for (int i = 0; i < 8; i++) {
    frame.data[i] = (i < dlc && i < parsed - 3) ? d[i] : 0;
  }

  return true;
}
#endif
