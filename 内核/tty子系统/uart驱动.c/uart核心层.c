uart 核心层代码路径： drivers/tty/serial/serial_core.c

1、结构体
/* This structure describes all the operations that can be done on the physical hardware.  See Documentation/serial/driver for details.*/
struct uart_ops {
    unsigned int    (*tx_empty)(struct uart_port *);
    void        (*set_mctrl)(struct uart_port *, unsigned int mctrl);
    unsigned int    (*get_mctrl)(struct uart_port *);
    void        (*stop_tx)(struct uart_port *);
    void        (*start_tx)(struct uart_port *);
    void        (*throttle)(struct uart_port *);
    void        (*unthrottle)(struct uart_port *);
    void        (*send_xchar)(struct uart_port *, char ch);
    void        (*stop_rx)(struct uart_port *);
    void        (*enable_ms)(struct uart_port *);
    void        (*break_ctl)(struct uart_port *, int ctl);
    int     (*startup)(struct uart_port *);
    void        (*shutdown)(struct uart_port *);
    void        (*flush_buffer)(struct uart_port *);
    void        (*set_termios)(struct uart_port *, struct ktermios *new,
                       struct ktermios *old);
    void        (*set_ldisc)(struct uart_port *, struct ktermios *);
    void        (*pm)(struct uart_port *, unsigned int state,
                  unsigned int oldstate);

    /*
     * Return a string describing the type of the port
     */
    const char  *(*type)(struct uart_port *);

    /*
     * Release IO and memory resources used by the port.
     * This includes iounmap if necessary.
     */
    void        (*release_port)(struct uart_port *);

    /*
     * Request IO and memory resources used by the port.
     * This includes iomapping the port if necessary.
     */
    int     (*request_port)(struct uart_port *);
    void        (*config_port)(struct uart_port *, int);
    int     (*verify_port)(struct uart_port *, struct serial_struct *);
    int     (*ioctl)(struct uart_port *, unsigned int, unsigned long);
#ifdef CONFIG_CONSOLE_POLL
    int     (*poll_init)(struct uart_port *);
    void        (*poll_put_char)(struct uart_port *, unsigned char);
    int     (*poll_get_char)(struct uart_port *);
#endif
};

struct uart_port {
    spinlock_t      lock;           /* port lock */
    unsigned long       iobase;         /* in/out[bwl] */
    unsigned char __iomem   *membase;       /* read/write[bwl] */
    unsigned int        (*serial_in)(struct uart_port *, int);
    void            (*serial_out)(struct uart_port *, int, int);
    void            (*set_termios)(struct uart_port *,
                               struct ktermios *new,
                               struct ktermios *old);
    void            (*set_mctrl)(struct uart_port *, unsigned int);
    int         (*startup)(struct uart_port *port);
    void            (*shutdown)(struct uart_port *port);
    void            (*throttle)(struct uart_port *port);
    void            (*unthrottle)(struct uart_port *port);
    int         (*handle_irq)(struct uart_port *);
    void            (*pm)(struct uart_port *, unsigned int state,
                      unsigned int old);
    void            (*handle_break)(struct uart_port *);
    int         (*rs485_config)(struct uart_port *,
                        struct serial_rs485 *rs485);
    unsigned int        irq;            /* irq number */
    unsigned long       irqflags;       /* irq flags  */
    unsigned int        uartclk;        /* base uart clock */
    unsigned int        fifosize;       /* tx fifo size */
    unsigned char       x_char;         /* xon/xoff char */
    unsigned char       regshift;       /* reg offset shift */
    unsigned char       iotype;         /* io access style */
    unsigned char       unused1;

#define UPIO_PORT       (SERIAL_IO_PORT)    /* 8b I/O port access */
#define UPIO_HUB6       (SERIAL_IO_HUB6)    /* Hub6 ISA card */
#define UPIO_MEM        (SERIAL_IO_MEM)     /* 8b MMIO access */
#define UPIO_MEM32      (SERIAL_IO_MEM32)   /* 32b little endian */
#define UPIO_AU         (SERIAL_IO_AU)      /* Au1x00 and RT288x type IO */
#define UPIO_TSI        (SERIAL_IO_TSI)     /* Tsi108/109 type IO */
#define UPIO_MEM32BE        (SERIAL_IO_MEM32BE) /* 32b big endian */

    unsigned int        read_status_mask;   /* driver specific */
    unsigned int        ignore_status_mask; /* driver specific */
    struct uart_state   *state;         /* pointer to parent state */
    struct uart_icount  icount;         /* statistics */

    struct console      *cons;          /* struct console, if any */
#if defined(CONFIG_SERIAL_CORE_CONSOLE) || defined(SUPPORT_SYSRQ)
    unsigned long       sysrq;          /* sysrq timeout */
#endif

    /* flags must be updated while holding port mutex */
    upf_t           flags;

