#ifndef BTCLITE_INIT_H
#define BTCLITE_INIT_H

#include "util.h"

#include <string>

void Interrupt();
void Shutdown();
bool get_g_shutdown_requested();
void PrintUsage();
bool AppInitParameter(int, char* const*);
bool AppInitParameterInteraction();
bool AppInitBasicSetup();
bool AppInitLogging(int, char* const*);
bool AppInitMain();


#endif // BTCLITE_INIT_H
