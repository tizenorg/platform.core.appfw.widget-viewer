extern GDBusProxy *dbus_proxy(void);
extern int dbus_async_request(struct livebox *handler, const char *funcname, GVariant *param, void (*ret_cb)(struct livebox *handler, int ret, void *data), void *data);
extern int dbus_sync_request(struct livebox *handler, const char *func, GVariant *param);
extern int dbus_init(void);
extern int dbus_fini(void);

/* End of a file */