    /*
     * These flags must be equivalent to the flags defined in
     * include/uapi/linux/tty_flags.h which are the userspace definitions
     * assigned from the serial_struct flags in uart_set_info()
     * [for bit definitions in the UPF_CHANGE_MASK]
     *
     * Bits [0..UPF_LAST_USER] are userspace defined/visible/changeable
     * except bit 15 (UPF_NO_TXEN_TEST) which is masked off.
     * The remaining bits are serial-core specific and not modifiable by
     * userspace.
     */
#define UPF_FOURPORT        ((__force upf_t) ASYNC_FOURPORT       /* 1  */ )
#define UPF_SAK         ((__force upf_t) ASYNC_SAK            /* 2  */ )
#define UPF_SPD_HI      ((__force upf_t) ASYNC_SPD_HI         /* 4  */ )
#define UPF_SPD_VHI     ((__force upf_t) ASYNC_SPD_VHI        /* 5  */ )
#define UPF_SPD_CUST        ((__force upf_t) ASYNC_SPD_CUST   /* 0x0030 */ )
#define UPF_SPD_WARP        ((__force upf_t) ASYNC_SPD_WARP   /* 0x1010 */ )
#define UPF_SPD_MASK        ((__force upf_t) ASYNC_SPD_MASK   /* 0x1030 */ )
#define UPF_SKIP_TEST       ((__force upf_t) ASYNC_SKIP_TEST      /* 6  */ )
#define UPF_AUTO_IRQ        ((__force upf_t) ASYNC_AUTO_IRQ       /* 7  */ )
#define UPF_HARDPPS_CD      ((__force upf_t) ASYNC_HARDPPS_CD     /* 11 */ )
#define UPF_SPD_SHI     ((__force upf_t) ASYNC_SPD_SHI        /* 12 */ )
#define UPF_LOW_LATENCY     ((__force upf_t) ASYNC_LOW_LATENCY    /* 13 */ )
#define UPF_BUGGY_UART      ((__force upf_t) ASYNC_BUGGY_UART     /* 14 */ )
#define UPF_NO_TXEN_TEST    ((__force upf_t) (1 << 15))
#define UPF_MAGIC_MULTIPLIER    ((__force upf_t) ASYNC_MAGIC_MULTIPLIER /* 16 */ )

/* Port has hardware-assisted h/w flow control */
#define UPF_AUTO_CTS        ((__force upf_t) (1 << 20))
#define UPF_AUTO_RTS        ((__force upf_t) (1 << 21))
#define UPF_HARD_FLOW       ((__force upf_t) (UPF_AUTO_CTS | UPF_AUTO_RTS))
/* Port has hardware-assisted s/w flow control */
#define UPF_SOFT_FLOW       ((__force upf_t) (1 << 22))
#define UPF_CONS_FLOW       ((__force upf_t) (1 << 23))
#define UPF_SHARE_IRQ       ((__force upf_t) (1 << 24))
#define UPF_EXAR_EFR        ((__force upf_t) (1 << 25))
#define UPF_BUG_THRE        ((__force upf_t) (1 << 26))
/* The exact UART type is known and should not be probed.  */
#define UPF_FIXED_TYPE      ((__force upf_t) (1 << 27))
#define UPF_BOOT_AUTOCONF   ((__force upf_t) (1 << 28))
#define UPF_FIXED_PORT      ((__force upf_t) (1 << 29))
#define UPF_DEAD        ((__force upf_t) (1 << 30))
#define UPF_IOREMAP     ((__force upf_t) (1 << 31))

#define __UPF_CHANGE_MASK   0x17fff
#define UPF_CHANGE_MASK     ((__force upf_t) __UPF_CHANGE_MASK)
#define UPF_USR_MASK        ((__force upf_t) (UPF_SPD_MASK|UPF_LOW_LATENCY))

#if __UPF_CHANGE_MASK > ASYNC_FLAGS
#error Change mask not equivalent to userspace-visible bit defines
#endif

    /*
     * Must hold termios_rwsem, port mutex and port lock to change;
     * can hold any one lock to read.
     */
    upstat_t        status;

#define UPSTAT_CTS_ENABLE   ((__force upstat_t) (1 << 0))
#define UPSTAT_DCD_ENABLE   ((__force upstat_t) (1 << 1))
#define UPSTAT_AUTORTS      ((__force upstat_t) (1 << 2))
#define UPSTAT_AUTOCTS      ((__force upstat_t) (1 << 3))
#define UPSTAT_AUTOXOFF     ((__force upstat_t) (1 << 4))

    int         hw_stopped;     /* sw-assisted CTS flow state */
    unsigned int        mctrl;          /* current modem ctrl settings */
    unsigned int        timeout;        /* character-based timeout */
    unsigned int        type;           /* port type */
    const struct uart_ops   *ops;
    unsigned int        custom_divisor;
    unsigned int        line;           /* port index */
    unsigned int        minor;
    resource_size_t     mapbase;        /* for ioremap */
    resource_size_t     mapsize;
    struct device       *dev;           /* parent device */
    unsigned char       hub6;           /* this should be in the 8250 driver */
    unsigned char       suspended;
    unsigned char       irq_wake;
    unsigned char       unused[2];
    struct attribute_group  *attr_group;        /* port specific attributes */
    const struct attribute_group **tty_groups;  /* all attributes (serial core use only) */
    struct serial_rs485     rs485;
    void            *private_data;      /* generic platform data pointer */
};

/* This is the state information which is persistent across opens. */
struct uart_state {
    struct tty_port     port;

    enum uart_pm_state  pm_state;
    struct circ_buf     xmit;

    struct uart_port    *uart_port;
};

