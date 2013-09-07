/*kernel/clock.c*/
void init_clock();
void clock_handler(int irq);
void milli_delay(int milli_delay);
int get_ticks();
int get_milli_seconds();
/**/
