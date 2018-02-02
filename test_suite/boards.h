#ifdef ROOMSENSE
int gpio_pins[] =  {PA19, PA17, PA16, PA21, PA20, PA7, PA3, PA9, PA8, PB23, PB22};
int pwm_pins[] = {7,8,9,10};
int ain_pins[] = {PA2, PB2, PB3, PA4, PA5, PA6};
#endif
#ifdef STRATA
int gpio_pins[] = {PB0, PB1, PB2, PB3, PB4, PB5, PB6, PB7, PC7, PD5, PD6};
int pwm_pins[] = {PD7};
int ain_pins[] = {};
#endif
