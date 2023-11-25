1. 初始化部分
    int __init buses_init(void)
        bus_kset = kset_create_and_add("bus", &bus_uevent_ops, NULL); // 创建一个kset，体现为 /sys/bus 目录
        system_kset = kset_create_and_add("system", NULL, &devices_kset->kobj); // 创建一个kset，体现为 /sys/system 目录
        return 0;

2. 