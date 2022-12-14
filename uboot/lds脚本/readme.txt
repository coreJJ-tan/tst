不知道原文链接。。。

LDS 相关的资料很少， 这篇算是总结的非常全面的了，先转来慢慢学习

/*================================================================================*/


最近在看Linux内核时，总是遇到一些和连接脚本相关的东东，搞得人一头雾水，终于下定决心把它搞明白，写下一点心得，希望对和我一样的人有所帮助！ 
     

连接脚本的格式
====================

连接脚本是文本文件.

你写了一系列的命令作为一个连接脚本. 每一个命令是一个带有参数的关键字,或者是一个对符号的赋值. 你可
以用分号分隔命令. 空格一般被忽略.

文件名或格式名之类的字符串一般可以被直接键入. 如果文件名含有特殊字符,比如一般作为分隔文件名用的逗
号, 你可以把文件名放到双引号中. 文件名中间无法使用双引号.

你可以象在C语言中一样,在连接脚本中使用注释, 用'/*'和'*/'隔开. 就像在C中,注释在语法上等同于空格.

简单的连接脚本示例
============================

许多脚本是相当的简单的.

可能的最简单的脚本只含有一个命令: 'SECTIONS'. 你可以使用'SECTIONS'来描述输出文件的内存布局.

'SECTIONS'是一个功能很强大的命令. 这里这们会描述一个很简单的使用. 让我们假设你的程序只有代码节,
初始化过的数据节, 和未初始化过的数据节. 这些会存在于'.text','.data'和'.bss'节, 另外, 让我们进一
步假设在你的输入文件中只有这些节.

对于这个例子, 我们说代码应当被载入到地址'0x10000'处, 而数据应当从0x8000000处开始. 下面是一个实现
这个功能的脚本:

    SECTIONS
    {
      . = 0x10000;
      .text : { *(.text) }
      . = 0x8000000;
      .data : { *(.data) }
      .bss : { *(.bss) }
    }

你使用关键字'SECTIONS'写了这个SECTIONS命令, 后面跟有一串放在花括号中的符号赋值和输出节描述的内容.

上例中, 在'SECTIONS'命令中的第一行是对一个特殊的符号'.'赋值, 这是一个定位计数器. 如果你没有以其
它的方式指定输出节的地址(其他方式在后面会描述), 那地址值就会被设为定位计数器的现有值. 定位计数器
然后被加上输出节的尺寸. 在'SECTIONS'命令的开始处, 定位计数器拥有值'0'.

第二行定义一个输出节,'.text'. 冒号是语法需要,现在可以被忽略. 节名后面的花括号中,你列出所有应当被
放入到这个输出节中的输入节的名字. '*'是一个通配符,匹配任何文件名. 表达式'*(.text)'意思是所有的输
入文件中的'.text'输入节.

因为当输出节'.text'定义的时候, 定位计数器的值是'0x10000',连接器会把输出文件中的'.text'节的地址设
为'0x10000'.

余下的内容定义了输出文件中的'.data'节和'.bss'节. 连接器会把'.data'输出节放到地址'0x8000000'处. 连接
器放好'.data'输出节之后, 定位计数器的值是'0x8000000'加上'.data'输出节的长度. 得到的结果是连接器会
把'.bss'输出节放到紧接'.data'节后面的位置.

连接器会通过在必要时增加定位计数器的值来保证每一个输出节具有它所需的对齐. 在这个例子中, 为'.text'
和'.data'节指定的地址会满足对齐约束, 但是连接器可能会需要在'.data'和'.bss'节之间创建一个小的缺口.

就这样,这是一个简单但完整的连接脚本.

简单的连接脚本命令.
=============================

在本章中,我们会描述一些简单的脚本命令.

设置入口点.
-----------------------

在运行一个程序时第一个被执行到的指令称为"入口点". 你可以使用'ENTRY'连接脚本命令来设置入口点.参数
是一个符号名:
    ENTRY(SYMBOL)

有多种不同的方法来设置入口点.连接器会通过按顺序尝试以下的方法来设置入口点, 如果成功了,就会停止.

  * `-e'入口命令行选项;

  * 连接脚本中的`ENTRY(SYMBOL)'命令;

  * 如果定义了start, 就使用start的值;

  * 如果存在,就使用'.text'节的首地址;

  * 地址`0'.

处理文件的命令.
---------------------------

有几个处理文件的连接脚本命令.

`INCLUDE FILENAME'
在当前点包含连接脚本文件FILENAME. 在当前路径下或用'-L'选项指定的所有路径下搜索这个文件,
你可以嵌套使用'INCLUDE'达10层.

`INPUT(FILE, FILE, ...)'
`INPUT(FILE FILE ...)'
'INPUT'命令指示连接器在连接时包含文件, 就像它们是在命令行上指定的一样.

比如,如果你在连接的时候总是要包含文件'subr.o',但是你对每次连接时要在命令行上输入感到厌烦
, 你就可以在你的连接脚本中输入'INPUT (subr.o).

事实上,如果你喜欢,你可以把你所有的输入文件列在连接脚本中, 然后在连接的时候什么也不需要,
只要一个'-T'选项就够了.

在一个'系统根前缀'被配置的情况下, 一个文件名如果以'/'字符打头, 并且脚本也存放在系统根
前缀的某个子目录下, 文件名就会被在系统根前缀下搜索. 否则连接器就会企图打开当前目录下的文
件. 如果没有发现, 连接器会通过档案库搜索路径进行搜索.

如果你使用了'INPUT (-lFILE)', 'ld'会把文件名转换为'libFILE.a', 就象命令行参数'-l'一样.

当你在一个隐式连接脚本中使用'INPUT'命令的时候, 文件就会在连接时连接脚本文件被包含的点上
被包含进来. 这会影响到档案搜索.

`GROUP(FILE, FILE, ...)'
`GROUP(FILE FILE ...)'
除了文件必须全是档案文件之外, 'GROUP'命令跟'INPUT'相似, 它们会被反复搜索,直至没有未定义
的引用被创建.

`OUTPUT(FILENAME)'
'OUTPUT'命令命名输出文件. 在连接脚本中使用'OUTPUT(FILENAME)'命令跟在命令行中使用'-o 
FILENAME'命令是完全等效的. 如果两个都使用了, 那命令行选项优先.

你可以使用'OUTPUT'命令为输出文件创建一个缺省的文件名,而不是常用的'a.out'.

`SEARCH_DIR(PATH)'
`SEARCH_DIR'命令给'ld'用于搜索档案文件的路径中再增加新的路径. 使用`SEARCH_DIR(PATH)'跟在
命令行上使用'-L PATH'选项是完全等效的. 如果两个都使用了, 那连接器会两个路径都搜索. 用命
令行选项指定的路径首先被搜索.

`STARTUP(FILENAME)'
除了FILENAME会成为第一个被连接的输入文件, 'STARTUP'命令跟'INPUT'命令完全相似, 就象这个文
件是在命令行上第一个被指定的文件一样. 如果在一个系统中, 入口点总是存在于第一个文件中,那
这个就很有用.

处理目标文件格式的命令.
-----------------------------------------

有两个处理目标文件格式的连接脚本命令.

`OUTPUT_formAT(BFDNAME)'
`OUTPUT_formAT(DEFAULT, BIG, LITTLE)'
`OUTPUT_formAT'命令为输出文件使用的BFD格式命名. 使用`OUTPUT_formAT(BFDNAME)'跟在命令行上
使用'-oformat BFDNAME'是完全等效的. 如果两个都使用了, 命令行选项优先.

你可在使用`OUTPUT_formAT'时带有三个参数以使用不同的基于'-EB'和'-EL'的命令行选项的格式.

如果'-EB'和'-EL'都没有使用, 那输出格式会是第一个参数DEFAULT, 如果使用了'-EB',输出格式会是
第二个参数BIG, 如果使用了'-EL', 输出格式会是第三个参数, LITTLE.

比如, 缺省的基于MIPS ELF平台连接脚本使用如下命令:

        OUTPUT_formAT(elf32-bigmips, elf32-bigmips, elf32-littlemips)
    这表示缺省的输出文件格式是'elf32-bigmips', 但是当用户使用'-EL'命令行选项的时候, 输出文件就会
    被以`elf32-littlemips'格式创建.

