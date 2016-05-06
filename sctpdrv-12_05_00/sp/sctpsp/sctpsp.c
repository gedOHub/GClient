/*
 * Copyright (c) 2008 CO-CONV, Corp.
 * Copyright (c) 2009 Bruce Cran.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */
#define UNICODE

//#include <windows.h>
//#include <windef.h>
//#include <guiddef.h>
#include <winsock2.h>
//#include <winerror.h>
//#include <io.h>
#include <mswsock.h>
#include <ws2tcpip.h>
#include <ws2spi.h>
#include <strsafe.h>
#include <stdio.h>

#include <sys/errno.h>
#include <sys/queue.h>
#include <sys/types.h>
#include <sys/syscall.h>
//include <sys/systm.h>

#include <netinet/sctp.h>
#include <netinet/sctp_peeloff.h>
#include <sctpsp.h>

struct SocketInfo
{
	SOCKET s;
	DWORD accessFlags;
	DWORD shareFlags;
	DWORD attributeFlags;
	long events;
	HANDLE eventsHandle;
	struct sockaddr_storage localAddress;
	int localAddressLen;
	struct sockaddr_storage remoteAddress;
	int remoteAddressLen;
	DWORD dwCatalogEntryId;
	TAILQ_ENTRY(SocketInfo) entries;
};

/* SctpDrv use of the WSAOVERLAPPED structure:
 *
 * Internal: not used
 * InternalHigh: not used
 * Offset: OverlappedType: EVENT or CALLBACK
 * OffsetHigh: dwFlags
 * Pointer: a pointer to the buffer if OverlappedType is EVENT,
 *       or a pointer to the OverlappedInfo structure if CALLBACK
 */

enum OverlappedType
{
	OVERLAPPED_TYPE_EVENT = 0x1,
	OVERLAPPED_TYPE_CALLBACK = 0x2
};

typedef struct _SOCKET_OVERLAPPED {
	SOCKET socket;
	PVOID buffer;
	LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine;
	WSATHREADID  threadId;
} OverlappedInfo, *POverlappedInfo;

/* The WSASendMsg extension function */
int
WSPAPI
_WSASendMsg(
    SOCKET s,
    LPWSAMSG lpMsg,
    DWORD dwFlags,
    LPDWORD lpNumberOfBytesSent,
    LPWSAOVERLAPPED lpOverlapped,
    LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine);

/* The WSARecvMsg extension function */
int
WSPAPI
_WSARecvMsg(
    SOCKET s,
    LPWSAMSG lpMsg,
    LPDWORD lpdwNumberOfBytesRecvd,
    LPWSAOVERLAPPED lpOverlapped,
    LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine);

static struct SocketInfo* GetSocketInfo(SOCKET s);

static void getsocket(fd_set *fds, SOCKET *s);

static BOOL
WSPAPI
ConfigureOverlapped(
	LPWSAOVERLAPPED lpOverlapped,
	SOCKET s,
	LPVOID buffer,
	LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine,
	LPWSATHREADID lpThreadId,
	LPINT lpErrno);

static void
WSPAPI
SetErrorCode(LPINT lpErrno);

void
CALLBACK
ApcFunc(DWORD_PTR dwContext);

DWORD
WINAPI
WaitOverlappedComplete(LPVOID lpParameter);

/* some per-process context we need to keep
 * in order to match data up between calls */
TAILQ_HEAD(SocketList, SocketInfo) sockets;

WSPUPCALLTABLE Upcalls;
HMODULE hIPv4Dll = NULL;
HMODULE hIPv6Dll = NULL;

#if defined(SCTP_PROVIDER_DEBUG)
#define DBGPRINT printf
#else
__inline void
_DbgPrint(
    IN PCHAR  Format,
    ...)
{
}
#define DBGPRINT _DbgPrint
#endif

static
BOOL
ValidSocket(SOCKET s, struct SocketInfo **socketInfo, LPINT lpErrno)
{
	BOOL ret = TRUE;
	struct SocketInfo *info;

	info = GetSocketInfo(s);

	if (info == NULL) {
		*lpErrno = WSAENOTSOCK;
		ret = FALSE;
	}

	if (ret && socketInfo != NULL)
		*socketInfo = info;

	return ret;
}

static
void
ClearSocketInfo(struct SocketInfo *info)
{
	ZeroMemory(info, sizeof(*info));
	info->s = INVALID_SOCKET;
}

static
void WSPAPI
SetErrorCode(LPINT lpErrno)
{
	DWORD winsockErr;
	DWORD error = GetLastError();

	/* if we have a SOCKET_STATUS NTSTATUS value,
	 * use just the lower 16 bits - the error code
	 */
	if ((error >> 16) == 0xE0FF)
		error &= 0xFFFF;

	/* TODO make sure the WSAE* mapping is correct for
	 * all functions. Some may require a different interpretation */
	switch (error) {
	case ERROR_INVALID_HANDLE:
	case ERROR_INVALID_FUNCTION:
		winsockErr = WSAENOTSOCK;
		break;
	case ERROR_IO_PENDING:
		winsockErr = WSA_IO_PENDING;
		break;
	case ERROR_NO_SYSTEM_RESOURCES:
	case ENOMEM:
	case ENOBUFS:
		winsockErr = WSAENOBUFS;
		break;
	case ERROR_NOACCESS:
	case EFAULT:
		winsockErr = WSAEFAULT;
		break;
	case ERROR_TIMEOUT: /* STATUS_TIMEOUT */
		winsockErr = WSAETIMEDOUT;
		break;
	case EAFNOSUPPORT:
		winsockErr = WSAEAFNOSUPPORT;
		break;
	case EADDRNOTAVAIL:
		winsockErr = WSAEADDRNOTAVAIL;
		break;
	case EINVAL:
	case EDOM:
		winsockErr = WSAEINVAL;
		break;
	case EADDRINUSE:
		winsockErr = WSAEADDRINUSE;
		break;
	case EWOULDBLOCK:
		winsockErr = WSAEWOULDBLOCK;
		break;
	case EALREADY:
		winsockErr = WSAEALREADY;
		break;
	case EISCONN:
		winsockErr = WSAEISCONN;
		break;
	case ECONNABORTED:
	case ECONNRESET:
		winsockErr = WSAECONNRESET;
		break;
	case ECONNREFUSED:
		winsockErr = WSAECONNREFUSED;
		break;
	case ENOTCONN:
		winsockErr = WSAENOTCONN;
		break;
	case EOPNOTSUPP:
		winsockErr = WSAEOPNOTSUPP;
		break;
	case ENOPROTOOPT:
		winsockErr = WSAENOPROTOOPT;
		break;
	case ENOENT:
		winsockErr = WSANO_DATA;
		break;
	case EPIPE:
	case EMSGSIZE:
		winsockErr = WSAEMSGSIZE;
		break;
	default:
		winsockErr = WSAENETDOWN;
		break;
	}

	if (lpErrno == NULL)
		WSASetLastError(winsockErr);
	else
		*lpErrno = winsockErr;
}

static
struct SocketInfo*
GetSocketInfo(SOCKET s)
{
	struct SocketInfo *sockInfo = NULL;

	TAILQ_FOREACH(sockInfo, &sockets, entries)
	{
		if (sockInfo->s == s) {
			break;
		}
	}

	return sockInfo;
}

static
void
getsocket(fd_set *fds, SOCKET *s)
{
	unsigned int i;

	if (fds != NULL && *s == INVALID_SOCKET) {
		for (i = 0; i < fds->fd_count && i < FD_SETSIZE; i++) {
			if (fds->fd_array[i] != INVALID_SOCKET) {
				*s = fds->fd_array[i];
				break;
			}
		}
	}
}

static
BOOL
WSPAPI
ConfigureOverlapped(
		LPWSAOVERLAPPED lpOverlapped,
		SOCKET s,
		PVOID buffer,
		LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine,
		LPWSATHREADID lpThreadId,
		LPINT lpErrno)
{
	int bSuccess = TRUE;
	OverlappedInfo *info = NULL;

	/* Check if there's anything for us to do */
	if (lpOverlapped == NULL)
		return TRUE;

	/* Clear out the fields we use */
	lpOverlapped->Offset = 0;
	lpOverlapped->OffsetHigh = 0;
	lpOverlapped->Pointer = NULL;

	if (lpCompletionRoutine != NULL) {

		info = malloc(sizeof(OverlappedInfo));
		if (info == NULL) {
			SetErrorCode(lpErrno);
			return FALSE;
		}

		lpOverlapped->Offset = OVERLAPPED_TYPE_CALLBACK;

		lpOverlapped->hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

		if (lpOverlapped->hEvent == NULL) {
			bSuccess = FALSE;
			SetErrorCode(lpErrno);
		}

		info->socket = s;
		info->buffer = buffer;
		info->lpCompletionRoutine = lpCompletionRoutine;
		CopyMemory(&info->threadId, lpThreadId, sizeof(info->threadId));

		lpOverlapped->Pointer = info;

		if (CreateThread(NULL, 0, WaitOverlappedComplete, lpOverlapped, 0, NULL) == NULL) {
			free(info);
			return FALSE;
		}
	} else {
		lpOverlapped->Offset = OVERLAPPED_TYPE_EVENT;
		lpOverlapped->Pointer = buffer;
	}

	return bSuccess;
}

