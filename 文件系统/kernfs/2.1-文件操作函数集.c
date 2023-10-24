const struct file_operations kernfs_file_fops = { 
	.read		= kernfs_fop_read,
	.write		= kernfs_fop_write,
	.llseek		= generic_file_llseek,
	.mmap		= kernfs_fop_mmap,
	.open		= kernfs_fop_open,
	.release	= kernfs_fop_release,
	.poll		= kernfs_fop_poll,
};
static int kernfs_fop_open(struct inode *inode, struct file *file)
{
	struct kernfs_node *kn = file->f_path.dentry->d_fsdata;
	struct kernfs_root *root = kernfs_root(kn);
	const struct kernfs_ops *ops;
	struct kernfs_open_file *of;
	bool has_read, has_write, has_mmap;
	int error = -EACCES;
	if (!kernfs_get_active(kn))
		return -ENODEV;
	ops = kernfs_ops(kn);
	has_read = ops->seq_show || ops->read || ops->mmap; // 有读函数
	has_write = ops->write || ops->mmap; // 有写函数
	has_mmap = ops->mmap;   // 有 mmap 函数
	/* see the flag definition for details */
	if (root->flags & KERNFS_ROOT_EXTRA_OPEN_PERM_CHECK) {
		if ((file->f_mode & FMODE_WRITE) && (!(inode->i_mode & S_IWUGO) || !has_write))
			goto err_out;
		if ((file->f_mode & FMODE_READ) && (!(inode->i_mode & S_IRUGO) || !has_read))
			goto err_out;
	}
	/* allocate a kernfs_open_file for the file */
	error = -ENOMEM;
	of = kzalloc(sizeof(struct kernfs_open_file), GFP_KERNEL);
	if (!of)
		goto err_out;
	/*
	 * The following is done to give a different lockdep key to
	 * @of->mutex for files which implement mmap.  This is a rather
	 * crude way to avoid false positive lockdep warning around
	 * mm->mmap_sem - mmap nests @of->mutex under mm->mmap_sem and
	 * reading /sys/block/sda/trace/act_mask grabs sr_mutex, under
	 * which mm->mmap_sem nests, while holding @of->mutex.  As each
	 * open file has a separate mutex, it's okay as long as those don't
	 * happen on the same file.  At this point, we can't easily give
	 * each file a separate locking class.  Let's differentiate on
	 * whether the file has mmap or not for now.
	 *
	 * Both paths of the branch look the same.  They're supposed to
	 * look that way and give @of->mutex different static lockdep keys.
	 */
	if (has_mmap)
		mutex_init(&of->mutex);
	else
		mutex_init(&of->mutex);

	of->kn = kn;
	of->file = file;

	/*
	 * Write path needs to atomic_write_len outside active reference.
	 * Cache it in open_file.  See kernfs_fop_write() for details.
	 */
	of->atomic_write_len = ops->atomic_write_len;

	error = -EINVAL;
	/*
	 * ->seq_show is incompatible with ->prealloc,
	 * as seq_read does its own allocation.
	 * ->read must be used instead.
	 */
	if (ops->prealloc && ops->seq_show)
		goto err_free;
	if (ops->prealloc) {
		int len = of->atomic_write_len ?: PAGE_SIZE;
		of->prealloc_buf = kmalloc(len + 1, GFP_KERNEL);
		error = -ENOMEM;
		if (!of->prealloc_buf)
			goto err_free;
	}

	/*
	 * Always instantiate seq_file even if read access doesn't use
	 * seq_file or is not requested.  This unifies private data access
	 * and readable regular files are the vast majority anyway.
	 */
	if (ops->seq_show)
		error = seq_open(file, &kernfs_seq_ops);
	else
		error = seq_open(file, NULL);
	if (error)
		goto err_free;

	((struct seq_file *)file->private_data)->private = of;

	/* seq_file clears PWRITE unconditionally, restore it if WRITE */
	if (file->f_mode & FMODE_WRITE)
		file->f_mode |= FMODE_PWRITE;

	/* make sure we have open node struct */
	error = kernfs_get_open_node(kn, of);   // 将 kernfs_open_file 加入 kernfs_open_node 链表中，如果还没申请过 kernfs_open_node，申请一个
	if (error)
		goto err_close;

	/* open succeeded, put active references */
	kernfs_put_active(kn);
	return 0;

err_close:
	seq_release(inode, file);
err_free:
	kfree(of->prealloc_buf);
	kfree(of);
err_out:
	kernfs_put_active(kn);
	return error;
}
static int kernfs_fop_release(struct inode *inode, struct file *filp)
{
	struct kernfs_node *kn = filp->f_path.dentry->d_fsdata;
	struct kernfs_open_file *of = kernfs_of(filp);

	kernfs_put_open_node(kn, of); // 将 kernfs_open_file 从 kernfs_open_node 链表中删除，如果 kernfs_open_node 中没有 kernfs_open_file 了，则释放 kernfs_open_node 的内存
	seq_release(inode, filp);
	kfree(of->prealloc_buf);
	kfree(of);

	return 0;
}
/**
 * kernfs_fop_read - kernfs vfs read callback
 * @file: file pointer
 * @user_buf: data to write
 * @count: number of bytes
 * @ppos: starting offset
 */
