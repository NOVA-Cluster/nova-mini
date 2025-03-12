#ifndef RMTMIDIOUTPUT_H
#define RMTMIDIOUTPUT_H

#include <Arduino.h>
#include "driver/rmt.h"         // ESP32 RMT driver header
#include "freertos/semphr.h"    // FreeRTOS semaphore header
#include "../configuration.h"    // Include configuration header

// Default RMT bit duration if not defined in configuration
#ifndef RMT_BIT_DURATION_TICKS
#define RMT_BIT_DURATION_TICKS 2560  // Default to nova-core value
#endif

// RMTMIDIOutput subclasses Stream so that it can be used as the output stream
// for the Arduino MIDI Library. It converts each byte into a MIDI serial frame
// (start bit, 8 data bits, stop bit) and sends it via the ESP32 RMT peripheral.
// A FreeRTOS mutex protects the RMT hardware so that it is safe to use across tasks.
class RMTMIDIOutput : public Stream {
public:
  // Use default RMT channel 0 if not otherwise defined.
#ifndef MIDI_RMT_CHANNEL
#define MIDI_RMT_CHANNEL RMT_CHANNEL_0
#endif

  // Default constructor: uses the default RMT channel.
  RMTMIDIOutput();

  // Optional constructor to specify a different RMT channel.
  RMTMIDIOutput(rmt_channel_t channel);

  // Initializes the RMT transmitter on the specified TX pin.
  void begin(uint8_t txPin);

  // The write() method converts a single byte into a MIDI frame and sends it via RMT.
  virtual size_t write(uint8_t b);

  // The following methods are required by the Stream class.
  virtual int available();
  virtual int read();
  virtual int peek();
  virtual void flush();

private:
  rmt_channel_t _channel;       // RMT channel used for transmission.
  SemaphoreHandle_t _mutex;     // Mutex to protect access to the RMT peripheral.
};

#endif  // RMTMIDIOUTPUT_H