struct uart_driver {
    struct module       *owner;
    const char      *driver_name;
    const char      *dev_name;
    int          major;
    int          minor;
    int          nr;
    struct console      *cons;

    /* these are private; the low level driver should not touch these; they should be initialised to NULL */
    struct uart_state   *state;
    struct tty_driver   *tty_driver;
};


2、API
/* This routine is used by the interrupt handler to schedule processing in the software interrupt portion of the driver. */
void uart_write_wakeup(struct uart_port *port)
{
    struct uart_state *state = port->state;
    /* This means you called this function _after_ the port was closed.  No cookie for you. */
    BUG_ON(!state);
    tty_wakeup(state->port.tty);
}

/**
 *  uart_update_timeout - update per-port FIFO timeout.
 *  @port:  uart_port structure describing the port
 *  @cflag: termios cflag value
 *  @baud:  speed of the port
 *
 *  Set the port FIFO timeout value.  The @cflag value should reflect the actual hardware settings.
 */
void uart_update_timeout(struct uart_port *port, unsigned int cflag, unsigned int baud)
{
    unsigned int bits;
    /* byte size and parity */
    switch(cflag & CSIZE) { // c_cflag bit   // #define CSIZE   0000060
    case CS5:
        bits = 7;
        break;
    case CS6:
        bits = 8;
        break;
    case CS7:
        bits = 9;
        break;
    default:
        bits = 10;
        break; /* CS8 */
    }
    if(cflag & CSTOPB) // c_cflag bit   // #define CSTOPB  0000100
        bits++;
    if(cflag & PARENB) // c_cflag bit   // #define PARENB  0000400
        bits++;
    /* The total number of bits to be transmitted in the fifo. */
    bits = bits * port->fifosize;
    /* Figure the timeout to send the above number of bits. Add .02 seconds of slop */
    port->timeout = (HZ * bits) / baud + HZ/50;
}

/**
 *  uart_get_baud_rate - return baud rate for a particular port
 *  @port: uart_port structure describing the port in question.
 *  @termios: desired termios settings.
 *  @old: old termios (or NULL)
 *  @min: minimum acceptable baud rate
 *  @max: maximum acceptable baud rate
 *
 *  Decode the termios structure into a numeric baud rate,
 *  taking account of the magic 38400 baud rate (with spd_*
 *  flags), and mapping the %B0 rate to 9600 baud.
 *
 *  If the new baud rate is invalid, try the old termios setting.
 *  If it's still invalid, we try 9600 baud.
 *
 *  Update the @termios structure to reflect the baud rate
 *  we're actually going to be using. Don't do this for the case
 *  where B0 is requested ("hang up").
 */
unsigned int
uart_get_baud_rate(struct uart_port *port, struct ktermios *termios,
           struct ktermios *old, unsigned int min, unsigned int max)
{
    unsigned int try, baud, altbaud = 38400;
    int hung_up = 0;
    upf_t flags = port->flags & UPF_SPD_MASK;

    if (flags == UPF_SPD_HI)
        altbaud = 57600;
    else if (flags == UPF_SPD_VHI)
        altbaud = 115200;
    else if (flags == UPF_SPD_SHI)
        altbaud = 230400;
    else if (flags == UPF_SPD_WARP)
        altbaud = 460800;

    for (try = 0; try < 2; try++) {
        baud = tty_termios_baud_rate(termios);

        /*
         * The spd_hi, spd_vhi, spd_shi, spd_warp kludge...
         * Die! Die! Die!
         */
        if (try == 0 && baud == 38400)
            baud = altbaud;

        /*
         * Special case: B0 rate.
         */
        if (baud == 0) {
            hung_up = 1;
            baud = 9600;
        }

        if (baud >= min && baud <= max)
            return baud;

        /*
         * Oops, the quotient was zero.  Try again with
         * the old baud rate if possible.
         */
        termios->c_cflag &= ~CBAUD;
        if (old) {
            baud = tty_termios_baud_rate(old);
            if (!hung_up)
                tty_termios_encode_baud_rate(termios,
                                baud, baud);
            old = NULL;
            continue;
        }

        /*
         * As a last resort, if the range cannot be met then clip to
         * the nearest chip supported rate.
         */
        if (!hung_up) {
            if (baud <= min)
                tty_termios_encode_baud_rate(termios,
                            min + 1, min + 1);
            else
                tty_termios_encode_baud_rate(termios,
                            max - 1, max - 1);
        }
    }
    /* Should never happen */
    WARN_ON(1);
    return 0;
}

/**
 *  uart_get_divisor - return uart clock divisor
 *  @port: uart_port structure describing the port.
 *  @baud: desired baud rate
 *
 *  Calculate the uart clock divisor for the port.
 */
unsigned int
uart_get_divisor(struct uart_port *port, unsigned int baud)
{
    unsigned int quot;

    /*
     * Old custom speed handling.
     */
    if (baud == 38400 && (port->flags & UPF_SPD_MASK) == UPF_SPD_CUST)
        quot = port->custom_divisor;
    else
        quot = DIV_ROUND_CLOSEST(port->uartclk, 16 * baud);

    return quot;
}

static const struct file_operations uart_proc_fops = {
    .owner      = THIS_MODULE,
    .open       = uart_proc_open,
    .read       = seq_read,
    .llseek     = seq_lseek,
    .release    = single_release,
};

