**仅针对32位arm CPU

1、原子变量相关的结构体
typedef struct {
    int counter;
} atomic_t;
    从结构体中的成员可知，原子变量保护的是一个 int 类型的变量，对于其他类型，并不支持。

2、定义并初始化原子变量
2.1 方式一  仅定义，还未初始化
    atomic_t a; // 定义原子变量 a，但还未初始化

2.2 方式二  定义的同时进行初始化
    atomic_t b = ATOMIC_INIT(0); // 定义原子变量 b 并赋初值为 0


3、原子整型操作API 
int atomic_read(atomic_t *v) // 读取 v 的值，并且返回。
void atomic_set(atomic_t *v, int i) // 向 v 写入 i 值。
void atomic_add(int i, atomic_t *v) // 给 v 加上 i 值。
void atomic_sub(int i, atomic_t *v) // 从 v 减去 i 值。
void atomic_inc(atomic_t *v) // 给 v 加 1，也就是自增。
void atomic_dec(atomic_t *v) // 从 v 减 1，也就是自减
int atomic_dec_return(atomic_t *v) // 从 v 减 1，并且返回 v 的值。
int atomic_inc_return(atomic_t *v) // 给 v 加 1，并且返回 v 的值。
int atomic_sub_and_test(int i, atomic_t *v) // 从 v 减 i，如果结果为 0 就返回真，否则返回假
int atomic_dec_and_test(atomic_t *v) // 从 v 减 1，如果结果为 0 就返回真，否则返回假
int atomic_inc_and_test(atomic_t *v) // 给 v 加 1，如果结果为 0 就返回真，否则返回假
int atomic_add_negative(int i, atomic_t *v) // 给 v 加 i，如果结果为负就返回真，否则返回假

4、原子位操作API
void set_bit(int nr, void *p) // 将 p 地址的第 nr 位置 1。
void clear_bit(int nr,void *p) // 将 p 地址的第 nr 位清零。
void change_bit(int nr, void *p) // 将 p 地址的第 nr 位进行翻转。
int test_bit(int nr, void *p) // 获取 p 地址的第 nr 位的值。
int test_and_set_bit(int nr, void *p) // 将 p 地址的第 nr 位置 1，并且返回 nr 位原来的值。
int test_and_clear_bit(int nr, void *p) // 将 p 地址的第 nr 位清零，并且返回 nr 位原来的值。
int test_and_change_bit(int nr, void *p) // 将 p 地址的第 nr 位翻转，并且返回 nr 位原来的值。

    原子位操作API的第二个形参是void *类型的，也就是说原子位操作的数据类型不局限于atomic_t类型，可以保护 p 地址的内存的第 nr 位