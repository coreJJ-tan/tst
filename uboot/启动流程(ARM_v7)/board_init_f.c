board_init_f函数在common/board_f.c中定义
void board_init_f(ulong boot_flags)
    #ifdef CONFIG_SYS_GENERIC_GLOBAL_DATA
        /* For some archtectures, global data is initialized and used before calling this function. The data should be preserved. For others,
        * CONFIG_SYS_GENERIC_GLOBAL_DATA should be defined and use the stack here to host global data until relocation.*/
        gd_t data;
        gd = &data;
        /* Clear global data before it is accessed at debug print in initcall_run_list. Otherwise the debug print probably get the wrong vaule of gd->have_console. */
        zero_global_data();
    #endif
    gd->flags = boot_flags;
    gd->have_console = 0;
    if (initcall_run_list(init_sequence_f))
        hang();
    #if !defined(CONFIG_ARM) && !defined(CONFIG_SANDBOX) && !defined(CONFIG_EFI_APP)
        hang(); /* NOTREACHED - jump_to_copy() does not return */
    #endif

    init_sequence_f是一个数组，成员是各个初始化，initcall_run_list会依次从该数组中取出各个函数，并运行，若有一个函数返回值非0，那么将直接报错退出，并打印如下字符串：
    "### ERROR ### Please RESET the board ###"