#if defined(CONFIG_SERIAL_CORE_CONSOLE) || defined(CONFIG_CONSOLE_POLL)
/**
 *  uart_console_write - write a console message to a serial port
 *  @port: the port to write the message
 *  @s: array of characters
 *  @count: number of characters in string to write
 *  @putchar: function to write character to port
 */
void uart_console_write(struct uart_port *port, const char *s,
            unsigned int count,
            void (*putchar)(struct uart_port *, int))
{
    unsigned int i;

    for (i = 0; i < count; i++, s++) {
        if (*s == '\n')
            putchar(port, '\r');
        putchar(port, *s);
    }
}
EXPORT_SYMBOL_GPL(uart_console_write);

/*
 *  Check whether an invalid uart number has been specified, and
 *  if so, search for the first available port that does have
 *  console support.
 */
struct uart_port * __init
uart_get_console(struct uart_port *ports, int nr, struct console *co)
{
    int idx = co->index;

    if (idx < 0 || idx >= nr || (ports[idx].iobase == 0 &&
                     ports[idx].membase == NULL))
        for (idx = 0; idx < nr; idx++)
            if (ports[idx].iobase != 0 ||
                ports[idx].membase != NULL)
                break;

    co->index = idx;

    return ports + idx;
}

/**
 *  uart_parse_earlycon - Parse earlycon options
 *  @p:   ptr to 2nd field (ie., just beyond '<name>,')
 *  @iotype:  ptr for decoded iotype (out)
 *  @addr:    ptr for decoded mapbase/iobase (out)
 *  @options: ptr for <options> field; NULL if not present (out)
 *
 *  Decodes earlycon kernel command line parameters of the form
 *     earlycon=<name>,io|mmio|mmio32,<addr>,<options>
 *     console=<name>,io|mmio|mmio32,<addr>,<options>
 *
 *  The optional form
 *     earlycon=<name>,0x<addr>,<options>
 *     console=<name>,0x<addr>,<options>
 *  is also accepted; the returned @iotype will be UPIO_MEM.
 *
 *  Returns 0 on success or -EINVAL on failure
 */
int uart_parse_earlycon(char *p, unsigned char *iotype, unsigned long *addr,
            char **options)
{
    if (strncmp(p, "mmio,", 5) == 0) {
        *iotype = UPIO_MEM;
        p += 5;
    } else if (strncmp(p, "mmio32,", 7) == 0) {
        *iotype = UPIO_MEM32;
        p += 7;
    } else if (strncmp(p, "io,", 3) == 0) {
        *iotype = UPIO_PORT;
        p += 3;
    } else if (strncmp(p, "0x", 2) == 0) {
        *iotype = UPIO_MEM;
    } else {
        return -EINVAL;
    }

    *addr = simple_strtoul(p, NULL, 0);
    p = strchr(p, ',');
    if (p)
        p++;

    *options = p;
    return 0;
}

/**
 *  uart_parse_options - Parse serial port baud/parity/bits/flow control.
 *  @options: pointer to option string
 *  @baud: pointer to an 'int' variable for the baud rate.
 *  @parity: pointer to an 'int' variable for the parity.
 *  @bits: pointer to an 'int' variable for the number of data bits.
 *  @flow: pointer to an 'int' variable for the flow control character.
 *
 *  uart_parse_options decodes a string containing the serial console
 *  options.  The format of the string is <baud><parity><bits><flow>,
 *  eg: 115200n8r
 */
void
uart_parse_options(char *options, int *baud, int *parity, int *bits, int *flow)
{
    char *s = options;

    *baud = simple_strtoul(s, NULL, 10);
    while (*s >= '0' && *s <= '9')
        s++;
    if (*s)
        *parity = *s++;
    if (*s)
        *bits = *s++ - '0';
    if (*s)
        *flow = *s;
}

struct baud_rates {
    unsigned int rate;
    unsigned int cflag;
};

static const struct baud_rates baud_rates[] = {
    { 921600, B921600 },
    { 460800, B460800 },
    { 230400, B230400 },
    { 115200, B115200 },
    {  57600, B57600  },
    {  38400, B38400  },
    {  19200, B19200  },
    {   9600, B9600   },
    {   4800, B4800   },
    {   2400, B2400   },
    {   1200, B1200   },
    {      0, B38400  }
};

/**
 *  uart_set_options - setup the serial console parameters
 *  @port: pointer to the serial ports uart_port structure
 *  @co: console pointer
 *  @baud: baud rate
 *  @parity: parity character - 'n' (none), 'o' (odd), 'e' (even)
 *  @bits: number of data bits
 *  @flow: flow control character - 'r' (rts)
 */