SOCKET
WSPAPI
WSPAccept(
    SOCKET s,
    struct sockaddr *addr,
    LPINT addrlen,
    LPCONDITIONPROC lpfnCondition,
    DWORD_PTR dwCallbackData,
    LPINT lpErrno)
{
	int ret = 0;
	SOCKET hAcceptSocket;
	SOCKET hModifiedSocket = INVALID_SOCKET;
	struct SocketInfo *socketInfo;
	struct sockaddr_storage remoteAddress;
	int remoteAddressLen;
	WSABUF callerId;
	WSABUF calleeId;
	WSABUF calleeData;
	GROUP g;
	int condReturn;
	DWORD bytesReturned;
	BOOL bSuccess;

	DBGPRINT("WSPAccept - enter\n");

	if (addr != NULL && (addrlen == NULL || *addrlen <= 0)) {
		*lpErrno = WSAEFAULT;
		DBGPRINT("WSPAccept - leave #1\n");
		return INVALID_SOCKET;
	}

	if (!ValidSocket(s, &socketInfo, lpErrno))
		return INVALID_SOCKET;

	hAcceptSocket = (SOCKET)CreateFile(WIN_SCTP_SOCKET_DEVICE_NAME,
					socketInfo->accessFlags,
					socketInfo->shareFlags,
					NULL,
					OPEN_EXISTING,
					socketInfo->attributeFlags,
					NULL);

	if ((HANDLE)hAcceptSocket == INVALID_HANDLE_VALUE) {
		SetErrorCode(lpErrno);
		ret = INVALID_SOCKET;
	}

	if (!ret) {
		bSuccess = DeviceIoControl((HANDLE)s, IOCTL_SOCKET_ACCEPT,
				&hAcceptSocket, sizeof(hAcceptSocket),
				&remoteAddress, sizeof(remoteAddress),
				&bytesReturned, NULL);

		DBGPRINT("IOCTL_SOCKET_ACCEPT: bSuccess=%d\n", bSuccess);
		if (bSuccess && (addr == NULL || (addr != NULL && (DWORD)(*addrlen) >= bytesReturned))) {
			remoteAddressLen = bytesReturned;
			if (addr != NULL) {
				CopyMemory(addr, &remoteAddress, bytesReturned);
				*addrlen = bytesReturned;
			}
		} else {
			CloseHandle((HANDLE)hAcceptSocket);
			hAcceptSocket = INVALID_SOCKET;

			if (bSuccess && addr != NULL)
				*lpErrno = WSAEFAULT; /* addrlen must be too small */
			else
				SetErrorCode(lpErrno);

			ret = INVALID_SOCKET;
		}
	}

	if (!ret && lpfnCondition != NULL) {
		callerId.len = remoteAddressLen;
		callerId.buf = (char*)&remoteAddress;
		calleeId.len = sizeof(socketInfo->localAddress);
		calleeId.buf = (char*) &(socketInfo->localAddress);
		calleeData.len = 0;
		calleeData.buf = NULL;

		condReturn = lpfnCondition(&callerId, NULL, NULL, NULL, &calleeId, &calleeData, &g, dwCallbackData);

		if (condReturn == CF_REJECT) {
			CloseHandle((HANDLE)hAcceptSocket);
			*lpErrno = WSAECONNREFUSED;
			ret = INVALID_SOCKET;
		} else if (condReturn == CF_DEFER) {
			CloseHandle((HANDLE)hAcceptSocket);
			*lpErrno = WSATRY_AGAIN;
			ret = INVALID_SOCKET;
		}
	}

	if (!ret) {
		hModifiedSocket = Upcalls.lpWPUModifyIFSHandle(socketInfo->dwCatalogEntryId, hAcceptSocket, lpErrno);

		if (hModifiedSocket == INVALID_SOCKET) {
			CloseHandle((HANDLE)hAcceptSocket);
			SetErrorCode(lpErrno);
			ret = INVALID_SOCKET;
		}
	}

	if (!ret) {
		struct SocketInfo *sockInfo = malloc(sizeof(struct SocketInfo));
		if (sockInfo == NULL) {
			*lpErrno = WSAENOBUFS;
			return INVALID_SOCKET;
		}

		/* Add a new socket info entry */
		ZeroMemory(sockInfo, sizeof(struct SocketInfo));
		sockInfo->s              = hModifiedSocket;
		sockInfo->accessFlags    = socketInfo->accessFlags;
		sockInfo->shareFlags     = socketInfo->shareFlags;
		sockInfo->attributeFlags = socketInfo->attributeFlags;
		sockInfo->events         = socketInfo->events;
		sockInfo->eventsHandle   = socketInfo->eventsHandle;
		sockInfo->localAddressLen = socketInfo->localAddressLen;
		sockInfo->remoteAddressLen = socketInfo->remoteAddressLen;
		sockInfo->dwCatalogEntryId = socketInfo->dwCatalogEntryId;
		CopyMemory(&(sockInfo->localAddress), &(socketInfo->localAddress), socketInfo->localAddressLen);
		CopyMemory(&(sockInfo->remoteAddress), &remoteAddress, remoteAddressLen);
		TAILQ_INSERT_TAIL(&sockets, sockInfo, entries);
	}

	DBGPRINT("WSPAccept - leave\n");
	return hModifiedSocket;
}

typedef INT (WINAPI *PWSH_ADDRESS_TO_STRING)(LPSOCKADDR,INT,LPWSAPROTOCOL_INFOW,LPWSTR,LPDWORD);
static PWSH_ADDRESS_TO_STRING WSHAddressToString = NULL;
static PWSH_ADDRESS_TO_STRING WSHAddressToStringIPv6 = NULL;

int
WSPAPI
WSPAddressToString(
    LPSOCKADDR lpsaAddress,
    DWORD dwAddressLength,
    LPWSAPROTOCOL_INFOW lpProtocolInfo,
    LPWSTR lpszAddressString,
    LPDWORD lpdwAddressStringLength,
    LPINT lpErrno)
{
	int ret = 0;

	DBGPRINT("WSPAddressToString - enter\n");

	if (lpsaAddress->sa_family == AF_INET &&
	    WSHAddressToString != NULL) {
		*lpErrno = WSHAddressToString(lpsaAddress, dwAddressLength,
		    NULL, lpszAddressString, lpdwAddressStringLength);
	} else if (
	    lpsaAddress->sa_family == AF_INET6 &&
	    WSHAddressToStringIPv6!= NULL) {
		*lpErrno = WSHAddressToStringIPv6(lpsaAddress, dwAddressLength,
		    NULL, lpszAddressString, lpdwAddressStringLength);
	} else
		*lpErrno = WSAEINVAL;

	if (*lpErrno != 0)
		ret = INVALID_SOCKET;

	DBGPRINT("WSPAddressToString - leave\n");
	return ret;
}

int
WSPAPI
WSPAsyncSelect(
    SOCKET s,
    HWND hWnd,
    unsigned int wMsg,
    long lEvent,
    LPINT lpErrno)
{
	DBGPRINT("WSPAsyncSelect - enter\n");
	*lpErrno = WSAENETDOWN;
	DBGPRINT("WSPAsyncSelect - leave\n");
	return SOCKET_ERROR;
}

int
WSPAPI
WSPBind(
    SOCKET s,
    const struct sockaddr *name,
    int namelen,
    LPINT lpErrno)
{
	int ret = 0;
	DWORD bytesReturned;
	BOOL bSuccess;
	struct SocketInfo *socketInfo;

	if (!ValidSocket(s, &socketInfo, lpErrno))
		return SOCKET_ERROR;

	DBGPRINT("WSPBind - enter\n");
	if (name == NULL || namelen <= 0) {
		*lpErrno = WSAEFAULT;
		DBGPRINT("WSPBind - leave\n");
		return SOCKET_ERROR;
	}

	if (!ret) {
		bSuccess = DeviceIoControl((HANDLE)s, IOCTL_SOCKET_BIND,
						(PVOID)name, (ULONG)namelen,
						NULL, 0,
						&bytesReturned, NULL);

		DBGPRINT("IOCTL_SOCKET_BIND: bSuccess=%d\n", bSuccess);
		if (!bSuccess) {
			SetErrorCode(lpErrno);
			ret = SOCKET_ERROR;
		}
	}

	if (!ret) {
		/* we've just bound our socket to a local address.
		 * Update our information */
		socketInfo->localAddressLen = namelen;
		CopyMemory(&(socketInfo->localAddress), name, namelen);
	}

	DBGPRINT("WSPBind - leave\n");
	return ret;
}

int
WSPAPI
WSPCancelBlockingCall(
    LPINT lpErrno)
{
	DBGPRINT("WSPCancelBlockingCall - enter\n");
	*lpErrno = WSAEOPNOTSUPP;
	DBGPRINT("WSPCancelBlockingCall - leave\n");
	return SOCKET_ERROR;
}

int
WSPAPI
WSPCleanup(
    LPINT lpErrno)
{
	struct SocketInfo *sockInfo;

	DBGPRINT("WSPCleanup - enter\n");

	sockInfo = TAILQ_FIRST(&sockets);

	while (sockInfo != NULL) {
		TAILQ_REMOVE(&sockets, sockInfo, entries);
		free(sockInfo);
		sockInfo = TAILQ_FIRST(&sockets);
	}

	/* TODO cancel events and overlapped operations,
	 * close sockets and do any other cleanup
	 */


	if (hIPv4Dll != NULL) {
		FreeLibrary(hIPv4Dll);
	}
	if (hIPv6Dll != NULL) {
		FreeLibrary(hIPv6Dll);
	}

	DBGPRINT("WSPCleanup - leave\n");
	return 0;
}

int
WSPAPI
WSPCloseSocket(
    SOCKET s,
    LPINT lpErrno)
{
	int ret = 0;
	BOOL bSuccess;
	struct SocketInfo *socketInfo;

	DBGPRINT("WSPCloseSocket - enter\n");

	if (!ValidSocket(s, &socketInfo, lpErrno))
		return SOCKET_ERROR;

	bSuccess = CloseHandle((HANDLE)s);
	DBGPRINT("CloseHandle: bSuccess=%d\n", bSuccess);
	if (!bSuccess) {
		SetErrorCode(lpErrno);
		ret = SOCKET_ERROR;
	}

	if (!ret) {
		/* remove socket info */
		TAILQ_REMOVE(&sockets, socketInfo, entries);
		free(socketInfo);
	}

	DBGPRINT("WSPCloseSocket - leave\n");
	return ret;
}