static ssize_t kernfs_fop_read(struct file *file, char __user *user_buf, size_t count, loff_t *ppos)
{
	struct kernfs_open_file *of = kernfs_of(file);

	if (of->kn->flags & KERNFS_HAS_SEQ_SHOW)
		return seq_read(file, user_buf, count, ppos); // read操作由系统的seq_read来完成，根据seq_file结构体内的成员信息将buf缓冲区内容返回个用户空间
	else
		return kernfs_file_direct_read(of, user_buf, count, ppos);
}
/**
 *	seq_read -	->read() method for sequential files.
 *	@file: the file to read from
 *	@buf: the buffer to read to
 *	@size: the maximum number of bytes to read
 *	@ppos: the current position in the file
 *
 *	Ready-made ->f_op->read()
 */
ssize_t seq_read(struct file *file, char __user *buf, size_t size, loff_t *ppos)
{
	struct seq_file *m = file->private_data;
	size_t copied = 0;
	loff_t pos;
	size_t n;
	void *p;
	int err = 0;

	mutex_lock(&m->lock);

	/*
	 * seq_file->op->..m_start/m_stop/m_next may do special actions
	 * or optimisations based on the file->f_version, so we want to
	 * pass the file->f_version to those methods.
	 *
	 * seq_file->version is just copy of f_version, and seq_file
	 * methods can treat it simply as file version.
	 * It is copied in first and copied out after all operations.
	 * It is convenient to have it as  part of structure to avoid the
	 * need of passing another argument to all the seq_file methods.
	 */
	m->version = file->f_version;

	/* Don't assume *ppos is where we left it */
	if (unlikely(*ppos != m->read_pos)) {
		while ((err = traverse(m, *ppos)) == -EAGAIN)
			;
		if (err) {
			/* With prejudice... */
			m->read_pos = 0;
			m->version = 0;
			m->index = 0;
			m->count = 0;
			goto Done;
		} else {
			m->read_pos = *ppos;
		}
	}

	/* grab buffer if we didn't have one */
	if (!m->buf) {
		m->buf = seq_buf_alloc(m->size = PAGE_SIZE);
		if (!m->buf)
			goto Enomem;
	}
	/* if not empty - flush it first */
	if (m->count) {
		n = min(m->count, size);
		err = copy_to_user(buf, m->buf + m->from, n);
		if (err)
			goto Efault;
		m->count -= n;
		m->from += n;
		size -= n;
		buf += n;
		copied += n;
		if (!m->count)
			m->index++;
		if (!size)
			goto Done;
	}
	/* we need at least one record in buffer */
	pos = m->index;
	p = m->op->start(m, &pos);
	while (1) {
		err = PTR_ERR(p);
		if (!p || IS_ERR(p))
			break;
		err = m->op->show(m, p);
		if (err < 0)
			break;
		if (unlikely(err))
			m->count = 0;
		if (unlikely(!m->count)) {
			p = m->op->next(m, p, &pos);
			m->index = pos;
			continue;
		}
		if (m->count < m->size)
			goto Fill;
		m->op->stop(m, p);
		kvfree(m->buf);
		m->count = 0;
		m->buf = seq_buf_alloc(m->size <<= 1);
		if (!m->buf)
			goto Enomem;
		m->version = 0;
		pos = m->index;
		p = m->op->start(m, &pos);
	}
	m->op->stop(m, p);
	m->count = 0;
	goto Done;
Fill:
	/* they want more? let's try to get some more */
	while (m->count < size) {
		size_t offs = m->count;
		loff_t next = pos;
		p = m->op->next(m, p, &next);
		if (!p || IS_ERR(p)) {
			err = PTR_ERR(p);
			break;
		}
		err = m->op->show(m, p);
		if (seq_has_overflowed(m) || err) {
			m->count = offs;
			if (likely(err <= 0))
				break;
		}
		pos = next;
	}
	m->op->stop(m, p);
	n = min(m->count, size);
	err = copy_to_user(buf, m->buf, n);
	if (err)
		goto Efault;
	copied += n;
	m->count -= n;
	if (m->count)
		m->from = n;
	else
		pos++;
	m->index = pos;
Done:
	if (!copied)
		copied = err;
	else {
		*ppos += copied;
		m->read_pos += copied;
	}
	file->f_version = m->version;
	mutex_unlock(&m->lock);
	return copied;
Enomem:
	err = -ENOMEM;
	goto Done;
Efault:
	err = -EFAULT;
	goto Done;
}
/* As reading a bin file can have side-effects, the exact offset and bytes specified in read(2) call should be passed to the read callback making
 * it difficult to use seq_file.  Implement simplistic custom buffering for bin files. */