int
uart_set_options(struct uart_port *port, struct console *co,
         int baud, int parity, int bits, int flow)
{
    struct ktermios termios;
    static struct ktermios dummy;
    int i;

    /*
     * Ensure that the serial console lock is initialised
     * early.
     * If this port is a console, then the spinlock is already
     * initialised.
     */
    if (!(uart_console(port) && (port->cons->flags & CON_ENABLED))) {
        spin_lock_init(&port->lock);
        lockdep_set_class(&port->lock, &port_lock_key);
    }

    memset(&termios, 0, sizeof(struct ktermios));

    termios.c_cflag = CREAD | HUPCL | CLOCAL;

    /*
     * Construct a cflag setting.
     */
    for (i = 0; baud_rates[i].rate; i++)
        if (baud_rates[i].rate <= baud)
            break;

    termios.c_cflag |= baud_rates[i].cflag;

    if (bits == 7)
        termios.c_cflag |= CS7;
    else
        termios.c_cflag |= CS8;

    switch (parity) {
    case 'o': case 'O':
        termios.c_cflag |= PARODD;
        /*fall through*/
    case 'e': case 'E':
        termios.c_cflag |= PARENB;
        break;
    }

    if (flow == 'r')
        termios.c_cflag |= CRTSCTS;

    /*
     * some uarts on other side don't support no flow control.
     * So we set * DTR in host uart to make them happy
     */
    port->mctrl |= TIOCM_DTR;

    port->ops->set_termios(port, &termios, &dummy);
    /*
     * Allow the setting of the UART parameters with a NULL console
     * too:
     */
    if (co)
        co->cflag = termios.c_cflag;

    return 0;
}
#endif

struct uart_match {
    struct uart_port *port;
    struct uart_driver *driver;
};

int uart_suspend_port(struct uart_driver *drv, struct uart_port *uport)
{
    struct uart_state *state = drv->state + uport->line;
    struct tty_port *port = &state->port;
    struct device *tty_dev;
    struct uart_match match = {uport, drv};

    mutex_lock(&port->mutex);

    tty_dev = device_find_child(uport->dev, &match, serial_match_port);
    if (device_may_wakeup(tty_dev)) {
        if (!enable_irq_wake(uport->irq))
            uport->irq_wake = 1;
        put_device(tty_dev);
        mutex_unlock(&port->mutex);
        return 0;
    }
    put_device(tty_dev);

    /* Nothing to do if the console is not suspending */
    if (!console_suspend_enabled && uart_console(uport))
        goto unlock;

    uport->suspended = 1;

    if (port->flags & ASYNC_INITIALIZED) {
        const struct uart_ops *ops = uport->ops;
        int tries;

        set_bit(ASYNCB_SUSPENDED, &port->flags);
        clear_bit(ASYNCB_INITIALIZED, &port->flags);

        spin_lock_irq(&uport->lock);
        ops->stop_tx(uport);
        ops->set_mctrl(uport, 0);
        ops->stop_rx(uport);
        spin_unlock_irq(&uport->lock);

        /*
         * Wait for the transmitter to empty.
         */
        for (tries = 3; !ops->tx_empty(uport) && tries; tries--)
            msleep(10);
        if (!tries)
            dev_err(uport->dev, "%s%d: Unable to drain transmitter\n",
                drv->dev_name,
                drv->tty_driver->name_base + uport->line);

        ops->shutdown(uport);
    }

    /*
     * Disable the console device before suspending.
     */
    if (uart_console(uport))
        console_stop(uport->cons);

    uart_change_pm(state, UART_PM_STATE_OFF);
unlock:
    mutex_unlock(&port->mutex);

    return 0;
}

int uart_resume_port(struct uart_driver *drv, struct uart_port *uport)
{
    struct uart_state *state = drv->state + uport->line;
    struct tty_port *port = &state->port;
    struct device *tty_dev;
    struct uart_match match = {uport, drv};
    struct ktermios termios;

    mutex_lock(&port->mutex);

    tty_dev = device_find_child(uport->dev, &match, serial_match_port);
    if (!uport->suspended && device_may_wakeup(tty_dev)) {
        if (uport->irq_wake) {
            disable_irq_wake(uport->irq);
            uport->irq_wake = 0;
        }
        put_device(tty_dev);
        mutex_unlock(&port->mutex);
        return 0;
    }
    put_device(tty_dev);
    uport->suspended = 0;

    /*
     * Re-enable the console device after suspending.
     */
    if (uart_console(uport)) {
        /*
         * First try to use the console cflag setting.
         */
        memset(&termios, 0, sizeof(struct ktermios));
        termios.c_cflag = uport->cons->cflag;

        /*
         * If that's unset, use the tty termios setting.
         */
        if (port->tty && termios.c_cflag == 0)
            termios = port->tty->termios;

        if (console_suspend_enabled)
            uart_change_pm(state, UART_PM_STATE_ON);
        uport->ops->set_termios(uport, &termios, NULL);
        if (console_suspend_enabled)
            console_start(uport->cons);
    }

    if (port->flags & ASYNC_SUSPENDED) {
        const struct uart_ops *ops = uport->ops;
        int ret;

        uart_change_pm(state, UART_PM_STATE_ON);
        spin_lock_irq(&uport->lock);
        ops->set_mctrl(uport, 0);
        spin_unlock_irq(&uport->lock);
        if (console_suspend_enabled || !uart_console(uport)) {
            /* Protected by port mutex for now */
            struct tty_struct *tty = port->tty;
            ret = ops->startup(uport);
            if (ret == 0) {
                if (tty)
                    uart_change_speed(tty, state, NULL);
                spin_lock_irq(&uport->lock);
                ops->set_mctrl(uport, uport->mctrl);
                ops->start_tx(uport);
                spin_unlock_irq(&uport->lock);
                set_bit(ASYNCB_INITIALIZED, &port->flags);
            } else {
                /*
                 * Failed to resume - maybe hardware went away?
                 * Clear the "initialized" flag so we won't try
                 * to call the low level drivers shutdown method.
                 */
                uart_shutdown(tty, state);
            }
        }

        clear_bit(ASYNCB_SUSPENDED, &port->flags);
    }

    mutex_unlock(&port->mutex);

    return 0;
}

