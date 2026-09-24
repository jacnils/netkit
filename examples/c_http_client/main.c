#include <netkit/c/netkit.h>
#include <stdio.h>
#include <string.h>

int main(void) {
	nk_addr_t* addr = NULL;
	nk_addr_create("example.com", 80, NK_ADDR_HOSTNAME, NK_RESOLVE_OS, &addr);

	nk_http_client_t* client = NULL;
	nk_http_client_create(addr, NK_HTTP_HTTP, &client);
	nk_addr_destroy(addr);

	nk_http_client_set_user_agent(client, "my-app/1.0");

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
	}

	const char* json = "{\"hello\":\"world\"}";
	nk_body_t* req_body = NULL;
	nk_body_create_buffer_from(json, strlen(json), &req_body);

	nk_http_headers_t* extra = NULL;
	nk_http_headers_create(&extra);
	nk_http_headers_add(extra, "my-header-here", "some shit value here");
	nk_http_client_set_content_type(client, "application/json");

	nk_http_response_t* post_resp = NULL;
	if (nk_http_client_post(client, "/some/path/here", req_body, extra, &post_resp) == NK_OK) {
		printf("POST status: %d\n", nk_http_response_status_code(post_resp));
		nk_http_response_destroy(post_resp);
	}

	nk_body_destroy(req_body);
	nk_http_headers_destroy(extra);
	nk_http_client_destroy(client);
	return 0;
}