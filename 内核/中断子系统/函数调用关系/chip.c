/**
 *  irq_set_chip - set the irq chip for an irq
 *  @irq:   irq number
 *  @chip:  pointer to irq chip description structure
 */
int irq_set_chip(unsigned int irq, struct irq_chip *chip)
{
    unsigned long flags;
    struct irq_desc *desc = irq_get_desc_lock(irq, &flags, 0); // 获取irq number对应的中断描述符。这里用关闭中断＋spin lock来保护中断描述符，
                                                               // flag中就是保存的关闭中断之前的状态flag，函数后面会恢复中断flag。
    if (!desc)
        return -EINVAL;
    if (!chip)
        chip = &no_irq_chip;
    desc->irq_data.chip = chip;
    irq_put_desc_unlock(desc, flags);
    irq_mark_irq(irq);  // 如果使用线性映射中断号，设置 allocated_irqs 对应的bit为1，allocated_irqs的含义参见irqdesc.c中的描述
                        // 如果使用radix tree映射中断号，则该函数为空，因为该操作在alloc的时候已经设定了
    return 0;
}
/**
 *  irq_set_type - set the irq trigger type for an irq
 *  @irq:   irq number
 *  @type:  IRQ_TYPE_{LEVEL,EDGE}_* value - see include/linux/irq.h
 */
int irq_set_irq_type(unsigned int irq, unsigned int type)
{
    unsigned long flags;
    struct irq_desc *desc = irq_get_desc_buslock(irq, &flags, IRQ_GET_DESC_CHECK_GLOBAL);
    int ret = 0;

    if (!desc)
        return -EINVAL;

    type &= IRQ_TYPE_SENSE_MASK;
    ret = __irq_set_trigger(desc, irq, type);
    irq_put_desc_busunlock(desc, flags);
    return ret;
}
/**
 *  irq_set_chip_data - set irq chip data for an irq
 *  @irq:   Interrupt number
 *  @data:  Pointer to chip specific data
 *
 *  Set the hardware irq chip data for an irq
 */
int irq_set_chip_data(unsigned int irq, void *data)
{
    unsigned long flags;
    struct irq_desc *desc = irq_get_desc_lock(irq, &flags, 0);

    if (!desc)
        return -EINVAL;
    desc->irq_data.chip_data = data;
    irq_put_desc_unlock(desc, flags);
    return 0;
}
void __irq_set_handler(unsigned int irq, irq_flow_handler_t handle, int is_chained, const char *name)
{
    unsigned long flags;
    struct irq_desc *desc = irq_get_desc_buslock(irq, &flags, 0);
    if (!desc)
        return;
    if (!handle) {
        handle = handle_bad_irq;
    } else {
        struct irq_data *irq_data = &desc->irq_data;
#ifdef CONFIG_IRQ_DOMAIN_HIERARCHY
        /*
         * With hierarchical domains we might run into a
         * situation where the outermost chip is not yet set
         * up, but the inner chips are there.  Instead of
         * bailing we install the handler, but obviously we
         * cannot enable/startup the interrupt at this point.
         */
        while (irq_data) {
            if (irq_data->chip != &no_irq_chip)
                break;
            /*
             * Bail out if the outer chip is not set up
             * and the interrrupt supposed to be started
             * right away.
             */
            if (WARN_ON(is_chained))
                goto out;
            /* Try the parent */
            irq_data = irq_data->parent_data;
        }
#endif
        if (WARN_ON(!irq_data || irq_data->chip == &no_irq_chip))
            goto out;
    }
    /* Uninstall? */
    if (handle == handle_bad_irq) {
        if (desc->irq_data.chip != &no_irq_chip)
            mask_ack_irq(desc);
        irq_state_set_disabled(desc);
        desc->depth = 1;
    }
    desc->handle_irq = handle;
    desc->name = name;
    if (handle != handle_bad_irq && is_chained) {
        irq_settings_set_noprobe(desc);
        irq_settings_set_norequest(desc);
        irq_settings_set_nothread(desc);
        irq_startup(desc, true);
    }
out:
    irq_put_desc_busunlock(desc, flags);
}
void irq_set_chip_and_handler_name(unsigned int irq, struct irq_chip *chip, irq_flow_handler_t handle, const char *name)
{
    irq_set_chip(irq, chip);
    __irq_set_handler(irq, handle, 0, name);
}
static inline void irq_set_chip_and_handler(unsigned int irq, struct irq_chip *chip, irq_flow_handler_t handle)
{
    irq_set_chip_and_handler_name(irq, chip, handle, NULL);
}

