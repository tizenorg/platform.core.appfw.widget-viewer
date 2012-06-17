#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

#include <dlog.h>

#include "debug.h"
#include "packet.h"
#include "util.h"

int errno;

struct {
	unsigned long seq;
} s_info = {
	.seq = 0lu,
};

struct data {
	struct {
		int version;
		int payload_size;
		char command[PACKET_MAX_CMD];
		enum packet_type type;
		unsigned long seq;
	} head;

	char payload[];
};

struct packet {
	int refcnt;
	struct data *data;
};

const enum packet_type const packet_type(const struct packet *packet)
{
	return packet->data->head.type;
}

const int const packet_version(const struct packet *packet)
{
	return packet->data->head.version;
}

const int const packet_header_size(void)
{
	struct data packet; /* Only for getting the size of header of packet */

	return sizeof(packet.head);
}

const int const packet_size(const struct packet *packet)
{
	return sizeof(*packet->data) + packet->data->head.payload_size;
}

const unsigned long const packet_seq(const struct packet *packet)
{
	return packet->data->head.seq;
}

const int const packet_payload_size(const struct packet *packet)
{
	return packet->data->head.payload_size;
}

const char * const packet_command(const struct packet *packet)
{
	return packet->data->head.command;
}

const void * const packet_data(const struct packet *packet)
{
	return packet->data;
}

static inline struct data *check_and_expand_packet(struct data *packet, int *payload_size)
{
	struct data *new_packet;

	if (packet->head.payload_size < *payload_size)
		return packet;

	new_packet = realloc(packet, sizeof(*packet) + *payload_size + BUFSIZ); /*!< Expanding to +BUFSIZ */
	if (!new_packet) {
		ErrPrint("Heap: %s\n", strerror(errno));
		free(packet);
		return NULL;
	}

	*payload_size += BUFSIZ;
	return new_packet;
}

static inline struct packet *packet_body_filler(struct packet *packet, int payload_size, const char *ptr, va_list va)
{
	char *payload;
	char *str;

	while (*ptr) {
		payload = packet->data->payload + packet->data->head.payload_size;

		switch (*ptr) {
		case 'i':
		case 'I':
			packet->data->head.payload_size += sizeof(int);
			packet->data = check_and_expand_packet(packet->data, &payload_size);
			if (!packet->data) {
				free(packet);
				packet = NULL;
				goto out;
			}

			*((int *)payload) = (int)va_arg(va, int);
			break;
		case 's':
		case 'S':
			str = (char *)va_arg(va, char *);

			packet->data->head.payload_size += strlen(str) + 1; /*!< Including NIL */
			packet->data = check_and_expand_packet(packet->data, &payload_size);
			if (!packet->data) {
				free(packet);
				packet = NULL;
				goto out;
			}

			strcpy(payload, str); /*!< Including NIL */
			break;
		case 'd':
		case 'D':
			packet->data->head.payload_size += sizeof(double);
			packet->data = check_and_expand_packet(packet->data, &payload_size);
			if (!packet->data) {
				free(packet);
				packet = NULL;
				goto out;
			}

			*((double *)payload) = (double)va_arg(va, double);
			break;
		default:
			ErrPrint("Invalid type %c\n", *ptr);
			free(packet->data);
			free(packet);
			packet = NULL;
			goto out;
		}

		ptr++;
	}

out:
	return packet;
}

struct packet *packet_create_reply(struct packet *packet, const char *fmt, ...)
{
	int payload_size;
	struct packet *result;
	va_list va;

	result = malloc(sizeof(*result));
	if (!result) {
		ErrPrint("Heap: %s\n", strerror(errno));
		return NULL;
	}

	payload_size = sizeof(*result) + BUFSIZ;
	result->refcnt = 0;
	result->data = malloc(payload_size);
	if (!packet->data) {
		ErrPrint("Heap: %s\n", strerror(errno));
		return NULL;
	}

	result->data->head.seq = packet->data->head.seq;
	result->data->head.type = PACKET_ACK;
	result->data->head.version = packet->data->head.version;
	strcpy(result->data->head.command, packet->data->head.command);
	result->data->head.payload_size = 0;
	payload_size -= sizeof(*result->data);

	va_start(va, fmt);
	result = packet_body_filler(result, payload_size, fmt, va);
	va_end(va);

	return packet_ref(result);
}

struct packet *packet_create(const char *cmd, const char *fmt, ...)
{
	struct packet *packet;
	va_list va;
	int payload_size;

	if (strlen(cmd) >= PACKET_MAX_CMD) {
		ErrPrint("Command is too long\n");
		return NULL;
	}