static const struct tty_operations uart_ops = {
    .open       = uart_open,
    .close      = uart_close,
    .write      = uart_write,
    .put_char   = uart_put_char,
    .flush_chars    = uart_flush_chars,
    .write_room = uart_write_room,
    .chars_in_buffer= uart_chars_in_buffer,
    .flush_buffer   = uart_flush_buffer,
    .ioctl      = uart_ioctl,
    .throttle   = uart_throttle,
    .unthrottle = uart_unthrottle,
    .send_xchar = uart_send_xchar,
    .set_termios    = uart_set_termios,
    .set_ldisc  = uart_set_ldisc,
    .stop       = uart_stop,
    .start      = uart_start,
    .hangup     = uart_hangup,
    .break_ctl  = uart_break_ctl,
    .wait_until_sent= uart_wait_until_sent,
#ifdef CONFIG_PROC_FS
    .proc_fops  = &uart_proc_fops,
#endif
    .tiocmget   = uart_tiocmget,
    .tiocmset   = uart_tiocmset,
    .get_icount = uart_get_icount,
#ifdef CONFIG_CONSOLE_POLL
    .poll_init  = uart_poll_init,
    .poll_get_char  = uart_poll_get_char,
    .poll_put_char  = uart_poll_put_char,
#endif
};

static const struct tty_port_operations uart_port_ops = {
    .activate   = uart_port_activate,
    .shutdown   = uart_port_shutdown,
    .carrier_raised = uart_carrier_raised,
    .dtr_rts    = uart_dtr_rts,
};

/**
 *  uart_register_driver - register a driver with the uart core layer
 *  @drv: low level driver structure
 *
 *  Register a uart driver with the core driver.  We in turn register
 *  with the tty layer, and initialise the core driver per-port state.
 *
 *  We have a proc file in /proc/tty/driver which is named after the
 *  normal driver.
 *
 *  drv->port should be NULL, and the per-port structures should be
 *  registered using uart_add_one_port after this call has succeeded.
 */
int uart_register_driver(struct uart_driver *drv)
{
    struct tty_driver *normal;
    int i, retval;

    BUG_ON(drv->state);

    /*
     * Maybe we should be using a slab cache for this, especially if
     * we have a large number of ports to handle.
     */
    drv->state = kzalloc(sizeof(struct uart_state) * drv->nr, GFP_KERNEL);
    if (!drv->state)
        goto out;

    normal = alloc_tty_driver(drv->nr);
    if (!normal)
        goto out_kfree;

    drv->tty_driver = normal;

    normal->driver_name = drv->driver_name;
    normal->name        = drv->dev_name;
    normal->major       = drv->major;
    normal->minor_start = drv->minor;
    normal->type        = TTY_DRIVER_TYPE_SERIAL;
    normal->subtype     = SERIAL_TYPE_NORMAL;
    normal->init_termios    = tty_std_termios;
    normal->init_termios.c_cflag = B9600 | CS8 | CREAD | HUPCL | CLOCAL;
    normal->init_termios.c_ispeed = normal->init_termios.c_ospeed = 9600;
    normal->flags       = TTY_DRIVER_REAL_RAW | TTY_DRIVER_DYNAMIC_DEV;
    normal->driver_state    = drv;
    tty_set_operations(normal, &uart_ops);

    /*
     * Initialise the UART state(s).
     */
    for (i = 0; i < drv->nr; i++) {
        struct uart_state *state = drv->state + i;
        struct tty_port *port = &state->port;

        tty_port_init(port);
        port->ops = &uart_port_ops;
    }

    retval = tty_register_driver(normal);
    if (retval >= 0)
        return retval;

    for (i = 0; i < drv->nr; i++)
        tty_port_destroy(&drv->state[i].port);
    put_tty_driver(normal);
out_kfree:
    kfree(drv->state);
out:
    return -ENOMEM;
}

/**
 *  uart_unregister_driver - remove a driver from the uart core layer
 *  @drv: low level driver structure
 *
 *  Remove all references to a driver from the core driver.  The low
 *  level driver must have removed all its ports via the
 *  uart_remove_one_port() if it registered them with uart_add_one_port().
 *  (ie, drv->port == NULL)
 */
void uart_unregister_driver(struct uart_driver *drv)
{
    struct tty_driver *p = drv->tty_driver;
    unsigned int i;

    tty_unregister_driver(p);
    put_tty_driver(p);
    for (i = 0; i < drv->nr; i++)
        tty_port_destroy(&drv->state[i].port);
    kfree(drv->state);
    drv->state = NULL;
    drv->tty_driver = NULL;
}

struct tty_driver *uart_console_device(struct console *co, int *index)
{
    struct uart_driver *p = co->data;
    *index = co->index;
    return p->tty_driver;
}

