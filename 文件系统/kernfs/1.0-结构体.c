/*
 * kernfs_node - the building block of kernfs hierarchy.  Each and every
 * kernfs node is represented by single kernfs_node.  Most fields are
 * private to kernfs and shouldn't be accessed directly by kernfs users.
 *
 * As long as s_count reference is held, the kernfs_node itself is
 * accessible.  Dereferencing elem or any other outer entity requires
 * active reference.
 */
struct kernfs_node { // 每个 kernfs_node 都代表着 kernfs 的一个节点，这个节点可能是一个文件、一个目录或者一个链接
                     // 大多数成员仅由 kernfs 访问，任何基于 kernfs 的文件系统都不应该访问它们
	atomic_t		count;	// 引用计数
	atomic_t		active;	// 活动的引用计数
#ifdef CONFIG_DEBUG_LOCK_ALLOC
	struct lockdep_map	dep_map;
#endif
	/*
	 * Use kernfs_get_parent() and kernfs_name/path() instead of
	 * accessing the following two fields directly.  If the node is
	 * never moved to a different parent, it is safe to access the
	 * parent directly.
	 */
	struct kernfs_node	*parent;	// 指向父节点
	const char		*name;	// 父节点的名字

	struct rb_node		rb;	// 接入 sysfs 红黑树的链接项

	const void		*ns;	/* namespace tag */
	unsigned int		hash;	/* ns + name hash */	// 红黑树的 key
	union { // 共用体，实际用哪个成员需要调用者自己分辨
		struct kernfs_elem_dir		dir;
		struct kernfs_elem_symlink	symlink;
		struct kernfs_elem_attr		attr;
	};

	void			*priv;

	unsigned short		flags;	o{// 标记位：目录、链接、文件是否已经被删除，取值如下：
		enum kernfs_node_flag {
			KERNFS_ACTIVATED    = 0x0010,
			KERNFS_NS       = 0x0020,
			KERNFS_HAS_SEQ_SHOW = 0x0040,
			KERNFS_HAS_MMAP     = 0x0080,
			KERNFS_LOCKDEP      = 0x0100,
			KERNFS_SUICIDAL     = 0x0400,
			KERNFS_SUICIDED     = 0x0800,
			KERNFS_EMPTY_DIR    = 0x1000,
		};
	}
	umode_t			mode;	// 该节点的访问权限
	unsigned int		ino;	// 唯一编号
	struct kernfs_iattrs	*iattr;	// 用于设置非默认的 inode 属性，如果没有则置为 NULL
};
/* type-specific structures for kernfs_node union members */
struct kernfs_elem_dir {	// 目录
	unsigned long		subdirs;
	/* children rbtree starts here and goes through kn->rb */
	struct rb_root		children;

	/*
	 * The kernfs hierarchy this directory belongs to.  This fits
	 * better directly in kernfs_node but is here to save space.
	 */
	struct kernfs_root	*root;
};

struct kernfs_elem_symlink { // 链接
	struct kernfs_node	*target_kn;	// 该链接指向的 kernfs_node 节点
};

struct kernfs_elem_attr { // 文件
	const struct kernfs_ops	*ops;	// 文件的操作函数集合
	struct kernfs_open_node	*open;
	loff_t			size;	// 文件的大小
	struct kernfs_node	*notify_next;	/* for kernfs_notify() */
};

/*
 * kernfs_syscall_ops may be specified on kernfs_create_root() to support
 * syscalls.  These optional callbacks are invoked on the matching syscalls
 * and can perform any kernfs operations which don't necessarily have to be
 * the exact operation requested.  An active reference is held for each
 * kernfs_node parameter.
 */
struct kernfs_syscall_ops {
	int (*remount_fs)(struct kernfs_root *root, int *flags, char *data);
	int (*show_options)(struct seq_file *sf, struct kernfs_root *root);

	int (*mkdir)(struct kernfs_node *parent, const char *name,
		     umode_t mode);
	int (*rmdir)(struct kernfs_node *kn);
	int (*rename)(struct kernfs_node *kn, struct kernfs_node *new_parent,
		      const char *new_name);
};