int
WSPAPI
WSPConnect(
    SOCKET s,
    const struct sockaddr *name,
    int namelen,
    LPWSABUF lpCallerData,
    LPWSABUF lpCalleeData,
    LPQOS lpSQOS,
    LPQOS lpGQOS,
    LPINT lpErrno)
{
	int ret = 0;
	DWORD bytesReturned;
	BOOL bSuccess;
	struct SocketInfo *socketInfo;

	if (!ValidSocket(s, &socketInfo, lpErrno))
		return SOCKET_ERROR;

	DBGPRINT("WSPConnect - enter\n");

	if (name == NULL || namelen <= 0) {
		*lpErrno = WSAEINVAL;
		ret = SOCKET_ERROR;
		DBGPRINT("WSPConnect - leave#1\n");
		return SOCKET_ERROR;
	}

	bSuccess = DeviceIoControl((HANDLE)s, IOCTL_SOCKET_CONNECT,
					(PVOID)name, (ULONG)namelen,
					NULL, 0,
					&bytesReturned, NULL);

	DBGPRINT("IOCTL_SOCKET_CONNECT: bSuccess=%d\n", bSuccess);
	if (!bSuccess) {
		SetErrorCode(lpErrno);
		ret = SOCKET_ERROR;
	}

	if (!ret) {
		/* We've just connected to a remote address.
		 * Update our information */
		socketInfo->remoteAddressLen = namelen;
		CopyMemory(&(socketInfo->remoteAddress), name, namelen);
	}

	DBGPRINT("WSPConnect - leave\n");
	return ret;
}

int
WSPAPI
WSPDuplicateSocket(
    SOCKET s,
    DWORD dwProcessId,
    LPWSAPROTOCOL_INFO lpProtocolInfo,
    LPINT lpErrno)
{
	int ret = 0;
	struct SocketInfo *socketInfo;
	HANDLE hDuplicate;
	HANDLE hTargetProcess = NULL;
	SOCKET hModifiedSocket = INVALID_SOCKET;
	struct SocketInfo *sockInfo;
	BOOL bSuccess;

	DBGPRINT("WSPDuplicateSocket - enter\n");

	if (lpProtocolInfo == NULL) {
		*lpErrno = WSAEINVAL;
		DBGPRINT("WSPDuplicateSocket - leave\n");
		return SOCKET_ERROR;
	}

	if (!ValidSocket(s, &socketInfo, lpErrno))
		return SOCKET_ERROR;

	if (!ret) {
		hTargetProcess = OpenProcess(PROCESS_DUP_HANDLE, FALSE, dwProcessId);

		if (hTargetProcess == NULL) {
			SetErrorCode(lpErrno);
			ret = SOCKET_ERROR;
		}
	}

	if (!ret)
	{
		bSuccess = DuplicateHandle(GetCurrentProcess(), (HANDLE)s,
				hTargetProcess, &hDuplicate, 0, FALSE, DUPLICATE_SAME_ACCESS);
		if (!bSuccess) {
			SetErrorCode(lpErrno);
			ret = SOCKET_ERROR;
		}
	}

	if (hTargetProcess != NULL)
		CloseHandle(hTargetProcess);

	if (!ret) {
		hModifiedSocket = Upcalls.lpWPUModifyIFSHandle(socketInfo->dwCatalogEntryId, (SOCKET)hDuplicate, lpErrno);

		if (hModifiedSocket == INVALID_SOCKET) {
			SetErrorCode(lpErrno);
			ret = SOCKET_ERROR;
		}
	}

	if (!ret) {
		/* store the duplicated handle so we can return it in the WSPSocket call */
		lpProtocolInfo->dwProviderReserved = (DWORD)hModifiedSocket;

		/* we've just created another socket */
		/* so add a new socket info entry */
		sockInfo = malloc(sizeof(struct SocketInfo));
		if (sockInfo == NULL) {
			*lpErrno = WSAENOBUFS;
			return SOCKET_ERROR;
		}

		ZeroMemory(sockInfo, sizeof(struct SocketInfo));
		sockInfo->s              = hModifiedSocket;
		sockInfo->accessFlags    = socketInfo->accessFlags;
		sockInfo->shareFlags     = socketInfo->shareFlags;
		sockInfo->attributeFlags = socketInfo->attributeFlags;
		sockInfo->events         = socketInfo->events;
		sockInfo->eventsHandle   = socketInfo->eventsHandle;
		sockInfo->localAddressLen= socketInfo->localAddressLen;
		sockInfo->remoteAddressLen= socketInfo->remoteAddressLen;
		sockInfo->dwCatalogEntryId= socketInfo->dwCatalogEntryId;
		CopyMemory(&(sockInfo->localAddress), &(socketInfo->localAddress), socketInfo->localAddressLen);
		CopyMemory(&(sockInfo->remoteAddress),&(socketInfo->remoteAddress),socketInfo->remoteAddressLen);
		TAILQ_INSERT_TAIL(&sockets, sockInfo, entries);
	}

	if (ret) {
		SetErrorCode(lpErrno);
		ret = SOCKET_ERROR;
	}

	DBGPRINT("WSPDuplicateSocket - leave\n");
	return ret;
}

int
WSPAPI
WSPEnumNetworkEvents(
    SOCKET s,
    WSAEVENT hEventObject,
    LPWSANETWORKEVENTS lpNetworkEvents,
    LPINT lpErrno)
{
	int ret = 0;
	BOOL bSuccess;
	DWORD bytesReturned;
	SOCKET_ENUMNETWORKEVENTS_REQUEST enumEvents = {0};

	DBGPRINT("WSPEnumNetworkEvents - enter\n");

	if (lpNetworkEvents == NULL) {
		*lpErrno = WSAEFAULT;
		DBGPRINT("WSPEnumNetworkEvents - leave#1\n");
		return SOCKET_ERROR;
	}

	enumEvents.hEventObject = (HANDLE)hEventObject;

	bSuccess = DeviceIoControl((HANDLE)s, IOCTL_SOCKET_ENUMNETWORKEVENTS,
			&enumEvents, sizeof(enumEvents),
			&enumEvents, sizeof(enumEvents),
			&bytesReturned, NULL);

	DBGPRINT("IOCTL_SOCKET_ENUMNETWORKEVENTS: bSuccess=%d\n", bSuccess);
	if (bSuccess) {
		CopyMemory(lpNetworkEvents, &enumEvents.networkEvents, sizeof(WSANETWORKEVENTS));
	} else {
		SetErrorCode(lpErrno);
		ret = SOCKET_ERROR;
	}

	DBGPRINT("WSPEnumNetworkEvents - leave\n");
	return ret;
}

int
WSPAPI
WSPEventSelect(
    SOCKET s,
    WSAEVENT hEventObject,
    long lNetworkEvents,
    LPINT lpErrno)
{
	int ret = 0;
	BOOL bSuccess;
	DWORD bytesReturned;
	SOCKET_EVENTSELECT_REQUEST eventSelect = {0};

	DBGPRINT("WSPEventSelect - enter\n");

	eventSelect.hEventObject = (HANDLE)hEventObject;
	eventSelect.lNetworkEvents = lNetworkEvents;

	bSuccess = DeviceIoControl((HANDLE)s, IOCTL_SOCKET_EVENTSELECT,
			&eventSelect, sizeof(eventSelect),
			NULL, 0,
			&bytesReturned, NULL);

	DBGPRINT("IOCTL_SOCKET_EVENTSELECT: bSuccess=%d\n", bSuccess);
	if (!bSuccess) {
		SetErrorCode(lpErrno);
		ret = SOCKET_ERROR;
	}

	DBGPRINT("WSPEventSelect - leave\n");
	return ret;
}

BOOL
WSPAPI
WSPGetOverlappedResult(
    SOCKET s,
    LPWSAOVERLAPPED lpOverlapped,
    LPDWORD lpcbTransfer,
    BOOL fWait,
    LPDWORD lpdwFlags,
    LPINT lpErrno)
{
	BOOL bSuccess = TRUE;
	DWORD bytesReturned;

	DBGPRINT("WSPGetOverlappedResult - enter\n");
	if (lpOverlapped == NULL || lpcbTransfer == NULL || lpdwFlags == NULL) {
		*lpErrno = WSAEINVAL;
		DBGPRINT("WSPGetOverlappedResult - leave#1\n");
		return FALSE;
	}

	bSuccess = GetOverlappedResult((HANDLE)s, lpOverlapped, &bytesReturned, fWait);

	if (bSuccess) {
		*lpcbTransfer = (DWORD)lpOverlapped->Pointer;
		*lpdwFlags = (DWORD)lpOverlapped->OffsetHigh;
	}

	if (!bSuccess)
		SetErrorCode(lpErrno);

	DBGPRINT("WSPGetOverlappedResult - leave\n");
	return bSuccess;
}

int
WSPAPI
WSPGetPeerName(
	SOCKET s,
    struct sockaddr *name,
    LPINT namelen,
    LPINT lpErrno)
{
	int ret = 0;
	BOOL bSuccess;
	DWORD bytesReturned;

	DBGPRINT("WSPGetPeerName - enter\n");

	if (name == NULL || namelen == NULL || *namelen <= 0) {
		*lpErrno = WSAEFAULT;
		DBGPRINT("WSPGetPeerName - leave#1\n");
		return SOCKET_ERROR;
	}

	bSuccess = DeviceIoControl((HANDLE)s, IOCTL_SOCKET_GETPEERNAME,
				NULL, 0,
				name, *namelen,
				&bytesReturned, NULL);

	DBGPRINT("IOCTL_SOCKET_GETPEERNAME: bSuccess=%d\n", bSuccess);
	if (bSuccess) {
		*namelen = (UINT)bytesReturned;
	} else {
		SetErrorCode(lpErrno);
		ret = SOCKET_ERROR;
	}

	DBGPRINT("WSPGetPeerName - leave\n");
	return ret;
}