`TARGET(BFDNAME)'
'TARGET'命令在读取输入文件时命名BFD格式. 它会影响到后来的'INPUT'和'GROUP'命令. 这个命令跟
在命令行上使用`-b BFDNAME'相似. 如果使用了'TARGET'命令但`OUTPUT_formAT'没有指定, 最后的
'TARGET'命令也被用来设置输出文件的格式.

其它的连接脚本命令.
----------------------------

还有一些其它的连接脚本命令.

`ASSERT(EXP, MESSAGE)'
确保EXP不等于零,如果等于零, 连接器就会返回一个错误码退出,并打印出MESSAGE.

`EXTERN(SYMBOL SYMBOL ...)'
强制SYMBOL作为一个无定义的符号输入到输出文件中去. 这样做了,可能会引发从标准库中连接一些
节外的库. 你可以为每一个EXTERN'列出几个符号, 而且你可以多次使用'EXTERN'. 这个命令跟'-u'
命令行选项具有相同的效果.

`FORCE_COMMON_ALLOCATION'
这个命令跟命令行选项'-d'具有相同的效果: 就算指定了一个可重定位的输出文件('-r'),也让'ld'
为普通符号分配空间.

`INHIBIT_COMMON_ALLOCATION'
这个命令跟命令行选项`--no-define-common'具有相同的效果: 就算是一个不可重位输出文件, 也让
'ld'忽略为普通符号分配的空间.

`NOCROSSREFS(SECTION SECTION ...)'
这个命令在遇到在某些特定的节之间引用的时候会产生一条错误信息.

在某些特定的程序中, 特别是在使用覆盖技术的嵌入式系统中, 当一个节被载入内存时,另外一个节
就不会在内存中. 任何在两个节之间的直接引用都会是一个错误. 比如, 如果节1中的代码调用了另
一个节中的一个函数,这就会产生一个错误.

`NOCROSSREFS'命令带有一个输出节名字的列表. 如果'ld'遇到任何在这些节之间的交叉引用, 它就
会报告一个错误,并返回一个非零退出码. 注意, `NOCROSSREFS'命令使用输出节名,而不是输入节名.

`OUTPUT_ARCH(BFDARCH)'
指定一个特定的输出机器架构. 这个参数是BFD库中使用的一个名字. 你可以通过使用带有'-f'选项
的'objdump'程序来查看一个目标文件的架构.

为符号赋值.
===========================

你可以在一个连接脚本中为一个符号赋一个值. 这会把一个符号定义为一个全局符号.

简单的赋值.
------------------

你可以使用所有的C赋值符号为一个符号赋值.

`SYMBOL = EXPRESSION ;'
`SYMBOL += EXPRESSION ;'
`SYMBOL -= EXPRESSION ;'
`SYMBOL *= EXPRESSION ;'
`SYMBOL /= EXPRESSION ;'
`SYMBOL <<= EXPRESSION ;'
`SYMBOL >>= EXPRESSION ;'
`SYMBOL &= EXPRESSION ;'
`SYMBOL |= EXPRESSION ;'

第一个情况会把SYMBOL定义为值EXPRESSION. 其它情况下, SYMBOL必须是已经定义了的, 而值会作出相应的调
整.

特殊符号名'.'表示定位计数器. 你只可以在'SECTIONS'命令中使用它.

EXPRESSION后面的分号是必须的.

表达式下面会定义.

你在写表达式赋值的时候,可以把它们作为单独的部分,也可以作为'SECTIONS'命令中的一个语句,或者作为
'SECTIONS'命令中输出节描述的一个部分.

符号所在的节会被设置成表达式所在的节.

下面是一个关于在三处地方使用符号赋值的例子:

    floating_point = 0;
    SECTIONS
    {
      .text :
        {
          *(.text)
          _etext = .;
        }
      _bdata = (. + 3) & ~ 3;
      .data : { *(.data) }
    }

在这个例子中, 符号`floating_point'被定义为零. 符号'-etext'会被定义为前面一个'.text'节尾部的地址.
而符号'_bdata'会被定义为'.text'输出节后面的一个向上对齐到4字节边界的一个地址值.

PROVIDE
-------

在某些情况下, 一个符号被引用到的时候只在连接脚本中定义,而不在任何一个被连接进来的目标文件中定
义. 这种做法是比较明智的. 比如, 传统的连接器定义了一个符号'etext'. 但是, ANSI C需要用户能够把
'etext'作为一个函数使用而不会产生错误. 'PROVIDE'关键字可以被用来定义一个符号, 比如'etext', 这个
定义只在它被引用到的时候有效,而在它被定义的时候无效.语法是 `PROVIDE(SYMBOL = EXPRESSION)'.

下面是一个关于使用'PROVIDE'定义'etext'的例子:

    SECTIONS
    {
      .text :
        {
          *(.text)
          _etext = .;
          PROVIDE(etext = .);
        }
    }

在这个例子中, 如果程序定义了一个'_etext'(带有一个前导下划线), 连接器会给出一个重定义错误. 如果,
程序定义了一个'etext'(不带前导下划线), 连接器会默认使用程序中的定义. 如果程序引用了'etext'但不
定义它, 连接器会使用连接脚本中的定义.

SECTIONS命令
================

'SECTIONS'命令告诉连接器如何把输入节映射到输出节, 并如何把输出节放入到内存中.

'SECTIONS'命令的格式如下:

    SECTIONS
    {
      SECTIONS-COMMAND
      SECTIONS-COMMAND
      ...
    }

每一个SECTIONS-COMMAND可能是如下的一种:

  * 一个'ENTRY'命令.

  * 一个符号赋值.

  * 一个输出节描述.

  * 一个重叠描述.

'ENTRY'命令和符号赋值在'SECTIONS'命令中是允许的, 这是为了方便在这些命令中使用定位计数器. 这也可
以让连接脚本更容易理解, 因为你可以在更有意义的地方使用这些命令来控制输出文件的布局.

输出节描述和重叠描述在下面描述.

如果你在连接脚本中不使用'SECTIONS'命令, 连接器会按在输入文件中遇到的节的顺序把每一个输入节放到同
名的输出节中. 如果所有的输入节都在第一个文件中存在,那输出文件中的节的顺序会匹配第一个输入文件中
的节的顺序. 第一个节会在地址零处.

输出节描述
--------------------------

一个完整的输出节的描述应该是这个样子的:

    SECTION [ADDRESS] [(TYPE)] : [AT(LMA)]
      {
        OUTPUT-SECTION-COMMAND
        OUTPUT-SECTION-COMMAND
        ...
      } [>REGION] [AT>LMA_REGION] [:PHDR :PHDR ...] [=FILLEXP]

大多数输出节不使用这里的可选节属性.

SECTION边上的空格是必须的, 所以节名是明确的. 冒号跟花括号也是必须的. 断行和其他的空格是可选的.

每一个OUTPUT-SECTION-COMMAND可能是如下的情况:

  * 一个符号赋值.

  * 一个输入节描述.

  * 直接包含的数据值.

  * 一个特定的输出节关键字.
  
输出节名.
-------------------

输出节的名字是SECTION. SECTION必须满足你的输出格式的约束. 在一个只支持限制数量的节的格式中,比如
'a.out',这个名字必须是格式支持的节名中的一个(比如, 'a.out'只允许'.text', '.data'或'.bss').如果
输出格式支持任意数量的节, 但是只支持数字,而没有名字(就像Oasys中的情况), 名字应当以一个双引号中的
数值串的形式提供.一个节名可以由任意数量的字符组成,但是一个含有任意非常用字符(比如逗号)的字句必须
用双引号引起来.

输出节描述
--------------------------

ADDRESS是关于输出节中VMS的一个表达式. 如果你不提供ADDRESS, 连接器会基于REGION(如果存在)设置它,或
者基于定位计数器的当前值.