static DEVICE_ATTR(type, S_IRUSR | S_IRGRP, uart_get_attr_type, NULL);
static DEVICE_ATTR(line, S_IRUSR | S_IRGRP, uart_get_attr_line, NULL);
static DEVICE_ATTR(port, S_IRUSR | S_IRGRP, uart_get_attr_port, NULL);
static DEVICE_ATTR(irq, S_IRUSR | S_IRGRP, uart_get_attr_irq, NULL);
static DEVICE_ATTR(flags, S_IRUSR | S_IRGRP, uart_get_attr_flags, NULL);
static DEVICE_ATTR(xmit_fifo_size, S_IRUSR | S_IRGRP, uart_get_attr_xmit_fifo_size, NULL);
static DEVICE_ATTR(uartclk, S_IRUSR | S_IRGRP, uart_get_attr_uartclk, NULL);
static DEVICE_ATTR(close_delay, S_IRUSR | S_IRGRP, uart_get_attr_close_delay, NULL);
static DEVICE_ATTR(closing_wait, S_IRUSR | S_IRGRP, uart_get_attr_closing_wait, NULL);
static DEVICE_ATTR(custom_divisor, S_IRUSR | S_IRGRP, uart_get_attr_custom_divisor, NULL);
static DEVICE_ATTR(io_type, S_IRUSR | S_IRGRP, uart_get_attr_io_type, NULL);
static DEVICE_ATTR(iomem_base, S_IRUSR | S_IRGRP, uart_get_attr_iomem_base, NULL);
static DEVICE_ATTR(iomem_reg_shift, S_IRUSR | S_IRGRP, uart_get_attr_iomem_reg_shift, NULL);

static struct attribute *tty_dev_attrs[] = {
    &dev_attr_type.attr,
    &dev_attr_line.attr,
    &dev_attr_port.attr,
    &dev_attr_irq.attr,
    &dev_attr_flags.attr,
    &dev_attr_xmit_fifo_size.attr,
    &dev_attr_uartclk.attr,
    &dev_attr_close_delay.attr,
    &dev_attr_closing_wait.attr,
    &dev_attr_custom_divisor.attr,
    &dev_attr_io_type.attr,
    &dev_attr_iomem_base.attr,
    &dev_attr_iomem_reg_shift.attr,
    NULL,
};

static const struct attribute_group tty_dev_attr_group = {
    .attrs = tty_dev_attrs,
};

/**
 *  uart_add_one_port - attach a driver-defined port structure
 *  @drv: pointer to the uart low level driver structure for this port
 *  @uport: uart port structure to use for this port.
 *
 *  This allows the driver to register its own uart_port structure
 *  with the core driver.  The main purpose is to allow the low
 *  level uart drivers to expand uart_port, rather than having yet
 *  more levels of structures.
 */
int uart_add_one_port(struct uart_driver *drv, struct uart_port *uport)
{
    struct uart_state *state;
    struct tty_port *port;
    int ret = 0;
    struct device *tty_dev;
    int num_groups;

    BUG_ON(in_interrupt());

    if (uport->line >= drv->nr)
        return -EINVAL;

    state = drv->state + uport->line;
    port = &state->port;

    mutex_lock(&port_mutex);
    mutex_lock(&port->mutex);
    if (state->uart_port) {
        ret = -EINVAL;
        goto out;
    }

    /* Link the port to the driver state table and vice versa */
    state->uart_port = uport;
    uport->state = state;

    state->pm_state = UART_PM_STATE_UNDEFINED;
    uport->cons = drv->cons;
    uport->minor = drv->tty_driver->minor_start + uport->line;

    /*
     * If this port is a console, then the spinlock is already
     * initialised.
     */
    if (!(uart_console(uport) && (uport->cons->flags & CON_ENABLED))) {
        spin_lock_init(&uport->lock);
        lockdep_set_class(&uport->lock, &port_lock_key);
    }
    if (uport->cons && uport->dev)
        of_console_check(uport->dev->of_node, uport->cons->name, uport->line);

    uart_configure_port(drv, state, uport);

    num_groups = 2;
    if (uport->attr_group)
        num_groups++;

    uport->tty_groups = kcalloc(num_groups, sizeof(*uport->tty_groups),
                    GFP_KERNEL);
    if (!uport->tty_groups) {
        ret = -ENOMEM;
        goto out;
    }
    uport->tty_groups[0] = &tty_dev_attr_group;
    if (uport->attr_group)
        uport->tty_groups[1] = uport->attr_group;

    /*
     * Register the port whether it's detected or not.  This allows
     * setserial to be used to alter this port's parameters.
     */
    tty_dev = tty_port_register_device_attr(port, drv->tty_driver,
            uport->line, uport->dev, port, uport->tty_groups);
    if (likely(!IS_ERR(tty_dev))) {
        device_set_wakeup_capable(tty_dev, 1);
    } else {
        dev_err(uport->dev, "Cannot register tty device on line %d\n",
               uport->line);
    }

    /*
     * Ensure UPF_DEAD is not set.
     */
    uport->flags &= ~UPF_DEAD;

 out:
    mutex_unlock(&port->mutex);
    mutex_unlock(&port_mutex);

    return ret;
}

