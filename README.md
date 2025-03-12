# Nova Mini Project Suite

## Overview
The Nova Mini project suite consists of two main components that work together to create an interactive memory game experience:

1. Nova Mini (Receiver)
2. Nova Mini Remote (Sender)

These devices communicate wirelessly using ESP-NOW protocol to create an engaging, interactive game experience based on the classic Simon memory game.

## Components

### Nova Mini (Receiver)
The main game controller that:
- Handles core game logic and state management
- Controls LED patterns and audio feedback
- Processes player inputs
- Manages game progression through levels and rounds
- Provides hardware-level control and feedback

### Nova Mini Remote (Sender)
The user interface controller that:
- Initiates game commands and controls
- Sends instructions to the Nova Mini receiver
- Provides real-time game state updates
- Offers enhanced game mode features
- Enables remote gameplay control

## Technical Stack

### Hardware
Both devices are built on the ESP32 platform, featuring:
- ESP32 microcontroller
- LED indicators for visual feedback
- Push buttons for user input
- Buzzer for audio feedback
- Custom PCB design

### Software
Built using modern embedded development tools and frameworks:
- PlatformIO development environment
- Arduino framework for ESP32
- ESP-NOW protocol for wireless communication
- Custom libraries:
  - ESPUI for web interface
  - AsyncWebServer for configuration
  - MIDI Library for audio control
  - ArduinoJson for data serialization
  - OneButton for enhanced button control

## Communication

The devices use ESP-NOW, a connectionless wireless communication protocol that enables:
- Low-latency communication
- Peer-to-peer data exchange
- Efficient power usage
- Reliable message delivery

## Game Features

- Multiple difficulty levels and progressive rounds
- Real-time LED and audio feedback
- Enhanced state management with stages:
  - Sequence Generation
  - Input Collection
  - Verification
  - Round Transition
  - Game Win/Loss states
- Remote control capabilities
- Cheat mode for testing

## Setup and Configuration

The system includes a web-based configuration interface that allows:
- ESP-NOW pairing between devices
- MAC address management
- Game mode settings
- System preferences

## Development

This project is built using PlatformIO, offering:
- Cross-platform development
- Integrated dependency management
- Automated builds
- Extensive debugging capabilities
- Serial monitoring at 921600 baud

## Project Structure
```
nova-mini/
├── nova-mini (receiver)/        # Main game controller
│   ├── src/                    # Source files
│   ├── hardware/              # PCB and schematic files
│   └── platformio.ini         # Project configuration
└── nova-mini-remote (sender)/  # Remote controller
    ├── src/                    # Source files
    ├── docs/                   # Documentation
    ├── hardware/              # PCB and schematic files
    └── platformio.ini         # Project configuration
```

## Getting Started

1. Clone the repository
2. Open the project in PlatformIO
3. Install required dependencies
4. Configure the ESP-NOW settings through the web interface
5. Upload to respective devices
6. Access the web interface for additional configuration

## Building

Both projects use PlatformIO for building and deployment. Key build flags include:
- Thread name support
- ESP32 exception decoding
- Web server integration
- MIDI capabilities
- Custom debugging options

## Documentation

Detailed documentation is available in the `docs/` directory, including:
- ESP-NOW configuration guide
- Relationship between devices
- Hardware schematics
- PCB design files

## License

[Insert License Information]

## Contributing

[Insert Contribution Guidelines]