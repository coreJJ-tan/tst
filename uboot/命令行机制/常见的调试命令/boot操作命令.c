bootz
    bootz ${loadaddr} - ${fdt_addr};
bootm
    bootm 用于将内核镜像加载到内存的指定地址处，如果有需要还要解压镜像，然后根据操作系统和体系结构的不同给内核传递不同的启动参数，最后启动内核。
boot