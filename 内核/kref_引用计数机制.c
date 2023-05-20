	kref是一个引用计数器，它被嵌套进其它的结构中，记录所嵌套结构的引用计数，并在计数清零时调用相应的清理函数。kref的原理和实现都非常简单，但要想用好却不容易，或者说kref被创建就是为了跟踪复杂情况下地结构引用销毁情况。

	在Linux内核里，引用计数是通过struct kref结构来实现的。
struct kref {
    atomic_t refcount;
};
	要使用kref，那么需要在自己的结构体中加上这么一个成员，如下：
struct my_data
{
    .   
	.   
	struct kref refcount;
	.   
	.   
};

	在使用kref之前，必须先使用 kref_init() 函数进行初始化，该初始化会将引用计数的值设置为1:
{
	struct my_data *data;
	data = kmalloc(sizeof(*data), GFP_KERNEL);
	if (!data)
		return -ENOMEM;
	kref_init(&data->refcount);
}
	其他相关函数：
	void kref_get(struct kref *kref) //增加引用计数
	int kref_put(struct kref *kref, void (*release)(struct kref *kref)) //减少引用计数，release函数是引用计数减到0时会自动调用的函数，一般用来释放结构体占用的内存。
	int kref_put_spinlock_irqsave(struct kref *kref, void (*release)(struct kref *kref), spinlock_t *lock)
	int kref_put_mutex(struct kref *kref, void (*release)(struct kref *kref), struct mutex *lock)
	int __must_check kref_get_unless_zero(struct kref *kref)

	在初始化kref之后，要遵守如下规则：
	（1）如果kref所在的结构体被当作参数传递，特别是传递给另一个线程执行时，那么在传递之前必须使用kref_get()函数。
	（2）kref_get()和kref_put()必须成对使用。
	（3）If the code attempts to gain a reference to a kref-ed structure without already holding a valid pointer, it must serialize access where a kref_put() cannot occur during the kref_get(), and the structure must remain valid during the kref_get().
	