int
WSPAPI
WSPGetSockName(
	SOCKET s,
    struct sockaddr *name,
    LPINT namelen,
    LPINT lpErrno)
{
	int ret = 0;
	BOOL bSuccess;
	DWORD bytesReturned;

	DBGPRINT("WSPGetSockName - enter\n");

	if (name == NULL || namelen == NULL || *namelen <= 0) {
		*lpErrno = WSAEFAULT;
		DBGPRINT("WSPGetSockName - leave#1\n");
		return SOCKET_ERROR;
	}

	bSuccess = DeviceIoControl((HANDLE)s, IOCTL_SOCKET_GETSOCKNAME,
				NULL, 0,
				name, *namelen,
				&bytesReturned, NULL);

	DBGPRINT("IOCTL_SOCKET_GETSOCKNAME: bSuccess=%d\n", bSuccess);
	if (bSuccess) {
		*namelen = (UINT)bytesReturned;
	} else {
		SetErrorCode(lpErrno);
		ret = SOCKET_ERROR;
	}

	DBGPRINT("WSPGetSockName - leave\n");
	return ret;
}

int
WSPAPI
WSPGetSockOpt(
	SOCKET s,
	int level,
	int optname,
    char *optval,
    LPINT optlen,
    LPINT lpErrno)
{
	int ret = 0;
	SOCKET hModifiedSocket;
	BOOL bSuccess;
	DWORD bytesReturned;
	DWORD bytesTransferred;
	SOCKET_SOCKOPT_REQUEST optReq;
	struct SocketInfo *socketInfo;

	if (!ValidSocket(s, &socketInfo, lpErrno))
		return SOCKET_ERROR;

	DBGPRINT("WSPGetSockOpt - enter\n");

	if (optval == NULL || optlen == NULL || *optlen <= 0) {
		*lpErrno = WSAEFAULT;
		DBGPRINT("WSPGetSockOpt - leave#1\n");
		return SOCKET_ERROR;
	}

	if (level == IPPROTO_SCTP && optname == SCTP_PEELOFF) {
		struct sctp_peeloff_opt *peeloff = NULL;
		SOCKET socket = INVALID_SOCKET;
		SOCKET_PEELOFF_REQUEST peeloffReq;

		DBGPRINT("WSPGetSockOpt:SCTP_PEELOFF - enter\n");

		if (*optlen < sizeof(struct sctp_peeloff_opt)) {
			*lpErrno = WSAEFAULT;
			ret = SOCKET_ERROR;
		}

		if (!ret) {
			peeloff = (struct sctp_peeloff_opt *)optval;

			if ((HANDLE)s != peeloff->s) {
				*lpErrno = WSAEFAULT;
				ret = SOCKET_ERROR;
			}
		}

		if (!ret) {

			socket = (SOCKET)CreateFile(WIN_SCTP_SOCKET_DEVICE_NAME,
					socketInfo->accessFlags,
					socketInfo->shareFlags,
					NULL,
					OPEN_EXISTING,
					socketInfo->attributeFlags,
					NULL);

			if ((HANDLE)socket == INVALID_HANDLE_VALUE) {
				SetErrorCode(lpErrno);
				ret = INVALID_SOCKET;
			}
		}

		if (!ret) {
			peeloffReq.assoc_id = peeloff->assoc_id;
			peeloffReq.socket = (HANDLE)socket;

			bSuccess = DeviceIoControl((HANDLE)s, IOCTL_SOCKET_PEELOFF,
						&peeloffReq, sizeof(peeloffReq),
						NULL, 0,
						&bytesReturned, NULL);

			DBGPRINT("IOCTL_SOCKET_SCTP_PEELOFF: bSuccess=%d\n", bSuccess);
			if (!bSuccess) {
				CloseHandle((HANDLE)socket);
				SetErrorCode(lpErrno);
				socket = INVALID_SOCKET;
			}
		}

		if (!ret) {
			hModifiedSocket = Upcalls.lpWPUModifyIFSHandle(socketInfo->dwCatalogEntryId, socket, lpErrno);
			if (hModifiedSocket == INVALID_SOCKET) {
				CloseHandle((HANDLE)socket);
				ret = SOCKET_ERROR;
			}
		}

		if (!ret) {
			/* we've just created another socket */
			/* so add a new socket info entry */
			struct SocketInfo *sockInfo = malloc(sizeof(struct SocketInfo));
			if (sockInfo == NULL) {
				*lpErrno = WSAENOBUFS;
				return SOCKET_ERROR;
			}

			ZeroMemory(sockInfo, sizeof(struct SocketInfo));
			sockInfo->s              = hModifiedSocket;
			sockInfo->accessFlags    = socketInfo->accessFlags;
			sockInfo->shareFlags     = socketInfo->shareFlags;
			sockInfo->attributeFlags = socketInfo->attributeFlags;
			sockInfo->events         = socketInfo->events;
			sockInfo->eventsHandle   = socketInfo->eventsHandle;
			sockInfo->localAddressLen= socketInfo->localAddressLen;
			sockInfo->remoteAddressLen= socketInfo->remoteAddressLen;
			sockInfo->dwCatalogEntryId= socketInfo->dwCatalogEntryId;
			CopyMemory(&(sockInfo->localAddress), &(socketInfo->localAddress), socketInfo->localAddressLen);
			CopyMemory(&(sockInfo->remoteAddress),&(socketInfo->remoteAddress),socketInfo->remoteAddressLen);
			TAILQ_INSERT_HEAD(&sockets, sockInfo, entries);
		}

		peeloff->new_sd = (HANDLE)hModifiedSocket;
		*optlen = sizeof(struct sctp_peeloff_opt);
		DBGPRINT("WSPGetSockOpt:SCTP_PEELOFF - leave\n");
	} else if (level == IPPROTO_SCTP && optname == SCTP_GET_HANDLE) {
		if (*optlen < sizeof(int)) {
			*lpErrno = WSAEFAULT;
			ret = SOCKET_ERROR;
		} else {
			SOCKET *sock = (SOCKET*)optval;
			*optlen = sizeof(SOCKET);
			*sock = s;
		}
	} else {
		optReq.level = level;
		optReq.optname = optname;
		optReq.optlen = *optlen;
		optReq.optval = optval;

		bSuccess = DeviceIoControl((HANDLE)s, IOCTL_SOCKET_GETSOCKOPT,
							(PVOID)&optReq, sizeof(optReq),
							&bytesTransferred, sizeof(bytesTransferred),
							&bytesReturned, NULL);

		DBGPRINT("IOCTL_SOCKET_GETSOCKOPT: bSuccess=%d,bytesReturned=%d\n", bSuccess, bytesReturned);
		if (bSuccess) {
			*optlen = bytesTransferred;
		} else {
			SetErrorCode(lpErrno);
			ret = INVALID_SOCKET;
		}

		DBGPRINT("WSPGetSockOpt - leave\n");
	}

	return ret;
}

BOOL
WSPAPI
WSPGetQOSByName(
	SOCKET s,
    LPWSABUF lpQOSName,
    LPQOS lpQOS,
    LPINT lpErrno)
{
	DBGPRINT("WSPGetQOSByName - enter\n");
	*lpErrno = WSAENETDOWN;
	DBGPRINT("WSPGetQOSByName - leave\n");
	return FALSE;
}

static GUID WSARecvMsg_GUID = WSAID_WSARECVMSG;
#if (_WIN32_WINNT >= 0x0600)
static GUID WSASendMsg_GUID = WSAID_WSASENDMSG;
#endif

int
WSPAPI
WSPIoctl(
	SOCKET s,
	DWORD dwIoControlCode,
	LPVOID lpvInBuffer,
	DWORD cbInBuffer,
    LPVOID lpvOutBuffer,
    DWORD cbOutBuffer,
    LPDWORD lpcbBytesReturned,
    LPWSAOVERLAPPED lpOverlapped,
    LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine,
    LPWSATHREADID lpThreadId,
    LPINT lpErrno)
{
	int ret = 0;

	DBGPRINT("WSPIoctl - enter\n");

	if (lpcbBytesReturned == NULL) {
		*lpErrno = WSAEFAULT;
		DBGPRINT("WSPIoctl - leave#1\n");
		return SOCKET_ERROR;
	}

	if (dwIoControlCode == SIO_GET_EXTENSION_FUNCTION_POINTER &&
				(lpvInBuffer == NULL || lpvOutBuffer == NULL ||
				cbInBuffer < sizeof(GUID))) {
		*lpErrno = WSAEFAULT;
		DBGPRINT("WSPIoctl - leave#3\n");
		return SOCKET_ERROR;
	}

	if (dwIoControlCode == SIO_GET_EXTENSION_FUNCTION_POINTER) {
		// Gets the function pointer for WSARecvMsg
		if (IsEqualGUID((GUID *)lpvInBuffer, &WSARecvMsg_GUID)) {
			if (cbOutBuffer >= sizeof(LPFN_WSARECVMSG)) {
				*(LPFN_WSARECVMSG *)lpvOutBuffer = &_WSARecvMsg;
			} else {
				*lpErrno = WSAEFAULT;
				ret = SOCKET_ERROR;
			}
		}
#if (_WIN32_WINNT >= 0x0600)
		else if (
		    IsEqualGUID((GUID *)lpvInBuffer, &WSASendMsg_GUID)) {
				// Gets the function pointer for WSASendMsg
				if (cbOutBuffer >= sizeof(LPFN_WSASENDMSG)) {
					*(LPFN_WSASENDMSG *)lpvOutBuffer = &_WSASendMsg;
				} else {
					*lpErrno = WSAEFAULT;
					ret = SOCKET_ERROR;
				}
		}
#endif
		else
		{
			*lpErrno = WSAEOPNOTSUPP;
			ret = SOCKET_ERROR;
		}
	} else {
		// Pass on any other request to the driver
		
		BOOL bSuccess;
		DWORD bytesReturned;
		SOCKET_IOCTL_REQUEST ioctlReq;

		RtlZeroMemory(&ioctlReq, sizeof(ioctlReq));
		ioctlReq.dwIoControlCode = dwIoControlCode;
		ioctlReq.lpvInBuffer = lpvInBuffer;
		ioctlReq.cbInBuffer = cbInBuffer;
		ioctlReq.lpvOutBuffer = lpvOutBuffer;
		ioctlReq.cbOutBuffer = cbOutBuffer;

		bSuccess = ConfigureOverlapped(lpOverlapped, s, NULL, lpCompletionRoutine, lpThreadId, lpErrno);
		
		bSuccess = DeviceIoControl((HANDLE)s, IOCTL_SOCKET_IOCTL,
						&ioctlReq, sizeof(ioctlReq),
						NULL, 0,
						&bytesReturned, lpOverlapped);

		DBGPRINT("IOCTL_SOCKET_IOCTL: bSuccess=%d, bytesReturned=%d\n", bSuccess, bytesReturned);

		if (lpOverlapped != NULL && lpCompletionRoutine != NULL &&
				(bSuccess || (!bSuccess && GetLastError() != ERROR_IO_PENDING))) {
			free(lpOverlapped->Pointer);
		}

		if (!bSuccess) {
			SetErrorCode(lpErrno);
			*lpErrno = WSAENETDOWN;
			ret = SOCKET_ERROR;
		}
	}

	if (lpcbBytesReturned != NULL) {
		/* We don't support any GET_ ioctls */
		*lpcbBytesReturned = 0;
	}

	DBGPRINT("WSPIoctl - leave\n");
	return ret;
}

