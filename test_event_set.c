/*
 * SPDX-License-Identifier: LGPL-3.0-or-later
 *
 * Check that sp_wait() reports a port that was closed after being added
 * to an event set, rather than returning as if an event had occurred.
 * A pipe stands in for the port, so no serial hardware is needed.
 */

/* The checks call the code under test: keep them in release builds */
#undef NDEBUG

#include <config.h>
#include "libserialport.h"
#include "libserialport_internal.h"
#include <assert.h>
#include <unistd.h>

int main(int argc, char *argv[])
{
	(void) argc;
	(void) argv;
	char name[] = "pipe";
	struct sp_port port = { .name = name };
	struct sp_event_set *set;
	int fds[2];

	assert(pipe(fds) == 0);
	port.fd = fds[0];
	assert(sp_new_event_set(&set) == SP_OK);
	assert(sp_add_port_events(set, &port, SP_EVENT_RX_READY) == SP_OK);

	printf("Waiting on an empty pipe\n");
	assert(sp_wait(set, 100) == SP_OK);

	printf("Waiting with data ready\n");
	assert(write(fds[1], "x", 1) == 1);
	assert(sp_wait(set, 1000) == SP_OK);

	printf("Waiting after closing the port\n");
	assert(close(fds[0]) == 0);
	assert(sp_wait(set, 1000) == SP_ERR_FAIL);
	assert(sp_last_error_code() == EBADF);

	sp_free_event_set(set);
	close(fds[1]);
	return 0;
}
