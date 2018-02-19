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
#ifdef AEROCORE
int gpio_pins[] = {PB9, PB8, PC9, PB0, PE5, PE6, PC6, PC7, PC8, PA8, PA9, PA10, PE10, PE9};
int pwm_pins[] = {PD12, PD13, PD14, PD15, PA0, PA1, PA2, PA3};
int ain_pins[] = {PC1, PC2, PC3};
#endif
