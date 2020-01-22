#include <string.h>
#include <stdio.h>
#include <api_os.h>
#include <api_gps.h>
#include <api_event.h>
#include <api_hal_uart.h>
#include <api_debug.h>
#include "buffer.h"
#include "math.h"
#include "gps.h"
#include "gps_task.h"

#define MAIN_TASK_STACK_SIZE    (2048 * 2)
#define MAIN_TASK_PRIORITY      1
#define MAIN_TASK_NAME          "GPS Task"

bool isGpsOn = true;
GPS_Info_t* gpsInfo = NULL;

void getGPSDate(char *date)
{
    if(NULL != gpsInfo)
    {
        sprintf(date, "20%d%02d%02d %02d:%02d:%02d", 
                                    gpsInfo->rmc.date.year, gpsInfo->rmc.date.month, gpsInfo->rmc.date.day,
                                    gpsInfo->rmc.time.hours, gpsInfo->rmc.time.minutes, gpsInfo->rmc.time.seconds);
    }
}

void getGPSInfo(char* longitude, char* latitude, char *altitude, char* speed, int* direction)
{
    int temp = (int)(gpsInfo->rmc.latitude.value/gpsInfo->rmc.latitude.scale/100);
    double fLatitude = temp+(double)(gpsInfo->rmc.latitude.value - temp*gpsInfo->rmc.latitude.scale*100)/gpsInfo->rmc.latitude.scale/60.0;
    temp = (int)(gpsInfo->rmc.longitude.value/gpsInfo->rmc.longitude.scale/100);
    double fLongitude = temp+(double)(gpsInfo->rmc.longitude.value - temp*gpsInfo->rmc.longitude.scale*100)/gpsInfo->rmc.longitude.scale/60.0;

    sprintf(longitude, "%f", fLongitude);
    sprintf(latitude, "%f", fLatitude);
    if(gpsInfo->gga.altitude.scale != 0)
    {
        sprintf(altitude, "%.2f", (float)gpsInfo->gga.altitude.value/gpsInfo->gga.altitude.scale);
    }
    else
    {
        sprintf(altitude, "%f", 0.0);
    }

    if(gpsInfo->rmc.speed.scale != 0)
    {
        sprintf(speed, "%.2f", (float)gpsInfo->rmc.speed.value/gpsInfo->rmc.speed.scale * 1.852);
    }
    else
    {
        sprintf(speed, "%f", 0.0);
    }

    if(gpsInfo->rmc.course.scale != 0)
    {
        *direction = (int)(gpsInfo->rmc.course.value/gpsInfo->rmc.course.scale);
    }
    else
    {
        *direction = 0;
    }
    
    
    
    
    // sprintf(speed, "%f", gpsInfo->rmc.speed.value);
}

void gps_Task(void *pData)
{
    gpsInfo = Gps_GetInfo();
    uint8_t buffer[300];
    uint8_t isFixed = 0;

    //wait for gprs register complete
    //The process of GPRS registration network may cause the power supply voltage of GPS to drop,
    //which resulting in GPS restart.
    while(!getIsGPRSReady())
    {
        Trace(1,"wait for gprs regiter complete");
        OS_Sleep(2000);
    }

    //open GPS hardware(UART2 open either)
    GPS_Init();
    GPS_Open(NULL);

    //wait for gps start up, or gps will not response command
    while(gpsInfo->rmc.latitude.value == 0)
        OS_Sleep(1000);
    

    // set gps nmea output interval
    for(uint8_t i = 0;i<5;++i)
    {
        bool ret = GPS_SetOutputInterval(10000);
        Trace(1,"set gps ret:%d",ret);
        if(ret)
            break;
        OS_Sleep(1000);
    }

    
    if(!GPS_GetVersion(buffer,150))
        Trace(1,"get gps firmware version fail");
    else
        Trace(1,"gps firmware version:%s",buffer);

    // if(!GPS_SetFixMode(GPS_FIX_MODE_LOW_SPEED))
        // Trace(1,"set fix mode fail");

    if(!GPS_SetOutputInterval(1000))
        Trace(1,"set nmea output interval fail");
    
    Trace(1,"init ok");

    while(1)
    {
        if(isGpsOn)
        {
            //show fix info
            isFixed = gpsInfo->gsa[0].fix_type > gpsInfo->gsa[1].fix_type ?gpsInfo->gsa[0].fix_type:gpsInfo->gsa[1].fix_type;
            char* isFixedStr = NULL;;            
            if(isFixed == 2)
                isFixedStr = "2D fix";
            else if(isFixed == 3)
            {
                if(gpsInfo->gga.fix_quality == 1)
                    isFixedStr = "3D fix";
                else if(gpsInfo->gga.fix_quality == 2)
                    isFixedStr = "3D/DGPS fix";
            }
            else
                isFixedStr = "no fix";

            //convert unit ddmm.mmmm to degree(°) 
            int temp = (int)(gpsInfo->rmc.latitude.value/gpsInfo->rmc.latitude.scale/100);
            double latitude = temp+(double)(gpsInfo->rmc.latitude.value - temp*gpsInfo->rmc.latitude.scale*100)/gpsInfo->rmc.latitude.scale/60.0;
            temp = (int)(gpsInfo->rmc.longitude.value/gpsInfo->rmc.longitude.scale/100);
            double longitude = temp+(double)(gpsInfo->rmc.longitude.value - temp*gpsInfo->rmc.longitude.scale*100)/gpsInfo->rmc.longitude.scale/60.0;

            
            //you can copy ` latitude,longitude ` to http://www.gpsspg.com/maps.htm check location on map

            snprintf(buffer,sizeof(buffer),"GPS fix mode:%d, BDS fix mode:%d, fix quality:%d, satellites tracked:%d, gps sates total:%d, is fixed:%s, coordinate:WGS84, Latitude:%f, Longitude:%f, unit:degree,altitude:%f",gpsInfo->gsa[0].fix_type, gpsInfo->gsa[1].fix_type,
                                                                gpsInfo->gga.fix_quality,gpsInfo->gga.satellites_tracked, gpsInfo->gsv[0].total_sats, isFixedStr, latitude,longitude,gpsInfo->gga.altitude);
            //show in tracer
            Trace(2,buffer);
        }

        OS_Sleep(5000);
    }
}