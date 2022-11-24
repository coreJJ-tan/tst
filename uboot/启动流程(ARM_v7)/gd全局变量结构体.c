typedef struct global_data {
    bd_t *bd;
    unsigned long flags;
    unsigned int baudrate;
    unsigned long cpu_clk;  /* CPU clock in Hz!     */
    unsigned long bus_clk;
    /* We cannot bracket this with CONFIG_PCI due to mpc5xxx */
    unsigned long pci_clk;
    unsigned long mem_clk;
#if defined(CONFIG_LCD) || defined(CONFIG_VIDEO)
    unsigned long fb_base;  /* Base address of framebuffer mem */
#endif
#if defined(CONFIG_POST) || defined(CONFIG_LOGBUFFER)
    unsigned long post_log_word;  /* Record POST activities */
    unsigned long post_log_res; /* success of POST test */
    unsigned long post_init_f_time;  /* When post_init_f started */
#endif
#ifdef CONFIG_BOARD_TYPES
    unsigned long board_type;
#endif
    unsigned long have_console; /* serial_init() was called */
#ifdef CONFIG_PRE_CONSOLE_BUFFER
    unsigned long precon_buf_idx;   /* Pre-Console buffer index */
#endif
    unsigned long env_addr; /* Address  of Environment struct */    // 环境变量的基地址
    unsigned long env_valid;    /* Checksum of Environment valid? */

    unsigned long ram_top;  /* Top address of RAM used by U-Boot */

    unsigned long relocaddr;    /* Start address of U-Boot in RAM */
    phys_size_t ram_size;   /* RAM size */
#ifdef CONFIG_SYS_MEM_RESERVE_SECURE
#define MEM_RESERVE_SECURE_SECURED  0x1
#define MEM_RESERVE_SECURE_MAINTAINED   0x2
#define MEM_RESERVE_SECURE_ADDR_MASK    (~0x3)
    /*
     * Secure memory addr
     * This variable needs maintenance if the RAM base is not zero,
     * or if RAM splits into non-consecutive banks. It also has a
     * flag indicating the secure memory is marked as secure by MMU.
     * Flags used: 0x1 secured
     *             0x2 maintained
     */
    phys_addr_t secure_ram;
#endif
    unsigned long mon_len;  /* monitor len */   // uboot镜像的大小
    unsigned long irq_sp;       /* irq stack pointer */
    unsigned long start_addr_sp;    /* start_addr_stackpointer */
    unsigned long reloc_off;
    struct global_data *new_gd; /* relocated global data */

#ifdef CONFIG_DM
    struct udevice  *dm_root;   /* Root instance for Driver Model */
    struct udevice  *dm_root_f; /* Pre-relocation root instance */
    struct list_head uclass_root;   /* Head of core tree */
#endif
#ifdef CONFIG_TIMER
    struct udevice  *timer; /* Timer instance for Driver Model */
#endif

    const void *fdt_blob;   /* Our device tree, NULL if none */
    void *new_fdt;      /* Relocated FDT */
    unsigned long fdt_size; /* Space reserved for relocated FDT */
    struct jt_funcs *jt;        /* jump table */
    char env_buf[32];   /* buffer for getenv() before reloc. */
#ifdef CONFIG_TRACE
    void        *trace_buff;    /* The trace buffer */
#endif
#if defined(CONFIG_SYS_I2C)
    int     cur_i2c_bus;    /* current used i2c bus */
#endif
#ifdef CONFIG_SYS_I2C_MXC
    void *srdata[10];
#endif
    unsigned long timebase_h;
    unsigned long timebase_l;
#ifdef CONFIG_SYS_MALLOC_F_LEN
    unsigned long malloc_base;  /* base address of early malloc() */    // malloc 区域的基地址
    unsigned long malloc_limit; = CONFIG_SYS_MALLOC_F_LEN /* limit address */   // malloc区域的大小
    unsigned long malloc_ptr;   /* current address */   // malloc 当前指向的地址
#endif
#ifdef CONFIG_PCI
    struct pci_controller *hose;    /* PCI hose for early use */
    phys_addr_t pci_ram_top;    /* top of region accessible to PCI */
#endif
#ifdef CONFIG_PCI_BOOTDELAY
    int pcidelay_done;
#endif
    struct udevice *cur_serial_dev; /* current serial device */
    struct arch_global_data arch;   /* architecture-specific data */
#ifdef CONFIG_CONSOLE_RECORD
    struct membuff console_out; /* console output */
    struct membuff console_in;  /* console input */
#endif
#ifdef CONFIG_DM_VIDEO
    ulong video_top;        /* Top of video frame buffer area */
    ulong video_bottom;     /* Bottom of video frame buffer area */
#endif
} gd_t;
