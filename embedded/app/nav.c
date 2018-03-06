#include <stdio.h>

#include "system.h"
#include "board.h"
#include "console.h"

#define APP_NAME		"nav"
#define BLINK_INTERVAL	250

inline void printPrompt(void) {
	printf("[%s-build] $ ", APP_NAME);
	fflush(stdout);
}

int nav_init(void) {

	/* nav specific initializations */

	return 0;
}

int main(void) {

	if (io_init() || periph_init() || nav_init())
		fault();

	printf("\r\nProgram '%s' start\r\n", APP_NAME);
	printPrompt();

	while (1) {
		check_input();
		blink_handler(BLINK_INTERVAL);
	}

	return 0;
}
