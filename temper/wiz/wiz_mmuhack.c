#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <fcntl.h>
#include <sys/mman.h>
#include <linux/fb.h>
#include <unistd.h>
#include <stropts.h>
#include <string.h>
#include <errno.h>

extern int errno;
int memfd;

void invalidate_icache_all();
void invalidate_dcache_all();

void hack_coarse_pagetable(int mem_fd, int pagetable_entry)
{
  int coarse_pagetable[256];
  int coarse_pagetable_offset = pagetable_entry & 0xFFFFFC00;
  int coarse_pagetable_entry;
  int physical_offset;
  int modified_coarse_pagetable = 0;
  int i;

  // Seek to the course pagetable in the memory table.
  lseek(mem_fd, coarse_pagetable_offset, SEEK_SET);

  // Load it into our array for adjustment.
  read(mem_fd, coarse_pagetable, 256 * 4);

  // Modify each entry in the coarse pagetable such that it's cached and has
  // write buffering enabled if it's from 0x3000000 to 0x4000000

  for(i = 0; i < 256; i++)
  {
    coarse_pagetable_entry = coarse_pagetable[i];
    if(coarse_pagetable_entry)
    {
      physical_offset = coarse_pagetable_entry & 0xFFFF0000;
      if((coarse_pagetable_entry & 0x3) == 0x2)
      {
        if(((physical_offset >= 0x02A00000) && (physical_offset <= 0x04000000))
         && ((coarse_pagetable_entry & 0xC) == 0))
        {
          coarse_pagetable[i] = coarse_pagetable_entry | 0xFFC;
          modified_coarse_pagetable = 1;
        }
      }
    }
  }

  if(modified_coarse_pagetable)
  {
    lseek(mem_fd, coarse_pagetable_offset, SEEK_SET);
    write(mem_fd, coarse_pagetable, 256 * 4);
  }
}


void hack_pagetable(int mem_fd, int pagetable_offset)
{
  int base_pagetable[4096];
  int pagetable_entry;
  int i;

  lseek(mem_fd, pagetable_offset, SEEK_SET);
  read(mem_fd, base_pagetable, 4096 * 4);

  for(i = 0; i < 4096; i++)
  {
    pagetable_entry = base_pagetable[i];
    if(pagetable_entry)
    {
      if((pagetable_entry & 0x3) == 1)
        hack_coarse_pagetable(mem_fd, pagetable_entry);
    }
  }
}

void backup_uname(int mem_fd, int uname_offset, char *buffer)
{
  if(lseek(mem_fd, uname_offset, SEEK_SET) < 0)
  {
    printf("backup_uname: lseek failure %s\n", strerror(errno));
    return;
  }

  if(read(mem_fd, buffer, 16) != 16)
    printf("backup_uname: read failure\n");
}

void restore_uname(int mem_fd, int uname_offset, char *buffer)
{
  if(lseek(mem_fd, uname_offset, SEEK_SET) < 0)
  {
    printf("restore_uname: lseek failure %s\n", strerror(errno));
    return;
  }

  if(write(mem_fd, buffer, 16) != 16)
    printf("restore_uname: write failure\n");
}

void patch_uname(int mem_fd, int uname_offset, void *new_routine,
 int routine_length)
{
  if(lseek(mem_fd, uname_offset, SEEK_SET) < 0)
  {
    printf("patch_uname: lseek failure %s\n", strerror(errno));
    return;
  }

  if(write(mem_fd, new_routine, routine_length) != routine_length)
  {
    printf("patch_uname: write failure.\n");
    return;
  }

  invalidate_dcache_all();
  invalidate_icache_all();
}

int invoke_kernel_custom_code(int input);

int get_pagetable_offset(int mem_fd, int uname_offset)
{
  int get_pagetable_offset_routine[2] =
  {
    0xEE120F10, // mrc p15, 0, r0, c2, c0, 0
    0xE12FFF1E  // bx lr
  };

  patch_uname(mem_fd, uname_offset, get_pagetable_offset_routine, 8);
  return invoke_kernel_custom_code(0);
}

