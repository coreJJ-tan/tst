1、信号量结构体
struct semaphore {
    raw_spinlock_t lock;
    unsigned int count;
    struct list_head wait_list;
};


2、信号量操作API
DEFINE_SEAMPHORE(name) // 定义一个信号量，并且设置信号量的值为 1。
void sema_init(struct semaphore *sem, int val) // 初始化信号量 sem，设置信号量值为 val。
void down(struct semaphore *sem) // 获取信号量，因为会导致休眠，因此不能在中断中使用。
int down_trylock(struct semaphore *sem); // 尝试获取信号量，如果能获取到信号量就获取，并且返回 0。如果不能就返回非 0，并且不会进入休眠。
int down_interruptible(struct semaphore *sem) // 获取信号量，和 down 类似，只是使用 down 进入休眠状态的线程不能被信号打断。而使用此函数进入休眠以后是可以被信号打断的。
void up(struct semaphore *sem) // 释放信号量