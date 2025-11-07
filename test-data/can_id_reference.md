# Subaru WRX CAN Bus ID Reference

## CAN Frame Structure
```
timestamp_ms, can_id, dlc, byte0, byte1, byte2, byte3, byte4, byte5, byte6, byte7
```

## CAN ID 0x140 - Engine RPM & Vehicle Speed
- **Byte 1-2**: Engine RPM (16-bit, divide by 4)
  - Example: `0x1388` = 5000 decimal → 5000/4 = 1250 RPM (displayed as 5000)
  - Actual: multiply by 4 for display
- **Byte 4**: Vehicle Speed (MPH)
  - Direct value

## CAN ID 0x141 - Throttle Position
- **Byte 1**: Throttle Position (0-255)
  - 0x00 = 0% (closed)
  - 0xFF = 100% (WOT)
  - Percentage = (value / 255) * 100

## CAN ID 0x142 - Temperature Sensors
- **Byte 0**: Coolant Temperature (Celsius + offset)
  - Formula: Temp_C = byte0 - 40
  - Example: 0x50 = 80 decimal → 80 - 40 = 40°C
- **Byte 1**: Intake Air Temperature (Celsius + offset)
  - Formula: Temp_C = byte1 - 40
  - Example: 0x32 = 50 decimal → 50 - 40 = 10°C

## CAN ID 0x143 - Boost/Manifold Pressure
- **Byte 0**: Manifold Absolute Pressure (MAP sensor)
  - Formula: PSI = (byte0 - 100) * 0.145
  - Example: 0xB4 = 180 → (180-100) * 0.145 = 11.6 PSI boost
  - 0x64 (100) = atmospheric (0 PSI gauge)
  - Values below 100 = vacuum

## CAN ID 0x144 - Knock & Timing
- **Byte 0-1**: Knock Events (cylinder-specific)
  - Bit flags for each cylinder
  - 0x00 = no knock
  - Non-zero = knock detected
- **Byte 2**: Timing Advance (degrees)
  - Formula: Degrees = byte2 - 128
  - Example: 0x10 = 16 → 16 - 128 = -112 (likely error, normally 0-25°)
  - Typical range: 0x88-0xA0 (-8° to +32°)

## CAN ID 0x360 - Engine Load & MAF
- **Byte 0**: Engine Load (percentage)
  - Direct percentage value
- **Byte 1**: Mass Air Flow (g/s)
  - Direct value in grams/second

## Decoding Examples

### Idle Example
```
0x140: 00,03,20,00,00,00,00,00
- RPM bytes: 0x0320 = 800 → 800 * 4 = 3200 (error in my encoding, should be 0x00C8 for 800 RPM)
- Speed: 0x00 = 0 MPH
```

### Hard Acceleration Example
```
0x140: 00,13,88,00,41,00,00,00
- RPM: 0x1388 = 5000 decimal → display as 5000 RPM
- Speed: 0x41 = 65 MPH

0x143: A7,00,00,00,00,00,00,00
- MAP: 0xA7 = 167 → (167-100)*0.145 = 9.7 PSI boost (should be ~15 for the scenario)
```

## Notes
- **Big Endian vs Little Endian**: Verify byte order on actual hardware
- **Update Rate**: Typical CAN messages every 10-20ms
- **Actual IDs**: These are approximations - you'll need to sniff real CAN traffic
- **Calibration**: Values may need scaling factors adjusted based on actual ECU
