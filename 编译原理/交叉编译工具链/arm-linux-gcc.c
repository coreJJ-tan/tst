https://blog.csdn.net/Nickter/article/details/10953837

1、概述
首先介绍下编译器的工作过程，在 使用GCC编译程序时，编译过程分为四个阶段：
(1) 预处理(Pre-Processing)
(2) 编译(Compiling)
(3) 汇编(Assembling)
(4) 链接(Linking)

2、以文件example.c为例介绍gcc的相关参数
2.1 arm-linux-gcc -o example example.c
    不加-c、-S、-E参数，编译器将执行预处理、编译、汇编、连接操作直接生成可执行代码。
    -o 参数用于指定输出的文件，输出文件名为example,如果不指定输出文件，则默认输出a.out

2.2 arm-linux-gcc -c -o example.o example.c
   -c 参数将对源程序example.c进行预处理、编译、汇编操作，生成example.o文件      ---- 不链接
   去掉指定输出选项"-o example.o"自动输出为example.o,所以说在这里-o加不加都可以

2.3 arm-linux-gcc -S -o example.s example.c
   -S 参数将对源程序example.c进行预处理、编译，生成example.s文件               ---- 不汇编、不链接
   -o 选项同上

2.4 arm-linux-gcc -E -o example.i example.c
   -E 参数将对源程序example.c进行预处理，生成example.i文件                     ---- 不编译、不汇编、不链接
   就是将#include，#define等进行文件插入及宏扩展等操作。

2.5 arm-linux-gcc -v -o example example.c
    加上 -v 参数，显示编译时的详细信息，编译器的版本，编译过程等。

2.6 arm-linux-gcc -g -o example example.c
    -g 选项，加入GDB能够使用的调试信息,使用GDB调试时比较方便。

2.7 arm-linux-gcc -Wall -o example example.c
    -Wall 选项打开了所有需要注意的警告信息，像在声明之前就使用的函数，声明后却没有使用的变量等。

2.8 arm-linux-gcc -Ox -o example example.c
    -Ox 使用优化选项，X的值为空、0、1、2、3
    0 为不优化，优化的目的是减少代码空间和提高执行效率等，但相应的编译过程时间将较长并占用较大的内存空间。