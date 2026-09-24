#include <netkit/c/netkit.h>
#include <stdio.h>

int main(void) {
	nk_addr_t* addr = NULL;
	nk_addr_create("www.google.com", 443, NK_ADDR_HOSTNAME, NK_RESOLVE_OS, &addr);

	nk_http_client_t* client = NULL;
	nk_http_client_create(addr, NK_HTTP_HTTPS, &client);
	nk_addr_destroy(addr);

	nk_http_client_set_user_agent(client, "netkit/1.0");

	nk_http_response_t* resp = NULL;
	if (nk_http_client_get(client, "/", NULL, &resp) == NK_OK) {
		printf("status: %d\n", nk_http_response_status_code(resp));

		nk_body_t* body = nk_http_response_take_body(resp);

		char* data = NULL;
		size_t len = 0;

		nk_body_read_all(body, true, 1 << 20, &data, &len);

		printf("body (%zu bytes): %.*s\n", len, (int) len, data);

		nk_free_string(data);
		nk_body_destroy(body);

		nk_http_response_destroy(resp);
	} else {
		printf("status: %d\n", nk_http_response_status_code(resp));
	}

	nk_http_client_destroy(client);

	return 0;
}