/*
 * Copyright (c) 2004-2008 Texas Instruments
 *
 * (C) Copyright 2002
 * Gary Jennejohn, DENX Software Engineering, <garyj@denx.de>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

OUTPUT_FORMAT("elf32-littlearm", "elf32-littlearm", "elf32-littlearm")		/* 指定输出可执行文件:"elf32位小端格式" */
OUTPUT_ARCH(arm)			/* 指定输出可执行文件的目标架构:"arm" */
ENTRY(_start)				/* 整体入口函数 _start */
SECTIONS
{
	/* 
	 * 设置0的原因是,arm内核的处理器,上电后默认是从0x00000000处启动
	 * 1.以stm32为例片内的nor-flash起始地址是0x08000000,这里是我们烧写u-boot.bin的位置
	 * 2.上电后,系统会自动将该地址(0x0800:0000)映射到0x0000:0000(硬件完成)
	 */
	. = 0x00000000;		/* 起始地址 */

	. = ALIGN(4);			/* 代码以4字节对齐 .text为代码段，各个段按先后顺序依次排列，在cortex-m的内核中,首地址存放的是主堆栈的地址,其次存放中断向量表 */
	.text :
	{
		__image_copy_start = .;		/* 对__image_copy_start变量赋值 */
		*(.vectors)					/* 代码段的起始 verctor */
		CPUDIR/start.o (.text*)		/* 存放CPUDIR/start.o中的所有.text段 */
		*(.text*)					/* 存放其他.o中的所有.text段 */
	}
	/* 从上面的结构中可以发现这种格式都是:地址(.段) 这样的形式出现 */

	. = ALIGN(4);		/* .rodata段,确保是以4字节对齐处开始 */
	.rodata : { *(SORT_BY_ALIGNMENT(SORT_BY_NAME(.rodata*))) }		/* 只读数据段 */

	. = ALIGN(4);		/* data段,确保是以4字节对齐处开始 */
	.data : {
		*(.data*)		/* 读写数据段 */
	}

	/* 
	 * u_boot_list段,确保是以4字节对齐处开始 
	 * 这里存放的都是u_boot_list中的函数
	 * 例如:base/bdinfo/blkcache/cmp....
	 * 具体的可参看./u-boot.map .u_boot_list
	 * tips:要想优化编译出来的u-boot.bin大小,可以参看此文件进行对照裁剪
	 */
	. = ALIGN(4);
#ifdef CONFIG_SPL_DM		/* uboot中是否使用驱动模型 */
	.u_boot_list : {
		KEEP(*(SORT(.u_boot_list_*_driver_*)));
		KEEP(*(SORT(.u_boot_list_*_uclass_*)));
	}
#endif
	. = .;
	.u_boot_list : {
		KEEP(*(SORT(.u_boot_list*_i2c_*)));
	}

	. = ALIGN(4);			/* __image_copy_end段,确保是以4字节对齐处开始镜像拷贝的完成 */

	__image_copy_end = .;		/* 给变量__image_copy_end赋值 */

	.rel.dyn : {			/* 和relocation相关 */
		__rel_dyn_start = .;
		*(.rel*)
		__rel_dyn_end = .;
	}

	.end :
	{
		*(.__end)				/* 给变量__end赋值 */
	}

	_image_binary_end = .;		/* bin文件结束 */

	.bss __rel_dyn_start (OVERLAY) : {
		__bss_start = .;		/* 变量__bss_start赋值 */
		*(.bss*)
		 . = ALIGN(4);
		__bss_end = .;			/* 变量__bss_end赋值 */
	}
	__bss_size = __bss_end - __bss_start;
	.dynsym _image_binary_end : { *(.dynsym) }
	.dynbss : { *(.dynbss) }
	.dynstr : { *(.dynstr*) }
	.dynamic : { *(.dynamic*) }
	.hash : { *(.hash*) }
	.plt : { *(.plt*) }
	.interp : { *(.interp*) }
	.gnu : { *(.gnu*) }
	.ARM.exidx : { *(.ARM.exidx*) }
}

#if defined(CONFIG_SPL_MAX_SIZE)
ASSERT(__image_copy_end - __image_copy_start < (CONFIG_SPL_MAX_SIZE), \
	"SPL image too big");
#endif

#if defined(CONFIG_SPL_BSS_MAX_SIZE)
ASSERT(__bss_end - __bss_start < (CONFIG_SPL_BSS_MAX_SIZE), \
	"SPL image BSS too big");
#endif

#if defined(CONFIG_SPL_MAX_FOOTPRINT)
ASSERT(__bss_end - _start < (CONFIG_SPL_MAX_FOOTPRINT), \
	"SPL image plus BSS too big");
#endif