如果你提供了ADDRESS, 那输出节的地址会被精确地设为这个值. 如果你既不提供ADDRESS也不提供REGION, 那
输出节的地址会被设为当前的定位计数器向上对齐到输出节需要的对齐边界的值. 输出节的对齐要求是所有输
入节中含有的对齐要求中最严格的一个.

比如:
    .text . : { *(.text) }

和
    .text : { *(.text) }

有细微的不同. 第一个会把'.text'输出节的地址设为当前定位计数器的值. 第二个会把它设为定位计数器的
当前值向上对齐到'.text'输入节中对齐要求最严格的一个边界.

ADDRESS可以是任意表达式; 比如,如果你需要把节对齐对0x10字节边界,这样就可以让低四字节的节地址值为
零, 你可以这样做:

    .text ALIGN(0x10) : { *(.text) }

这个语句可以正常工作,因为'ALIGN'返回当前的定位计数器,并向上对齐到指定的值.

指定一个节的地址会改变定位计数器的值.

输入节描述
-------------------------

最常用的输出节命令是输入节描述.

输入节描述是最基本的连接脚本操作. 你使用输出节来告诉连接器在内存中如何布局你的程序. 你使用输入节
来告诉连接器如何把输入文件映射到你的内存中.

输入节基础
---------------------------

一个输入节描述由一个文件名后跟有可选的括号中的节名列表组成.

文件名和节名可以通配符形式出现, 这个我们以后再介绍.

最常用的输入节描述是包含在输出节中的所有具有特定名字的输入节. 比如, 包含所有输入'.text'节,你可以
这样写:

    *(.text)

这里,'*'是一个通配符,匹配所有的文件名. 为把一部分文件排除在匹配的名字通配符之外, EXCLUDE_FILE可
以用来匹配所有的除了在EXCLUDE_FILE列表中指定的文件.比如:

    (*(EXCLUDE_FILE (*crtend.o *otherfile.o) .ctors))

会让除了`crtend.o'文件和`otherfile.o'文件之外的所有的文件中的所有的.ctors节被包含进来.

有两种方法包含多于一个的节:

    *(.text .rdata)
    *(.text) *(.rdata)

上面两句的区别在于'.text'和'.rdata'输入节的输出节中出现的顺序不同. 在第一个例子中, 两种节会交替
出现,并以连接器的输入顺序排布. 在第二个例子中,所有的'.text'输入节会先出现,然后是所有的'.rdata'节.

你可以指定文件名,以从一个特定的文件中包含节. 如果一个或多个你的文件含有特殊的数据在内存中需要特
殊的定位,你可以这样做. 比如:

    data.o(.data)

如果你使用一个不带有节列表的文件名, 那输入文件中的所有的节会被包含到输出节中. 通常不会这样做, 但
是在某些场合下这个可能非常有用. 比如:

    data.o
    
当你使用一个不含有任何通配符的文件名时, 连接器首先会查看你是否在连接命令行上指定了文件名或者在
'INPUT'命令中. 如果你没有, 连接器会试图把这个文件作为一个输入文件打开, 就像它在命令行上出现一样.
注意这跟'INPUT'命令不一样, 因为连接器会在档案搜索路径中搜索文件.

输入节通配符
---------------------------------

在一个输入节描述中, 文件名或者节名,或者两者同时都可以是通配符形式.

文件名通配符'*'在很多例子中都可以看到,这是一个简单的文件名通配符形式.

通配符形式跟Unix Shell中使用的一样.

`*'
匹配任意数量的字符.

`?'
匹配单个字符.

`[CHARS]'
匹配CHARS中的任意单个字符; 字符'-'可以被用来指定字符的方讧, 比如[a-z]匹配任意小字字符.

`\'
转义其后的字符.

当一个文件名跟一个通配符匹配时, 通配符字符不会匹配一个'/'字符(在UNIX系统中用来分隔目录名), 一个
含有单个'*'字符的形式是个例外; 它总是匹配任意文件名, 不管它是否含有'/'. 在一个节名中, 通配符字
符会匹配'/'字符.

文件名通配符只匹配那些在命令行或在'INPUT'命令上显式指定的文件. 连接器不会通过搜索目录来展开通配
符.

如果一个文件名匹配多于一个通配符, 或者如果一个文件名显式出现同时又匹配了一个通配符, 连接器会使用
第一次匹配到的连接脚本. 比如, 下面的输入节描述序列很可能就是错误的,因为'data.o'规则没有被使用:

    .data : { *(.data) }
    .data1 : { data.o(.data) }

通常, 连接器会把匹配通配符的文件和节按在连接中被看到的顺序放置. 你可以通过'SORT'关键字改变它, 它
出现在括号中的通配符之前(比如, 'SORT(.text*)'). 当'SORT'关键字被使用时, 连接器会在把文件和节放到
输出文件中之前按名字顺序重新排列它们.

如果你对于输入节被放置到哪里去了感到很困惑, 那可以使用'-M'连接选项来产生一个位图文件. 位图文件会
精确显示输入节是如何被映射到输出节中的.

这个例子显示了通配符是如何被用来区分文件的. 这个连接脚本指示连接器把所有的'.text'节放到'.text'中, 把所有的'.bss'节放到'.bss'. 连接器会把所有的来自文件名以一个大写字母开始的文件中的'.data'节放进'.DATA'节中; 对于所有其他文件, 连接器会把'.data'节放进'.data'节中.

    SECTIONS {
      .text : { *(.text) }
      .DATA : { [A-Z]*(.data) }
      .data : { *(.data) }
      .bss : { *(.bss) }
    }

输入节中的普通符号.
-----------------------------------

对于普通符号,需要一个特殊的标识, 因为在很多目标格式中, 普通符号没有一个特定的输入节. 连接器会把
普通符号处理成好像它们在一个叫做'COMMON'的节中.

你可能像使用带有其他输入节的文件名一样使用带有'COMMON'节的文件名。你可以通过这个把来自一个特定输
入文件的普通符号放入一个节中，同时把来自其它输入文件的普通符号放入另一个节中。

在大多数情况下，输入文件中的普通符号会被放到输出文件的'.bss'节中。比如：

    .bss { *(.bss) *(COMMON) }

有些目标文件格式具有多于一个的普通符号。比如，MIPS ELF目标文件格式区分标准普通符号和小普通符号。
在这种情况下，连接器会为其他类型的普通符号使用一个不同的特殊节名。 在MIPS ELF的情况中， 连接器
为标准普通符号使用'COMMON'，并且为小普通符号使用'.common'。这就允许你把不同类型的普通符号映射到
内存的不同位置。

在一些老的连接脚本上，你有时会看到'[COMMON]'。这个符号现在已经过时了， 它等效于'*(COMMON)'。

输入节和垃圾收集
---------------------------------------

当连接时垃圾收集正在使用中时（'--gc-sections')，这在标识那些不应该被排除在外的节时非常有用。这
是通过在输入节的通配符入口外面加上'KEEP()'实现的，比如'KEEP(*(.init))'或者'KEEP(SORT(*)(.sorts))
'。

输入节示例
---------------------

接下来的例子是一个完整的连接脚本。它告诉连接器去读取文件'all.o'中的所有节，并把它们放到输出节
'outputa'的开始位置处， 该输出节是从位置'0x10000'处开始的。 从文件'foo.o'中来的所有节'.input1'
在同一个输出节中紧密排列。 从文件'foo.o'中来的所有节'.input2'全部放入到输出节'outputb'中，后面
跟上从'foo1.o'中来的节'.input1'。来自所有文件的所有余下的'.input1'和'.input2'节被写入到输出节
'outputc'中。

    SECTIONS {
      outputa 0x10000 :
        {
        all.o
        foo.o (.input1)
        }
      outputb :
        {
        foo.o (.input2)
        foo1.o (.input1)
        }
      outputc :
        {
        *(.input1)
        *(.input2)
        }
    }
    
输出节数据
-------------------