static init_fnc_t init_sequence_f[] = {
    #ifdef CONFIG_SANDBOX    sandbox 架构的CPU会执行
        setup_ram_buf,
    #endif
	setup_mon_len,   根据链接脚本中的符号，计算整个uboot的大小，赋值给gd->mon_len
    #ifdef CONFIG_OF_CONTROL     如果uboot下有设备树，则需要配置该项。注意：这个设备树是准备给内核使用的，如果使用设备树，那么就会存在设备树编译后放在哪里的问题，参见《小提示.c》中的介绍
        fdtdec_setup,    根据链接脚本，获取设备树的基地址，并赋值给gd->fdt_blob
    #endif
    #ifdef CONFIG_TRACE
        trace_early_init,
    #endif
	initf_malloc,	 配置 CONFIG_SYS_MALLOC_F_LEN 的情况下，初始化动态内存分配的区域，其实就是gd->malloc_base （crt0.S文件中初始化），gd->malloc_limit  ，gd->malloc_ptr 三者的赋值
	initf_console_record,
    #if defined(CONFIG_MPC85xx) || defined(CONFIG_MPC86xx)
        /* TODO: can this go into arch_cpu_init()? */
        probecpu,
    #endif
    #if defined(CONFIG_X86) && defined(CONFIG_HAVE_FSP)
        x86_fsp_init,
    #endif
	arch_cpu_init,		/* basic arch cpu dependent setup */     不同架构的CPU初始化，CPU厂家实现该函数
	initf_dm,    执行bind操作，初始化一个dm模型的树形结构，关于DM模型，参见其他章节
	arch_cpu_init_dm,
	mark_bootstage,		/* need timer, go after init dm */    记录当前 bootstage 为board_init_f
    #if defined(CONFIG_BOARD_EARLY_INIT_F)
        board_early_init_f,      单板早期的初始化，根据需要实现
    #endif
	/* TODO: can any of this go into arch_cpu_init()? */
    #if defined(CONFIG_PPC) && !defined(CONFIG_8xx_CPUCLK_DEFAULT)
        get_clocks,		/* get CPU and bus clocks (etc.) */  该函数由相应架构的CPU实现，并赋值给gd->arch.sdhc_clk
        #if defined(CONFIG_TQM8xxL) && !defined(CONFIG_TQM866M) && !defined(CONFIG_TQM885D)
            adjust_sdram_tbs_8xx,
        #endif
        /* TODO: can we rename this to timer_init()? */
        init_timebase,   只见到powerpc CPU实现了
    #endif
    #if defined(CONFIG_ARM) || defined(CONFIG_MIPS) ||  defined(CONFIG_BLACKFIN) || defined(CONFIG_NDS32) ||  defined(CONFIG_SPARC)
        timer_init,		/* initialize timer */  初始化定时器，内核时钟，给uboot提供时间，相应架构的CPU实现
    #endif
    #ifdef CONFIG_SYS_ALLOC_DPRAM
        #if !defined(CONFIG_CPM2)
            dpram_init,
        #endif
    #endif
    #if defined(CONFIG_BOARD_POSTCLK_INIT)
        board_postclk_init,  函数设置板子电压
    #endif
    #if defined(CONFIG_SYS_FSL_CLK) || defined(CONFIG_M68K)
        get_clocks,  函数获取时钟值，sd卡外设的时钟
    #endif
	env_init,		/* initialize environment */  初始化环境变量，gd->env_addr = (ulong)&default_environment[0];
    #if defined(CONFIG_8xx_CPUCLK_DEFAULT)
        /* get CPU and bus clocks according to the environment variable */
        get_clocks_866,
        /* adjust sdram refresh rate according to the new clock */
        sdram_adjust_866,
        init_timebase,
    #endif
	init_baud_rate,		/* initialze baudrate settings */   从环境变量设置串口波特率，gd->baudrate = getenv_ulong("baudrate", 10, CONFIG_BAUDRATE);
	serial_init,		/* serial communications setup */   初始化串口
	console_init_f,		/* stage 1 init of console */	设置gd->have_consloe为1，表示有个控制台，此函数将暂存在缓冲区的数据打印到控制台
    #ifdef CONFIG_SANDBOX
        sandbox_early_getopt_check,
    #endif
    #ifdef CONFIG_OF_CONTROL
        fdtdec_prepare_fdt,     准备设备树
    #endif
	display_options,	/* say that we are here */  打印uboot版本信息
	display_text_info,	/* show debugging info if required */   打印uboot代码的起始地址和结束地址，以及bss段的结束地址。提示：bss段紧挨着uboot代码段之后
    #if defined(CONFIG_MPC8260)
        prt_8260_rsr,
        prt_8260_clks,
    #endif /* CONFIG_MPC8260 */
    #if defined(CONFIG_MPC83xx)
        prt_83xx_rsr,
    #endif
    #if defined(CONFIG_PPC) || defined(CONFIG_M68K)
        checkcpu,
    #endif
	print_cpuinfo,		/* display cpu info (and speed) */  打印CPU信息
    #if defined(CONFIG_MPC5xxx)
        prt_mpc5xxx_clks,
    #endif /* CONFIG_MPC5xxx */
    #if defined(CONFIG_DISPLAY_BOARDINFO)
        show_board_info,
    #endif
	INIT_FUNC_WATCHDOG_INIT
    #if defined(CONFIG_MISC_INIT_F)
        misc_init_f,
    #endif
        INIT_FUNC_WATCHDOG_RESET
    #if defined(CONFIG_HARD_I2C) || defined(CONFIG_SYS_I2C)
        init_func_i2c,      初始化i2c，详见i2c驱动部分
    #endif
    #if defined(CONFIG_HARD_SPI)
        init_func_spi,      初始化spi，详见spi驱动部分
    #endif
	announce_dram_init,
	/* TODO: unify all these dram functions? */
    #if defined(CONFIG_ARM) || defined(CONFIG_X86) || defined(CONFIG_NDS32) || defined(CONFIG_MICROBLAZE) || defined(CONFIG_AVR32)
        dram_init,		/* configure available RAM banks */ // 初始化DDR，这个函数由CPU厂家实现，注意：内部调用的函数可能需要驱动工程师参与
    #endif
    #if defined(CONFIG_MIPS) || defined(CONFIG_PPC) || defined(CONFIG_M68K)
        init_func_ram,
    #endif
    #ifdef CONFIG_POST
        post_init_f,
    #endif
	INIT_FUNC_WATCHDOG_RESET
    #if defined(CONFIG_SYS_DRAM_TEST)
        testdram,       // SDRAM测试
    #endif /* CONFIG_SYS_DRAM_TEST */
	INIT_FUNC_WATCHDOG_RESET

    #ifdef CONFIG_POST
        init_post,
    #endif
	INIT_FUNC_WATCHDOG_RESET
	/*
	 * Now that we have DRAM mapped and working, we can relocate the code and continue running from DRAM.
	 *
	 * Reserve memory at end of RAM for (top down in that order):
	 *  - area that won't get touched by U-Boot and Linux (optional)
	 *  - kernel log buffer
	 *  - protected RAM
	 *  - LCD framebuffer
	 *  - monitor code
	 *  - board info struct
	 */
    // 下面这些是进行重定位uboot镜像之前划分内存的，在RAM末尾保留内存，用于重定位uboot及gd全局变量等，uboot和kernel无法写这部分内存
	setup_dest_addr,    // 进行内存划分前，先获取RAM的一些参数
    #if defined(CONFIG_BLACKFIN)
        /* Blackfin u-boot monitor should be on top of the ram */
        reserve_uboot,      留出重定位后的uboot所占的内存区域，uboot所占字节大小有ge->mon_len所指定
    #endif
    #if defined(CONFIG_SPARC)
        reserve_prom,       划分prom的内存
    #endif
    #if defined(CONFIG_LOGBUFFER) && !defined(CONFIG_ALT_LB_ADDR)
        reserve_logbuffer,  划分划分缓冲区的内存
    #endif
    #ifdef CONFIG_PRAM
        reserve_pram,       划分pram的内存
    #endif
	reserve_round_4k,       保留至少4kB大小的内存
    #if !(defined(CONFIG_SYS_ICACHE_OFF) && defined(CONFIG_SYS_DCACHE_OFF)) && defined(CONFIG_ARM)
        reserve_mmu,        留出MMU的TLB表的位置，分配MMU的TLB表内存以后会对gd->relocaddr做64KB字节对齐
    #endif
    #ifdef CONFIG_DM_VIDEO
        reserve_video,          划分video的内存
    #else
        # ifdef CONFIG_LCD
            reserve_lcd,            划分lcd的内存
        # endif
        /* TODO: Why the dependency on CONFIG_8xx? */
        # if defined(CONFIG_VIDEO) && (!defined(CONFIG_PPC) || defined(CONFIG_8xx)) && !defined(CONFIG_ARM) && !defined(CONFIG_X86) && !defined(CONFIG_BLACKFIN) && !defined(CONFIG_M68K)
            reserve_legacy_video,
        # endif
    #endif /* CONFIG_DM_VIDEO */
	reserve_trace,
#if !defined(CONFIG_BLACKFIN)
	reserve_uboot,          
#endif
#ifndef CONFIG_SPL_BUILD
	reserve_malloc,         留出malloc区域
	reserve_board,          留出板子bd所占的内存区，bd是结构体bd_t，大小为80字节
#endif
	setup_machine,          老uboot留出board id的位置，以匹配linux
	reserve_global_data,    保留gd_t的内存区域，gd_t结构体的大小为248B
	reserve_fdt,            保留设备树的内存
	reserve_arch,
	reserve_stacks,         留出栈空间，先对gd->start_addr_sp减去16，然后做16字节对齐，如果使能IRQ的话还要留出IRQ相应的内存
	setup_dram_config,      dram就是内存条，设置dram信息，就是设置gd->bd->bi_dram[0].start和gd->bd->bi_dram[0].size后面传递个linux内核，告诉linux dram的起始地址和大小
	show_dram_config,       打印dram的一些信息
#if defined(CONFIG_PPC) || defined(CONFIG_M68K) || defined(CONFIG_MIPS)
	setup_board_part1,
#endif
#if defined(CONFIG_PPC) || defined(CONFIG_M68K)
	INIT_FUNC_WATCHDOG_RESET
	setup_board_part2,
#endif
	display_new_sp,         显示新的sp位置，也就是gd->start_addr_sp
#ifdef CONFIG_SYS_EXTBDINFO
	setup_board_extra,
#endif
	INIT_FUNC_WATCHDOG_RESET
	reloc_fdt,              重定位设备树，如果设备树掺杂在uboot里面，这个时候需要重定位设备树，相应的，如果设备树在uboot镜像之后，则不用处理
	setup_reloc,            设置gd的其他成员变量，供后面定位使用，并且将一起的gd拷贝到gd->new_gd处
#if defined(CONFIG_X86) || defined(CONFIG_ARC)
	copy_uboot_to_ram,
	clear_bss,
	do_elf_reloc_fixups,
#endif
#if !defined(CONFIG_ARM) && !defined(CONFIG_SANDBOX)
	jump_to_copy,
#endif
	NULL,
};


如下是驱动开发者需要注意并可能会实现的函数：
board_early_init_f   #if defined(CONFIG_BOARD_EARLY_INIT_F)
dram_init
setup_dram_config 下的 dram_init_banksize 函数

    board_init_f函数及之前，代码还在片外的介质中，如nor flash中，进行uboot重定位之后才在RAM中取代码运行，同时，在board_init_f函数及之前，bss段不可用，因此无法使用全局变量，
gd的地址主要是存在CPU的R9寄存器中。所以说nor flash经常用与保存uboot文件，而不用nand flash，正因为这个阶段可以在nor flash中取指令运行