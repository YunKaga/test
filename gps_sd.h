#ifndef GPS_SD_H
#define GPS_SD_H
#include <TinyGPSPlus.h>
extern TinyGPS gps;
void gps_sd_init();
void gps_sd_update(double sensors_data[]);
#endif
