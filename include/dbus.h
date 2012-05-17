
extern GDBusProxy *dbus_get_proxy(void);
extern int dbus_push_command(struct livebox *handler, const char *funcname, GVariant *param, void (*ret_cb)(struct livebox *handler, int ret, void *data), void *data);
extern int dbus_sync_command(const char *funcname, GVariant *param);
extern int dbus_init(void);
extern int dbus_fini(void);

/* End of a file */
