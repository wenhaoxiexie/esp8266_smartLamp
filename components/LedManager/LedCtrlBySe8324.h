#ifndef __LEDCTRLBYSE8324_H_
#define __LEDCTRLBYSE8324_H_

void se8324_pwm_init(void);

void se8324_adjust_light(bool sel, uint8_t light);

void se8324_breath_light(uint8_t count);

#endif  //__LEDCTRLBYSE8324_H_



