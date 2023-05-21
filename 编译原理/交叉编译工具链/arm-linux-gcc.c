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

2.9 arm-linux-gcc -I/home/include -o example example.c
	-Idirname:将dirname所指出的目录加入到程序头文件目录列表中。如果在预设系统及当前目录中没有找到需要的文件，就到指定的dirname目录中去寻找。
	
2.10 arm-linux-gcc -L/home/lib -o example example.c
	-Ldirname：将dirname所指出的目录加入到库文件的目录列表中。在默认状态下，连接程序ld在系统的预设路径中(如/usr/lib)寻找所需要的库文件，这个
选项告诉连接程序，首先到-L指定的目录中去寻找，然后再到系统预设路径中寻找。

2.11 arm-linux-gcc –static -o libexample.a example.c
	静态链接库文件
	
几个常用的选项：
(1) 上面所提到的：-o  -c 
 
(2) -Dmacro 或-Dmacro=defn
	其作用类似于源程序里的#define。例如：gcc -c -DHAVE_GDBM -DHELP_FILE=\"help\"cdict.c其中第一个-D选项定义宏HAVE_GDBM，在程序里可以用#ifdef去检
查它是否被设置。第二个-D选项将宏HELP_FILE定义为字符串“help”（由于反斜线的作用，引号实际上已成为该宏定义的一部分），这对于控制程序打开哪个文件是
很有用的。

(3) -Umacro
	某些宏是被编译程序自动定义的。这些宏通常可以指定在其中进行编译的计算机系统类型的符号，用户可以在编译某程序时加上-v选项以查看gcc缺省定义了哪
些宏。如果用户想取消其中某个宏定义，用-Umacro选项，这相当于把#undefmacro放在要编译的源文件的开头。

(4) -Idir	----上面提到的-I
	将dir目录加到搜寻头文件的目录列表中去，并优先于在gcc缺省的搜索目录。在有多个-I选项的情况下，按命令行上-I选项的前后顺序搜索。dir可使用相对路
径，如-I../inc等。

(5) -O -O2	----上面提到的-Ox
	-O  对程序编译进行优化，编译程序试图减少被编译程序的长度和执行时间，但其编译速度比不做优化慢，而且要求较多的内存。
	-O2  允许比-O更好的优化，编译速度较慢，但结果程序的执行速度较快。
	
(6) -g		----上面提到的-g
	产生一张用于调试和排错的扩展符号表。-g选项使程序可以用GNU的调试程序GDB进行调试。优化和调试通常不兼容，同时使用-g和-O（-O2）选项经常会使程序
产生奇怪的运行结果。所以不要同时使用-g和-O（-O2）选项。

(7) -fpic或-fPIC
	产生位置无关的目标代码，可用于构造共享函数库。
	
(8)	-static -shared
	-static   禁止与共享函数库连接。
	-shared   尽量与共享函数库连接