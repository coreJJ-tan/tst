struct device_attribute dev_attr_uevent = {
    .attr = {.name = __stringify(uevent),
         .mode = VERIFY_OCTAL_PERMISSIONS(S_IWUSR | S_IRUGO) },
    .show   = uevent_show,
    .store  = uevent_store,
}
