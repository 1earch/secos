/* GPLv2 (c) Airbus */
#include <debug.h>
#include <info.h>
#include <segmem.h>
#include <gpr.h>
#include <intr.h>

extern info_t *info;


seg_desc_t GDT[6];
tss_t TSS;


#define C0_IDX  1
#define D0_IDX  2
#define C3_IDX  3
#define D3_IDX  4
#define TSS_IDX 5

/* Prints the current registered GDT. */
void print_gdt()
{
  gdt_reg_t gdtr;
  int i, n;

  // Get GDT
  get_gdtr(gdtr);
  debug("\nGDTR points on 0x%x (so GDT is here)\n", gdtr.addr);

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


/* Encode a GDT TSS entry. */
void encode_gdt_tss_entry(seg_desc_t* desc, offset_t base, uint64_t type)
{
  desc->raw = 0ULL;

  desc->base_1 = base & 0xFFFF;
  desc->base_2 = (base >> 16) & 0xFFFF;
  desc->base_3 = (base >> 24) & 0xFF;

  desc->limit_1 = sizeof(tss_t) & 0xFFFF;
  desc->limit_2 = (sizeof(tss_t) >> 16) & 0xF;

  desc->type = type;    // Segment type
  desc->dpl  = 0;       // descriptor privilege level

  desc->s = 0;          // 0, ie system
  desc->p = 1;          // Segment present
  desc->l = 0;          // Not 64-bit mode segment
  desc->d = 0;          // 16-bit segment
  desc->g = 0;          // Granularity => relative to limit?
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
  encode_gdt_entry(&GDT[C0_IDX], 0, 0xFFFFF, SEG_DESC_CODE_XR, 0);
  //   - Init data segment (base=0, limit=max, type=data_rw, dpl=ring0)
  encode_gdt_entry(&GDT[D0_IDX], 0, 0xFFFFF, SEG_DESC_DATA_RW, 0);
  //   - Init code segment (base=0, limit=max, type=code_xr, dpl=ring3)
  encode_gdt_entry(&GDT[C3_IDX], 0, 0xFFFFF, SEG_DESC_CODE_XR, 3);
  //   - Init data segment (base=0, limit=max, type=data_rw, dpl=ring3)
  encode_gdt_entry(&GDT[D3_IDX], 0, 0xFFFFF, SEG_DESC_DATA_RW, 3);

  // Configure gdtr new value
  gdtr.limit = sizeof(GDT) - 1;
  gdtr.desc  = GDT;

  // Load GDT
  set_gdtr(gdtr);

  // Load segment selectors
  set_cs(gdt_krn_seg_sel(C0_IDX));

  set_ss(gdt_krn_seg_sel(D0_IDX));
  set_ds(gdt_krn_seg_sel(D0_IDX));
  set_es(gdt_krn_seg_sel(D0_IDX));
  set_fs(gdt_krn_seg_sel(D0_IDX));
  set_gs(gdt_krn_seg_sel(D0_IDX));
}


/* Int 48 handler. */
void syscall_isr()
{
   asm volatile (
      "leave ; pusha        \n"
      "mov %esp, %eax      \n"
      "call syscall_handler \n"
      "popa ; iret"
      );
}

/* Syscall handler. */
void __regparm__(1) syscall_handler(int_ctx_t *ctx)
{
   debug("SYSCALL eax = %p\n", ctx->gpr.eax);
}


/* Userland function. */
void userland()
{
  debug("\nIn userland!!!\n");
  asm volatile ("int $48");
  debug("\nHalted!\n");
  while (true);
}


/* Work with registered segment. */
void work()
{
  debug("\nLoading ring3 segment selectors...\n");

  // Load segment selectors
  set_ds(gdt_usr_seg_sel(D3_IDX));
  debug("  ... ds loaded with: %p\n", gdt_usr_seg_sel(D3_IDX));
  set_es(gdt_usr_seg_sel(D3_IDX));
  debug("  ... es loaded with: %p\n", gdt_usr_seg_sel(D3_IDX));
  set_fs(gdt_usr_seg_sel(D3_IDX));
  debug("  ... fs loaded with: %p\n", gdt_usr_seg_sel(D3_IDX));
  set_gs(gdt_usr_seg_sel(D3_IDX));
  debug("  ... gs loaded with: %p\n", gdt_usr_seg_sel(D3_IDX));

  // Configure TSS
  TSS.s0.esp = get_ebp();
  TSS.s0.ss  = gdt_krn_seg_sel(D0_IDX);
  // Register TSS in GDT
  encode_gdt_tss_entry(&GDT[TSS_IDX], (offset_t) &TSS, SEG_DESC_SYS_TSS_AVL_32);
  // Register TSS in tr
  set_tr(gdt_krn_seg_sel(TSS_IDX));
  debug("TSS loaded with: %p\n\n", gdt_krn_seg_sel(TSS_IDX));

  // Configure int 48 DPL and handler
  idt_reg_t idtr;
  get_idtr(idtr);
  int_desc_t* int48 = &idtr.desc[48];
  int48->dpl = 3;

  raw32_t syscall_isr_addr = { .raw = (uint32_t) syscall_isr };
  int48->offset_1 = syscall_isr_addr.wlow;
  int48->offset_2 = syscall_isr_addr.whigh;

  // Go to userland
  asm volatile (
      "push %0\n"         // ss3
      "push $0x600000\n"  // esp3
      "pushf\n"           // eflags
      "push %1\n"         // cs3
      "push %2\n"         // new eip = @userland
      "iret"
      ::
        "i" (gdt_usr_seg_sel(D3_IDX)),
        "i" (gdt_usr_seg_sel(C3_IDX)),
        "r" (&userland)
      :
      );
}


/* Main TP function. */
void tp()
{
  print_gdt();
  init_gdt();
  print_gdt();
  work();
}

