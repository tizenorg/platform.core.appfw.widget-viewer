extern int master_rpc_async_request(struct livebox *handler, const char *funcname, GVariant *param, void (*ret_cb)(struct livebox *handler, GVariant *result, void *data), void *data);
extern int master_rpc_sync_request(struct livebox *handler, const char *func, GVariant *param);

/* End of a file */
