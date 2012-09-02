extern int master_rpc_async_request(struct livebox *handler, struct packet *packet, int urgent, void (*ret_cb)(struct livebox *handler, const struct packet *result, void *data), void *data);
extern int master_rpc_sync_request(struct packet *packet);
extern void master_rpc_check_and_fire_consumer(void);
extern int master_rpc_request_only(struct livebox *handler, struct packet *packet);
extern int master_rpc_clear_fault_package(const char *pkgname);
extern int master_rpc_clear_all_request(void);

/* End of a file */
