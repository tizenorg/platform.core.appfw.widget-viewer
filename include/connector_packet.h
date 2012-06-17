struct method {
	const char *cmd;
	struct packet *(*handler)(pid_t pid, int handle, struct packet *packet);
};

extern int connector_packet_async_send(int handle, struct packet *packet, int (*recv_cb)(pid_t, int handle, const struct packet *packet, void *data), void *data);
extern int connector_packet_send_only(int handle, struct packet *packet);
extern struct packet *connector_packet_oneshot_send(const char *addr, struct packet *packet);

extern int connector_packet_client_init(const char *addr, int is_sync, struct method *table);
extern int connector_packet_client_fini(int handle);
extern int connector_packet_server_init(const char *addr, struct method *table);
extern int connector_packet_server_fini(int handle);

/* End of a file */
