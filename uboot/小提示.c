1、对于我们拿到一个新的bootloader，我们怎么能使代码支持dtb模式，我们需要配置#define CONFIG_OF_LIBFDT, 可让u-boot支持内核设备树dts

2、uboot下涉及设备树有下面这几个常用的配置项
    CONFIG_OF_CONTROL   如果uboot下有设备树，则需要打开该配置项，u-boot 支持下面两种形式将 dtb 编译到 u-boot 的镜像中
    CONFIG_OF_EMBED     dtb 集成到 u-boot 的 bin 文件内部，在这种方式下，在编译 u-boot 的过程中，也会编译 dtb。最终 dtb 是包含到了u-boot 的 bin 文件内部的。官方不推荐这
种方式，建议仅用于调试
    CONFIG_OF_SEPARATE  dtb 和 u-boot 的 bin文件分离，在这种方式下，u-boot 的编译和 dtb 的编译是分开的，先生成 u-boot 的 bin 文件，然后再另外生成dtb 文件。
dtb 最终会自动追加到 u-boot 的 bin 文件的最后面。

3、串口波特率设置由 CONFIG_BAUDRATE 该配置项完成，若 baudrate 环境变量被定义，那么优先使用环境变量指定的值