你可以通过使用输出节命令'BYTE','SHORT','LONG','QUAD',或者'SQUAD'在输出节中显式包含几个字节的数据
每一个关键字后面都跟上一个圆括号中的要存入的值。表达式的值被存在当前的定位计数器的值处。

‘BYTE’，‘SHORT’，‘LONG’‘QUAD’命令分别存储一个，两个，四个，八个字节。存入字节后，定位计
数器的值加上被存入的字节数。

比如，下面的命令会存入一字节的内容1,后面跟上四字节，其内容是符号'addr'的值。

    BYTE(1)
    LONG(addr)

当使用64位系统时，‘QUAD’和‘SQUAD’是相同的；它们都会存储8字节，或者说是64位的值。而如果软硬件
系统都是32位的，一个表达式就会被作为32位计算。在这种情况下，‘QUAD’存储一个32位值，并把它零扩展
到64位， 而‘SQUAD’会把32位值符号扩展到64位。

如果输出文件的目标文件格式有一个显式的endianness，它在正常的情况下，值就会被以这种endianness存储
当一个目标文件格式没有一个显式的endianness时, 值就会被以第一个输入目标文件的endianness存储。

注意， 这些命令只在一个节描述内部才有效，而不是在它们之间， 所以，下面的代码会使连接器产生一个错
误信息：

    SECTIONS { .text : { *(.text) } LONG(1) .data : { *(.data) } }

而这个才是有效的：

    SECTIONS { .text : { *(.text) ; LONG(1) } .data : { *(.data) } }

你可能使用‘FILL’命令来为当前节设置填充样式。它后面跟有一个括号中的表达式。任何未指定的节内内存
区域（比如，因为输入节的对齐要求而造成的裂缝）会以这个表达式的值进行填充。一个'FILL'语句会覆盖到
它本身在节定义中出现的位置后面的所有内存区域；通过引入多个‘FILL’语句，你可以在输出节的不同位置
拥有不同的填充样式。

这个例子显示如何在未被指定的内存区域填充'0x90'：

    FILL(0x90909090)

‘FILL’命令跟输出节的‘=FILLEXP’属性相似，但它只影响到节内跟在‘FILL’命令后面的部分，而不是
整个节。如果两个都用到了，那‘FILL’命令优先。

输出节关键字
-----------------------

有两个关键字作为输出节命令的形式出现。

`CREATE_OBJECT_SYMBOLS'
这个命令告诉连接器为每一个输入文件创建一个符号。而符号的名字正好就是相关输入文件的名字。
而每一个符号的节就是`CREATE_OBJECT_SYMBOLS'命令出现的那个节。

这个命令一直是a.out目标文件格式特有的。 它一般不为其它的目标文件格式所使用。

`CONSTRUCTORS'
当使用a.out目标文件格式进行连接的时候， 连接器使用一组不常用的结构以支持C++的全局构造函
数和析构函数。当连接不支持专有节的目标文件格式时， 比如ECOFF和XCOFF，连接器会自动辩识C++
全局构造函数和析构函数的名字。对于这些目标文件格式，‘CONSTRUCTORS’命令告诉连接器把构造
函数信息放到‘CONSTRUCTORS’命令出现的那个输出节中。对于其它目标文件格式，‘CONSTRUCTORS’
命令被忽略。

符号`__CTOR_LIST__'标识全局构造函数的开始，而符号`__DTOR_LIST'标识结束。这个列表的第一个
WORD是入口的数量，紧跟在后面的是每一个构造函数和析构函数的地址，再然后是一个零WORD。编译
器必须安排如何实际运行代码。对于这些目标文件格式，GNU C++通常从一个`__main'子程序中调用
构造函数，而对`__main'的调用自动被插入到`main'的启动代码中。GNU C++通常使用'atexit'运行
析构函数，或者直接从函数'exit'中运行。

对于像‘COFF’或‘ELF’这样支持专有节名的目标文件格式，GNU C++通常会把全局构造函数与析构
函数的地址值放到'.ctors'和'.dtors'节中。把下面的代码序列放到你的连接脚本中去，这样会构建
出GNU C++运行时代码希望见到的表类型。

              __CTOR_LIST__ = .;
              LONG((__CTOR_END__ - __CTOR_LIST__) / 4 - 2)
              *(.ctors)
              LONG(0)
              __CTOR_END__ = .;
              __DTOR_LIST__ = .;
              LONG((__DTOR_END__ - __DTOR_LIST__) / 4 - 2)
              *(.dtors)
              LONG(0)
              __DTOR_END__ = .;

如果你正使用GNU C++支持来进行优先初始化，那它提供一些可以控制全局构造函数运行顺序的功能，
你必须在连接时给构造函数排好序以保证它们以正确的顺序被执行。当使用'CONSTRUCTORS'命令时，
替代为`SORT(CONSTRUCTORS)'。当使用'.ctors'和'dtors'节时，使用`*(SORT(.ctors))'和
`*(SORT(.dtors))' 而不是`*(.ctors)'和`*(.dtors)'。

通常，编译器和连接器会自动处理这些事情，并且你不必亲自关心这些事情。但是，当你正在使用
C++，并自己编写连接脚本时，你可能就要考虑这些事情了。

输出节的丢弃。
-------------------------

连接器不会创建那些不含有任何内容的输出节。这是为了引用那些可能出现或不出现在任何输入文件中的输入
节时方便。比如：

    .foo { *(.foo) }

如果至少在一个输入文件中有'.foo'节，它才会在输出文件中创建一个'.foo'节

如果你使用了其它的而不是一个输入节描述作为一个输出节命令，比如一个符号赋值，那这个输出节总是被
创建，即使没有匹配的输入节也会被创建。

一个特殊的输出节名`/DISCARD/'可以被用来丢弃输入节。任何被分配到名为`/DISCARD/'的输出节中的输入
节不包含在输出文件中。

输出节属性
-------------------------

上面，我们已经展示了一个完整的输出节描述，看下去就象这样：

    SECTION [ADDRESS] [(TYPE)] : [AT(LMA)]
      {
        OUTPUT-SECTION-COMMAND
        OUTPUT-SECTION-COMMAND
        ...
      } [>REGION] [AT>LMA_REGION] [:PHDR :PHDR ...] [=FILLEXP]

我们已经介绍了SECTION, ADDRESS, 和OUTPUT-SECTION-COMMAND. 在这一节中，我们将介绍余下的节属性。

输出节类型
...................

每一个输出节可以有一个类型。类型是一个放在括号中的关键字，已定义的类型如下所示：

`NOLOAD'
这个节应当被标式讵不可载入，所以当程序运行时，它不会被载入到内存中。

`DSECT'
`COPY'
`INFO'
`OVERLAY'
支持这些类型名只是为了向下兼容，它们很少使用。它们都具有相同的效果：这个节应当被标式讵不
可分配，所以当程序运行时，没有内存为这个节分配。

连接器通常基于映射到输出节的输入节来设置输出节的属性。你可以通过使用节类型来重设这个属性，
比如，在下面的脚本例子中，‘ROM’节被定址在内存地址零处，并且在程序运行时不需要被载入。
‘ROM’节的内容会正常出现在连接输出文件中。

    SECTIONS {
      ROM 0 (NOLOAD) : { ... }
      ...
    }

输出节LMA
..................

每一个节有一个虚地址（VMA）和一个载入地址（LMA）；出现在输出节描述中的地址表达式设置VMS

连接器通常把LMA跟VMA设成相等。你可以通过使用‘AT’关键字改变这个。跟在关键字‘AT’后面的表达式
LMA指定节的载入地址。或者，通过`AT>LMA_REGION'表达式， 你可以为节的载入地址指定一个内存区域。