/**
 *  uart_remove_one_port - detach a driver defined port structure
 *  @drv: pointer to the uart low level driver structure for this port
 *  @uport: uart port structure for this port
 *
 *  This unhooks (and hangs up) the specified port structure from the
 *  core driver.  No further calls will be made to the low-level code
 *  for this port.
 */
int uart_remove_one_port(struct uart_driver *drv, struct uart_port *uport)
{
    struct uart_state *state = drv->state + uport->line;
    struct tty_port *port = &state->port;
    struct tty_struct *tty;
    int ret = 0;

    BUG_ON(in_interrupt());

    if (state->uart_port != uport)
        dev_alert(uport->dev, "Removing wrong port: %p != %p\n",
            state->uart_port, uport);

    mutex_lock(&port_mutex);

    /*
     * Mark the port "dead" - this prevents any opens from
     * succeeding while we shut down the port.
     */
    mutex_lock(&port->mutex);
    if (!state->uart_port) {
        mutex_unlock(&port->mutex);
        ret = -EINVAL;
        goto out;
    }
    uport->flags |= UPF_DEAD;
    mutex_unlock(&port->mutex);

    /*
     * Remove the devices from the tty layer
     */
    tty_unregister_device(drv->tty_driver, uport->line);

    tty = tty_port_tty_get(port);
    if (tty) {
        tty_vhangup(port->tty);
        tty_kref_put(tty);
    }

    /*
     * If the port is used as a console, unregister it
     */
    if (uart_console(uport))
        unregister_console(uport->cons);

    /*
     * Free the port IO and memory resources, if any.
     */
    if (uport->type != PORT_UNKNOWN)
        uport->ops->release_port(uport);
    kfree(uport->tty_groups);

    /*
     * Indicate that there isn't a port here anymore.
     */
    uport->type = PORT_UNKNOWN;

    state->uart_port = NULL;
out:
    mutex_unlock(&port_mutex);

    return ret;
}

/*
 *  Are the two ports equivalent?
 */
int uart_match_port(struct uart_port *port1, struct uart_port *port2)
{
    if (port1->iotype != port2->iotype)
        return 0;

    switch (port1->iotype) {
    case UPIO_PORT:
        return (port1->iobase == port2->iobase);
    case UPIO_HUB6:
        return (port1->iobase == port2->iobase) &&
               (port1->hub6   == port2->hub6);
    case UPIO_MEM:
    case UPIO_MEM32:
    case UPIO_MEM32BE:
    case UPIO_AU:
    case UPIO_TSI:
        return (port1->mapbase == port2->mapbase);
    }
    return 0;
}

/**
 *  uart_handle_dcd_change - handle a change of carrier detect state
 *  @uport: uart_port structure for the open port
 *  @status: new carrier detect status, nonzero if active
 *
 *  Caller must hold uport->lock
 */
void uart_handle_dcd_change(struct uart_port *uport, unsigned int status)
{
    struct tty_port *port = &uport->state->port;
    struct tty_struct *tty = port->tty;
    struct tty_ldisc *ld;

    lockdep_assert_held_once(&uport->lock);

    if (tty) {
        ld = tty_ldisc_ref(tty);
        if (ld) {
            if (ld->ops->dcd_change)
                ld->ops->dcd_change(tty, status);
            tty_ldisc_deref(ld);
        }
    }

    uport->icount.dcd++;

    if (uart_dcd_enabled(uport)) {
        if (status)
            wake_up_interruptible(&port->open_wait);
        else if (tty)
            tty_hangup(tty);
    }
}

/**
 *  uart_handle_cts_change - handle a change of clear-to-send state
 *  @uport: uart_port structure for the open port
 *  @status: new clear to send status, nonzero if active
 *
 *  Caller must hold uport->lock
 */
void uart_handle_cts_change(struct uart_port *uport, unsigned int status)
{
    lockdep_assert_held_once(&uport->lock);

    uport->icount.cts++;

    if (uart_softcts_mode(uport)) {
        if (uport->hw_stopped) {
            if (status) {
                uport->hw_stopped = 0;
                uport->ops->start_tx(uport);
                uart_write_wakeup(uport);
            }
        } else {
            if (!status) {
                uport->hw_stopped = 1;
                uport->ops->stop_tx(uport);
            }
        }

    }
}

/**
 * uart_insert_char - push a char to the uart layer
 *
 * User is responsible to call tty_flip_buffer_push when they are done with
 * insertion.
 *
 * @port: corresponding port
 * @status: state of the serial port RX buffer (LSR for 8250)
 * @overrun: mask of overrun bits in @status
 * @ch: character to push
 * @flag: flag for the character (see TTY_NORMAL and friends)
 */
void uart_insert_char(struct uart_port *port, unsigned int status,
         unsigned int overrun, unsigned int ch, unsigned int flag)
{
    struct tty_port *tport = &port->state->port;

    if ((status & port->ignore_status_mask & ~overrun) == 0)
        if (tty_insert_flip_char(tport, ch, flag) == 0)
            ++port->icount.buf_overrun;

    /*
     * Overrun is special.  Since it's reported immediately,
     * it doesn't affect the current character.
     */
    if (status & ~port->ignore_status_mask & overrun)
        if (tty_insert_flip_char(tport, 0, TTY_OVERRUN) == 0)
            ++port->icount.buf_overrun;
}
