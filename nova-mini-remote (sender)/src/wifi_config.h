#ifndef WIFI_CONFIG_H
#define WIFI_CONFIG_H

// WiFi Networks Configuration
struct WifiNetwork {
    const char* ssid;
    const char* password;
};

// Add your networks here
static const WifiNetwork networks[] = {
    {"things", "stuffAndThings!"},
    {"your-ssid-2", "your-password-2"},
    // Add more networks as needed
};

// Number of networks in the array
static const int NETWORK_COUNT = sizeof(networks) / sizeof(networks[0]);

#endif // WIFI_CONFIG_H