这个特性是为了便于建立ROM映像而设计的。比如，下面的连接脚本创建了三个输出节：一个叫做‘.text’
从地址‘0x1000’处开始，一个叫‘.mdata’，尽管它的VMA是'0x2000'，它会被载入到'.text'节的后面，最
后一个叫做‘.bss’是用来放置未初始化的数据的，其地址从'0x3000'处开始。符号'_data'被定义为值
'0x2000', 它表示定位计数器的值是VMA的值，而不是LMA。

    SECTIONS
      {
      .text 0x1000 : { *(.text) _etext = . ; }
      .mdata 0x2000 :
        AT ( ADDR (.text) + SIZEOF (.text) )
        { _data = . ; *(.data); _edata = . ;  }
      .bss 0x3000 :
        { _bstart = . ;  *(.bss) *(COMMON) ; _bend = . ;}
    }

这个连接脚本产生的程序使用的运行时初始化代码会包含象下面所示的一些东西，以把初始化后的数据从ROM
映像中拷贝到它的运行时地址中去。注意这节代码是如何利用好连接脚本定义的符号的。

    extern char _etext, _data, _edata, _bstart, _bend;
    char *src = &_etext;
    char *dst = &_data;
    
    /* ROM has data at end of text; copy it. */
    while (dst < &_edata) {
      *dst++ = *src++;
    }
    
    /* Zero bss */
    for (dst = &_bstart; dst< &_bend; dst++)
      *dst = 0;

输出节区域
.....................

你可以通过使用`>REGION'把一个节赋给前面已经定义的一个内存区域。

这里有一个简单的例子:

    MEMORY { rom : ORIGIN = 0x1000, LENGTH = 0x1000 }
    SECTIONS { ROM : { *(.text) } >rom }

输出节Phdr
...................

你可以通过使用`:PHDR'把一个节赋给前面已定义的一个程序段。如果一个节被赋给一个或多个段，那后来分
配的节都会被赋给这些段，除非它们显式使用了':PHDR'修饰符。你可以使用':NONE'来告诉连接器不要把节
放到任何一个段中。

这儿有一个简单的例子：

    PHDRS { text PT_LOAD ; }
    SECTIONS { .text : { *(.text) } :text }

输出段填充
...................

你可以通过使用'=FILLEXP'为整个节设置填充样式。FILLEXP是一个表达式。任何没有指定的输出段内的内存
区域（比如，因为输入段的对齐要求而产生的裂缝）会被填入这个值。如果填充表达式是一个简单的十六进制
值，比如，一个以'0x'开始的十六进制数字组成的字符串，并且尾部不是'k'或'M'，那一个任意的十六进制数
字长序列可以被用来指定填充样式；前导零也变为样式的一部分。对于所有其他的情况，包含一个附加的括号
或一元操作符'+'，那填充样式是表达式的最低四字节的值。在所有的情况下，数值是big-endian.

你还可以通过在输出节命令中使用'FILL'命令来改变填充值。

这里是一个简单的例子:
    SECTIONS { .text : { *(.text) } =0x90909090 }

覆盖描述
-------------------

一个覆盖描述提供一个简单的描述办法，以描述那些要被作为一个单独内存映像的一部分载入内存，但是却要
在同一内存地址运行的节。在运行时，一些覆盖管理机制会把要被覆盖的节按需要拷入或拷出运行时内存地址，
并且多半是通过简单地处理内存位。 这个方法可能非常有用，比如在一个特定的内存区域比另一个快时。

覆盖是通过‘OVERLAY’命令进行描述。‘OVERLAY’命令在‘SECTIONS’命令中使用，就像输出段描述一样。
‘OVERLAY’命令的完整语法如下：

    OVERLAY [START] : [NOCROSSREFS] [AT ( LDADDR )]
      {
        SECNAME1
          {
            OUTPUT-SECTION-COMMAND
            OUTPUT-SECTION-COMMAND
            ...
          } [:PHDR...] [=FILL]
        SECNAME2
          {
            OUTPUT-SECTION-COMMAND
            OUTPUT-SECTION-COMMAND
            ...
          } [:PHDR...] [=FILL]
        ...
      } [>REGION] [:PHDR...] [=FILL]

除了‘OVERLAY’关键字，所有的都是可选的，每一个节必须有一个名字（上面的SECNAME1和SECNAME2）。在
‘OVERLAY’结构中的节定义跟通常的‘SECTIONS’结构中的节定义是完全相同的，除了一点，就是在‘OVERLAY’
中没有地址跟内存区域的定义。

节都被定义为同一个开始地址。所有节的载入地址都被排布，使它们在内存中从整个'OVERLAY'的载入地址开
始都是连续的（就像普通的节定义，载入地址是可选的，缺省的就是开始地址；开始地址也是可选的，缺省的
是当前的定位计数器的值。）

如果使用了关键字`NOCROSSREFS'， 并且在节之间存在引用，连接器就会报告一个错误。因为节都运行在同一
个地址上，所以一个节直接引用另一个节中的内容是错误的。

对于'OVERLAY'中的每一个节，连接器自动定义两个符号。符号`__load_start_SECNAME'被定义为节的开始载
入地址。符号`__load_stop_SECNAME'被定义为节的最后载入地址。SECNAME中的不符合C规定的任何字符都将
被删除。C（或者汇编语言）代码可能使用这些符号在必要的时间搬移覆盖代码。

在覆盖区域的最后，定位计数器的值被设为覆盖区域的开始地址加上最大的节的长度。

这里是一个例子。记住这只会出现在‘SECTIONS’结构的内部。

      OVERLAY 0x1000 : AT (0x4000)
      {
        .text0 { o1/*.o(.text) }
        .text1 { o2/*.o(.text) }
      }

这段代码会定义'.text0'和'.text1'，它们都从地址0x1000开始。‘.text0'会被载入到地址0x4000处，而
'.text1'会被载入到紧随'.text0'后的位置。下面的几个符号会被定义：`__load_start_text0', 
`__load_stop_text0', `__load_start_text1', `__load_stop_text1'.

拷贝'.text1'到覆盖区域的C代码看上去可能会像下面这样：

      extern char __load_start_text1, __load_stop_text1;
      memcpy ((char *) 0x1000, &__load_start_text1,
              &__load_stop_text1 - &__load_start_text1);

