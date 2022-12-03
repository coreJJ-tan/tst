1、互斥体结构体
struct mutex {
    /* 1: unlocked, 0: locked, negative: locked, possible waiters */
    atomic_t count;
    spinlock_t wait_lock;
};

2、互斥体 API 
DEFINE_MUTEX(name) // 定义并初始化一个 mutex 变量。
void mutex_init(mutex *lock) // 初始化 mutex。
void mutex_lock(struct mutex *lock) // 获取 mutex，也就是给 mutex 上锁。如果获取不到就进休眠。
void mutex_unlock(struct mutex *lock) // 释放 mutex，也就给 mutex 解锁。
int mutex_trylock(struct mutex *lock) // 尝试获取 mutex，如果成功就返回 1，如果失败就返回 0。
int mutex_is_locked(struct mutex *lock) // 判断 mutex 是否被获取，如果是的话就返回1，否则返回 0。
int mutex_lock_interruptible(struct mutex *lock) // 使用此函数获取信号量失败进入休眠以后可以被信号打断。