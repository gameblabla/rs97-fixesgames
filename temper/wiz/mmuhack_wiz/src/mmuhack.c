/*
 * MMU hack module. Real author unknown (NK?), modified for Wiz by notaz.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/miscdevice.h>
#include <linux/delay.h>
#include <linux/fs.h>
#include <linux/mm.h>

#define MMUHACK_MINOR 225
#define DEVICE_NAME "mmuhack"
#define PFX "mmuhack: "

/* assume RAM starts at phys addr 0 (this is really machine specific) */
#define RAM_PHYS_START	0
#define RAM_MAX_SIZE	0x10000000	/* 256M, try to be future proof */

static u32 hack_addr_start;
static u32 hack_addr_end;

static ssize_t mmuhack_open(struct inode *inode, struct file *filp)
{
	u32 *pgtable, *cpt;
	int i, j, count = 0;
	int ttb;

	// get the pointer to the translation table base...
	asm volatile (
		"mrc p15, 0, r0, c2, c0, 0\n\t"
		"mov %0, r0\n\t" : "=r"(ttb) :: "r0"
	);

	pgtable = __va((ttb) & 0xffffc000);

	for (i = 0; i < 4096; i ++) if ( pgtable[i] & 1 ) /* course of fine page table */
	{
		cpt = __va(pgtable[i] & 0xfffffc00);

		for (j = 0; j < 256; j ++) {
			u32 addr = cpt[j] & 0xfffff000;
			if (hack_addr_start <= addr && addr < hack_addr_end)
			{
				pr_debug(PFX "Set C&B bits %08x\n", cpt[j]);
				cpt[j] |= 0x0c;
				count++;
			}
		}
	}

	// drain the write buffer and flush the tlb caches...
	asm volatile (
		"mov    r0, #0\n\t"
		"mcr    15, 0, r0, cr7, cr10, 4\n\t"
		"mcr    15, 0, r0, cr8, cr7, 0\n\t"
		::: "r0"
	);

	pr_info(PFX "MMU hack applied (%d entries)\n", count);

	return 0;
}

static struct file_operations mmuhack_fops = {
	owner:	THIS_MODULE,
	open:	mmuhack_open,
};

static struct miscdevice mmuhack = {
	MMUHACK_MINOR, DEVICE_NAME, &mmuhack_fops
};

static int __init mmuhack_init(void)
{
	struct sysinfo info;
	int ret;

	ret = misc_register(&mmuhack);
	if (ret) {
		printk(KERN_ERR PFX "misc_register failed: %d\n", ret);
		return ret;
	}

	si_meminfo(&info);

	hack_addr_start = RAM_PHYS_START + (info.totalram << PAGE_SHIFT);
	hack_addr_end = RAM_PHYS_START + RAM_MAX_SIZE;

	if (hack_addr_end <= hack_addr_start) {
		misc_deregister(&mmuhack);
		printk(KERN_ERR PFX "failed to determine hackable location\n");
		return -1;
	}

	/* give time for /dev node to appear */
	mdelay(200);

	pr_info(PFX "loaded, will use %08x-%08x (%luMB RAM used by Linux)\n",
		hack_addr_start, hack_addr_end - 1, (info.totalram << PAGE_SHIFT) / 1024);

	return 0;
}

static void __exit mmuhack_exit(void)
{
	misc_deregister(&mmuhack);

	pr_info(PFX "unloaded.\n");
}

module_init(mmuhack_init);
module_exit(mmuhack_exit);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("MMU hack");
