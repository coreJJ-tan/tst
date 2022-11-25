uboot使用宏U_BOOT_CMD来定义命令，宏 U_BOOT_CMD 定义在文件 include/command.h 中，定义如下：
#define U_BOOT_CMD(_name, _maxargs, _rep, _cmd, _usage, _help)      \
    U_BOOT_CMD_COMPLETE(_name, _maxargs, _rep, _cmd, _usage, _help, NULL)

    具体功能是定义一个struct cmd_tbl_s的结构体变量，U_BOOT_CMD宏传递的参数是该结构体变量的成员变量。通过U_BOOT_CMD定义的变量会通过脚本链接到.u_boot_list段中，
然后可以通过find_cmd遍历这个段找到这个cmd，可以通过run_command(cmd, flag)函数执行具体命令。

在此仅了解如何使用即可，先不深究内部是如何实现的。
U_BOOT_CMD宏参数解释：
_name： 命令名，非字符串，但在U_BOOT_CMD中用“#”符号转化为字符串
_maxargs： 命令的最大参数个数
_rep： 是否自动重复（按Enter键是否会重复执行）
_cmd： 该命令对应的响应函数指针
_usage： 简短的使用说明（字符串）
_help： 较详细的使用说明（字符串）

其中，_cmd 的定义为 int do_xxx(cmd_tbl_t *cmdtp, int flag, int argc, char *const argv[])