注意'OVERLAY'命令只是为了语法上的便利，因为它所做的所有事情都可以用更加基本的命令加以代替。上面
的例子可以用下面的完全特效的写法：

      .text0 0x1000 : AT (0x4000) { o1/*.o(.text) }
      __load_start_text0 = LOADADDR (.text0);
      __load_stop_text0 = LOADADDR (.text0) + SIZEOF (.text0);
      .text1 0x1000 : AT (0x4000 + SIZEOF (.text0)) { o2/*.o(.text) }
      __load_start_text1 = LOADADDR (.text1);
      __load_stop_text1 = LOADADDR (.text1) + SIZEOF (.text1);
      . = 0x1000 + MAX (SIZEOF (.text0), SIZEOF (.text1));

 
 
  

============

连接器在缺省状态下被配置为允许分配所有可用的内存块。你可以使用‘MEMORY’命令重新配置这个设置。

‘MEMORY’命令描述目标平台上内存块的位置与长度。你可以用它来描述哪些内存区域可以被连接器使用，
哪些内存区域是要避免使用的。然后你就可以把节分配到特定的内存区域中。连接器会基于内存区域设置节
的地址，对于太满的区域，会提示警告信息。连接器不会为了适应可用的区域而搅乱节。

一个连接脚本最多可以包含一次'MEMORY'命令。但是，你可以在命令中随心所欲定义任意多的内存块，语法
如下：

    MEMORY
      {
        NAME [(ATTR)] : ORIGIN = ORIGIN, LENGTH = LEN
        ...
      }

NAME是用在连接脚本中引用内存区域的名字。出了连接脚本，区域名就没有任何实际意义。区域名存储在一个
单独的名字空间中，它不会和符号名，文件名，节名产生冲突，每一块内存区域必须有一个唯一的名字。

ATTR字符串是一个可选的属性列表，它指出是否为一个没有在连接脚本中进行显式映射地输入段使用一个特定
的内存区域。如果你没有为某些输入段指定一个输出段，连接器会创建一个跟输入段同名的输出段。如果你定
义了区域属性，连接器会使用它们来为它创建的输出段选择内存区域。

ATTR字符串必须包含下面字符中的一个，且必须只包含一个：
`R'
只读节。
`W'
    可读写节。
`X'
可执行节。
`A'
可分配节。
`I'
已初始化节。
`L'
    同‘I’
`!'
对前一个属性值取反。

如果一个未映射节匹配了上面除'!'之外的一个属性，它就会被放入该内存区域。'!'属性对该测试取反，所以
只有当它不匹配上面列出的行何属性时，一个未映射节才会被放入到内存区域。

ORIGIN是一个关于内存区域地始地址的表达式。在内存分配执行之前，这个表达式必须被求值产生一个常数，
这意味着你不可以使用任何节相关的符号。关键字'ORIGIN'可以被缩写为'org'或'o'(但是，不可以写为，比
如‘ORG’）

LEN是一个关于内存区域长充（以字节为单位）的表达式。就像ORIGIN表达式，这个表达式在分配执行前也
必须被求得为一个常数值。关键字'LENGTH'可以被简写为‘len'或'l'。

在下面的例子中，我们指定两个可用于分配的内存区域：一个从0开始，有256kb长度，另一个从0x4000000
开始，有4mb长度。连接器会把那些没有进行显式映射且是只读或可执行的节放到'rom'内存区域。并会把另
外的没有被显式映射地节放入到'ram'内存区域。

    MEMORY
      {
        rom (rx)  : ORIGIN = 0, LENGTH = 256K
        ram (!rx) : org = 0x40000000, l = 4M
      }

一旦你定义了一个内存区域，你也可以指示连接器把指定的输出段放入到这个内存区域中，这可以通过使用
'>REGION'输出段属性。比如，如果你有一个名为'mem'的内存区域，你可以在输出段定义中使用'>mem'。如
果没有为输出段指定地址，连接器就会把地址设置为内存区域中的下一个可用的地址。如果总共的映射到一
个内存区域的输出段对于区域来说太大了，连接器会提示一条错误信息。

PHDRS命令
=============

ELF目标文件格式使用“程序头”，它也就是人们熟知的“节”。程序头描述了程序应当如何被载入到内存中。
你可以通过使用带有'-p'选项的‘objdump’命令来打印出这个程序头。

当你在一个纯ELF系统上运行ELF程序时，系统的载入程序通过读取文件头来计算得到如何来载入这个文件。这
只在程序头被正确设置的情况下才会正常工作。本手册并不打算介绍系统载入程序如何解释文件头的相关细节
问题；关于更多信息，请参阅ELF ABI。

连接顺在缺省状态下会自己创建一个可用的程序头。但是，在某些情况下，你可能需要更为精确地指定程序头。
你可以使用命令‘PHDRS’达到这个目的。当连接器在连接脚本中看到‘PHDRS’命令时，它只会创建被指定了
的程序头。

连接器只在产生ELF输出文件时关心‘PHDRS’命令。在其它情况下，连接器只是简单地忽略‘PHDRS’。

下面是‘PHDRS’命令的语法。单词‘PHDRS’，‘FILEHDR’，‘AT’和‘FLAGS’都是关键字。

    PHDRS
    {
      NAME TYPE [ FILEHDR ] [ PHDRS ] [ AT ( ADDRESS ) ]
            [ FLAGS ( FLAGS ) ] ;
    }

NAME只在连接脚本的‘SECTIONS’命令中引用时用到。它不会被放到输出文件中。程序头的名字会被存储到单独
的名字空间中。每一个程序头都必须有一个唯一的名字。

某些特定类型的程序头描述系统载入程序要从文件中载入到内存的节。在连接脚本中，你通过把可载入的输出节放
到段中来指定这些段的内容。你可以使用‘:PHDR’输出节属性把一个节放到一个特定的段中。

把某些节放到多个段中也是正常的。这仅仅暗示了一个内存段中含有另一个段。你可以重复使用‘:PHDR’，在每
一个应当含有这个节的段中使用它一次。

如果你使用‘:PHDR’把一个节放到多个段中，那连接器把随后的所有没有指定‘:PHDR’的可分配节都放到同一个
段中。这是为了方便，因为通常一串连续的节会被放到一个单独的段中。你可以使用‘:NONE’来覆盖缺省的段，
告诉连接器不要把节放到任何一个段中。

你可能在程序头类型后面使用‘FILEHDR’和‘PHDRS’关键字来进一步描述段的内容。‘FILEHDR’关键字表示段应
当包含ELF文件头。‘PHDRS’关键字表示段应当包含ELF程序头本身。

TYPE可以是如下的一个。数字表示关键字的值。

`PT_NULL' (0)
表示一个不用的程序头。

`PT_LOAD' (1)
表示这个程序头描述了一个被从文件中载入的段。

`PT_DYNAMIC' (2)
    表示一个可以从中找到动态链接信息的段。

`PT_INTERP' (3)
  表示一个可以从中找到关于程序名解释的段。

`PT_NOTE' (4)
表示一个存有备注信息的段。

`PT_SHLIB' (5)
  一个保留的程序头类型，被定义了，但没有被ELF ABI指定。

`PT_PHDR' (6)
表示一个可以从中找到程序头的段。

EXPRESSION
一个给出程序头的数值类型的表达式。这可以在使用上面未定义的类型时使用。

你可以通过使用‘AT’表达式指定一个段应当被载入到内存中的一个特定的地址。这跟
在输出节属性中使用‘AT’命令是完全一样的。程序头中的‘AT’命令会覆盖输出节属
性中的。

连接器通常会基于组成段的节来设置段属性。你可以通过使用‘FLAGS’关键字来显式指
定段标志。FLAGS的值必须是一个整型值。它被用来设置程序头的‘p_flags'域。

这里是一个关于‘PHDRS’的例子。它展示一个在纯ELF系统上的一个标准的程序头设置。

    PHDRS
    {
      headers PT_PHDR PHDRS ;
      interp PT_INTERP ;
      text PT_LOAD FILEHDR PHDRS ;
      data PT_LOAD ;
      dynamic PT_DYNAMIC ;
    }
    
    SECTIONS
    {
      . = SIZEOF_HEADERS;
      .interp : { *(.interp) } :text :interp
      .text : { *(.text) } :text
      .rodata : { *(.rodata) } /* defaults to :text */
      ...
      . = . + 0x1000; /* move to a new page in memory */
      .data : { *(.data) } :data
      .dynamic : { *(.dynamic) } :data :dynamic
      ...
    }

VERSION命令
===============

在使用ELF时，连接器支持符号版本。符号版本只在使用共享库时有用。动态连接器在运行一个
可能跟一个更早版本的共享库链接程序时，可以使用符号版本来选择一个函数的特定版本。

你可以直接在主连接脚本中包含一个版本脚本，或者你可以以一个隐式连接脚本的形式提供这个
版本脚本。你也可以使用‘--version-script'连接器选项。

‘VERSION’命令的语法很简单：

    VERSION { version-script-commands }

版本脚本命令的格式跟Sun在Solaris 2.5中的连接器的格式是完全一样的。版本脚本定义一个版本
节点树。你可以在版本脚本中指定节点名和依赖关系。你可以指定哪些符号被绑定到哪些版本节点
上，你还可以把一组指定的符号限定到本地范围，这样在共享库的外面它们就不是全局可见的了。

最简单的演示版本脚本语言的方法是出示几个小例子：

    VERS_1.1 {
    global:
    foo1;
    local:
    old*;
    original*;
    new*;
    };
    
    VERS_1.2 {
    foo2;
    } VERS_1.1;
    
    VERS_2.0 {
    bar1; bar2;
    } VERS_1.2;

这个示例版本脚本定义了三个版本节点。第一个版本节点定义为‘VERS_1.1’它没有其它的依赖。
脚本把符号‘foo1’绑定给‘VERS_1.1’。它把一些数量的符号限定到本地范围，这样它们在共
享库的外面就不可见了；这是通过通配符来完成的，所以任何名字以‘old’，‘original’或
‘new’开头的符号都会被匹配。可用的通配符跟在shell中匹配文件名时一样。

