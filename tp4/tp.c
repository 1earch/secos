/* GPLv2 (c) Airbus */
#include <debug.h>
#include <info.h>

#include <cr.h>
#include <pagemem.h>

extern info_t *info;

void tp()
{
  cr3_reg_t cr3 = { .raw = get_cr3() };
  debug("\nCR3 = %p\n", cr3.raw);
  debug("  - PWT  = %d\n", cr3.pwt);
  debug("  - PCD  = %d\n", cr3.pcd);
  debug("  - addr = %3p\n", cr3.addr);

  debug("\nAllocating a PDE entry at 0x600000...\n");
  pde32_t* pgd = (pde32_t*) 0x600000;
  debug("  .. done!\n");

  debug("Updating cr3 with this address...\n");
  set_cr3((uint32_t) pgd);
  debug("  .. done!\n");

  debug("Activating cr0.pg...\n");
  cr0_reg_t cr0 = { .raw = get_cr0() };
  cr0.pg = 1;
  set_cr0(cr0);
  debug("  .. done!\n");
}
