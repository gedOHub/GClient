/*-
 * Copyright (c) 2001-2007 by Cisco Systems, Inc. All rights reserved.
 * Copyright (c) 2001-2007, by Michael Tuexen, tuexen@fh-muenster.de. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * a) Redistributions of source code must retain the above copyright notice,
 *   this list of conditions and the following disclaimer.
 *
 * b) Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *   the documentation and/or other materials provided with the distribution.
 *
 * c) Neither the name of Cisco Systems, Inc. nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <winsock2.h>
#include <mswsock.h>
#include <WS2tcpip.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <ws2sctp.h>
#include "sctp_utilities.h"
#include "api_tests.h"

/*
 * TEST-TITLE connect/non_listen
 * TEST-DESCR: On a 1-1 socket, get two sockets.
 * TEST-DESCR: Neither should listen, attempt to
 * TEST-DESCR: connect one to the other. This should fail.
 */
DEFINE_APITEST(connect, non_listen)
{
	int fdc, fds, n;
	struct sockaddr_in addr;
	socklen_t addr_len;

	fds = sctp_one2one(0, 0, 0);
	if (fds  < 0)
		return strerror(errno);

	addr_len = (socklen_t)sizeof(struct sockaddr_in);
	if (getsockname (fds, (struct sockaddr *) &addr, &addr_len) < 0) {
		closesocket(fds);
		return strerror(errno);
	}

	fdc = sctp_one2one(0, 0, 0);
	if (fdc  < 0) {
		closesocket(fds);
		return strerror(errno);
	}
	n = connect(fdc, (const struct sockaddr *)&addr, addr_len);

	closesocket(fds);
	closesocket(fdc);

	if (n < 0)
		return NULL;
	else
		return "connect was successful";
}

/*
 * TEST-TITLE connect/non_listen
 * TEST-DESCR: On a 1-1 socket, get two sockets.
 * TEST-DESCR: One should listen, attempt to
 * TEST-DESCR: connect one to the other. This should work..
 */
DEFINE_APITEST(connect, listen)
{
	int fdc, fds, n;
	struct sockaddr_in addr;
	socklen_t addr_len;

	fds = sctp_one2one(0, 1, 0);
	if (fds  < 0)
		return strerror(errno);

	addr_len = (socklen_t)sizeof(struct sockaddr_in);
	if (getsockname (fds, (struct sockaddr *) &addr, &addr_len) < 0) {
		closesocket(fds);
		return strerror(errno);
	}

	fdc = sctp_one2one(0, 0, 0);
	if (fdc  < 0) {
		closesocket(fds);
		return strerror(errno);
	}

	n = connect(fdc, (const struct sockaddr *)&addr, addr_len);
	closesocket(fds);
	closesocket(fdc);
	if (n < 0)
		return strerror(errno);
	else
		return NULL;
}

/*
 * TEST-TITLE connect/self_non_listen
 * TEST-DESCR: On a 1-1 socket, get a socket, no listen.
 * TEST-DESCR: Attempt to connect to itself.
 * TEST-DESCR: This should fail, since we are not listening.
 */
DEFINE_APITEST(connect, self_non_listen)
{
	int fd, n;
	struct sockaddr_in addr;
	socklen_t addr_len;

	fd = sctp_one2one(0, 0, 0);
	if (fd  < 0)
		return strerror(errno);

	addr_len = (socklen_t)sizeof(struct sockaddr_in);
	if (getsockname (fd, (struct sockaddr *) &addr, &addr_len) < 0) {
		closesocket(fd);
		return strerror(errno);
	}

	n = connect(fd, (const struct sockaddr *)&addr, addr_len);
	closesocket(fd);

	/*  This really depends on when connect() returns. On the
	 *  reception of the INIT-ACK or the COOKIE-ACK
	 */
	if (n < 0)
		return strerror(errno);
	else
		return NULL;


}
/*
 * TEST-TITLE connect/self_listen
 * TEST-DESCR: On a 1-1 socket, get a socket, and listen.
 * TEST-DESCR: Attempt to connect to itself.
 * TEST-DESCR: This should fail, since we are not allowed to
 * TEST-DESCR: connect when listening.
 */
DEFINE_APITEST(connect, self_listen)
{
	int fd, n;
	struct sockaddr_in addr;
	socklen_t addr_len;

	fd = sctp_one2one(0, 1, 0);
	if (fd  < 0)
		return strerror(errno);
	addr_len = (socklen_t)sizeof(struct sockaddr_in);
	if (getsockname(fd, (struct sockaddr *) &addr, &addr_len) < 0) {
		closesocket(fd);
		return strerror(errno);
	}

	n = connect(fd, (const struct sockaddr *)&addr, addr_len);
	closesocket(fd);
	if (n < 0)
		return NULL;
	else
		return "connect was successful";

}
