// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently,
// but are changed infrequently

#pragma once

#include <winsock2.h>
#include <ws2tcpip.h>
#include <mswsock.h>

#include <stdio.h>

typedef unsigned int sctp_assoc_t;

#ifdef __cplusplus
extern "C" {
#endif


struct sctp_sndrcvinfo {
	unsigned short sinfo_stream;
	unsigned short sinfo_ssn;
	unsigned short sinfo_flags;
	unsigned int sinfo_ppid;
	unsigned int sinfo_context;
	unsigned int sinfo_timetolive;
	unsigned int sinfo_tsn;
	unsigned int sinfo_cumtsn;
	sctp_assoc_t sinfo_assoc_id;
	unsigned char  __reserve_pad[96];
};

SOCKET WSAAPI internal_sctp_peeloff (SOCKET, sctp_assoc_t);
int  WSAAPI internal_sctp_bindx (SOCKET, struct sockaddr *, int, int);
int  WSAAPI internal_sctp_connectx (SOCKET, const struct sockaddr *, int, sctp_assoc_t *);
int  WSAAPI internal_sctp_getaddrlen (unsigned short);
int  WSAAPI internal_sctp_getpaddrs (SOCKET, sctp_assoc_t, struct sockaddr **);
void WSAAPI internal_sctp_freepaddrs (struct sockaddr *);
int  WSAAPI internal_sctp_getladdrs (SOCKET, sctp_assoc_t, struct sockaddr **);
void WSAAPI internal_sctp_freeladdrs (struct sockaddr *);
int  WSAAPI internal_sctp_opt_info (SOCKET, sctp_assoc_t, int, void *, socklen_t *);

long WSAAPI internal_sctp_sendmsg (SOCKET, const void *, size_t,
    const struct sockaddr *,
    socklen_t, unsigned long, unsigned long, unsigned short, unsigned long, unsigned long);

long WSAAPI internal_sctp_send (SOCKET sd, const void *msg, size_t len,
    const struct sctp_sndrcvinfo *sinfo, int flags);

long	WSAAPI internal_sctp_sendx (SOCKET sd, const void *msg, size_t len,
    struct sockaddr *addrs, int addrcnt,
    struct sctp_sndrcvinfo *sinfo, int flags);

long	WSAAPI internal_sctp_sendmsgx (SOCKET sd, const void *, size_t,
    struct sockaddr *, int,
    unsigned long, unsigned long, unsigned short, unsigned long, unsigned long);

sctp_assoc_t WSAAPI internal_sctp_getassocid (SOCKET sd, struct sockaddr *sa);

long WSAAPI internal_sctp_recvmsg (SOCKET, void *, size_t, struct sockaddr *,
    socklen_t *, struct sctp_sndrcvinfo *, int *);

enum OperationType
{
	SEND,
	RECV,
	IOCTL,
	ACCEPT,
	CLOSE
};

struct PER_IO_DATA {
	WSAOVERLAPPED overlapped;
	void *buffer;
	DWORD ioByteCount;
	OperationType operation;
	void *asyncResult;
	void *cb;
	void *sgBuffer;
	HANDLE hIoProcessed;
};

#ifdef __cplusplus
}
#endif