下面，版本脚本定义一个节点‘VER_1.2’。这个节点依赖‘VER_1.1’。脚本把符号‘foo2’绑
定给节点‘VERS_1.2’。

最后，版本脚本定义节点‘VERS_2.0’。这个节点依赖‘VERS_1.2’。脚本把符号‘bar1’和
‘bar2 ’绑定给版本节点‘VERS_2.0’。

当连接器发现一个定义在库中的符号没有被指定绑定到一个版本节点，它会把它绑定到一个未指
定基础版本的库。你可以通过使用‘global: *;’把所有未指定的符号绑定到一个给定的版本节
点上。

版本节点的名字没有任何特殊的含义只是为了方便人们阅读。版本‘2.0’可以出现在‘1.1’和
‘1.2’之间。但是，在书写版本脚本时，这会是一个引起混乱的办法。

如果在版本脚本中，这是一个唯一的版本节点，节点名可以被省略。这样的版本脚本不给符号赋
任何版本，只是选择哪些符号会被全局可见而哪些不会。

    { global: foo; bar; local: *; };

当你把一个程序跟一个带有版本符号的共享库连接时，程序自身知道每个符号的哪个版本是它需
要的，而且它还知道它连接的每一个节享库中哪些版本的节点是它需要的。这样，在运行时，动
态载入程序可以做一个快速的确认，以保证你连接的库确实提供了所有的程序需要用来解析所有
动态符号的版本节点。用这种方法，就有可能让每一个动态连接器知道所有的外部符号不需要通
过搜索每一个符号引用就能解析。

符号版本在SunOS上做次版本确认是一种很成熟的方法。一个被提出来的基本的问题是对于外部
函数的标准引用会在需要时被绑定到正确的版本，但不是在程序启动的时候全部被绑定。如果一
个共享库过期了，一个需要的界面可能就不存在了；当程序需要使用这个界面的时候，它可能会
突然地意外失败。有了符号版本后，当用户启动他们的程序时，如果要使用的共享库太老了的话，
用户会得到一条警告信息。

GNU对Sun的版本确认办法有一些扩展。首先就是能在符号定义的源文件中把一个符号绑定到一个
版本节点而不是在一个版本脚本中。这主要是为了减轻库维护的工作量。你可以通过类似下面的
代码实现这一点：

    __asm__(".symver original_foo,foo@VERS_1.1");

在C源文件中。这句会给函数'original_foo'取一个别名'foo'，并绑定到版本节点`VERS_1.1'。
操作符'local:'可以被用来阻止符号'original_foo'被导出。操作符'.symver'使这句优先于版
本脚本。

第二个GNU的扩展是在一个给定的共享库中允许同一个函数的多个版本。通过这种办法，你可以
不增加共享库的主版本号而对界面做完全不相容的修改。

要实现这个，你必须在一个源文件中多次使用'.symver'操作符。这里是一个例子：

    __asm__(".symver original_foo,foo@");
    __asm__(".symver old_foo,foo@VERS_1.1");
    __asm__(".symver old_foo1,foo@VERS_1.2");
    __asm__(".symver new_foo,foo@@VERS_2.0");

在这个例子中，'foo@'表示把符号'foo'绑定到一个没有指基版本的符号上。含有这个例子的源
文件必须定义4个C函数：`original_foo', `old_foo', `old_foo1', 和`new_foo'.

当你有一个给定符号的多个定义后，有必要有一个方法可以指定一个缺省的版本，对于这个符号
的外部引用就可以找到这个版本。用这种方法，你可以只声明一个符号的一个版本作为缺省版本，
否则，你会拥有同一个符号的多个定义。

如果你想要绑定一个引用到共享库中的符号的一个指定的版本，你可以很方便地使用别名（比如，
old_foo),或者你可以使用'.symver'操作符来指定绑定到一个外部函数的特定版本。

你也可以在版本脚本中指定语言。

    VERSION extern "lang" { version-script-commands }

被支持的'lang'有‘C’，‘C++’和‘Java’。

连接脚本中的表达式
=============================

连接脚本语言中的表达式的语法跟C的表达式是完全是致的。所有的表达式都以整型值被求值。所有
的表达式也被以相同的宽度求值。在32位系统是它是32位，否则是64位。
　　　　
你可以在表达式中使用和设置符号值。

连接器为了使用表达式，定义了几个具有特殊途的内建函数。

常数
---------

所有的常数都是整型值。

就像在C中，连接器把以'0'开头的整型数视为八进制数，把以'0x'或'0X'开头的视为十六进制。连接器
把其它的整型数视为十进制。

另外，你可以使用'K'和'M'后缀作为常数的度量单位，分别为'1024'和'1024*1024'。比如，下面的三个
常数表示同一个值。

    _fourk_1 = 4K;
    _fourk_2 = 4096;
    _fourk_3 = 0x1000;

符号名
------------

除了引用，符号名都是以一个字母，下划线或者句号开始，可以包含字母，数字，下划线，句点和连接号。
不是被引用的符号名必须不和任何关键字冲突。你可以指定一个含有不固定它符数或具有跟关键字相同名
字但符号名必须在双引号内：

    "SECTION" = 9;
    "with a space" = "also with a space" + 10;

因为符号可以含有很多非文字字符，所以以空格分隔符号是很安全的。比如，'A-B'是一个符号，而'A - B'
是一个执行减法运算的表达式。

定位计数器
--------------------

一个特殊的连接器变量"dot"'.'总是含有当前的输出定位计数器。因为'.'总引用输出段中的一个位置，它
只可以出现在'SECTIONS'命令中的表达式中。'.'符号可以出现在表达式中一个普能符号允许出现的任何位
置。

把一个值赋给'.'会让定位计数器产生移动。这会在输出段中产生空洞。定位计数器从不向前移动。

    SECTIONS
    {
      output :
        {
          file1(.text)
          . = . + 1000;
          file2(.text)
          . += 1000;
          file3(.text)
        } = 0x12345678;
    }

在前面的例子中，来自'file1'的'.text'节被定位在输出节'output'的起始位置。它后面跟有1000byte的
空隙。然后是来自'file2'的'.text'节，同样是后面跟有1000byte的空隙，最后是来自'file3'的'.text'
节。符号'=0x12345678'指定在空隙中填入什么样的数据。

注意：'.'实际上引用的是当前包含目标的从开始处的字节偏移。通常，它就是'SECTIONS'语句，其起始地
址是0，因为'.'可以被用作绝对地址。但是如果'.'被用在一个节描述中，它引用的是从这个节起始处开始
的偏移，而不是一个绝对地址。这样，在下面这样一个脚本中：

    SECTIONS
    {
        . = 0x100
        .text: {
          *(.text)
          . = 0x200
        }
        . = 0x500
        .data: {
          *(.data)
          . += 0x600
        }
    }

'.text'节被赋于起始地址0x100，尽管在'.text'输入节中没有足够的数据来填充这个区域，但其长
度还是0x200bytes。（如果数据太多，那会产生一条错误信息，因为这会试图把'.'向前移）。'.data'
节会从0x500处开始，并且它在结尾处还会有0x600的额外空间。

运算符
---------

连接器可以识别标准的C的算术运算符集, 以及它们的优先集.

    优先集        结合性          运算符                  备注
    (highest)
    1              left            !  -  ~                  (1)
    2              left            *  /  %
    3              left            +  -
    4              left            >>  <<
    5              left            ==  !=  >  <  <=  >=
    6              left            &
    7              left            |
    8              left            &&
    9              left            ||
    10              right          ? :
    11              right          &=  +=  -=  *=  /=      (2)
    (lowest)
  注: (1) 前缀运算符 (2) *Note Assignments::.

求值
----------

连接器是懒惰求表达式的值。它只在确实需要的时候去求一个表达式的值。

连接器需要一些信息，比如第一个节的起始地址的值，还有内存区域的起点与长度，在做任何连接的
时候这都需要。在连接器读取连接脚本的时候，这些值在可能的时候被计算出来。

