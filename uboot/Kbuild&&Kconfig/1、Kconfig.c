    在 Kbuild && Kconfig 这套构建系统中，当我们使用 make xxxconfg 类似的命令时，就会执行 Kconfig 流程。例如，当执行 make menuconfig 时会出现一个配置界面，
允许开发者通过类似于 UI 的方式来对内核进行配置，之所以我们可以看到这个类似于 UI 的界面，就是因为 Kconfig 从中产生了多个文件和工具来实现的。当我们在 U-Boot 根
目录执行 make menuconfig 或者 make xxx_deconfig 时，make 命令便会读取 U-Boot 根目录下的 Makefile 文件，然后解析并匹配 Makefile 文件中的规则 。
    make xxx_deconfig最终会生成.config文件，这个就是后续编译uboot所需要的；make menuconfig会根据.config文件，打开一个UI图形界面，进行进一步配置，如果还不
存在.config文件，那么就会使用默认值进行配置，随后会生成相应的.config文件。