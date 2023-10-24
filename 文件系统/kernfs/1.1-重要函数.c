static inline struct kernfs_node *kernfs_create_file_ns(struct kernfs_node *parent, const char *name, umode_t mode,
                                                        loff_t size, const struct kernfs_ops *ops, void *priv, const void *ns)
{   // 创建一个文件，并返回其 kernfs_node 节点
	struct lock_class_key *key = NULL;
#ifdef CONFIG_DEBUG_LOCK_ALLOC
	key = (struct lock_class_key *)&ops->lockdep_key;
#endif
	return __kernfs_create_file(parent, name, mode, size, ops, priv, ns, key);
}
/**
 * __kernfs_create_file - kernfs internal function to create a file
 * @parent: directory to create the file in
 * @name: name of the file
 * @mode: mode of the file
 * @size: size of the file
 * @ops: kernfs operations for the file
 * @priv: private data for the file
 * @ns: optional namespace tag of the file
 * @key: lockdep key for the file's active_ref, %NULL to disable lockdep
 *
 * Returns the created node on success, ERR_PTR() value on error.
 */
struct kernfs_node *__kernfs_create_file(struct kernfs_node *parent, const char *name, umode_t mode, loff_t size, const struct kernfs_ops *ops,
					                    void *priv, const void *ns, struct lock_class_key *key)
{
	struct kernfs_node *kn;
	unsigned flags;
	int rc;
	flags = KERNFS_FILE; // 表明创建的是文件
	kn = kernfs_new_node(parent, name, (mode & S_IALLUGO) | S_IFREG, flags);
	if (!kn)
		return ERR_PTR(-ENOMEM);
	kn->attr.ops = ops; // 设置文件的操作集 kernfs_ops
	kn->attr.size = size;   // 设置文件的大小
	kn->ns = ns;
	kn->priv = priv;    // 文件的私有数据
#ifdef CONFIG_DEBUG_LOCK_ALLOC
	if (key) {
		lockdep_init_map(&kn->dep_map, "s_active", key, 0);
		kn->flags |= KERNFS_LOCKDEP;
	}
#endif
	/* kn->attr.ops is accesible only while holding active ref.  We need to know whether some ops are implemented outside active ref.  Cache their existence in flags. */
	if (ops->seq_show) // 如果上层使用 kernfs 时定义了 kernfs_ops->seq_show() 函数，那么后续进行 read 时，调用的是 seq_read() 函数，否则调用 kernfs_file_direct_read() 函数
		kn->flags |= KERNFS_HAS_SEQ_SHOW;
	if (ops->mmap)
		kn->flags |= KERNFS_HAS_MMAP;
	rc = kernfs_add_one(kn);
	if (rc) {
		kernfs_put(kn);
		return ERR_PTR(rc);
	}
	return kn;
}