但是，其它的值（比如符号的值）直到内存被分配之后才会知道或需要。这样的值直到其它信息（比
如输出节的长度）可以被用来进行符号赋值的时候才被计算出来。

直到内存分配之后，节的长度才会被知道，所以依赖于节长度的赋值只能到内存分配之后才会被执行。

有些表达式，比如那些依赖于定位计数器'.'的表达式，必须在节分配的过程中被计算出来。

如果一个表达式的结果现在被需要，但是目前得不到这个值，这样会导致一个错误。比如，象下面这
样一个脚本：

    SECTIONS
      {
        .text 9+this_isnt_constant :
          { *(.text) }
      }

会产生一个错误信息'non constant expression for initial address'.

表达式的节
----------------------------

当一个连接器计算一个表达式时，得到的结果可能是一个绝对值，也可能跟某个节相关。一个节相关的
表达式是从一个节的基地址开始的固定的偏称值。

表达式在连接脚本中的位置决定了它是绝对的或节相关的。一个出现在输出节定义中的表达式是跟输出
节的基地址相关的。一个出现在其它地方的表达式则是绝对的。

如果你通过'-r'选项指定需要可重位输出，那一个被赋为节相关的表达式的符号就会是可重定位的。意
思是下一步的连接操作会改变这个符号的值。符号的节就是节相关的表达式所在的节。

一个被赋为绝对表达式的符号在后面进一步的连接操作中会始终保持它的值不变。符号会是绝对的，并
不会有任何的特定的相关节。

如果一个表达式有可能会是节相关的，你可以使用内建函数'ABSOLUTE'强制一个表达式为绝对的。比如，
要创建一个被赋为输出节'.data'的末尾地址的绝对符号：

    SECTIONS
      {
        .data : { *(.data) _edata = ABSOLUTE(.); }
      }

如果没有使用'ABSOLUTE'，'_edata'会跟节'.data'相关。

内建函数
-----------------

为了使用连接脚本表达式，连接脚本语言含有一些内建函数。

`ABSOLUTE(EXP)'
返回表达式EXP的绝对值（不可重定位，而不是非负）。主要在把一个绝对值赋给一个节定义内的
符号时有用。

`ADDR(SECTION)'
返回节SECTION的绝对地址（VMA）。你的脚本之前必须已经定义了这个节的地址。在接下来的例子
中，'symbol_1'和'symbol_2'被赋以相同的值。

        SECTIONS { ...
          .output1 :
            {
            start_of_output_1 = ABSOLUTE(.);
            ...
            }
          .output :
            {
            symbol_1 = ADDR(.output1);
            symbol_2 = start_of_output_1;
            }
        ... }

`ALIGN(EXP)'
返回定位计数器'.'对齐到下一个EXP指定的边界后的值。‘ALIGN’不改变定位计数器的值，它只是
在定位计数器上面作了一个算术运算。这里有一个例子，它在前面的节之后，把输出节'.data'对齐
到下一个'0x2000'字节的边界，并在输入节之后把节内的一个变量对齐到下一个'0x8000'字节的边界。

        SECTIONS { ...
          .data ALIGN(0x2000): {
            *(.data)
            variable = ALIGN(0x8000);
          }
        ... }

这个例子中前一个'ALIGN'指定一个节的位置，因为它是作为节定义的可选项ADDRESS属性出现的。第
二个‘ALIGN’被用来定义一个符号的值。

内建函数'NEXT'跟‘ALIGN’非常相似。

`BLOCK(EXP)'
这是'ALIGN'的同义词，是为了与其它的连接器保持兼容。这在设置输出节的地址时非常有用。

`DATA_SEGMENT_ALIGN(MAXPAGESIZE, COMMONPAGESIZE)'
    这跟下面的两个表达同义：
        (ALIGN(MAXPAGESIZE) + (. & (MAXPAGESIZE - 1)))
    或者：
        (ALIGN(MAXPAGESIZE) + (. & (MAXPAGESIZE - COMMONPAGESIZE)))

隐式连接脚本
=======================

如果你指定了一个连接器输出文件，而连接器不能识别它是一个目标文件还是档案文件，它会试图把它读作
一个连接脚本。如果这个文件不能作为一个连接脚本被分析，连接器就会报告一个错误。

一个隐式的连接器脚本不会替代缺省的连接器脚本。

一般，一个隐式的连接器脚本只包含符号赋值，或者'INPUT','GROUP'或'VERSION'命令。

BFD
***

连接器通过BFD库来对目标文件和档案文件进行操作。这些库允许连接器忽略目标文件的格式而使用相关的
例程来操作目标文件。只要简单地创建一个新的BFD后台并把它加到库中，一个不同的目标文件格式就会被
支持。但是为了节约运行时内存，连接器和相关的工具一般被配置为只支持可用的目标文件格式的一个子集，
你可以使用'objdump -i'来列出你配置的所有支持的格式。

就像大多数的案例，BFD是一个在多种相互有冲突的需求之间的一个折中，影响BFD设计的一个最主要的因
素是效率。因为BFD简化了程序和后台，更多的时间和精力被放在了优化算法以追求更快的速度。

BFD解决方案的一个副产品是你必须记住有信息丢失的潜在可能。在使用BFD机制时，有两处地方有用信息可
能丢失：在转化时和在输出时。

它如何工作: BFD概要。
===============================

当一个目标文件被打开时，BFD子程序自动确定输入目标文件的格式。然后它们在内存中用指向子程序的指针
构建一个描述符，这个描述符被用作存取目标文件的数据结构元素。

因为需要来自目标文件的不同信息，BFD从文件的不同节中读取它们，并处理。比如，连接器的一个非常普遍
的操作是处理符号表。每一个BFD后台提供一个在目标文件的符号表达形式跟内部规范格式之间的转化的函数，
当一个连接器需要一个目标文件的符号表时，它通过一个内存指针调用一个来自相应的BFD后台的子程序，这
个子程序读取表并把它转化为规范表。然后，连接器写输出文件的符号表，另一个BFD后台子程序被调用，以
创建新的符号表并把它转化为选定的输出格式。

信息丢失。
----------------

在输出的过程中，信息可能会被丢失。BFD支持的输出格式并不提供一致的特性，并且在某一种格式中可以被
描述的信息可能在另一种格式中没有地方可放。一个例子是在'b.out'中的对齐信息，在一个'a.out'格式的
文件中，没有地方可以存储对齐信息，所以当一个文件是从'b.out'连接而成的，并产生的是一个'a.out'的
文件，对齐信息就不会被传入到输出文件中（连接器还是在内部使用对齐信息，所以连接器的执行还是正确的）

另一个例子是COFF节名字。COFF文件中可以含有不限数量的节，每一个都有一个文字的节名。如果连接的目标是
一种不支持过多节的格式（比如，'a.out'）或者是一种不含有节名的格式（比如，Oasys格式），连接器不
能像通常那样简单地处理它。你可以通过把所需的输入输出节通过连接脚本语言进行详细映射来解决这下问题。

在规范化的过程中信息也会丢失。BFD内部的对应于外部格式的规范形式并不是完全详尽的；有些在输入格式
中的结构在内部并没有对应的表示方法。这意味着BFD后台在从外部到内部或从内部到外部的转化过程中不能
维护所有可能的数据。

这个限制只在一个程序读取一种格式并写成另一种格式的时候会是一个问题。每一个BFD后台有责任维护尽可能
多的数据，内部的BFD规范格式具有对BFD内核不透明的结构体，只导出给后台。当一个文件以一种格式读取后，
规范格式就会为之产生。同时，后台把所有可能丢失的信息进行存储。如果这些数据随后会写以相同的格式写
回，后台程序就可以使用BFD内核提供的跟选前准备的相同的规范格式。因为在后台之间有大量相同的东西，在
把big endianCOFF拷贝成littile endian COFF时，或者'a.out'到'b.out'时，不会有信息丢失。当一些混合格
式被连接到一起时，只有那些格式跟目标格式不同的文件会丢失信息。 