board_init_f初始化了一些外设和gd的成员变量，但并没有初始化所有的外设，这时候就需要board_init_r函数来实现后续的工作。

board_init_r函数在common/board_r.c中定义
void board_init_r(gd_t *new_gd, ulong dest_addr)
    #ifdef CONFIG_NEEDS_MANUAL_RELOC
        int i;
    #endif
    #ifdef CONFIG_AVR32
        mmu_init_r(dest_addr);
    #endif
    #if !defined(CONFIG_X86) && !defined(CONFIG_ARM) && !defined(CONFIG_ARM64)
        gd = new_gd;
    #endif
    #ifdef CONFIG_NEEDS_MANUAL_RELOC
        for (i = 0; i < ARRAY_SIZE(init_sequence_r); i++)
            init_sequence_r[i] += gd->reloc_off;
    #endif
	if (initcall_run_list(init_sequence_r))
		hang();
	hang();


init_fnc_t init_sequence_r[] = {
	initr_trace,
	initr_reloc,
	/* TODO: could x86/PPC have this also perhaps? */
    #ifdef CONFIG_ARM
        initr_caches,
        /* Note: For Freescale LS2 SoCs, new MMU table is created in DDR.
        *	 A temporary mapping of IFC high region is since removed,
        *	 so environmental variables in NOR flash is not availble
        *	 until board_init() is called below to remap IFC to high
        *	 region.
        */
    #endif
	initr_reloc_global_data,
    #if defined(CONFIG_SYS_INIT_RAM_LOCK) && defined(CONFIG_E500)
        initr_unlock_ram_in_cache,
    #endif
	initr_barrier,
	initr_malloc,
	initr_console_record,
    #ifdef CONFIG_SYS_NONCACHED_MEMORY
        initr_noncached,
    #endif
	bootstage_relocate,
    #ifdef CONFIG_DM
        initr_dm,
    #endif
	initr_bootstage,
    #if defined(CONFIG_ARM) || defined(CONFIG_NDS32)
        board_init,	/* Setup chipselects */
    #endif
	/*
	 * TODO: printing of the clock inforamtion of the board is now
	 * implemented as part of bdinfo command. Currently only support for
	 * davinci SOC's is added. Remove this check once all the board
	 * implement this.
	 */
    #ifdef CONFIG_CLOCKS
        set_cpu_clk_info, /* Setup clock information */
    #endif
	stdio_init_tables,
	initr_serial,
	initr_announce,
	INIT_FUNC_WATCHDOG_RESET
    #ifdef CONFIG_NEEDS_MANUAL_RELOC
        initr_manual_reloc_cmdtable,
    #endif
    #if defined(CONFIG_PPC) || defined(CONFIG_M68K)
        initr_trap,
    #endif
    #ifdef CONFIG_ADDR_MAP
        initr_addr_map,
    #endif
    #if defined(CONFIG_BOARD_EARLY_INIT_R)
        board_early_init_r,
    #endif
	INIT_FUNC_WATCHDOG_RESET
    #ifdef CONFIG_LOGBUFFER
        initr_logbuffer,
    #endif
    #ifdef CONFIG_POST
        initr_post_backlog,
    #endif
	INIT_FUNC_WATCHDOG_RESET
    #ifdef CONFIG_SYS_DELAYED_ICACHE
        initr_icache_enable,
    #endif
    #if defined(CONFIG_PCI) && defined(CONFIG_SYS_EARLY_PCI_INIT)
        /*
        * Do early PCI configuration _before_ the flash gets initialised,
        * because PCU ressources are crucial for flash access on some boards.
        */
        initr_pci,
    #endif
    #ifdef CONFIG_WINBOND_83C553
        initr_w83c553f,
    #endif
    #ifdef CONFIG_ARCH_EARLY_INIT_R
        arch_early_init_r,
    #endif
	power_init_board,
    #ifndef CONFIG_SYS_NO_FLASH
        initr_flash,
    #endif
	INIT_FUNC_WATCHDOG_RESET
    #if defined(CONFIG_PPC) || defined(CONFIG_M68K) || defined(CONFIG_X86) || defined(CONFIG_SPARC)
        /* initialize higher level parts of CPU like time base and timers */
        cpu_init_r,
    #endif
    #ifdef CONFIG_PPC
        initr_spi,
    #endif
    #ifdef CONFIG_CMD_NAND
        initr_nand,
    #endif
    #ifdef CONFIG_CMD_ONENAND
        initr_onenand,
    #endif
    #ifdef CONFIG_GENERIC_MMC
        initr_mmc,
    #endif
    #ifdef CONFIG_HAS_DATAFLASH
        initr_dataflash,
    #endif
	initr_env,
    #ifdef CONFIG_SYS_BOOTPARAMS_LEN
        initr_malloc_bootparams,
    #endif
	INIT_FUNC_WATCHDOG_RESET
	initr_secondary_cpu,
    #if defined(CONFIG_ID_EEPROM) || defined(CONFIG_SYS_I2C_MAC_OFFSET)
        mac_read_from_eeprom,
    #endif
	INIT_FUNC_WATCHDOG_RESET
    #if defined(CONFIG_PCI) && !defined(CONFIG_SYS_EARLY_PCI_INIT)
        /*
        * Do pci configuration
        */
        initr_pci,
    #endif
	stdio_add_devices,
	initr_jumptable,
    #ifdef CONFIG_API
        initr_api,
    #endif
        console_init_r,		/* fully init console as a device */
    #ifdef CONFIG_DISPLAY_BOARDINFO_LATE
        show_board_info,
    #endif
    #ifdef CONFIG_ARCH_MISC_INIT
        arch_misc_init,		/* miscellaneous arch-dependent init */
    #endif
    #ifdef CONFIG_MISC_INIT_R
        misc_init_r,		/* miscellaneous platform-dependent init */
    #endif
        INIT_FUNC_WATCHDOG_RESET
    #ifdef CONFIG_CMD_KGDB
        initr_kgdb,
    #endif
	interrupt_init,
    #if defined(CONFIG_ARM) || defined(CONFIG_AVR32)
        initr_enable_interrupts,
    #endif
    #if defined(CONFIG_MICROBLAZE) || defined(CONFIG_AVR32) || defined(CONFIG_M68K)
        timer_init,		/* initialize timer */
    #endif
    #if defined(CONFIG_STATUS_LED)
        initr_status_led,
    #endif
	/* PPC has a udelay(20) here dating from 2002. Why? */
    #ifdef CONFIG_CMD_NET
        initr_ethaddr,
    #endif
    #ifdef CONFIG_BOARD_LATE_INIT
        board_late_init,
    #endif
    #ifdef CONFIG_FSL_FASTBOOT
        initr_fastboot_setup,
    #endif
    #if defined(CONFIG_CMD_AMBAPP)
        ambapp_init_reloc,
        #if defined(CONFIG_SYS_AMBAPP_PRINT_ON_STARTUP)
            initr_ambapp_print,
        #endif
    #endif
    #ifdef CONFIG_CMD_SCSI
        INIT_FUNC_WATCHDOG_RESET
        initr_scsi,
    #endif
    #ifdef CONFIG_CMD_DOC
        INIT_FUNC_WATCHDOG_RESET
        initr_doc,
    #endif
    #ifdef CONFIG_BITBANGMII
        initr_bbmii,
    #endif
    #ifdef CONFIG_CMD_NET
        INIT_FUNC_WATCHDOG_RESET
        initr_net,
    #endif
    #ifdef CONFIG_POST
        initr_post,
    #endif
    #if defined(CONFIG_CMD_PCMCIA) && !defined(CONFIG_CMD_IDE)
        initr_pcmcia,
    #endif
    #if defined(CONFIG_CMD_IDE)
        initr_ide,
    #endif
    #ifdef CONFIG_LAST_STAGE_INIT
        INIT_FUNC_WATCHDOG_RESET
        /*
        * Some parts can be only initialized if all others (like
        * Interrupts) are up and running (i.e. the PC-style ISA
        * keyboard).
        */
        last_stage_init,
    #endif
    #ifdef CONFIG_CMD_BEDBUG
        INIT_FUNC_WATCHDOG_RESET
        initr_bedbug,
    #endif
    #if defined(CONFIG_PRAM) || defined(CONFIG_LOGBUFFER)
        initr_mem,
    #endif
    #ifdef CONFIG_PS2KBD
        initr_kbd,
    #endif
    #if defined(CONFIG_SPARC)
        prom_init,
    #endif
    #ifdef CONFIG_FSL_FASTBOOT
        initr_check_fastboot,
    #endif
	run_main_loop,
};

如下是驱动开发者需要注意并可能会实现的函数：
board_init
board_early_init_r
initr_pci
power_init_board
initr_flash
mac_read_from_eeprom
show_board_info
initr_ethaddr
board_late_init
initr_scsi
initr_net
last_stage_init