#ifndef WEB_SERVER_H
#define WEB_SERVER_H

#include <Arduino.h>

// Flags for cross-task communication (set by web handlers, read by main loop)
extern volatile bool flagWifiChanged;
extern volatile bool flagLoraChanged;
extern volatile bool flagForwardChanged;
extern volatile bool flagRestartRequested;
extern volatile bool flagRescanRequested;

void webServerInit();

#endif
