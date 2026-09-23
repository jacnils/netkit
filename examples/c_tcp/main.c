#include <netkit/c/netkit.h>

#include <stdio.h>

int main(void) {
	nk_addr_t* addr = NULL;
	nk_tcp_stream_t* stream = NULL;
	nk_status_t status = NK_OK;

	int ret = nk_addr_create(
		"google.com",
		80,
		NK_ADDR_HOSTNAME,
		NK_RESOLVE_OS,
		&addr
	);

	if (ret) {
		fprintf(stderr, "nk_addr_create failed: %d\n", ret);
	}

	status = nk_tcp_stream_create(addr, &stream);
	if (status) {
		fprintf(stderr, "nk_tcp_stream_create failed: %d\n", status);

		if (stream)
			nk_tcp_stream_destroy(stream);
	}

	status = nk_tcp_stream_connect(stream);
	if (status) {
		fprintf(stderr, "nk_tcp_stream_connect failed: %d\n", status);

		if (stream)
			nk_tcp_stream_destroy(stream);
	}

	static const char request[] =
		"GET / HTTP/1.1\r\n"
		"Host: google.com\r\n"
		"Connection: close\r\n"
		"\r\n";

	nk_stream_result_t result = nk_stream_write_all(
		(nk_stream_t*)stream,
		request,
		sizeof(request) - 1
	);

	if (result.status != NK_STREAM_SUCCESS) {
		fprintf(stderr, "nk_stream_write_all failed\n");
		if (stream)
			nk_tcp_stream_destroy(stream);
	}

	char* buf = NULL;
	size_t out_len = 0;

	status = nk_stream_read_all(
		(nk_stream_t*)stream,
		4096,
		&buf,
		&out_len
	);

	if (status) {
		return 1;
	}

	fwrite(buf, 1, out_len, stdout);

	if (addr)
		nk_addr_destroy(addr);

	return 0;
}