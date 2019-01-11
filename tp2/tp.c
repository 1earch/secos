/* GPLv2 (c) Airbus */
#include <debug.h>
#include <info.h>

extern info_t *info;

/* Enable interrupts. */
void test_irq()
{
  while(1)
    __asm__("sti");
}


void tp()
{
  //test_irq();
}
