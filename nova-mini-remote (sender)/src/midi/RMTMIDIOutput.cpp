#include "RMTMIDIOutput.h"

// Default constructor: uses the default RMT channel and sets the mutex pointer to NULL.
RMTMIDIOutput::RMTMIDIOutput() : _channel(MIDI_RMT_CHANNEL), _mutex(NULL) { }

// Optional constructor: uses the specified RMT channel.
RMTMIDIOutput::RMTMIDIOutput(rmt_channel_t channel) : _channel(channel), _mutex(NULL) { }

void RMTMIDIOutput::begin(uint8_t txPin) {
  // Configure the RMT transmitter.
  rmt_config_t config;
  config.rmt_mode = RMT_MODE_TX;         // Set RMT to transmit mode.
  config.channel = _channel;              // Select the RMT channel.
  config.gpio_num = static_cast<gpio_num_t>(txPin); // Set TX GPIO pin.
  config.mem_block_num = 1;               // Use one memory block.
  // Set clk_div to 1 so each tick is 12.5ns (80MHz)
  config.clk_div = 1;

  // TX-specific settings.
  config.tx_config.loop_en = false;       // Disable looping.
  config.tx_config.carrier_en = false;    // Disable carrier modulation.
  config.tx_config.idle_output_en = true;  // Keep output HIGH when idle.
  config.tx_config.idle_level = RMT_IDLE_LEVEL_HIGH;
  
  // Apply the RMT configuration.
  if (rmt_config(&config) != ESP_OK) {
    Serial.println("Error: rmt_config() failed");
  }

  // Install the RMT driver for the specified channel.
  if (rmt_driver_install(_channel, 0, 0) != ESP_OK) {
    Serial.println("Error: rmt_driver_install() failed");
  }

  // Create a FreeRTOS mutex to protect access to the RMT peripheral.
  _mutex = xSemaphoreCreateMutex();
  if (_mutex == NULL) {
    Serial.println("Error: Failed to create mutex for RMTMIDIOutput");
  } else {
    Serial.println("RMTMIDIOutput: Mutex created successfully");
  }
}

size_t RMTMIDIOutput::write(uint8_t b) {
  // Attempt to take the mutex to ensure exclusive access.
  if (xSemaphoreTake(_mutex, portMAX_DELAY) != pdTRUE) {
    Serial.println("Error: Failed to take mutex in RMTMIDIOutput::write()");
    return 0;
  }

  // Create the 10-bit frame: start bit, 8 data bits (LSB first), stop bit.
  const uint8_t totalBits = 10;
  uint8_t bits[totalBits];
  bits[0] = 0; // Start bit (LOW)
  for (int i = 0; i < 8; i++) {
    bits[i + 1] = (b >> i) & 0x01;  // Extract each data bit (LSB first)
  }
  bits[9] = 1; // Stop bit (HIGH)

  // Use the configured bit duration
  const uint32_t BIT_DURATION_TICKS = RMT_BIT_DURATION_TICKS;

  // Pack the 10 bits into 5 RMT items (each item can represent 2 pulses).
  rmt_item32_t items[5];
  for (int i = 0; i < 5; i++) {
    int bitIndex0 = i * 2;
    int bitIndex1 = bitIndex0 + 1;

    // First pulse for the first bit.
    uint8_t level0 = bits[bitIndex0] ? 1 : 0;
    uint32_t duration0 = BIT_DURATION_TICKS;

    // Second pulse for the second bit.
    uint8_t level1 = bits[bitIndex1] ? 1 : 0;
    uint32_t duration1 = BIT_DURATION_TICKS;

    // Do not merge pulses: output each pulse separately.
    items[i].level0 = level0;
    items[i].duration0 = duration0;
    items[i].level1 = level1;
    items[i].duration1 = duration1;
  }

  // Send the RMT items; this call blocks until the items are written into hardware.
  esp_err_t err = rmt_write_items(_channel, items, 5, true);
  if (err != ESP_OK) {
    Serial.print("Error: rmt_write_items() failed with code ");
    Serial.println(err);
    xSemaphoreGive(_mutex);
    return 0;
  }
  
  // Wait until the transmission is complete.
  err = rmt_wait_tx_done(_channel, portMAX_DELAY);
  if (err != ESP_OK) {
    Serial.print("Error: rmt_wait_tx_done() failed with code ");
    Serial.println(err);
    xSemaphoreGive(_mutex);
    return 0;
  }

  // Release the mutex.
  xSemaphoreGive(_mutex);
  return 1;  // Indicate that one byte was transmitted.
}

// The following Stream methods are required but not used for this write-only stream.
int RMTMIDIOutput::available() { return 0; }
int RMTMIDIOutput::read() { return -1; }
int RMTMIDIOutput::peek() { return -1; }
void RMTMIDIOutput::flush() { }