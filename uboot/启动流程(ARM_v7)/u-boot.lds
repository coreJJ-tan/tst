/*
 * Copyright (c) 2004-2008 Texas Instruments
 *
 * (C) Copyright 2002
 * Gary Jennejohn, DENX Software Engineering, <garyj@denx.de>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <config.h>

OUTPUT_FORMAT("elf32-littlearm", "elf32-littlearm", "elf32-littlearm")  /*指定输出可执行文件是elf格式，32位ARM指令，小端*/
OUTPUT_ARCH(arm)    /* cpu架构是arm */
ENTRY(_start)       /* uboot 入口地址是_start*/
SECTIONS
{
#if defined(CONFIG_ARMV7_SECURE_BASE) && defined(CONFIG_ARMV7_NONSEC)
	/*
	 * If CONFIG_ARMV7_SECURE_BASE is true, secure code will not
	 * bundle with u-boot, and code offsets are fixed. Secure zone
	 * only needs to be copied from the loading address to
	 * CONFIG_ARMV7_SECURE_BASE, which is the linking and running
	 * address for secure code.
	 *
	 * If CONFIG_ARMV7_SECURE_BASE is undefined, the secure zone will
	 * be included in u-boot address space, and some absolute address
	 * were used in secure code. The absolute addresses of the secure
	 * code also needs to be relocated along with the accompanying u-boot
	 * code.
	 *
	 * So DISCARD is only for CONFIG_ARMV7_SECURE_BASE.
	 */
	/DISCARD/ : { *(.rel._secure*) }
#endif
    /*指定可执行文件的全局入口点，通常这个地址都放在ROM(flash)0x0位置。必须使编译器知道这个地址，通常都是修改此处来完成*/
	. = 0x00000000;         /* 当前的链接地址为0x0，从该地址开始，依次存放下面的各个段 */

	. = ALIGN(4);           /* 代码以4字节对齐 */
	.text :                 /* 代码段 */
	{
		*(.__image_copy_start)  /* 起始地址为 __image_copy_start， u-boot将自己copy到RAM，此为需要copy的程序的start */
		*(.vectors)             /* 存放vector.S的代码 */
		CPUDIR/start.o (.text*) /* 存放start.S的代码 */
		*(.text*)           /*其他的代码段放在这里，即vector.S，start.S之后*/
	}

#ifdef CONFIG_ARMV7_NONSEC

#ifndef CONFIG_ARMV7_SECURE_BASE
#define CONFIG_ARMV7_SECURE_BASE
#endif

	.__secure_start : {
		. = ALIGN(0x1000);
		*(.__secure_start)
	}

	.secure_text CONFIG_ARMV7_SECURE_BASE :
		AT(ADDR(.__secure_start) + SIZEOF(.__secure_start))
	{
		*(._secure.text)
	}

	. = LOADADDR(.__secure_start) +
		SIZEOF(.__secure_start) +
		SIZEOF(.secure_text);

	__secure_end_lma = .;
	.__secure_end : AT(__secure_end_lma) {
		*(.__secure_end)
		LONG(0x1d1071c);	/* Must output something to reset LMA */
	}
#endif

	. = ALIGN(4);   /*代码段结束后，有可能4bytes不对齐了，此时做好4bytes对齐，以开始后面的.rodata段*/
	.rodata : { *(SORT_BY_ALIGNMENT(SORT_BY_NAME(.rodata*))) }  /*在代码段之后，存放只读数据段*/

	. = ALIGN(4);   /*和前面一样，4bytes对齐，以开始接下来的.data段*/
	.data : {       /*可读写数据段*/
		*(.data*)
	}

	. = ALIGN(4);   /*和前面一样，4bytes对齐*/

	. = .;

	. = ALIGN(4);
	.u_boot_list : {    /*.data段结束后，紧接着存放u-boot自有的一些function，例如u-boot command等*/
		KEEP(*(SORT(.u_boot_list*)));
	}

	. = ALIGN(4);

	.image_copy_end :
	{
        /*至此，u-boot需要自拷贝的内容结束，总结一下，包括代码段，数据段，以及u_boot_list， 整个uboot镜像大小为 __image_copy_end - __image_copy_start*/
		*(.__image_copy_end) 
	}
    /*在老的uboot中，如果我们想要uboot启动后把自己拷贝到内存中的某个地方，只要把要拷贝的地址写给 TEXT_BASE 即可，然后boot启动后就会把自己拷贝到TEXT_BASE内的地址处运行，
    在拷贝之前的代码都是相对的，不能出现绝对的跳转，否则会跑飞。在新版的uboot里（2013.07），TEXT_BASE的含义改变了。它表示用户要把这段代码加载到哪里，通常是通过串口等工具。
    然后搬移的时候由uboot自己计算一个地址来进行搬移。新版的uboot采用了动态链接技术，在lds文件中有__rel_dyn_start和__rel_dyn_end，这两个符号之间的区域存放着动态链接符号，
    只要给这里面的符号加上一定的偏移，拷贝到内存中代码的后面相应的位置处，就可以在绝对跳转中找到正确的函数。*/
	.rel_dyn_start :
	{
		*(.__rel_dyn_start)
	}

	.rel.dyn : {
		*(.rel*)    /*动态链接符存放在的段*/
	}

	.rel_dyn_end :
	{
		*(.__rel_dyn_end)   /*动态链接符段结束*/
	}

	.end :
	{
		*(.__end)
	}

	_image_binary_end = .;  /*bin文件结束*/

	/*
	 * Deprecated: this MMU section is used by pxa at present but
	 * should not be used by new boards/CPUs.
	 */
	. = ALIGN(4096);
	.mmutable : {
		*(.mmutable)
	}

/*
 * Compiler-generated __bss_start and __bss_end, see arch/arm/lib/bss.c
 * __bss_base and __bss_limit are for linker only (overlay ordering)
 */

	.bss_start __rel_dyn_start (OVERLAY) : {    /*bss段的描述*/
		KEEP(*(.__bss_start));
		__bss_base = .;
	}

	.bss __bss_base (OVERLAY) : {
		*(.bss*)
		 . = ALIGN(4);
		 __bss_limit = .;
	}

	.bss_end __bss_limit (OVERLAY) : {
		KEEP(*(.__bss_end));    /*bss段的描述结束*/
	}

	.dynsym _image_binary_end : { *(.dynsym) }
	.dynbss : { *(.dynbss) }
	.dynstr : { *(.dynstr*) }
	.dynamic : { *(.dynamic*) }
	.plt : { *(.plt*) }
	.interp : { *(.interp*) }
	.gnu.hash : { *(.gnu.hash) }
	.gnu : { *(.gnu*) }
	.ARM.exidx : { *(.ARM.exidx*) }
	.gnu.linkonce.armexidx : { *(.gnu.linkonce.armexidx.*) }
}
