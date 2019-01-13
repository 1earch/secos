/* GPLv2 (c) Airbus */
#include <debug.h>
#include <info.h>

#include <cr.h>
#include <pagemem.h>

#define PGD_ADDR        0x600000
#define PTB1_ADDR       0x601000
#define PTB2_ADDR       0x602000
#define PTB3_ADDR       0x603000
#define PGD_PAGED_ADDR  0xC0000000
#define SHARED_VADDR1   0x700000
#define SHARED_VADDR2   0x7ff000
#define SHARED_ADDR     0x2000

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
  pde32_t* pgd  = (pde32_t*) PGD_ADDR;
  pte32_t* ptb1 = (pte32_t*) PTB1_ADDR;
  pte32_t* ptb2 = (pte32_t*) PTB2_ADDR;

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
  debug("\n");
  for (i=0; i < 5; i++)
    if (pg_present(&ptb1[i]))
      debug("PTB[%d]: %p\n", i, ptb1[i].addr);


  // PTB3 mapping 0xC000.0000
  pte32_t* ptb3 = (pte32_t*) PTB3_ADDR;
  uint32_t ptb3_idx     = pt32_idx(PGD_PAGED_ADDR);
  uint32_t pgd_ptb3_idx = pd32_idx(PGD_PAGED_ADDR);

  memset((void*) ptb3, 0, PAGE_SIZE);
  pg_set_entry(&pgd[pgd_ptb3_idx], PG_RW | PG_KRN, page_nr(ptb3));
  pg_set_entry(&ptb3[ptb3_idx], PG_RW | PG_KRN, page_nr(pgd));

  debug("\n");
  debug("To map %p on PGD: PGD[%d] => PTB[%d] => %p\n", PGD_PAGED_ADDR, pgd_ptb3_idx, ptb3_idx, pgd);

  // Display a PGD entry
  pde32_t* pgd_0xc = (pde32_t*) PGD_PAGED_ADDR;
  for (i=0; i < 5; i++)
    debug("PGD[%d]: %p\n", i, pgd_0xc[i].addr);


  // Shared memory
  char* vstr1 = (char*) SHARED_VADDR1;
  char* vstr2 = (char*) SHARED_VADDR2;

  uint32_t ptb2_idx_vstr1 = pt32_idx(SHARED_VADDR1);
  uint32_t ptb2_idx_vstr2 = pt32_idx(SHARED_VADDR2);
  pg_set_entry(&ptb2[ptb2_idx_vstr1], PG_RW | PG_KRN, page_nr(SHARED_ADDR));
  pg_set_entry(&ptb2[ptb2_idx_vstr2], PG_RW | PG_KRN, page_nr(SHARED_ADDR));

  debug("\n");
  debug("%p = %s\n", vstr1, vstr1);
  debug("%p = %s\n", vstr2, vstr2);


  // 1st entry deletion: we must invalidate TLBs
  memset((void*) PGD_PAGED_ADDR, 0, sizeof(pde32_t));
  // invalidate(pgd);
  //or
  // set_cr3((uint32_t) PGD_ADDR);

  // Display a PTB entry
  debug("\n");
  for (i=0; i < 5; i++)
    if (pg_present(&ptb1[i]))
      debug("PTB[%d]: %p\n", i, ptb1[i].addr);
}

void tp()
{
  show_cr3();
  init_identity_mapping();
}
