# Building Car-Mon

## Desktop SDL Simulator

The desktop simulator allows you to develop and test graphics without hardware.

### Quick Start

```bash
./build.sh          # Build in Debug mode
./build.sh Release  # Build in Release mode
./build/car-mon     # Run the simulator
```

### Manual Build

```bash
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Debug -DTARGET_PLATFORM=DESKTOP
cmake --build . -j$(nproc)
```

### Controls

- **ESC** or close window to quit
- Window is resizable but renders at 960x320 (matching hardware)

## ESP32 Build (Coming Soon)

The ESP32 build will be configured with:

```bash
cmake .. -DCMAKE_BUILD_TYPE=Release -DTARGET_PLATFORM=ESP32
```

ESP-IDF toolchain integration will be added once hardware arrives.

## Project Structure

```
car-mon/
├── src/
│   ├── main.cpp              # Entry point (SDL on desktop, app_main on ESP32)
│   └── graphics/             # Graphics engine (platform-agnostic)
├── third-party/
│   └── SDL/                  # SDL3 (desktop only)
├── CMakeLists.txt            # Main build config
└── build.sh                  # Convenience build script
```

## Dependencies

- CMake 3.20+
- C++17 compiler (GCC/Clang)
- SDL3 dependencies (automatically handled via git submodule)