SOCKET
WSPAPI
WSPJoinLeaf(
	SOCKET s,
	const struct sockaddr* name,
	int namelen,
	LPWSABUF lpCallerData,
    LPWSABUF lpCalleeData,
    LPQOS lpSQOS,
    LPQOS lpGQOS,
    DWORD dwFlags,
    LPINT lpErrno)
{
	DBGPRINT("WSPJoinLeaf - enter\n");
	*lpErrno = WSAENETDOWN;
	DBGPRINT("WSPJoinLeaf - leave\n");
	return INVALID_SOCKET;
}

int
WSPAPI
WSPListen(
	SOCKET s,
	int backlog,
    LPINT lpErrno)
{
	int ret = 0;
	BOOL bSuccess;
	DWORD bytesReturned;

	DBGPRINT("WSPListen - enter\n");

	bSuccess = DeviceIoControl((HANDLE)s, IOCTL_SOCKET_LISTEN,
					&backlog, sizeof(backlog),
					NULL, 0,
					&bytesReturned, NULL);

	DBGPRINT("IOCTL_SOCKET_LISTEN: bSuccess=%d\n", bSuccess);
	if (!bSuccess) {
		SetErrorCode(lpErrno);
		ret = SOCKET_ERROR;
	}

	DBGPRINT("WSPListen - leave\n");
	return ret;
}

/* This function gets called by WaitOverlappedComplete and runs in the context
 * of the original thread that called one of the WSA* functions
 */
void
CALLBACK
ApcFunc(
		DWORD_PTR dwContext)
{
	BOOL ret = TRUE;
	DWORD bytesTransferred;
	DWORD bytesReturned;
	DWORD status;

	LPWSAOVERLAPPED lpOverlapped = (LPWSAOVERLAPPED) dwContext;
	OverlappedInfo *info = (OverlappedInfo*)lpOverlapped->Pointer;

	ret = GetOverlappedResult((HANDLE)info->socket, lpOverlapped, &bytesReturned, FALSE);

	if (!ret)
		status = 0;
	else
		status = GetLastError();

	/* XXX check this */
	if (info->buffer != NULL) {
		bytesTransferred = (DWORD) *((DWORD*)info->buffer);

	} else {
		bytesTransferred = bytesReturned;
	}

	if (ret && status == ERROR_IO_PENDING)
		DBGPRINT("ApcFunc was called but I/O isn't finished!\n");

	info->lpCompletionRoutine(status, bytesTransferred,
				    lpOverlapped, lpOverlapped->OffsetHigh);
}


/* Thread entry point for overlapped I/O when a completion routine
 * is specified
 */
DWORD
WINAPI
WaitOverlappedComplete(
		LPVOID lpParameter)
{
	int Errno;
	int ret = 0;
	LPWSAOVERLAPPED lpOverlapped = (LPWSAOVERLAPPED)lpParameter;
	OverlappedInfo *info = (OverlappedInfo*)lpOverlapped->Pointer;

	DBGPRINT("WaitOverlappedComplete - enter\n");

	/* XXX error handling */
	WaitForSingleObject(lpOverlapped->hEvent, INFINITE);

	ret = Upcalls.lpWPUQueueApc(&info->threadId, ApcFunc, (DWORD_PTR)lpOverlapped, &Errno);
	DBGPRINT("WaitOverlappedComplete - leave\n");

	return ret;
}

int
WSPAPI
WSPRecv(
	SOCKET s,
    LPWSABUF lpBuffers,
    DWORD dwBufferCount,
    LPDWORD lpNumberOfBytesRecvd,
    LPDWORD lpFlags,
    LPWSAOVERLAPPED lpOverlapped,
    LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine,
    LPWSATHREADID lpThreadId,
    LPINT lpErrno)
{
	int ret = 0;
	SOCKET_RECV_REQUEST recvReq;
	BOOL bSuccess;
	DWORD bytesReturned;
	DWORD *bytesTransferred;
	INT byteCount = 0;

	DBGPRINT("WSPRecv - enter\n");

	if (lpBuffers == NULL || lpFlags == NULL) {
		*lpErrno = WSAEFAULT;
		DBGPRINT("WSPRecv - leave#1\n");
		return SOCKET_ERROR;
	}

	ZeroMemory(&recvReq, sizeof(recvReq));
	recvReq.lpBuffers = (PSOCKET_WSABUF)lpBuffers;
	recvReq.dwBufferCount = dwBufferCount;
	recvReq.lpFlags = lpFlags;

	if (lpOverlapped != NULL && lpCompletionRoutine != NULL) {
		bytesTransferred = malloc(sizeof(DWORD));
		ZeroMemory(bytesTransferred, sizeof(DWORD));
		byteCount = sizeof(DWORD);
		bSuccess = ConfigureOverlapped(lpOverlapped, s, bytesTransferred, lpCompletionRoutine, lpThreadId, lpErrno);
	} else {
		// bytesTransferred can be NULL if byte count isn't required
		bytesTransferred = lpNumberOfBytesRecvd;
		if (bytesTransferred != NULL)
			byteCount = sizeof(DWORD);
	}

	bSuccess = DeviceIoControl((HANDLE)s, IOCTL_SOCKET_RECV,
			(PVOID)&recvReq, sizeof(recvReq),
			bytesTransferred, byteCount,
			&bytesReturned,
			lpOverlapped);
			
	DBGPRINT("IOCTL_SOCKET_RECV: bSuccess=%d, bytesReturned=%d\n", bSuccess, bytesReturned);

	if (lpOverlapped != NULL && lpCompletionRoutine != NULL &&
			(bSuccess || (!bSuccess && GetLastError() != ERROR_IO_PENDING))) {
		free(lpOverlapped->Pointer);
	}

	if (!bSuccess) {
		SetErrorCode(lpErrno);
		ret = SOCKET_ERROR;
	}
	
	// If the operation fully completed now and we're doing overlapped IO
	// OR we're doing overlapped IO and the request isn't pending, then free the memory
	if ((bSuccess && lpOverlapped != NULL) || (!bSuccess && lpOverlapped != NULL && *lpErrno != ERROR_IO_PENDING)) {
		DBGPRINT("WSPRecv: got %d (%d) bytes\n", *lpNumberOfBytesRecvd, *bytesTransferred);

		*lpFlags = lpOverlapped->OffsetHigh;
		if (lpCompletionRoutine != NULL)
			free(bytesTransferred);
	}

	DBGPRINT("WSPRecv - leave\n");
	return ret;
}

int
WSPAPI
WSPRecvDisconnect(
	SOCKET s,
    LPWSABUF lpInboundDisconnectData,
    LPINT lpErrno)
{
	DBGPRINT("WSPRecvDisconnect - enter\n");
	*lpErrno = WSAEOPNOTSUPP;
	DBGPRINT("WSPRecvDisconnect - leave\n");
	return SOCKET_ERROR;
}

int
WSPAPI
WSPRecvFrom(
	SOCKET s,
    LPWSABUF lpBuffers,
    DWORD dwBufferCount,
    LPDWORD lpNumberOfBytesRecvd,
    LPDWORD lpFlags,
    struct sockaddr* lpFrom,
    LPINT lpFromlen,
    LPWSAOVERLAPPED lpOverlapped,
    LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine,
    LPWSATHREADID lpThreadId,
    LPINT lpErrno)
{
	int ret = 0;
	BOOL bSuccess;
	DWORD bytesReturned;
	SOCKET_RECV_REQUEST recvReq;
	DWORD *bytesTransferred;
	INT byteCount = 0;

	DBGPRINT("WSPRecvFrom - enter\n");

	if (lpBuffers == NULL || lpFlags == NULL) {
		*lpErrno = WSAEFAULT;
		DBGPRINT("WSPRecvFrom - leave#1\n");
		return SOCKET_ERROR;
	}

	recvReq.lpBuffers = (PSOCKET_WSABUF)lpBuffers;
	recvReq.dwBufferCount = dwBufferCount;
	recvReq.lpFrom = lpFrom;
	recvReq.lpFromlen = lpFromlen;
	recvReq.lpFlags = lpFlags;

	if (lpOverlapped != NULL && lpCompletionRoutine != NULL) {
		bytesTransferred = malloc(sizeof(DWORD));
		byteCount = sizeof(DWORD);
		bSuccess = ConfigureOverlapped(lpOverlapped, s, bytesTransferred, lpCompletionRoutine, lpThreadId, lpErrno);
	} else {
		bytesTransferred = lpNumberOfBytesRecvd;
		if (bytesTransferred != NULL)
			byteCount = sizeof(DWORD);
	}

	bSuccess = DeviceIoControl((HANDLE)s, IOCTL_SOCKET_RECVFROM,
			(PVOID)&recvReq, sizeof(recvReq),
			bytesTransferred, byteCount,
			&bytesReturned,
			lpOverlapped);

	if (lpOverlapped != NULL && lpCompletionRoutine != NULL &&
			(bSuccess || (!bSuccess && GetLastError() != ERROR_IO_PENDING)))
		free(lpOverlapped->Pointer);

	DBGPRINT("IOCTL_SOCKET_RECVFROM: bSuccess=%d,bytesReturned=%d\n", bSuccess, bytesReturned);
	if (!bSuccess) {
		SetErrorCode(lpErrno);
		ret = SOCKET_ERROR;
	}

	// If we're doing overlapped IO and the operation fully finished now, OR
	// we're doing overlapped IO and the operation isn't pending, free the memory
	if ((bSuccess && lpOverlapped != NULL) || (lpOverlapped != NULL && *lpErrno != ERROR_IO_PENDING)) {
		if (lpCompletionRoutine != NULL)
			free(bytesTransferred);

		if (lpOverlapped != NULL)
			*lpFlags = lpOverlapped->OffsetHigh;
	}

	DBGPRINT("WSPRecvFrom - leave\n");
	return ret;
}

