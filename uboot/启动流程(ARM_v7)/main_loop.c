board_init_f函数在common/main.c中定义

void main_loop(void)
	const char *s;
	bootstage_mark_name(BOOTSTAGE_ID_MAIN_LOOP, "main_loop");
    #ifndef CONFIG_SYS_GENERIC_BOARD
        puts("Warning: Your board does not use generic board. Please read\n");
        puts("doc/README.generic-board and take action. Boards not\n");
        puts("upgraded by the late 2014 may break or be removed.\n");
    #endif
    #ifdef CONFIG_VERSION_VARIABLE
        setenv("ver", version_string);  /* set version variable */
    #endif /* CONFIG_VERSION_VARIABLE */
        cli_init();
        run_preboot_environment_command();
    #if defined(CONFIG_UPDATE_TFTP)
        update_tftp(0UL, NULL, NULL);
    #endif /* CONFIG_UPDATE_TFTP */
	s = bootdelay_process();
	if (cli_process_fdt(&s))
		cli_secure_boot_cmd(s);
	autoboot_command(s);
	cli_loop();
