# EJ-Mon
OBD-II and CAN bus monitoring device designed specifically for 2008-14 Subaru WRX (EJ motor)

## Basic Features / Characteristics
- Single din form factor
- It will have a 4.58" 320x960 pixel display for graphs and animations based on the cars operatting parameters.
- the displays ui should be minimal and kind of similar to the Teenage Engeineering OP-1
- Main engine parameters monitored:
  - Boost
  - Intercooler and intake temps
  - Coolant temps
  - Knock and timing adjustments
  - the main interface should feature a basic top down illusttrtation of an ej motor 
    - turbo to one side or diagonally above left side
    - intercooler either abover topdown view or on other side of the turbo
      - intercooler drawing could change based on intake temps
    - theree should be some symbolic collant lines and maybe coolant resevoir where the line color and resevoir color indicate color
    - pistons should animate to real time firing order (CAN-BUS data)
    - turbo diagram should spin relative to how fast the real turbo is (CAN-BUS data)
- UI Considerations
  - The screen should flash when at a certain rpm like 5500
  - could have multiple sections on the screen for differeent illustrations
  - could be cool to have an outline of the side profile of the car somewhere
  - tthe vibe should be kind of sci-phi and psychedilic but still remain somewhat minimal
- Project should also include a kill switch that disables injectors
  - kill switch should be hidden not on the device though, run wires to somewhere in the cabin

## Tech Stack
- **Adafruit Qualia ESP32 S3**: Main brain of the operation
  - Chosen because this version has drivers for the rgb-666 tft display on the board already with a ribbon connection ready to go
  - 240mhz clock rate
  - 512kb RAM
- **960x320 4.58" TTL RGB-666 TFT Display**
  - Physical size of 110.3mm x 36.77mm
- **MCP2515 TJA1050 CAN Bus Module**: Decodes CAN Bus data
  - interfaces with esp32 through spi
- **Buck Converter (12V to 3.3V)**: Power regulation
  - Steps down 12V vehicle power to 3.3V for the ESP32 and display
  - Provides stable power delivery from the car's electrical system
