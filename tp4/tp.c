/* GPLv2 (c) Airbus */
#include <debug.h>
#include <info.h>

#include <cr.h>
#include <pagemem.h>

extern info_t *info;

void show_cr3()
{
  cr3_reg_t cr3 = { .raw = get_cr3() };
  debug("\nCR3 = %p\n", cr3.raw);
  debug("  - PWT  = %d\n", cr3.pwt);
  debug("  - PCD  = %d\n", cr3.pcd);
  debug("  - addr = %3p\n", cr3.addr);
}


void init_identity_mapping()
{
  // Allocate PGD and PGT
  pde32_t* pgd = (pde32_t*) 0x600000;
  pte32_t* ptb = (pte32_t*) 0x601000;

  // PGT identity-mapping
  for (int i=0; i < 1024; i++)
    pg_set_entry(&ptb[i], PG_RW | PG_KRN, i);

  // PGD initialization
  memset((void*)pgd, 0, PAGE_SIZE);
  pg_set_entry(&pgd[0], PG_RW | PG_KRN, page_nr(ptb));

  // Paging activation
  debug("Updating cr3 with PGD address...\n");
  set_cr3((uint32_t) pgd);
  debug("  .. done!\n");

  debug("Activating cr0.pg...\n");
  cr0_reg_t cr0 = { .raw = get_cr0() };
  cr0.pg = 1;
  set_cr0(cr0);
  debug("  .. done!\n");
}

void tp()
{
  show_cr3();
  init_identity_mapping();
}
