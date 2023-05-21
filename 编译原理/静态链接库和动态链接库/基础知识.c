1、链接库：
	所谓链接库，其实就是将开源的库文件（例如上面提到的 myMath.c）进行编译、打包操作后得到的二进制文件。虽然链接库是二进制文件，但无法独立运行，必须
等待其它程序调用，才会被载入内存。

2、链接工作方式：
	一个目标文件中使用的函数或变量，可能定义在其他的目标文件中，也可能定义在某个链接库文件中。链接器完成完成链接工作的方式有两种，分别是：
（1）无论缺失的地址位于其它目标文件还是链接库，链接库都会逐个找到各目标文件中缺失的地址。采用此链接方式生成的可执行文件，可以独立载入内存运行；
（2）链接器先从所有目标文件中找到部分缺失的地址，然后将所有目标文件组织成一个可执行文件。如此生成的可执行文件，仍缺失部分函数和变量的地址，待文件执
行时，需连同所有的链接库文件一起载入内存，再由链接器完成剩余的地址修复工作，才能正常执行。
	我们通常将第一种链接方式称为静态链接，用到的链接库称为静态链接库；第二种链接方式中，链接所有目标文件的方法仍属静态链接，而载入内存后进行的链接操
作称为动态链接，用到的链接库称为动态链接库。

3、静态链接的过程由静态链接器负责完成，动态链接的过程由动态链接器负责完成。链接器的实现机制和操作系统有关，例如 Linux 平台上，动态链接器本质就是一个动态
链接库。

4、程序完成链接操作的方式有两种，一种是在生成可执行文件之前完成所有链接操作，使用的库文件称为静态链接库；另一种是将部分链接操作推迟到程序执行时才进行，
此过程使用的库文件称为动态链接库。