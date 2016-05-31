#ifndef UPS_H
#define UPS_H
#include <stdint.h>

void ups_init(void);
void ups_refresh(void);

uint16_t ups_main_voltage(void);
uint16_t ups_battery_voltage(void);
uint8_t ups_battery_charge(void);

void ups_connect_battery(void);
void ups_disconnect_battery(void);

#endif //UPS_H