/* our own function */
int
WSPAPI
_WSARecvMsg(
    SOCKET s,
    LPWSAMSG lpMsg,
    LPDWORD lpdwNumberOfBytesRecvd,
    LPWSAOVERLAPPED lpOverlapped,
    LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine)
{
	int ret = 0;
	BOOL bSuccess;
	DWORD bytesReturned;
	SOCKET_RECVMSG_REQUEST recvMsgReq;
	HANDLE hEvent = NULL;
	INT iErrno;
	WSATHREADID threadId;
	DWORD *bytesTransferred;
	INT byteCount = 0;

	DBGPRINT("WSARecvMsg - enter\n");

	if (lpMsg == NULL) {
		WSASetLastError(WSAEFAULT);
		DBGPRINT("WSARecvMsg - leave#1\n");
		return SOCKET_ERROR;
	}

	recvMsgReq.lpMsg = (PSOCKET_WSAMSG)lpMsg;

	if (lpOverlapped != NULL && lpCompletionRoutine != NULL) {
		threadId.ThreadHandle = GetCurrentThread();
		bytesTransferred = malloc(sizeof(DWORD));
		byteCount = sizeof(DWORD);
		bSuccess = ConfigureOverlapped(lpOverlapped, s, bytesTransferred, lpCompletionRoutine, &threadId, &iErrno);
	} else {
		bytesTransferred = lpdwNumberOfBytesRecvd;
		if (bytesTransferred != NULL)
			byteCount = sizeof(DWORD);
	}

	bSuccess = DeviceIoControl((HANDLE)s, IOCTL_SOCKET_RECVMSG,
        (PVOID)&recvMsgReq, sizeof(recvMsgReq),
        bytesTransferred, sizeof(*bytesTransferred),
        &bytesReturned, lpOverlapped);

	DBGPRINT("IOCTL_SOCKET_RECVMSG: bSuccess=%d,bytesReturned=%d\n", bSuccess, bytesReturned);
	if (lpOverlapped != NULL && lpCompletionRoutine != NULL &&
			(bSuccess || (!bSuccess && GetLastError() != ERROR_IO_PENDING)))
		free(lpOverlapped->Pointer);

	if (!bSuccess) {
		SetErrorCode(NULL);
		ret = SOCKET_ERROR;
	}

	if ((bSuccess && lpOverlapped != NULL) || (lpOverlapped != NULL && GetLastError() != ERROR_IO_PENDING)) {
		if (lpCompletionRoutine != NULL)
			free(bytesTransferred);
	}

	DBGPRINT("WSARecvMsg - leave\n");
	return ret;
}

int
WSPAPI
WSPSelect(
	int nfds,
    fd_set* readfds,
    fd_set* writefds,
    fd_set* exceptfds,
    const struct timeval *timeout,
    LPINT lpErrno)
{
	int ret = 0;
	BOOL bSuccess;
	DWORD bytesReturned;
	SOCKET s = INVALID_SOCKET;
	SOCKET_SELECT_REQUEST selectReq;

	DBGPRINT("WSPSelect - enter\n");

	getsocket(readfds, &s);
	getsocket(writefds, &s);
	getsocket(exceptfds, &s);

	ZeroMemory(&selectReq, sizeof(selectReq));
	selectReq.fd_setsize = FD_SETSIZE;
	selectReq.readfds = (PSOCKET_FD_SET)readfds;
	selectReq.writefds = (PSOCKET_FD_SET)writefds;
	selectReq.exceptfds = (PSOCKET_FD_SET)exceptfds;

	if (timeout != NULL) {
		*((struct timeval *)&selectReq.timeout) = *timeout;
	} else {
		selectReq.infinite = TRUE;
	}

	bSuccess = DeviceIoControl((HANDLE)s, IOCTL_SOCKET_SELECT,
					(PVOID)&selectReq, sizeof(selectReq),
					(PVOID)&selectReq, sizeof(selectReq),
					&bytesReturned, NULL);

	DBGPRINT("IOCTL_SOCKET_SELECT: bSuccess=%d,selectReq.nfds=%d\n", bSuccess, selectReq.nfds);
	if (bSuccess) {
		ret = selectReq.nfds;
	} else {
		SetErrorCode(lpErrno);
		ret = SOCKET_ERROR;
	}

	DBGPRINT("WSPSelect - leave\n");
	return ret;
}


int
WSPAPI
WSPSend(
	SOCKET s,
	LPWSABUF lpBuffers,
	DWORD dwBufferCount,
    LPDWORD lpNumberOfBytesSent,
    DWORD dwFlags,
    LPWSAOVERLAPPED lpOverlapped,
    LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine,
    LPWSATHREADID lpThreadId,
    LPINT lpErrno)
{
	int ret = 0;
	HANDLE hEvent = NULL;
	SOCKET_SEND_REQUEST sendReq;
	BOOL bSuccess;
	DWORD bytesReturned;
	DWORD *bytesTransferred = NULL;
	INT byteCount = 0;

	DBGPRINT("WSPSend - enter\n");

	if (lpNumberOfBytesSent == NULL && lpOverlapped == NULL) {
		*lpErrno = WSAEFAULT;
		DBGPRINT("WSPSend - leave#1\n");
		return SOCKET_ERROR;
	}

	if (lpOverlapped != NULL && lpCompletionRoutine != NULL) {
		bytesTransferred = malloc(sizeof(DWORD));
		ZeroMemory(bytesTransferred, sizeof(DWORD));
		byteCount = sizeof(DWORD);
		bSuccess = ConfigureOverlapped(lpOverlapped, s, bytesTransferred, lpCompletionRoutine, lpThreadId, lpErrno);
	} else {
		bytesTransferred = lpNumberOfBytesSent;
		if (bytesTransferred != NULL)
			byteCount = sizeof(DWORD);
	}

	ZeroMemory(&sendReq, sizeof(sendReq));
	sendReq.lpBuffers = (PSOCKET_WSABUF)lpBuffers;
	sendReq.dwBufferCount = dwBufferCount;
	sendReq.dwFlags = dwFlags;

	bSuccess = DeviceIoControl((HANDLE)s, IOCTL_SOCKET_SEND,
			(PVOID)&sendReq, sizeof(sendReq),
			bytesTransferred, byteCount,
			&bytesReturned, lpOverlapped);

	DBGPRINT("IOCTL_SOCKET_SEND: bSuccess=%d, bytesReturned=%d, bytesTransferred=%d\n", bSuccess, bytesReturned, *bytesTransferred);
	if (lpOverlapped != NULL && lpCompletionRoutine != NULL &&
			(bSuccess || (!bSuccess && GetLastError() != ERROR_IO_PENDING)))
		free(lpOverlapped->Pointer);

	if (!bSuccess) {
		SetErrorCode(lpErrno);
		ret = SOCKET_ERROR;
	}

	if ((bSuccess && lpOverlapped != NULL) || (!bSuccess && lpOverlapped != NULL && *lpErrno != ERROR_IO_PENDING)) {
		if (lpCompletionRoutine != NULL)
			free(bytesTransferred);
	}

	DBGPRINT("WSPSend - leave\n");
	return ret;
}

int
WSPAPI
WSPSendDisconnect(
    SOCKET s,
    LPWSABUF lpOutboundDisconnectData,
    LPINT lpErrno)
{
	DBGPRINT("WSPSendDisconnect - enter\n");
	*lpErrno = WSAEOPNOTSUPP;
	DBGPRINT("WSPSendDisconnect - leave\n");
	return SOCKET_ERROR;
}

int
WSPAPI
WSPSendTo(
	SOCKET s,
	LPWSABUF lpBuffers,
	DWORD dwBufferCount,
    LPDWORD lpNumberOfBytesSent,
    DWORD dwFlags,
    const struct sockaddr *lpTo,
    int iTolen,
    LPWSAOVERLAPPED lpOverlapped,
    LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine,
    LPWSATHREADID lpThreadId,
    LPINT lpErrno)
{
	int ret = 0;
	SOCKET_SEND_REQUEST sendReq;
	HANDLE hEvent = NULL;
	BOOL bSuccess;
	DWORD bytesReturned;
	DWORD *bytesTransferred;
	INT byteCount = 0;

	DBGPRINT("WSPSendTo - enter\n");

	// User must either give a pointer to hold the number of bytes sent, or 
	// be doing overlapped IO
	if (lpNumberOfBytesSent == NULL && lpOverlapped == NULL) {
		ret = SOCKET_ERROR;
		*lpErrno = WSAEFAULT;
		DBGPRINT("WSPSendTo - leave#1\n");
		return SOCKET_ERROR;
	}

	if (lpOverlapped != NULL && lpCompletionRoutine != NULL) {
		bytesTransferred = malloc(sizeof(DWORD));
		byteCount = sizeof(DWORD);
		bSuccess = ConfigureOverlapped(lpOverlapped, s, bytesTransferred, lpCompletionRoutine, lpThreadId, lpErrno);
	} else {
		bytesTransferred = lpNumberOfBytesSent;
		if (bytesTransferred != NULL)
			byteCount = sizeof(DWORD);
	}

	sendReq.lpBuffers = (PSOCKET_WSABUF)lpBuffers;
	sendReq.dwBufferCount = dwBufferCount;
	sendReq.dwFlags = dwFlags;
	sendReq.lpTo = (struct sockaddr *)lpTo;
	sendReq.iTolen = iTolen;

	bSuccess = DeviceIoControl((HANDLE)s, IOCTL_SOCKET_SENDTO,
			(PVOID)&sendReq, sizeof(sendReq),
			bytesTransferred, byteCount,
			&bytesReturned, lpOverlapped);

	DBGPRINT("IOCTL_SOCKET_SEND: bSuccess=%d\n", bSuccess);
	if (lpOverlapped != NULL && lpCompletionRoutine != NULL &&
			(bSuccess || (!bSuccess && GetLastError() != ERROR_IO_PENDING)))
		free(lpOverlapped->Pointer);

	if (!bSuccess) {
		SetErrorCode(lpErrno);
		ret = SOCKET_ERROR;
	}

	if ((bSuccess && lpOverlapped != NULL) || (lpOverlapped != NULL && *lpErrno != ERROR_IO_PENDING)) {
		if (lpCompletionRoutine != NULL)
			free(bytesTransferred);
	}

	DBGPRINT("WSPSendTo - leave\n");
	return ret;
}

