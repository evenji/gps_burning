#include "gprs.h"

bool isGPRSReady = false;   //GPRS is ready?

char getIsGPRSReady()
{
    return isGPRSReady;
}

void setIsGPRSReady(bool status)
{
    isGPRSReady = status;
}