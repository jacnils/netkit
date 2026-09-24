#include <netkit/c/netkit.h>
#include <stdio.h>
#include <string.h>

static nk_http_server_response_t* handle_request(const nk_http_request_t* req, void* user_data) {
	(void) user_data; // unused for this example
	const char* endpoint = nk_http_request_endpoint(req);
	printf("%s %s from %s\n", nk_http_request_method(req), endpoint, nk_http_request_ip_address(req));

	nk_http_server_response_t* resp = NULL;
	nk_http_server_response_create(&resp);

	if (strcmp(endpoint, "/test") == 0) {
		const char* name = NULL;
		nk_http_request_query(req, "name", &name);

		char msg[256];
		snprintf(msg, sizeof(msg), "{\"name\":\"%s!\"}", name ? name : "idk");

		nk_body_t* body = NULL;
		nk_body_create_buffer_from(msg, strlen(msg), &body);
		nk_http_server_response_set_body(resp, body);

		nk_http_server_response_set_session(resp, "last_name", name ? name : "idk");
	} else {
		nk_http_server_response_set_status(resp, 404);
	}

	return resp;
}

int main(void) {
	nk_http_server_settings_t* settings = NULL;
	nk_http_server_settings_create(&settings);
	nk_http_server_settings_set_port(settings, 8080);
	nk_http_server_settings_set_enable_session(settings, true);

	nk_http_server_t* server = NULL;
	if (nk_http_server_create(settings, handle_request, NULL, &server) != NK_OK) {
		fprintf(stderr, "failed to start: %s\n", nk_last_error());
		return 1;
	}

	printf("listening on port 8080\n");

	nk_http_server_run(server);
	nk_http_server_destroy(server);

	return 0;
}