struct kernfs_root {
	/* published fields */
	struct kernfs_node	*kn;
	unsigned int		flags;	o{ // 取值如下：/* KERNFS_ROOT_* flags */
		/* @flags for kernfs_create_root() */
		enum kernfs_root_flag {
			/*
			* kernfs_nodes are created in the deactivated state and invisible.
			* They require explicit kernfs_activate() to become visible.  This
			* can be used to make related nodes become visible atomically
			* after all nodes are created successfully.
			*/
			KERNFS_ROOT_CREATE_DEACTIVATED		= 0x0001,

			/*
			* For regular flies, if the opener has CAP_DAC_OVERRIDE, open(2)
			* succeeds regardless of the RW permissions.  sysfs had an extra
			* layer of enforcement where open(2) fails with -EACCES regardless
			* of CAP_DAC_OVERRIDE if the permission doesn't have the
			* respective read or write access at all (none of S_IRUGO or
			* S_IWUGO) or the respective operation isn't implemented.  The
			* following flag enables that behavior.
			*/
			KERNFS_ROOT_EXTRA_OPEN_PERM_CHECK	= 0x0002,
		};
	}

	/* private fields, do not use outside kernfs proper */
	struct ida		ino_ida;
	struct kernfs_syscall_ops *syscall_ops;

	/* list of kernfs_super_info of this root, protected by kernfs_mutex */
	struct list_head	supers;

	wait_queue_head_t	deactivate_waitq;
};

struct kernfs_open_file { // 每一个打开的文件都对应这么一个结构体
	/* published fields */
	struct kernfs_node	*kn;
	struct file		*file;	// 这个打开的文件对应的 file 结构体
	void			*priv;

	/* private fields, do not use outside kernfs proper */
	struct mutex		mutex;
	int			event; // 读取时，其值为其从属的 kernfs_open_node->event 成员
	struct list_head	list;   // 链表节点，链表头是 kernfs_open_node->files
	char			*prealloc_buf; // 读取时申请该内存，随后填充并拷贝其内容到用户空间
								   // 写入时申请内存，其内容由用户空间拷贝而来
	size_t			atomic_write_len; // 允许用户写入的最大字节数？如果不设置，则最大为 PAGE_SIZE ，即 1024 字节
	bool			mmapped;
	const struct vm_operations_struct *vm_ops;
};

struct kernfs_open_node { // 一连串打开的文件，即 kernfs_open_file 的组合
	atomic_t		refcnt;
	atomic_t		event;
	wait_queue_head_t	poll;
	struct list_head	files;  // 链表头，链接一连串打开的文件的结构体 kernfs_open_file，链接点是 kernfs_open_file->list
};

struct kernfs_ops {
	/*
	 * Read is handled by either seq_file or raw_read().
	 *
	 * If seq_show() is present, seq_file path is active.  Other seq
	 * operations are optional and if not implemented, the behavior is
	 * equivalent to single_open().  @sf->private points to the
	 * associated kernfs_open_file.
	 *
	 * read() is bounced through kernel buffer and a read larger than
	 * PAGE_SIZE results in partial operation of PAGE_SIZE.
	 */
	int (*seq_show)(struct seq_file *sf, void *v);

	void *(*seq_start)(struct seq_file *sf, loff_t *ppos);
	void *(*seq_next)(struct seq_file *sf, void *v, loff_t *ppos);
	void (*seq_stop)(struct seq_file *sf, void *v);

	ssize_t (*read)(struct kernfs_open_file *of, char *buf, size_t bytes, loff_t off);

	/*
	 * write() is bounced through kernel buffer.  If atomic_write_len
	 * is not set, a write larger than PAGE_SIZE results in partial
	 * operations of PAGE_SIZE chunks.  If atomic_write_len is set,
	 * writes upto the specified size are executed atomically but
	 * larger ones are rejected with -E2BIG.
	 */
	size_t atomic_write_len;
	/*
	 * "prealloc" causes a buffer to be allocated at open for
	 * all read/write requests.  As ->seq_show uses seq_read()
	 * which does its own allocation, it is incompatible with
	 * ->prealloc.  Provide ->read and ->write with ->prealloc.
	 */
	bool prealloc;
	ssize_t (*write)(struct kernfs_open_file *of, char *buf, size_t bytes,
			 loff_t off);

	int (*mmap)(struct kernfs_open_file *of, struct vm_area_struct *vma);

#ifdef CONFIG_DEBUG_LOCK_ALLOC
	struct lock_class_key	lockdep_key;
#endif
};