	packet = malloc(sizeof(*packet));
	if (!packet) {
		ErrPrint("Heap: %s\n", strerror(errno));
		return NULL;
	}

	payload_size = sizeof(*packet) + BUFSIZ;
	packet->refcnt = 0;
	packet->data = malloc(payload_size);
	if (!packet->data) {
		ErrPrint("Heap: %s\n", strerror(errno));
		free(packet);
		return NULL;
	}

	packet->data->head.seq = s_info.seq++;
	packet->data->head.type = PACKET_REQ;
	packet->data->head.version = PACKET_VERSION;
	strcpy(packet->data->head.command, cmd);
	packet->data->head.payload_size = 0;
	payload_size -= sizeof(*packet->data); /*!< Usable payload size (except head size) */

	va_start(va, fmt);
	packet = packet_body_filler(packet, payload_size, fmt, va);
	va_end(va);

	return packet_ref(packet);
}

struct packet *packet_create_noack(const char *cmd, const char *fmt, ...)
{
	int payload_size;
	struct packet *result;
	va_list va;

	if (strlen(cmd) >= PACKET_MAX_CMD) {
		ErrPrint("Command is too long\n");
		return NULL;
	}

	result = malloc(sizeof(*result));
	if (!result) {
		ErrPrint("Heap: %s\n", strerror(errno));
		return NULL;
	}

	payload_size = sizeof(*result) + BUFSIZ;
	result->refcnt = 0;
	result->data = malloc(payload_size);
	if (!result->data) {
		ErrPrint("Heap: %s\n", strerror(errno));
		free(result);
		return NULL;
	}

	result->data->head.seq = s_info.seq++;
	result->data->head.type = PACKET_REQ_NOACK;
	result->data->head.version = PACKET_VERSION;
	strcpy(result->data->head.command, cmd);
	result->data->head.payload_size = 0;
	payload_size -= sizeof(*result->data);

	va_start(va, fmt);
	result = packet_body_filler(result, payload_size, fmt, va);
	va_end(va);

	return packet_ref(result);
}

int packet_get(const struct packet *packet, const char *fmt, ...)
{
	const char *ptr;
	va_list va;
	int ret = 0;
	char *payload;
	int offset = 0;
	int *int_ptr;
	double *double_ptr;
	char **str_ptr;

	va_start(va, fmt);

	ptr = fmt;
	while (*ptr) {
		payload = packet->data->payload + offset;
		switch (*ptr) {
		case 'i':
		case 'I':
			int_ptr = (int *)va_arg(va, int *);
			*int_ptr = *((int *)payload);
			offset += sizeof(int);
			ret++;
			break;
		case 'd':
		case 'D':
			double_ptr = (double *)va_arg(va, double *);
			*double_ptr = *((double *)payload);
			offset += sizeof(double);
			ret++;
			break;
		case 's':
		case 'S':
			str_ptr = (char **)va_arg(va, char **);
			*str_ptr = payload;
			offset += (strlen(*str_ptr) + 1); /*!< Including NIL */
			ret++;
			break;
		default:
			ret = -EINVAL;
			goto out;
		}
		ptr++;
	}

out:
	va_end(va);
	return ret;
}

struct packet *packet_ref(struct packet *packet)
{
	if (!packet)
		return NULL;

	packet->refcnt++;
	return packet;
}

struct packet *packet_unref(struct packet *packet)
{
	if (!packet)
		return NULL;

	packet->refcnt--;
	if (packet->refcnt < 0) {
		ErrPrint("Invalid refcnt\n");
		return NULL;
	}

	if (packet->refcnt == 0) {
		free(packet->data);
		free(packet);
		return NULL;
	}

	return packet;
}

int packet_destroy(struct packet *packet)
{
	packet_unref(packet);
	return 0;
}

struct packet *packet_build(struct packet *packet, int offset, void *data, int size)
{
	char *ptr;

	if (packet == NULL) {
		if (offset) {
			ErrPrint("Invalid argument\n");
			return NULL;
		}

		packet = malloc(sizeof(*packet));
		if (!packet) {
			ErrPrint("Heap: %s\n", strerror(errno));
			return NULL;
		}

		packet->refcnt = 1;
		packet->data = malloc(size);
		if (!packet->data) {
			ErrPrint("Heap: %s\n", strerror(errno));
			free(packet);
			return NULL;
		}

		memcpy(packet->data, data, size);
		return packet;
	}

	ptr = realloc(packet->data, offset + size);
	if (!ptr) {
		ErrPrint("Heap: %s\n", strerror(errno));
		free(packet->data);
		free(packet);
		return NULL;
	}

	packet->data = (struct data *)ptr;
	memcpy(ptr + offset, data, size);

	return packet;
}

/* End of a file */