void drain_wb_flush_tlb(int mem_fd, int uname_offset)
{
  int drain_wb_flush_tlb_routine[] =
  {
    0xE3A00000, // mov r0, #0
    0xEE070F9A, // mcr p15, 0, r0, cr7, cr10, 4
    0xEE080F17, // mcr p15, 0, r0, cr8, cr7, 0
    0xE1A0F00E, // mov pc, lr
  };

  patch_uname(mem_fd, uname_offset, drain_wb_flush_tlb_routine, 16);
  invoke_kernel_custom_code(0);
}

int test_patch(int mem_fd, int uname_offset, unsigned char test_value)
{
  int test_patch_routine[] =
  {
    0xE3A00000 | test_value, // mov r0, #test_value
    0xE12FFF1E               // bx lr
  };
  char new_buffer[16];
  int *uname_backup_int = (int *)new_buffer;

  patch_uname(mem_fd, uname_offset, test_patch_routine, 8);
  backup_uname(mem_fd, uname_offset, new_buffer);

  printf("uname now: %x %x %x %x\n",
   uname_backup_int[0], uname_backup_int[1],
   uname_backup_int[2], uname_backup_int[3]);

  return invoke_kernel_custom_code(0);
}

// NOTE: You must pass this a handle to /dev/mem - I did it this way because
// you're liable to already have an open one sitting around. If not do it
// like this:
// int mem_fd = open("/dev/mem");
// wiz_mmuhack(mem_fd);
// close(mem_fd);

int wiz_mmuhack(int mem_fd)
{
  FILE *kallsyms_file;
  int uname_offset;
  unsigned int pagetable_offset;
  char kallsyms_line[128];
  char uname_backup[16];
  int *uname_backup_int = (int *)uname_backup;
  int test_value;
  int failure = 0;

  kallsyms_file = fopen("/proc/kallsyms", "rb");
  if(kallsyms_file == NULL)
  {
    printf("Could not find /proc/kallsyms, trying /proc/ksyms.\n");
    kallsyms_file = fopen("/proc/ksyms", "rb");
    if(kallsyms_file == NULL)
    {
      printf("Could not find /proc/ksyms, exiting.\n");
      return -1;
    }
  }

  while(1)
  {
    if(fgets(kallsyms_line, 128, kallsyms_file))
    {
      if(!strncmp(kallsyms_line + 11, "sys_newuname", 12))
      {
        uname_offset = strtoul(kallsyms_line, NULL, 16);
        break;
      }
    }
    else
    {
      fclose(kallsyms_file);
      printf("Could not find sys_newuname offset.\n");
      return -1;
    }
  }

  fclose(kallsyms_file);

  uname_offset -= 0xC0000000;

  printf("got uname location %x\n", uname_offset);

  backup_uname(mem_fd, uname_offset, uname_backup);

  printf("uname backup: %x %x %x %x\n",
   uname_backup_int[0], uname_backup_int[1],
   uname_backup_int[2], uname_backup_int[3]);

  test_value = test_patch(mem_fd, uname_offset, 0xA3);
  printf("test 1: expected 0xA3, got %x\n", test_value);
  test_value = test_patch(mem_fd, uname_offset, 0xE9);
  printf("test 2: expected 0xE9, got %x\n", test_value);

  if(test_value == 0xE9)
  {
    pagetable_offset = get_pagetable_offset(mem_fd, uname_offset);

    printf("modifying pagetable at %x\n", pagetable_offset);

    if((pagetable_offset >= (1024 * 1024 * 64)) || (pagetable_offset == 0xE9))
    {
      printf("Error: pagetable offset out of range.\n");
      failure = -1;
    }
    else
    {
      hack_pagetable(mem_fd, pagetable_offset);
      drain_wb_flush_tlb(mem_fd, uname_offset);
    }
  }
  else
  {
    failure = -1;
  }
  restore_uname(mem_fd, uname_offset, uname_backup);

  return failure;
}