static ssize_t kernfs_file_direct_read(struct kernfs_open_file *of, char __user *user_buf, size_t count, loff_t *ppos)
{
	ssize_t len = min_t(size_t, count, PAGE_SIZE);
	const struct kernfs_ops *ops;
	char *buf;

	buf = of->prealloc_buf;
	if (!buf)
		buf = kmalloc(len, GFP_KERNEL);
	if (!buf)
		return -ENOMEM;

	/*
	 * @of->mutex nests outside active ref and is used both to ensure that
	 * the ops aren't called concurrently for the same open file, and
	 * to provide exclusive access to ->prealloc_buf (when that exists).
	 */
	mutex_lock(&of->mutex);
	if (!kernfs_get_active(of->kn)) {
		len = -ENODEV;
		mutex_unlock(&of->mutex);
		goto out_free;
	}

	of->event = atomic_read(&of->kn->attr.open->event);
	ops = kernfs_ops(of->kn);
	if (ops->read)
		len = ops->read(of, buf, len, *ppos);   // 读取时最终调用到 kernfs_ops 的 read() 函数
	else
		len = -EINVAL;

	if (len < 0)
		goto out_unlock;

	if (copy_to_user(user_buf, buf, len)) { // 拷贝内容到用户空间
		len = -EFAULT;
		goto out_unlock;
	}

	*ppos += len;

 out_unlock:
	kernfs_put_active(of->kn);
	mutex_unlock(&of->mutex);
 out_free:
	if (buf != of->prealloc_buf)
		kfree(buf);
	return len;
}
/**
 * kernfs_fop_write - kernfs vfs write callback
 * @file: file pointer
 * @user_buf: data to write
 * @count: number of bytes
 * @ppos: starting offset
 *
 * Copy data in from userland and pass it to the matching kernfs write
 * operation.
 *
 * There is no easy way for us to know if userspace is only doing a partial
 * write, so we don't support them. We expect the entire buffer to come on
 * the first write.  Hint: if you're writing a value, first read the file,
 * modify only the the value you're changing, then write entire buffer
 * back.
 */
static ssize_t kernfs_fop_write(struct file *file, const char __user *user_buf, size_t count, loff_t *ppos)
{
	struct kernfs_open_file *of = kernfs_of(file);
	const struct kernfs_ops *ops;
	size_t len;
	char *buf;
	if (of->atomic_write_len) {
		len = count;
		if (len > of->atomic_write_len)
			return -E2BIG;
	} else {
		len = min_t(size_t, count, PAGE_SIZE);
	}

	buf = of->prealloc_buf;
	if (!buf)
		buf = kmalloc(len + 1, GFP_KERNEL);
	if (!buf)
		return -ENOMEM;

	/*
	 * @of->mutex nests outside active ref and is used both to ensure that
	 * the ops aren't called concurrently for the same open file, and
	 * to provide exclusive access to ->prealloc_buf (when that exists).
	 */
	mutex_lock(&of->mutex);
	if (!kernfs_get_active(of->kn)) {
		mutex_unlock(&of->mutex);
		len = -ENODEV;
		goto out_free;
	}

	if (copy_from_user(buf, user_buf, len)) {
		len = -EFAULT;
		goto out_unlock;
	}
	buf[len] = '\0';	/* guarantee string termination */

	ops = kernfs_ops(of->kn);
	if (ops->write)
		len = ops->write(of, buf, len, *ppos);   // 写入时最终调用到 kernfs_ops 的 write() 函数
	else
		len = -EINVAL;

	if (len > 0)
		*ppos += len;

out_unlock:
	kernfs_put_active(of->kn);
	mutex_unlock(&of->mutex);
out_free:
	if (buf != of->prealloc_buf)
		kfree(buf);
	return len;
}
