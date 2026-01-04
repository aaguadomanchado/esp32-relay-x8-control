#ifndef WEB_SERVER_HANDLERS_H
#define WEB_SERVER_HANDLERS_H

#include <WebServer.h>
#include "relay_manager.h"
#include "timer_manager.h"

void registerHandlers(WebServer &server, RelayManager &rm, TimerManager &tm, Preferences &prefs, void (*logCb)(const char*));

#endif