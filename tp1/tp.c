/* GPLv2 (c) Airbus */
#include <debug.h>
#include <info.h>
#include <segmem.h>
#include <string.h>

extern info_t *info;

seg_desc_t GDT[4];

#define code_index 1
#define data_index 2
#define extra_index 3


/* Prints the current registered GDT. */
void print_gdt()
{
  gdt_reg_t gdtr;
  int i, n;

  // Get GDT
  get_gdtr(gdtr);
  debug("\nGDTR points on 0x%x (so GDT is here)\n", gdtr);

  // Print GDT content
  n = gdtr.limit / sizeof(seg_desc_t);
  for (i=0; i <= n; i++)
  {
    seg_desc_t* segm_desc = &gdtr.desc[i];

    uint32_t base  = (segm_desc->base_3 << 24)  | (segm_desc->base_2 << 16) | segm_desc->base_1;
    uint32_t limit = (segm_desc->limit_2 << 16) |  segm_desc->limit_1;

    debug("  GDT[%d]: .raw= 0x%llx | .base= 0x%x | .limit=0x%x | .type= 0x%x\n",
        i, segm_desc->raw, base, limit, segm_desc->type);
  }
}


/* Encode a GDT entry. */
void encode_gdt_entry(seg_desc_t* desc, uint32_t base, uint32_t limit, uint64_t type, uint64_t dpl)
{
  desc->raw = 0ULL;

  desc->base_1 = base & 0xFFFF;
  desc->base_2 = (base >> 16) & 0xFFFF;
  desc->base_3 = (base >> 24) & 0xFF;

  desc->limit_1 = limit & 0xFFFF;
  desc->limit_2 = (limit >> 16) & 0xF;

  desc->type = type;    // Segment type
  desc->dpl  = dpl;     // descriptor privilege level

  desc->s = 1;          // 1, ie code or data
  desc->p = 1;          // Segment present
  desc->l = 0;          // Not 64-bit mode segment
  desc->d = 1;          // 32-bit segment
  if (limit == 0xFFFFF)
    desc->g = 1;        // Granularity => pages of 4kB
  else
    desc->g = 0;        // Granularity => relative to limit?
  desc->avl = 0;        // Not available for use by system software
}

/* GDT initialization. */
void init_gdt()
{
  gdt_reg_t gdtr;

  // Fill GDT:
  //   - Init null segment
  GDT[0].raw = 0ULL;
  //   - Init code segment (base=0, limit=max, type=code_xr, dpl=ring0)
  encode_gdt_entry(&GDT[code_index], 0, 0xFFFFF, SEG_DESC_CODE_XR, 0);
  //   - Init data segment (base=0, limit=max, type=data_rw, dpl=ring0)
  encode_gdt_entry(&GDT[data_index], 0, 0xFFFFF, SEG_DESC_DATA_RW, 0);
  //   - Empty last segment
  GDT[extra_index].raw = 0ULL;

  // Configure gdtr new value
  gdtr.limit = sizeof(GDT) - 1;
  gdtr.desc  = GDT;

  // Load GDT
  set_gdtr(gdtr);

  // Load segment selectors
  set_cs(gdt_krn_seg_sel(code_index));

  set_ds(gdt_krn_seg_sel(data_index));
  set_es(gdt_krn_seg_sel(data_index));
  set_ss(gdt_krn_seg_sel(data_index));
  set_fs(gdt_krn_seg_sel(data_index));
  set_gs(gdt_krn_seg_sel(data_index));
}


/* Work with registered segment. */
void work()
{
  char src[64];
  char *dst = 0;

  memset(src, 0xff, 64);
  debug("\nmemset OK!\n");

  // Create extra segment (base=0x600000, limit=32, type=data_rw, dpl=ring0)
  encode_gdt_entry(&GDT[extra_index], 0x600000, 32, SEG_DESC_DATA_RW, 0);
  set_es(gdt_krn_seg_sel(extra_index));

  _memcpy8(dst, src, 32);
  debug("memcpy8 32 OK!\n");
  _memcpy8(dst, src, 64);
  debug("memcpy8 64 OK!\n");
}


/* Main TP function. */
void tp()
{
  print_gdt();
  init_gdt();
  print_gdt();
  work();
}
