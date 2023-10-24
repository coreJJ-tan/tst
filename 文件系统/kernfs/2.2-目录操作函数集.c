const struct file_operations kernfs_dir_fops = {
	.read		= generic_read_dir, // just return -EISDIR;
	.iterate	= kernfs_fop_readdir,
	.release	= kernfs_dir_fop_release,
	.llseek		= kernfs_dir_fop_llseek,
};