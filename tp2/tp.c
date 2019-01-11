/* GPLv2 (c) Airbus */
#include <debug.h>
#include <info.h>
#include <excp.h>
#include <intr.h>

extern info_t *info;

extern int_desc_t IDT[IDT_NR_DESC];

/* Enable interrupts. */
void test_irq()
{
  while(1)
    asm volatile ("sti");
}


/* #BP handler. */
void bp_handler()
{
  debug("Breakpoint reached!\n");
}

/* #BP triggerer. */
void bp_trigger()
{
  asm volatile ("int3");
}


/* Main function. */
void tp()
{
  //test_irq();

  idt_reg_t idtr;
  get_idtr(idtr);
  debug("\nIDT loaded at: 0x%x\n", idtr.addr);

  debug("\nBefore breakpoint...\n");
  idtr.desc[BP_EXCP].offset_1 = (uint16_t) ((uint32_t)bp_handler & 0xFFFF);
  idtr.desc[BP_EXCP].offset_2 = (uint16_t) ((uint32_t)bp_handler>>16 & 0xFFFF);
  bp_trigger();
  debug("After breakpoint...\n");
}
