#include "test_network_utility.h"
#include "test_sock_addr.h"

#include <ogc/system.h>
#include <gccore.h>

int main(void) {
	VIDEO_Init();
	WII_Initialize();

	const auto rmode = VIDEO_GetPreferredMode(nullptr);
	const auto xfb = MEM_K0_TO_K1(SYS_AllocateFramebuffer(rmode));

	console_init(xfb,20,20,rmode->fbWidth,rmode->xfbHeight,rmode->fbWidth*VI_DISPLAY_PIX_SZ);

	VIDEO_Configure(rmode);
	VIDEO_SetNextFramebuffer(xfb);
	VIDEO_SetBlack(FALSE);
	VIDEO_Flush();
	VIDEO_WaitVSync();

	if (rmode->viTVMode&VI_NON_INTERLACE) {
		VIDEO_WaitVSync();
	}

	test_network_utility();
	//test_sock_addr();

	while (true) {}
}