int
WSPAPI
_WSASendMsg(
	SOCKET s,
	LPWSAMSG lpMsg,
	DWORD dwFlags,
    LPDWORD lpNumberOfBytesSent,
    LPWSAOVERLAPPED lpOverlapped,
    LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine)
{
	int ret = 0;
	SOCKET_SENDMSG_REQUEST sendMsgReq;
	HANDLE hEvent = NULL;
	BOOL bSuccess;
	DWORD bytesReturned;
	DWORD *bytesTransferred;
	WSATHREADID threadId;
	INT iErrno;
	INT byteCount = 0;

	DBGPRINT("WSASendMsg - enter\n");

	if (lpMsg == NULL || (lpNumberOfBytesSent == NULL && lpOverlapped == NULL)) {
		WSASetLastError(WSAEFAULT);
		DBGPRINT("WSASendMsg - leave#1\n");
		return SOCKET_ERROR;
	}

	threadId.ThreadHandle = GetCurrentThread();
	threadId.Reserved = 0;

	if (lpOverlapped != NULL && lpCompletionRoutine != NULL) {
		bytesTransferred = malloc(sizeof(DWORD));
		byteCount = sizeof(DWORD);
		bSuccess = ConfigureOverlapped(lpOverlapped, s, bytesTransferred, lpCompletionRoutine, &threadId, &iErrno);
	} else {
		// lpNumberOfBytesSent can be NULL, in which case it's ignored
		bytesTransferred = lpNumberOfBytesSent;
		if (bytesTransferred != NULL)
			byteCount = sizeof(DWORD);
	}

	sendMsgReq.lpMsg = (PSOCKET_WSAMSG)lpMsg;
	sendMsgReq.dwFlags = dwFlags;

	bSuccess = DeviceIoControl((HANDLE)s, IOCTL_SOCKET_SENDMSG,
			(PVOID)&sendMsgReq, sizeof(sendMsgReq),
			bytesTransferred, byteCount,
			&bytesReturned, lpOverlapped);

	DBGPRINT("IOCTL_SOCKET_SENDMSG: bSuccess=%d\n", bSuccess);
	if (lpOverlapped != NULL && lpCompletionRoutine != NULL &&
			(bSuccess || (!bSuccess && GetLastError() != ERROR_IO_PENDING)))
		free(lpOverlapped->Pointer);

	if (!bSuccess) {
		SetErrorCode(NULL);
		ret = SOCKET_ERROR;
	}
		
	if (bSuccess && lpOverlapped != NULL && lpNumberOfBytesSent != NULL) {
			*lpNumberOfBytesSent = *bytesTransferred;
	}

	if ((bSuccess && lpOverlapped != NULL) || (lpOverlapped != NULL && GetLastError() != ERROR_IO_PENDING)) {
		if (lpCompletionRoutine != NULL)
			free(bytesTransferred);
	}

	DBGPRINT("WSPSendMsg - leave\n");
	return ret;
}

int
WSPAPI
WSPSetSockOpt(
	SOCKET s,
	int level,
	int optname,
	const char *optval,
	int optlen,
	LPINT lpErrno)
{
	int ret = 0;
	SOCKET_SOCKOPT_REQUEST optReq;
	BOOL bSuccess;
	DWORD bytesReturned;

	DBGPRINT("WSPSetSockOpt - enter\n");

	if ((optval == NULL && optlen != 0) || optlen < 0) {
		*lpErrno = WSAEFAULT;
		DBGPRINT("WSPSetSockOpt - leave#1\n");
		return SOCKET_ERROR;
	}

	optReq.level = level;
	optReq.optname = optname;
	optReq.optval = (char *)optval;
	optReq.optlen = optlen;

	bSuccess = DeviceIoControl((HANDLE)s, IOCTL_SOCKET_SETSOCKOPT,
							(PVOID)&optReq, sizeof(optReq),
							NULL, 0,
							&bytesReturned, NULL);

	DBGPRINT("IOCTL_SOCKET_SETSOCKOPT: bSuccess=%d\n", bSuccess);
	if (!bSuccess) {
		SetErrorCode(lpErrno);
		DBGPRINT("WSPSetSockOpt - leave#2\n");
		ret = SOCKET_ERROR;
	}

	DBGPRINT("WSPSetSockOpt - leave\n");
	return ret;
}

int
WSPAPI
WSPShutdown(
	SOCKET s,
	int how,
    LPINT lpErrno)
{
	int ret = 0;
	BOOL bSuccess;
	DWORD bytesReturned;

	DBGPRINT("WSPShutdown - enter\n");

	if (how != SD_RECEIVE && how != SD_SEND && how != SD_BOTH) {
		*lpErrno = WSAEINVAL;
		DBGPRINT("WSPShutdown - leave#1\n");
		return SOCKET_ERROR;
	}

	bSuccess = DeviceIoControl((HANDLE)s, IOCTL_SOCKET_SHUTDOWN,
						&how, sizeof(how),
						NULL, 0,
						&bytesReturned, NULL);

	DBGPRINT("IOCTL_SOCKET_SHUTDOWN: bSuccess=%d\n", bSuccess);
	if (!bSuccess) {
		SetErrorCode(lpErrno);
		ret = SOCKET_ERROR;
	}

	DBGPRINT("WSPShutdown - leave\n");
	return ret;
}

SOCKET
WSPAPI
WSPSocket(
	int af,
	int type,
	int protocol,
	LPWSAPROTOCOL_INFO lpProtocolInfo,
	GROUP g,
    DWORD dwFlags,
    LPINT lpErrno)
{
	SOCKET socket = INVALID_SOCKET;
	SOCKET hModifiedSocket = INVALID_SOCKET;
	SOCKET_OPEN_REQUEST openReq;
	BOOL bSuccess;
	DWORD bytesReturned;
	DWORD accessFlags    = (GENERIC_READ | GENERIC_WRITE);
	DWORD shareFlags     = (FILE_SHARE_READ | FILE_SHARE_WRITE);
	DWORD attributeFlags = FILE_ATTRIBUTE_NORMAL;

	/* We only support SCTP - make sure we're being asked for IPPROTO_SCTP */
	if (protocol != IPPROTO_SCTP ||
	   (protocol == FROM_PROTOCOL_INFO && lpProtocolInfo->iProtocol != IPPROTO_SCTP))
	{
		*lpErrno = WSAEAFNOSUPPORT;
		return INVALID_SOCKET;
	}

	if (dwFlags & WSA_FLAG_OVERLAPPED)
		attributeFlags |= FILE_FLAG_OVERLAPPED;

	DBGPRINT("WSPSocket - enter\n");

	/* Check to see if WSADuplicateSocket has been called and we've been given a socket */
	if (lpProtocolInfo != NULL && GetSocketInfo(lpProtocolInfo->dwProviderReserved) != NULL) {
		socket = lpProtocolInfo->dwProviderReserved;
	} else {
		socket = (SOCKET)CreateFile(WIN_SCTP_SOCKET_DEVICE_NAME,
				accessFlags,
				shareFlags,
				NULL,
				OPEN_EXISTING,
				attributeFlags,
				NULL);
	}

	if ((HANDLE)socket == INVALID_HANDLE_VALUE) {
		SetErrorCode(lpErrno);
		DBGPRINT("WSPSocket - leave#1\n");
		socket = INVALID_SOCKET;
	}

	if (socket != INVALID_SOCKET) {
		openReq.af = af;
		openReq.type = type;
		openReq.protocol = protocol;

		if (af == FROM_PROTOCOL_INFO)
			openReq.af = lpProtocolInfo->iAddressFamily;
		if (type == FROM_PROTOCOL_INFO)
			openReq.type = lpProtocolInfo->iSocketType;
		if (protocol == FROM_PROTOCOL_INFO)
			openReq.protocol = lpProtocolInfo->iProtocol;

		bSuccess = DeviceIoControl((HANDLE)socket, IOCTL_SOCKET_OPEN,
						(PVOID)&openReq, sizeof(openReq),
						NULL, 0,
						&bytesReturned, NULL);

		DBGPRINT("IOCTL_SOCKET_OPEN: bSuccess=%d\n", bSuccess);
		if (!bSuccess) {
			SetErrorCode(lpErrno);
			socket = INVALID_SOCKET;
		}
	}

	if (socket != INVALID_SOCKET) {
		hModifiedSocket = Upcalls.lpWPUModifyIFSHandle(lpProtocolInfo->dwCatalogEntryId, socket, lpErrno);
		if (hModifiedSocket == INVALID_SOCKET) {
			CloseHandle((HANDLE)socket);
		}
	}

	if (hModifiedSocket != INVALID_SOCKET)
	{
		/* Add a new socket info entry */
		struct SocketInfo *sockInfo = malloc(sizeof(struct SocketInfo));
		if (sockInfo == NULL) {
			*lpErrno = WSAENOBUFS;
			return INVALID_SOCKET;
		}

		ZeroMemory(sockInfo, sizeof(struct SocketInfo));
		sockInfo->s              = hModifiedSocket;
		sockInfo->accessFlags    = accessFlags;
		sockInfo->shareFlags     = shareFlags;
		sockInfo->attributeFlags = attributeFlags;
		sockInfo->events         = 0;
		sockInfo->eventsHandle   = INVALID_HANDLE_VALUE;
		sockInfo->dwCatalogEntryId = lpProtocolInfo->dwCatalogEntryId;
		TAILQ_INSERT_TAIL(&sockets, sockInfo, entries);
	}

	DBGPRINT("WSPSocket - leave\n");
	return hModifiedSocket;
}

