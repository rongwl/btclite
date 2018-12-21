#ifndef BTCLITE_INIT_H
#define BTCLITE_INIT_H

#include "util.h"

#include <string>

void Interrupt();
void Shutdown();
bool get_shutdown_requested();
void PrintUsage();
bool AppInitParameterInteraction();
bool AppInitBasicSetup();
bool AppInitMain();

#endif // BTCLITE_INIT_H
