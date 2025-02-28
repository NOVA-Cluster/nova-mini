# ESP-NOW Configuration Tab Documentation

## Overview
The ESP-NOW configuration tab provides a user interface for setting up the wireless communication between the NOVA Mini Remote (sender) and the NOVA Mini (receiver) using ESP-NOW protocol.

## UI Layout
The tab is organized in the following order:
1. Instructions section
2. Local MAC Address display
3. Saved Receiver MAC Address display
4. Status message
5. Set Receiver MAC Address input field

## Interface Elements

### Instructions Section
- Clear step-by-step guidance
- Example MAC address format: 40:91:51:XX:XX:XX
- Example valid addresses for reference
- Format requirements clearly stated

### Local MAC Address
- Displays this device's MAC address
- Read-only label
- Used for reference/debugging

### Saved Receiver MAC Address
- Shows currently configured receiver MAC
- Updates immediately when new MAC is saved
- Shows "Not set" when no MAC is configured

### Status Messages
- Real-time validation feedback
- Shows character count while typing
- Indicates when format is valid
- Prompts to press Enter to save
- Confirms successful saves
- Shows detailed error messages

### MAC Address Input
- Label: "Set Receiver MAC Address"
- Validates in real-time
- Auto-converts to uppercase
- Checks format: XX:XX:XX:XX:XX:XX

## Validation Rules
1. Length must be exactly 17 characters
2. Format must be XX:XX:XX:XX:XX:XX where:
   - X = hex digit (0-9, A-F)
   - : = colon separator
3. Colons must be at positions 3, 6, 9, 12, and 15

## User Workflow
1. View instructions and example format
2. Enter MAC address in input field
3. Receive immediate validation feedback
4. Press Enter when format is valid
5. View confirmation in status and saved MAC display
6. Clear input field ready for next use

## Data Storage
- Stored in preferences namespace "nova"
- Key: "receiver_mac"
- Persists across reboots
- Automatically loaded on startup

## ESP-NOW Integration
- MAC address saved in preferences
- Used to initialize ESP-NOW peer
- Automatically reconnects after changes
