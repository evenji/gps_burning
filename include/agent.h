#ifndef _AGENT_H_
#define _AGENT_H_
#include "fault_code.h"

char* getDevInfoJsonStr();
E_FAULT_CODE parserJson(const char *command);

#endif