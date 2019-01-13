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
  uint32_t i;

  // Allocate PGD and PGT
  pde32_t* pgd  = (pde32_t*) 0x600000;
  pte32_t* ptb1 = (pte32_t*) 0x601000;
  pte32_t* ptb2 = (pte32_t*) 0x602000;

  // PTB1 identity-mapping
  for (i=0; i < PTE32_PER_PT; i++)
    pg_set_entry(&ptb1[i], PG_RW | PG_KRN, i);

  // PTB2 mapping 0x601000:
  //   in entry n°513 of PTB2, we must have mapped address 0x601 (1537)
  //   so we must put in first entry of PTB2 1024 (ie 1537-513=1024)
  for (i=0; i < PTE32_PER_PT; i++)
    pg_set_entry(&ptb2[i], PG_RW | PG_KRN, i+1024);

  // PGD initialization
  memset((void*)pgd, 0, PAGE_SIZE);
  pg_set_entry(&pgd[0], PG_RW | PG_KRN, page_nr(ptb1));
  pg_set_entry(&pgd[1], PG_RW | PG_KRN, page_nr(ptb2));

  // Paging activation
  debug("Updating cr3 with PGD address...\n");
  set_cr3((uint32_t) pgd);
  debug("  .. done!\n");

  debug("Activating cr0.pg...\n");
  cr0_reg_t cr0 = { .raw = get_cr0() };
  cr0.pg = 1;
  set_cr0(cr0);
  debug("  .. done!\n");

  // Display a PTB entry
  for (i=0; i < 10; i++)
    debug("PTB[%d]: %p\n", i, ptb1[i].addr);
}

void tp()
{
  show_cr3();
  init_identity_mapping();
}
