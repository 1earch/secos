/* GPLv2 (c) Airbus */
#include <debug.h>
#include <info.h>

extern info_t   *info;
extern uint32_t __kernel_start__;
extern uint32_t __kernel_end__;

void tp()
{
  debug("\n");
  debug("kernel mem [0x%x - 0x%x]\n", &__kernel_start__, &__kernel_end__);
  debug("MBI flags 0x%x\n", info->mbi->flags);
  debug("\n");

  // Get MBI struct
  struct multiboot_info* mbi = info->mbi;

  // Check that bit 6 of flags is set (in which case: mmap_addr is valid)
  if (mbi->flags & (1<<6))
  {
    debug("... valid mmap_addr\n");
    debug("\n");

    debug("MMAP_ADDR   0x%x\n", mbi->mmap_addr);
    debug("MMAP_LENGTH 0x%x\n", mbi->mmap_length);


    // Print memory map
    debug("\n");
    debug("Memory map:\n");

    multiboot_memory_map_t* mbi_entry = (multiboot_memory_map_t*) mbi->mmap_addr;
    multiboot_memory_map_t* end   =
      // mmap_length gives us the total buffer length containing the memory table
      (multiboot_memory_map_t*) (mbi->mmap_addr + mbi->mmap_length);

    debug("  [mem BEGINNING\tLENGTH]\t\tTYPE\n");
    while (mbi_entry < end) {
      debug("  [mem 0x%x\t", mbi_entry->addr);
      if (mbi_entry->addr < 0x1000000)
        debug("\t");

      debug("0x%x]\t", mbi_entry->len);
      if (mbi_entry->len < 0x10000)
        debug("\t");

      debug(" %d - ", mbi_entry->type);
      switch (mbi_entry->type)
      {
        case MULTIBOOT_MEMORY_AVAILABLE:
          debug("available");
          break;

        case MULTIBOOT_MEMORY_RESERVED:
          debug("reserved");
          break;

        case MULTIBOOT_MEMORY_ACPI_RECLAIMABLE:
          debug("ACPI data");
          break;

        case MULTIBOOT_MEMORY_NVS:
          debug("ACPI nvs (needs to be preserved on hibernation)");
          break;

        default:
          debug("unknown /!\\");
      }
      debug("\n");

      mbi_entry++;
    }


    // Test memory areas writability
    debug("\nTest pointer available (type 1):\n");
    int* avail = (int*) 0x0;
    debug("  - r = %d\n", *avail);
    *avail = 3;
    debug("  - w = %d\n", *avail);

    debug("\nTest pointer reserved (type 2):\n");
    int* reserved = (int*) 0x9fc00;
    debug("  - r = %d\n", *reserved);
    *reserved = 3;
    debug("  - w = %d\n", *reserved);

    debug("\nOutside available RAM (128MB):\n");
    int* outside = (int*) 0xc0000;
    debug("  - r = %d\n", *outside);
    *outside = 3;
    debug("  - w = %d\n", *outside);
  }
}
