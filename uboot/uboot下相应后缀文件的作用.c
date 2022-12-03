./u-boot.map    是 uboot 的映射文件，可以从此文件看到某个文件或者函数链接到了哪个地址
                u-boot.map中包含了链接过程中涉及的目标文件将其所依赖的库文件，然后所链接的目标文件的先后顺序列出各目标文件中各符号所链接的地址。
./System.map    该文件按链接地址由小到大的顺序列出所有符号
./config.mk u-boot根目录下自带一个config.mk文件，是一个makefile文件，以后会被主Makefile调用。
Kbuild      用于生成一些和汇编有关的文件。
Kconfig     图形配置界面描述文件。
u-boot.bin：编译出来的二进制格式的 uboot 可执行镜像文件。
u-boot：编译出来的 ELF 格式的 uboot 镜像文件。
u-boot.imx： u-boot.bin 添加头部信息以后的文件， NXP 的 CPU 专用文件。



System.map 中的符号类型解释：
小写字母表示局部; 大写字母表示全局(外部).
A   a
Absolute
符号的值是绝对值，并且在进一步链接过程中不会被改变

B   b
BSS
符号在未初始化数据区或区（section）中，即在BSS段中

C   c
Common
符号是公共的。公共符号是未初始化的数据。在链接时，多个公共符号可能具有同一名称。如果该符号定义在其他地方，则公共符号被看作是未定义的引用

D   d
Data
符号在已初始化数据区中

G   g
Global
符号是在小对象已初始化数据区中的符号。某些目标文件的格式允许对小数据对象（例如一个全局整型变量）可进行更有效的访问

I
Inderect
符号是对另一个符号的间接引用

N
Debugging
符号是一个调试符号

R
Read only
符号在一个只读数据区中

S
Small
符号是小对象未初始化数据区中的符号

T   t
Text
符号是代码区中的符号

U
Undefined
符号是外部的，并且其值为0（未定义）

-
Stabs
符号是a.out目标文件中的一个stab符号，用于保存调试信息

?
Unknown
符号的类型未知，或者与具体文件格式有关