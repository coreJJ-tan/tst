int bus_add_driver(struct device_driver *drv)
	struct bus_type *bus;
	struct driver_private *priv;
	int error = 0;
	bus = bus_get(drv->bus);
	if (!bus)
		return -EINVAL;
	pr_debug("bus: '%s': add driver %s\n", bus->name, drv->name);
	priv = kzalloc(sizeof(*priv), GFP_KERNEL);
	if (!priv)
		error = -ENOMEM;
		goto out_put_bus;
	klist_init(&priv->klist_devices, NULL, NULL);
	priv->driver = drv;
	drv->p = priv;
	priv->kobj.kset = bus->p->drivers_kset;
	error = kobject_init_and_add(&priv->kobj, &driver_ktype, NULL, "%s", drv->name);
	if (error)
		goto out_unregister;
	klist_add_tail(&priv->knode_bus, &bus->p->klist_drivers);
	if (drv->bus->p->drivers_autoprobe)
		error = driver_attach(drv);
		if (error)
			goto out_unregister;
	module_add_driver(drv->owner, drv);
	error = driver_create_file(drv, &driver_attr_uevent);
	if (error)
		printk(KERN_ERR "%s: uevent attr (%s) failed\n", __func__, drv->name);
	error = driver_add_groups(drv, bus->drv_groups);
	if (error)
		/* How the hell do we get out of this pickle? Give up */
		printk(KERN_ERR "%s: driver_create_groups(%s) failed\n", __func__, drv->name);
	if (!drv->suppress_bind_attrs)
		error = add_bind_files(drv);
		if (error)
			/* Ditto */
			printk(KERN_ERR "%s: add_bind_files(%s) failed\n", __func__, drv->name);
	return 0;
	out_unregister:
		kobject_put(&priv->kobj);
		kfree(drv->p);
		drv->p = NULL;
	out_put_bus:
		bus_put(bus);
		return error;

