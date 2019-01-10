/* GPLv2 (c) Airbus */
#include <debug.h>
#include <info.h>

#include <segmem.h>

extern info_t *info;

/* Adds value, then tabulation(s) to debug buffer:
 *   - only 1 if only 8 chars
 *   - 2 if more than 8 chars
 */
void tabulate_hex(uint32_t value)
{
  debug("0x%x", value);
  if (value < 0x10000000)
    debug("\t\t");
  else
    debug("\t");
}
void tabulate_dec(uint32_t value)
{
  debug("%d", value);
  if (value < 10000000)
    debug("\t\t");
  else
    debug("\t");
}



/* Prints the current registered GDT. */
void print_gdt()
{
  gdt_reg_t gdtr;
  int i, n;

  // Get GDT
  get_gdtr(gdtr);
  debug("\nGDTR points on 0x%x (so GDT is here)\n", gdtr);

  // Print GDT content
  debug("  i\tBASE\t\tLIMIT\t\tTYPE\n");

  n = gdtr.limit / sizeof(seg_desc_t);
  for (i=0; i <= n; i++)
  {
    seg_desc_t* segm_desc = &gdtr.desc[i];

    uint32_t base  = (segm_desc->base_3 << 24)  | (segm_desc->base_2 << 16) | segm_desc->base_1;
    uint32_t limit = (segm_desc->limit_2 << 16) |  segm_desc->limit_1;

    debug("  %d\t", i);
    tabulate_hex(base);
    tabulate_hex(limit);
    debug("0x%x\n", segm_desc->type);
  }
}



/* Main TP function. */
void tp()
{
  print_gdt();
}
