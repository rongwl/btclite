#ifndef BTCDEMO_INIT_H
#define BTCDEMO_INIT_H

#include "util.h"

#include <string>

void Interrupt(boost::thread_group*);
void Shutdown();
bool get_shutdown_requested();
std::string HelpMessage();
void PrintUsage();
bool AppInitParameterInteraction();
bool AppInitBasicSetup();
bool AppInitMain();

#endif // BTCDEMO_INIT_H
