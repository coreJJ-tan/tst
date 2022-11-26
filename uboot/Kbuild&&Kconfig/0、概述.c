https://itexp.blog.csdn.net/article/details/122239526
1、介绍
    Kbuild && Kconfig 隶属于 Linux Kernel Build System。在宏观上，Kbuild && Kconfig 可以统称为 Kbuild，从微观上来说，Kbuild 指的是编译的过程，而 Kconfig 指的在编
译之前对内核进行配置的过程（该过程中会编译一些工具来实现配置过程）。
    本文约定俗成的称呼：构建 指的 Kbuild + Kconfig；配置 指的 Kconfig；编译 指的 Kbuild。

    Kbuild && Kconfig 这套构建系统一个显著的特点就是每一级目录都会有单独的相关文件，然后会被上一级相同的文件引用。这样就保证了每一级目录都是相互独立的。尤其是对于源码的
维护者来说，这个是至关重要。每个维护者可以只关心他所负责的代码部分，任何更改只需要变更它自己的 Kbuild && Kconfig 相关文件即可。它本身主要包含以下几类文件：
    Makefile： Kbuild && Kconfig 这套构建系统本身属于 make 功能的扩展，因此，整个工作过程就是一些列 Makefile 文件的调用。其中入口就是根目录下的 Makefile 文件，Makefile 
中会调用各种工具以实现不同的功能。
    注意，为了区分不同的功能，在源码中对于 Makefile 的命名有时候会加一个后缀。例如，config.mk、Makefile.build、Makefile.clean 等这些都属于 Makefile 文件。
    Makefile 文件无法在线调试，对于理解一个复杂的 Makefile 很不友好。一般我们可以使用 --debug 参数让 make 打印详细的信息来协助理解。或者是在 Makefile 中添加一些打印信
息，常用打印方式有两种：
    使用 $(info, xxxx$(xxx))、 $(warning, xxxx$(xxx))、 $(error, xxxx$(xxx))，其中，$(xxx) 表示某个变量。这三个命令可以加到 Makefile 的任意地方，注意 
$(error, xxxx$(xxx)) 会终止 Make 过程。使用 @echo "xxxx $xx xxxx"，其中，$(xxx) 表示某个变量。这个命令只能用在目标后边，且前面是个TAB。这个就是标准 Makefile 语法中的
一个命令。
    再顺带说一句 make 的工作的一些机制：
    （1）如果给出了参数，则 make 优先去找匹配的规则（匹配规则：完整匹配 > 通配符半匹配 > 完全通配符匹配）去执行；如果没有给出参数，make 会自动找到 Makefile 中第一个目标中
没有通配符的规则执行。
    （2）如果中间遇到 include 其他文件，就会紧接着执行 include 的文件，完成后再继续执行本文件。
    （3）make 总是从 Makefile 的开头开始解析，并不是找到匹配目标之后仅执行匹配目标的命令。也就是说，在匹配之前，Make 可能已经解析了很多判断条件。
    （4）对于匹配的规则如果有依赖，优先解析依赖。注意，依赖的匹配也符合 1 中所说的规则。
    （5）命令前面加了 @ 字符，则不显示命令本身而只显示它的结果。命令前面加了 - 号，即使这条命令出错，make 也会继续执行后续命令。
    （6）如果 Makefile 中存在多条同名规则，则 make 程序会尝试将他们合并。但是如果这些同名规则都有命令的话，make 会给出警告，并用后面的命令代替前面的命令。

2、一些文件
    （1）Kconfig 文件： 这个文件就是 Kconfig 系统的菜单项，当我们使用命令：make menuconfig 时，Kconfig 系统读取该文件，根据该文件的内容生成各级菜单。U-Boot 源码根目录下
的 Kconfig 就是顶级的配置菜单，其中会在引入其他目录下的 Kconfig 作为二级菜单，依次类推。
    （2）.config 文件： 这个文件记录了我们菜单项中的配置的具体值，我们所有对于构建的配置的存放在这个文件中，我们在，Kconfig 系统菜单中的更改，最终都会改写该文件。注意：该文
件默认是个隐藏文件，可以使用 ls -al 查看。
    （3）xxxx_defconfig 文件： xxxx_defconfig 文件就为 Kconfig 系统的菜单项提供一个默认值。不过，Kconfig 系统不会直接读取 xxxx_defconfig 文件，而是需要使用方式是通过 
make xxx_deconfig 生成带默认值的 .config。这样，在加载 Kconfig 时，就可以同时加载 .config 以提供默认值。简单来说，xxxx_defconfig 就是为了方便支持更多个性化配置，从而可以
尽可能少的改动源代码。
    （4）Kbuild 文件： 这个是 Kbuild 系统使用的文件，该文件用于定义一些源码使用的需要根据编译环境产生的中间文件。
    （5）config.mk 文件： 用来处理一些编译过程中的环境变量。Linux Kernel 没有这个文件，U-Boot 需要使用它。


**** Kbuild && Kconfig 这套构建系统中定义了很多命令，我们可以使用 make help 来进行查看（就在根目录的 Makefile 文件中）

3、构建过程
    在 Kbuild && Kconfig 这套构建系统中，源码中使用的有些文件是要靠 Kbuild && Kconfig 这套系统来生成的，直接在源码中是找不到。这就要求我们必须要了解 Kbuild && Kconfig 
是如何工作的，更重要的是要知道 Kbuild && Kconfig 会产生哪些源码使用的文件。整个构建过程的这个入口就是源码根目录下的 Makefile 文件。
