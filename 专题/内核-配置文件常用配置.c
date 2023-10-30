1、 CONFIG_SPARSE_IRQ
    如果内核配置了CONFIG_SPARSE_IRQ，那么它就会用基数树(radix tree)来代替irq_desc数组。SPARSE的意思是“稀疏”，假设大小为1000的数组中
只用到2个数组项，那不是浪费嘛？所以在中断比较“稀疏”的情况下可以用基数树来代替数组。

2、 CONFIG_SMP
    配置内核是否支持SMP，SMP 即多核 CPU

3、 CONFIG_SERIAL_CORE_CONSOLE

4、 CONFIG_BLOCK
    内核支持块设备