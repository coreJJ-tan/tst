1. 无读写接口的文件
static const struct kernfs_ops sysfs_file_kfops_empty = { // 不可读不可写
};

2. 文本文件的读写接口
sysfs 的文本文件最大只能是一个 PAGE_SIZE, 即 1024 个字节

static const struct kernfs_ops sysfs_file_kfops_ro = {  // 只读，上层的 sysfs_ops 只定义 ->show() 函数
	.seq_show	= sysfs_kf_seq_show,
};
static const struct kernfs_ops sysfs_file_kfops_wo = {  // 只写，上层的 sysfs_ops 只定义 ->store() 函数
	.write		= sysfs_kf_write,
};
static const struct kernfs_ops sysfs_file_kfops_rw = {  // 可读可写，上层的 sysfs_ops 定义了 ->show() 和 ->store() 函数
	.seq_show	= sysfs_kf_seq_show,
	.write		= sysfs_kf_write,
};
static const struct kernfs_ops sysfs_prealloc_kfops_ro = {  // 只读，文件的操作模式 attribute->mode & 010000 != 0, 上层的 sysfs_ops 只定义 ->show() 函数
	.read		= sysfs_kf_read,
	.prealloc	= true, // prealloc 的作用是啥？
};
static const struct kernfs_ops sysfs_prealloc_kfops_wo = {  // 只写，文件的操作模式 attribute->mode & 010000 != 0, 上层的 sysfs_ops 只定义 ->store() 函数
	.write		= sysfs_kf_write,
	.prealloc	= true,
};
static const struct kernfs_ops sysfs_prealloc_kfops_rw = {  // 可读可写，文件的操作模式 attribute->mode & 010000 != 0, 上层的 sysfs_ops 定义了 ->show() 和 ->store() 函数
	.read		= sysfs_kf_read,
	.write		= sysfs_kf_write,
	.prealloc	= true,
};

3. 二进制文件的读写接口
sysfs 的二进制文件大小由 bin_attribute->size 定义

static const struct kernfs_ops sysfs_bin_kfops_ro = {  // 只读，上层的 bin_attribute 只定义 ->read() 函数
	.read		= sysfs_kf_bin_read,
};
static const struct kernfs_ops sysfs_bin_kfops_wo = {  // 只写，上层的 bin_attribute 只定义 ->write() 函数
	.write		= sysfs_kf_bin_write,
};
static const struct kernfs_ops sysfs_bin_kfops_rw = {  // 可读可写，上层的 bin_attribute 定义了 ->read() 和 ->write() 函数
	.read		= sysfs_kf_bin_read,
	.write		= sysfs_kf_bin_write,
};
static const struct kernfs_ops sysfs_bin_kfops_mmap = {
	.read		= sysfs_kf_bin_read,
	.write		= sysfs_kf_bin_write,
	.mmap		= sysfs_kf_bin_mmap,
};


/*
 * Reads on sysfs are handled through seq_file, which takes care of hairy
 * details like buffering and seeking.  The following function pipes
 * sysfs_ops->show() result through seq_file.
 */
static int sysfs_kf_seq_show(struct seq_file *sf, void *v)
{
	struct kernfs_open_file *of = sf->private;
	struct kobject *kobj = of->kn->parent->priv;
	const struct sysfs_ops *ops = sysfs_file_ops(of->kn);
	ssize_t count;
	char *buf;

	/* acquire buffer and ensure that it's >= PAGE_SIZE and clear */
	count = seq_get_buf(sf, &buf);
	if (count < PAGE_SIZE) {
		seq_commit(sf, -1);
		return 0;
	}
	memset(buf, 0, PAGE_SIZE);

	/*
	 * Invoke show().  Control may reach here via seq file lseek even
	 * if @ops->show() isn't implemented.
	 */
	if (ops->show) {
		count = ops->show(kobj, of->kn->priv, buf);
		if (count < 0)
			return count;
	}

	/*
	 * The code works fine with PAGE_SIZE return but it's likely to
	 * indicate truncated result or overflow in normal use cases.
	 */
	if (count >= (ssize_t)PAGE_SIZE) {
		print_symbol("fill_read_buffer: %s returned bad count\n",
			(unsigned long)ops->show);
		/* Try to struggle along */
		count = PAGE_SIZE - 1;
	}
	seq_commit(sf, count);
	return 0;
}

static ssize_t sysfs_kf_bin_read(struct kernfs_open_file *of, char *buf,
				 size_t count, loff_t pos)
{
	struct bin_attribute *battr = of->kn->priv;
	struct kobject *kobj = of->kn->parent->priv;
	loff_t size = file_inode(of->file)->i_size;

	if (!count)
		return 0;

	if (size) {
		if (pos > size)
			return 0;
		if (pos + count > size)
			count = size - pos;
	}

	if (!battr->read)
		return -EIO;

	return battr->read(of->file, kobj, battr, buf, pos, count);
}

/* kernfs read callback for regular sysfs files with pre-alloc */
static ssize_t sysfs_kf_read(struct kernfs_open_file *of, char *buf,
			     size_t count, loff_t pos)
{
	const struct sysfs_ops *ops = sysfs_file_ops(of->kn);
	struct kobject *kobj = of->kn->parent->priv;

	/*
	 * If buf != of->prealloc_buf, we don't know how
	 * large it is, so cannot safely pass it to ->show
	 */
	if (pos || WARN_ON_ONCE(buf != of->prealloc_buf))
		return 0;
	return ops->show(kobj, of->kn->priv, buf);
}

/* kernfs write callback for regular sysfs files */
static ssize_t sysfs_kf_write(struct kernfs_open_file *of, char *buf,
			      size_t count, loff_t pos)
{
	const struct sysfs_ops *ops = sysfs_file_ops(of->kn);
	struct kobject *kobj = of->kn->parent->priv;

	if (!count)
		return 0;

	return ops->store(kobj, of->kn->priv, buf, count);
}

/* kernfs write callback for bin sysfs files */
static ssize_t sysfs_kf_bin_write(struct kernfs_open_file *of, char *buf,
				  size_t count, loff_t pos)
{
	struct bin_attribute *battr = of->kn->priv;
	struct kobject *kobj = of->kn->parent->priv;
	loff_t size = file_inode(of->file)->i_size;

	if (size) {
		if (size <= pos)
			return -EFBIG;
		count = min_t(ssize_t, count, size - pos);
	}
	if (!count)
		return 0;

	if (!battr->write)
		return -EIO;

	return battr->write(of->file, kobj, battr, buf, pos, count);
}

static int sysfs_kf_bin_mmap(struct kernfs_open_file *of,
			     struct vm_area_struct *vma)
{
	struct bin_attribute *battr = of->kn->priv;
	struct kobject *kobj = of->kn->parent->priv;

	return battr->mmap(of->file, kobj, battr, vma);
}