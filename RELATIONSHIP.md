# Relationship between Nova Mini and Nova Mini Remote

**Nova Mini (Receiver):**
- Acts as an ESPNOW receiver.
- Listens for incoming PooferMessage commands via ESPNOW.
- Triggers actions (e.g., relay operations and LED effects) based on the received message.
- Sends an acknowledgment (ACK) back to the sender using ESPNOW.

**Nova Mini Remote (Sender):**
- Provides a user interface to send control commands.
- Transmits commands over the network (e.g., HTTP requests) to Nova Mini.
- Allows remote control of Nova Mini's functionalities.

**How They Work Together:**
1. The Nova Mini Remote sends a command (like triggering a relay or changing modes) over the network.
2. Nova Mini, acting as the receiver, listens for these commands on its web server or other endpoints.
3. Once a command is received, Nova Mini executes the corresponding action, updating its hardware status and web interface.
4. This integration enables remote operation of the Nova Mini device.
