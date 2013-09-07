/*kernel/i8259.c*/
void init_8259A();
void spurious(int irq);
void put_irq_handler(int irq,irq_handler handler);
/**/