typedef INT (WINAPI *PWSH_STRING_TO_ADDRESS)(LPWSTR,DWORD,LPWSAPROTOCOL_INFOW,LPSOCKADDR,LPDWORD);
static PWSH_STRING_TO_ADDRESS WSHStringToAddress = NULL;
static PWSH_STRING_TO_ADDRESS WSHStringToAddressIPv6 = NULL;

int
WSPAPI
WSPStringToAddress(
	LPWSTR AddressString,
	INT AddressFamily,
	LPWSAPROTOCOL_INFOW lpProtocolInfo,
	LPSOCKADDR lpAddress,
    LPINT lpAddressLength,
    LPINT lpErrno)
{
	int ret = 0;

	DBGPRINT("WSPStringToAddress - enter\n");
	switch (AddressFamily) {
	case AF_INET:
		*lpErrno = WSHStringToAddress(AddressString, AddressFamily,
		    NULL, lpAddress, lpAddressLength);
		break;
	case AF_INET6:
		*lpErrno = WSHStringToAddressIPv6(AddressString, AddressFamily,
		    NULL, lpAddress, lpAddressLength);
		break;
	default:
		*lpErrno = WSAEINVAL;
	}

	if (*lpErrno != 0) {
		ret = SOCKET_ERROR;
	}

	DBGPRINT("WSPStringToAddress - leave\n");
	return ret;
}

ssize_t
WSPAPI
sctp_generic_recvmsg(
    SOCKET s,
    char *data,
    size_t len,
    struct sockaddr *from,
    socklen_t *fromlen,
    struct sctp_sndrcvinfo *sinfo,
    int *msg_flags)
{
	/* ret is the number of bytes received. So, handle errors
	 * the opposite way around from normal here - set rc to the error value */
	ssize_t ret = -1;
	BOOL bSuccess;
	DWORD bytesReturned;
	DWORD bytesTransferred;
	SOCKET_SCTPRECV_REQUEST sctpRecvReq;

	DBGPRINT("sctp_generic_recvmsg - enter\n");

	if (data == NULL || len <= 0 || (from != NULL && (fromlen == NULL || *fromlen == 0))) {
		WSASetLastError(WSAEFAULT);
		DBGPRINT("sctp_generic_recvmsg - leave#1\n");
		return -1;
	}

	sctpRecvReq.data = data;
	sctpRecvReq.len = len;
	sctpRecvReq.from = from;
	sctpRecvReq.fromlen = fromlen;
	sctpRecvReq.sinfo = sinfo;
	sctpRecvReq.msg_flags = msg_flags;

	bSuccess = DeviceIoControl((HANDLE)s, IOCTL_SOCKET_SCTPRECV,
						(PVOID)&sctpRecvReq, sizeof(sctpRecvReq),
						&bytesTransferred, sizeof(bytesTransferred),
						&bytesReturned, NULL);

	DBGPRINT("IOCTL_SOCKET_SCTPRECV: bSuccess=%d,bytesReturned=%d\n", bSuccess, bytesReturned);

	if (bSuccess) {
		ret = (ssize_t)bytesTransferred;
	} else {
		SetErrorCode(NULL);
		ret = -1;
	}

	DBGPRINT("sctp_generic_recvmsg - leave\n");
	return ret;
}

int
WSPAPI
sctp_generic_sendmsg(
    SOCKET s,
    const void *data,
    size_t len,
    const struct sockaddr *to,
    int tolen,
    const struct sctp_sndrcvinfo *sinfo,
    int flags)
{
	int ret = -1;
	BOOL bSuccess;
	DWORD bytesReturned;
	DWORD bytesTransferred;
	SOCKET_SCTPSEND_REQUEST sctpSendReq;

	DBGPRINT("sctp_generic_sendmsg - enter\n");

	if (data == NULL || len <= 0 || (to == NULL && tolen > 0)) {
		WSASetLastError(WSAEFAULT);
		DBGPRINT("sctp_generic_sendmsg - leave#1\n");
		return -1;
	}

	sctpSendReq.data = (void *)data;
	sctpSendReq.len = len;
	sctpSendReq.to = (struct sockaddr *)to;
	sctpSendReq.tolen = tolen;
	sctpSendReq.sinfo = (struct sctp_sndrcvinfo *)sinfo;
	sctpSendReq.flags = flags;

	bSuccess = DeviceIoControl((HANDLE)s, IOCTL_SOCKET_SCTPSEND,
						(PVOID)&sctpSendReq, sizeof(sctpSendReq),
						&bytesTransferred, sizeof(bytesTransferred),
						&bytesReturned, NULL);

	DBGPRINT("IOCTL_SOCKET_SCTPSEND: bSuccess=%d,bytesReturned=%d\n", bSuccess, bytesReturned);
	if (bSuccess) {
		ret = (ssize_t)bytesTransferred;
	} else {
		SetErrorCode(NULL);
		ret = -1;
	}

	DBGPRINT("sctp_generic_sendmsg - leave\n");
	return ret;
}

int
WSPAPI
WSPStartup(
	WORD wVersionRequested,
    LPWSPDATA lpWSPData,
    LPWSAPROTOCOL_INFOW lpProtocolInfo,
    WSPUPCALLTABLE UpcallTable,
    LPWSPPROC_TABLE lpProcTable)
{
	DBGPRINT("WSPStartup - enter\n");

	TAILQ_INIT(&sockets);

	Upcalls = UpcallTable;

	lpProcTable->lpWSPAccept = WSPAccept;
	lpProcTable->lpWSPAddressToString = WSPAddressToString;
	lpProcTable->lpWSPAsyncSelect = WSPAsyncSelect;
	lpProcTable->lpWSPBind = WSPBind;
	lpProcTable->lpWSPCancelBlockingCall = WSPCancelBlockingCall;
	lpProcTable->lpWSPCleanup = WSPCleanup;
	lpProcTable->lpWSPCloseSocket = WSPCloseSocket;
	lpProcTable->lpWSPConnect = WSPConnect;
	lpProcTable->lpWSPDuplicateSocket = WSPDuplicateSocket;
	lpProcTable->lpWSPEnumNetworkEvents = WSPEnumNetworkEvents;
	lpProcTable->lpWSPEventSelect = WSPEventSelect;
	lpProcTable->lpWSPGetOverlappedResult = WSPGetOverlappedResult;
	lpProcTable->lpWSPGetPeerName = WSPGetPeerName;
	lpProcTable->lpWSPGetSockName = WSPGetSockName;
	lpProcTable->lpWSPGetSockOpt = WSPGetSockOpt;
	lpProcTable->lpWSPGetQOSByName = WSPGetQOSByName;
	lpProcTable->lpWSPIoctl = WSPIoctl;
	lpProcTable->lpWSPJoinLeaf = WSPJoinLeaf;
	lpProcTable->lpWSPListen = WSPListen;
	lpProcTable->lpWSPRecv = WSPRecv;
	lpProcTable->lpWSPRecvDisconnect = WSPRecvDisconnect;
	lpProcTable->lpWSPRecvFrom = WSPRecvFrom;
	lpProcTable->lpWSPSelect = WSPSelect;
	lpProcTable->lpWSPSend = WSPSend;
	lpProcTable->lpWSPSendDisconnect = WSPSendDisconnect;
	lpProcTable->lpWSPSendTo = WSPSendTo;
	lpProcTable->lpWSPSetSockOpt = WSPSetSockOpt;
	lpProcTable->lpWSPShutdown = WSPShutdown;
	lpProcTable->lpWSPSocket = WSPSocket;
	lpProcTable->lpWSPStringToAddress = WSPStringToAddress;

	lpWSPData->wVersion = MAKEWORD(2, 2);
	lpWSPData->wHighVersion = MAKEWORD(2, 2);

	hIPv4Dll = LoadLibrary(TEXT("wship"));
	if (hIPv4Dll != NULL) {
		WSHAddressToString = (PWSH_ADDRESS_TO_STRING)GetProcAddress(hIPv4Dll, "WSHAddressToString");
		WSHStringToAddress = (PWSH_STRING_TO_ADDRESS)GetProcAddress(hIPv4Dll, "WSHStringToAddress");
	}

	hIPv6Dll = LoadLibrary(TEXT("wship6"));
	if (hIPv6Dll != NULL) {
		WSHAddressToStringIPv6 = (PWSH_ADDRESS_TO_STRING)GetProcAddress(hIPv6Dll, "WSHAddressToString");
		WSHStringToAddressIPv6 = (PWSH_STRING_TO_ADDRESS)GetProcAddress(hIPv6Dll, "WSHStringToAddress");
	}

	DBGPRINT("WSPStartup - leave\n");
	return 0;
}

BOOL
WINAPI
DllMain(
    HINSTANCE hinstDLL,
    DWORD fdwReason,
    LPVOID lpvReserved)
{
	DBGPRINT("DllMain - enter\n");

	switch (fdwReason) {
	case DLL_PROCESS_ATTACH:
		DisableThreadLibraryCalls(hinstDLL);
		break;
	case DLL_THREAD_ATTACH:
	case DLL_PROCESS_DETACH:
	case DLL_THREAD_DETACH:
	default:
		break;
	}

	DBGPRINT("DllMain - leave\n");
	return TRUE;
}
