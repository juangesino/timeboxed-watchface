#ifndef __TIMEBOXED_WEATHER_
#define __TIMEBOXED_WEATHER_

#include <pebble.h>

void update_weather(bool force);
void update_weather_values(int temp_val, int weather_val);
void update_forecast_values(int max_val, int min_val);
void update_wind_values(int speed, int direction);
void update_sunrise(int sunrise);
void update_sunset(int sunset);
void store_weather_values(int temp_val, int max_val, int min_val, int weather_val, int speed_val, int direction_val, int sunrise_val, int sunset_val);
void toggle_weather(uint8_t reload_origin);
bool is_weather_enabled();
char* get_wind_direction(int degrees);
char* get_wind_direction_text(int degrees);

#endif
