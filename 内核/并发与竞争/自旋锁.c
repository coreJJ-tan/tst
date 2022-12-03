1、概念
    自旋锁（spinlock）：是指当一个线程在获取锁的时候，如果锁已经被其它线程获取，那么该线程将循环等待，然后不断的判断锁是否能够被成功获取，直到获取到锁才会退出循环。

    注意上面讲到的“循环等待”，这就意味着会浪费处理器时间，降低系统性能，所以自旋锁的持有时间不能太长。

2、自旋锁相关的结构体
typedef struct spinlock {
    union {
        struct raw_spinlock rlock;

#ifdef CONFIG_DEBUG_LOCK_ALLOC
# define LOCK_PADSIZE (offsetof(struct raw_spinlock, dep_map))
        struct {
            u8 __padding[LOCK_PADSIZE];
            struct lockdep_map dep_map;
        };
#endif
    };
} spinlock_t;

3、自旋锁定义
2.1 方式一  仅定义，还未初始化
    spinlock_t lock; // 定义自旋锁,但还未初始化
    int spin_lock_init(spinlock_t *lock) // 初始化自旋锁。

2.2 方式二  定义的同时进行初始化
    DEFINE_SPINLOCK(spinlock_t lock); // 定义自旋锁并初始化
    
    定义并初始化好自旋锁变量以后就可以使用相应的 API 函数来操作自旋锁。

4、自旋锁基础操作API
void spin_lock(spinlock_t *lock) // 获取指定的自旋锁，也叫做加锁。
void spin_unlock(spinlock_t *lock) // 释放指定的自旋锁。
int spin_trylock(spinlock_t *lock) // 尝试获取指定的自旋锁，如果没有获取到就返回 0
int spin_is_locked(spinlock_t *lock) // 检查指定的自旋锁是否被获取，如果没有被获取就返回非 0，否则返回 0。

5、线程与中断并发访问处理 API 函数
    中断里面可以使用自旋锁，如果自旋锁在中断中使用，那么就需要下面的API
void spin_lock_irq(spinlock_t *lock) // 禁止本地中断，并获取自旋锁。
void spin_unlock_irq(spinlock_t *lock) // 激活本地中断，并释放自旋锁。
void spin_lock_irqsave(spinlock_t *lock, unsigned long flags) // 保存中断状态，禁止本地中断，并获取自旋锁。
void spin_unlock_irqrestore(spinlock_t *lock, unsigned long flags) // 将中断状态恢复到以前的状态，并且激活本地中断，释放自旋锁

6、下半部竞争处理API
    下半部(BH)也会竞争共享资源
void spin_lock_bh(spinlock_t *lock) // 关闭下半部，并获取自旋锁。
void spin_unlock_bh(spinlock_t *lock) // 打开下半部，并释放自旋锁。

7、死锁
    被自旋锁保护的临界区一定不能调用任何能够引起睡眠和阻塞的API 函数，否则的话会可能会导致死锁现象的发生。
    
8、其他自旋锁
8.1 读写自旋锁

8.2 顺序锁
