/*
 * Copyright (c) 2010 Bruce Cran.
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
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include "StdAfx.h"
#include "SctpSocket.h"
#include "SctpSocketAsyncResult.h"

using namespace System::Threading;
using namespace System::Diagnostics;
using namespace System::IO;
using namespace System::Runtime::InteropServices;
using namespace SctpDrv;

SctpSocket::SctpSocket(AddressFamily ^addressFamily, SocketType ^socketType)
{
	int af = AF_INET;
	int st = SOCK_STREAM;

	m_listening = false;

	if (addressFamily == AddressFamily::InterNetwork)
		af = AF_INET;
	else if (addressFamily == AddressFamily::InterNetworkV6)
		af = AF_INET6;

	if (socketType == SocketType::Stream)
		st = SOCK_STREAM;
	else if (socketType == SocketType::Dgram)
		st = SOCK_DGRAM;
	else if (socketType == SocketType::Seqpacket)
		st = SOCK_SEQPACKET;

	if (!m_winsock_initialized) {
		WSADATA wsd;
		WSAStartup(MAKEWORD(2,2), &wsd);
		m_winsock_initialized = true;
	}

	// We now have one more object using Winsock
	m_instances++;

	m_socket = WSASocket(af, st, IPPROTO_SCTP, NULL, 0, WSA_FLAG_OVERLAPPED);

	if (m_socket == INVALID_SOCKET)
		throw gcnew SocketException();

	// Set up the completion port and worker thread
	m_completionPort = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
	m_stopping = false;
	m_stopped = false;
	Thread ^t = gcnew Thread(gcnew ThreadStart(this, &SctpSocket::IoCompletionThread));
	t->Name = "IO Completion Thread";
	t->Start();

	// Associate the completion port with the socket
	if (CreateIoCompletionPort((HANDLE)m_socket, m_completionPort, 0, 0) == NULL)
		throw gcnew SocketException();
}

SctpSocket::SctpSocket(SOCKET sock)
{
	m_socket = sock;

	// We don't need to initialize Winsock here, because it must have
	// already been done.

	// We now have one more object using Winsock
	m_instances++;

	m_completionPort = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
	m_stopping = false;
	m_stopped = false;
	Thread ^t = gcnew Thread(gcnew ThreadStart(this, &SctpSocket::IoCompletionThread));
	t->Name = "IO Completion Thread";
	t->Start();
	// Associate the completion port with the socket
	if (CreateIoCompletionPort((HANDLE)m_socket, m_completionPort, 0, 0) == NULL)
		throw gcnew SocketException();
}

void
SctpSocket::IoCompletionThread()
{
	DWORD bytes;
	ULONG_PTR completionKey;
	BOOL ioOK;
	PER_IO_DATA *perIO;
	OVERLAPPED *ovl;
	const DWORD timeout = INFINITE;

	while (!m_stopping) {
		ioOK = GetQueuedCompletionStatus(m_completionPort, &bytes, &completionKey, &ovl, timeout);

		if (!ioOK) {
			DWORD err = GetLastError();

			if (err == WAIT_TIMEOUT)
				continue;
			if (err == ERROR_ABANDONED_WAIT_0)
				break;
			else
				throw gcnew SocketException();
		}

		// Successfully dequeued a packet, so get its info
		perIO = CONTAINING_RECORD(ovl, PER_IO_DATA, overlapped);

		if (perIO->cb != NULL) {
			GCHandle ^cbHandle = GCHandle::FromIntPtr((System::IntPtr)perIO->cb);
			AsyncCallback ^cb = (AsyncCallback^)cbHandle->Target;
			GCHandle ^arHandle = GCHandle::FromIntPtr((System::IntPtr)perIO->asyncResult);
			IAsyncResult ^ar = (IAsyncResult^)arHandle->Target;
			cb(ar);
			cbHandle->Free();
			arHandle->Free();
			delete perIO->sgBuffer;
		}

		if (perIO->hIoProcessed != INVALID_HANDLE_VALUE) {
			if ((perIO->operation == SEND || perIO->operation == RECV) && perIO->ioByteCount == 0)
				continue;

			if (SetEvent(perIO->hIoProcessed) == FALSE)
				throw gcnew SocketException();
		}
	}

	m_stopped = true;
}

SctpSocket::!SctpSocket()
{
	m_stopping = true;
	int wait = 0;
	
	while (!m_stopped && wait < 50) { 
		Sleep(100);
		wait++;
	}

	Debug::Assert(wait < 50);

	if (--m_instances == 0)
		WSACleanup();
}

SctpSocket::~SctpSocket()
{

}

SctpSocket^
SctpSocket::Accept()
{
	if (!m_listening)
		throw gcnew InvalidOperationException();

	SOCKET sock = accept(m_socket, NULL, NULL);
	
	if (sock == INVALID_SOCKET)
		throw gcnew SocketException();

	return gcnew SctpSocket(sock);
}
 
void
SctpSocket::Bind(EndPoint ^localEP)
{
	if (localEP == nullptr)
		throw gcnew ArgumentNullException();

	struct sockaddr *sa;
	int size;
	struct sockaddr_in in;
	struct sockaddr_in6 in6;
	IPEndPoint ^ipEndPoint = (IPEndPoint^)localEP;

	if (localEP->AddressFamily == AddressFamily::InterNetwork) {
		sa = (struct sockaddr*)&in;
		size = sizeof(in);
		in.sin_family = AF_INET;
		in.sin_port = htons(ipEndPoint->Port);
		array<unsigned char> ^addrBytes = ipEndPoint->Address->GetAddressBytes();
		pin_ptr<unsigned char> bytes = &addrBytes[0];
		memcpy(&in.sin_addr.s_addr, bytes, addrBytes->Length);
	}
	else if (localEP->AddressFamily == AddressFamily::InterNetworkV6) {
		sa = (struct sockaddr*)&in6;
		size = sizeof(in6);
		in6.sin6_family = AF_INET6;
		in6.sin6_port = htons(ipEndPoint->Port);
		array<unsigned char> ^addrBytes = ipEndPoint->Address->GetAddressBytes();
		pin_ptr<unsigned char> bytes = &addrBytes[0];
		memcpy(&in6.sin6_addr, bytes, addrBytes->Length);
	}

	if (bind(m_socket, sa, size) ==	SOCKET_ERROR)
		throw gcnew SocketException();
}

void
SctpSocket::Close()
{
	m_stopping = true;
	closesocket(m_socket);
	m_socket = INVALID_SOCKET;
}

void
SctpSocket::Close(int timeout)
{
	linger l;
	l.l_onoff = 1;
	l.l_linger = timeout;
	setsockopt(m_socket, SOL_SOCKET, SO_LINGER, (const char*)&l, sizeof(struct linger));
	Close();
}

struct sockaddr* SctpSocket::GetSockAddr(EndPoint ^ep, int& namelen)
{
	struct sockaddr *sa = nullptr;
	IPEndPoint ^ipEP = (IPEndPoint^)ep;

	if (ipEP->AddressFamily == AddressFamily::InterNetwork) {
		struct sockaddr_in *in = new struct sockaddr_in;
		sa = (struct sockaddr*)in;
		namelen = sizeof(*in);
		in->sin_family = AF_INET;
		in->sin_port = htons(ipEP->Port);
		array<unsigned char> ^addrBytes = ipEP->Address->GetAddressBytes();
		pin_ptr<unsigned char> bytes = &addrBytes[0];
		memcpy(&in->sin_addr.s_addr, bytes, addrBytes->Length);
	} else {
		struct sockaddr_in6 *in6 = new struct sockaddr_in6;
		sa = (struct sockaddr*)in6;
		namelen = sizeof(*in6);
		in6->sin6_family = AF_INET6;
		in6->sin6_port = htons(ipEP->Port);
		array<unsigned char> ^addrBytes = ipEP->Address->GetAddressBytes();
		pin_ptr<unsigned char> bytes = &addrBytes[0];
		memcpy(&in6->sin6_addr, bytes, addrBytes->Length);
	}

	return sa;
}

struct sockaddr*
SctpSocket::GetSockAddr(IPAddress ^address, int port, int& size)
{
	return GetSockAddr(gcnew IPEndPoint(address, port), size);
}

void
SctpSocket::FreeSockAddr(sockaddr *sa)
{
	if (sa->sa_family == AF_INET) {
		struct sockaddr_in *sin = (struct sockaddr_in*)sa;
		delete sin;
	} else if (sa->sa_family == AF_INET6) {
		struct sockaddr_in6 *sin6 = (struct sockaddr_in6*)sa;
		delete sin6;
	}
}

void
SctpSocket::Listen(int backlog)
{
	if (listen(m_socket, backlog) == SOCKET_ERROR)
		throw gcnew SocketException();

	m_listening = true;
}

void
SctpSocket::Connect(EndPoint ^remoteEP)
{
	if (remoteEP == nullptr)
		throw gcnew ArgumentNullException();

	int namelen;
	struct sockaddr *sa = GetSockAddr(remoteEP, namelen);

	if (connect(m_socket, sa, namelen) == SOCKET_ERROR) {
		FreeSockAddr(sa);
		throw gcnew SocketException();
	}

	DWORD nbio_mode = 1;
	PER_IO_DATA *io = new PER_IO_DATA();
	io->hIoProcessed = CreateEvent(NULL, FALSE, FALSE, NULL);
	io->operation = IOCTL;
	int rc = WSAIoctl(m_socket, FIONBIO, &nbio_mode, sizeof(DWORD), NULL, 0, &(io->ioByteCount), &io->overlapped, NULL);

	if (rc == SOCKET_ERROR && WSAGetLastError() != ERROR_IO_PENDING)
		throw gcnew SocketException();

	if (WaitForSingleObject(io->hIoProcessed, INFINITE) != WAIT_OBJECT_0)
		throw gcnew SocketException();

	delete io;
}

void
SctpSocket::Connect(IPAddress ^address, int port)
{
	IPEndPoint ^ipEP = gcnew IPEndPoint(address, port);
	Connect(ipEP);
}

void
SctpSocket::Connect(String ^host, int port)
{
	IPAddress ^address = IPAddress::Parse(host);
	Connect(address, port);
}

void
SctpSocket::Disconnect(bool reuseSocket)
{
	closesocket(m_socket);
	if (!reuseSocket)
		m_socket = INVALID_SOCKET;
}

void
SctpSocket::Shutdown(SocketShutdown how)
{
	int sdhow;

	if (how == SocketShutdown::Receive)
		sdhow = SD_RECEIVE;
	else if (how == SocketShutdown::Send)
		sdhow = SD_SEND;
	else if (how == SocketShutdown::Both)
		sdhow = SD_BOTH;

	if (shutdown(m_socket, sdhow) == SOCKET_ERROR)
		throw gcnew SocketException();
}

SctpSocket^
SctpSocket::PeelOff(SctpSocket ^socket, int associd)
{
	SOCKET s = internal_sctp_peeloff(socket->m_socket, (sctp_assoc_t)(int)associd);
	return gcnew SctpSocket(s);
}

int
SctpSocket::Bind( array<IPEndPoint^> ^endPoints, int flags )
{
	/* First, check how many IPv4 and IPv6 addresses we have */
	int num_v4 = 0;
	int num_v6 = 0;

	for each (IPEndPoint ^ep in endPoints) {
		if (ep->AddressFamily == AddressFamily::InterNetwork)
			num_v4++;
		else if (ep->AddressFamily == AddressFamily::InterNetworkV6)
			num_v6++;
		else
			// Neither IPv4 or IPv6 - we can't handle this
			return -1;
	}

	struct sockaddr *addrs = (struct sockaddr*)malloc(num_v4*sizeof(struct sockaddr_in) + num_v6*sizeof(struct sockaddr_in6));
	struct sockaddr *sa = addrs;
	int size;
			
	for each (IPEndPoint ^ep in endPoints) {
		if (ep->AddressFamily == AddressFamily::InterNetwork) {
			struct sockaddr_in *in = (struct sockaddr_in*)sa;
			size = sizeof(*in);
			in->sin_family = AF_INET;
			in->sin_port = htons(ep->Port);
			array<unsigned char> ^addrBytes = ep->Address->GetAddressBytes();
			pin_ptr<unsigned char> bytes = &addrBytes[0];
			memcpy(&in->sin_addr.s_addr, bytes, addrBytes->Length);
					
		} else if (ep->AddressFamily == AddressFamily::InterNetworkV6) {
			struct sockaddr_in6 *in6 = (struct sockaddr_in6*)sa;
			size = sizeof(*in6);
			in6->sin6_family = AF_INET6;
			in6->sin6_port = htons(ep->Port);
			array<unsigned char> ^addrBytes = ep->Address->GetAddressBytes();
			pin_ptr<unsigned char> bytes = &addrBytes[0];
			memcpy(&in6->sin6_addr, bytes, addrBytes->Length);
		}

		sa = (struct sockaddr *)((char*)sa + size);
	}

	return internal_sctp_bindx(m_socket, addrs, endPoints->Length, (int)flags);
}
	
int
SctpSocket::Connect(array<IPEndPoint^> ^endPoints, unsigned int %associd)
{
	int totalSize = 0;
	// Work out the total size
	for each (IPEndPoint ^ep in endPoints) {
		if (ep->AddressFamily == AddressFamily::InterNetwork)
			totalSize += sizeof(struct sockaddr_in);
		else if (ep->AddressFamily == AddressFamily::InterNetworkV6)
			totalSize += sizeof(struct sockaddr_in6);
	}

	struct sockaddr *addrs = (struct sockaddr*)malloc(totalSize);
	struct sockaddr *sa = addrs;

	int size;

	for each (IPEndPoint ^ep in endPoints) {
		if (ep->AddressFamily == AddressFamily::InterNetwork) {
			struct sockaddr_in *sin = (struct sockaddr_in*)sa;
			size = sizeof(*sin);
			sin->sin_family = AF_INET;
			sin->sin_port = ep->Port;
			array<unsigned char> ^addrBytes = ep->Address->GetAddressBytes();
			pin_ptr<unsigned char> bytes = &addrBytes[0];
			memcpy(&sin->sin_addr.s_addr, bytes, addrBytes->Length);
		} else if (ep->AddressFamily == AddressFamily::InterNetworkV6) {
			struct sockaddr_in6 *sin6 = (struct sockaddr_in6*)sa;
			size = sizeof(*sin6);
			sin6->sin6_family = AF_INET6;
			sin6->sin6_port = ep->Port;
			array<unsigned char> ^addrBytes = ep->Address->GetAddressBytes();
			pin_ptr<unsigned char> bytes = &addrBytes[0];
			memcpy(&sin6->sin6_addr, bytes, addrBytes->Length);
		}

		sa = (struct sockaddr*) ((char*)sa + size);
	}

	pin_ptr<unsigned int> aid = &associd;
	return internal_sctp_connectx(m_socket, addrs, endPoints->Length, aid);
}

int
SctpSocket::GetSocketOption(int associd, int option, array<byte> ^arg, int %len)
{
	pin_ptr<unsigned char> argbytes = &arg[0];
	pin_ptr<int> lenbytes = &len;
	return internal_sctp_opt_info(m_socket, (int)associd, (int)option, (void*)argbytes, (socklen_t*)lenbytes);
}

int
SctpSocket::Send(array<byte> ^msg, SctpSocket::SendReceiveInfo ^sinfo, int flags)
{
	pin_ptr<sctp_sndrcvinfo> sinfo_ptr = (sctp_sndrcvinfo*)&sinfo;
	pin_ptr<unsigned char> msg_ptr = &msg[0];
	return internal_sctp_send(m_socket, (const void*)msg_ptr, msg->Length, (const sctp_sndrcvinfo*)sinfo_ptr, (int)flags);
}

int
SctpSocket::GetAssociationId (EndPoint ^endPoint)
{
	int salen;
	struct sockaddr *sa = GetSockAddr(endPoint, salen);
	int rc = internal_sctp_getassocid(m_socket, sa);
	FreeSockAddr(sa);
	return rc;
}

IPEndPoint^
SctpSocket::GetIPEndPoint(sockaddr* sa)
{
	IPEndPoint ^ipEP = nullptr;
	IPAddress ^address;
	sockaddr_in *sin;
	sockaddr_in6 *sin6;

	if (sa == NULL)
		return nullptr;

	switch (sa->sa_family) {
	case AF_INET:
	{
		sin = (sockaddr_in*)sa;
		address = gcnew IPAddress(sin->sin_addr.s_addr);
		ipEP = gcnew IPEndPoint(address, ntohs(sin->sin_port));
		break;
	}
	case AF_INET6:
		sin6 = (sockaddr_in6*)sa;
		__int64 *addr = (__int64*)&sin6->sin6_addr;
		address = gcnew IPAddress(*addr);
		ipEP = gcnew IPEndPoint(address, ntohs(sin6->sin6_port));
		break;
	}

	return ipEP;
}

int
SctpSocket::ReceiveFrom(array<byte> ^msg, int len, IPEndPoint ^remoteEP, SendReceiveInfo ^sinfo, int %flags)
{
	pin_ptr<unsigned char> msg_ptr = &msg[0];
	pin_ptr<sctp_sndrcvinfo> sinfo_ptr = (sctp_sndrcvinfo*)&sinfo;
	pin_ptr<int> flags_ptr = &flags;

	sockaddr_in6 sin6;
	socklen_t sin6_len = sizeof(sin6);

	int rc = internal_sctp_recvmsg(m_socket, (char*)msg_ptr, msg->Length, (sockaddr*)&sin6, &sin6_len, sinfo_ptr, flags_ptr);
	remoteEP = GetIPEndPoint((sockaddr*)&sin6);
	return rc;
}

bool
SctpSocket::AcceptAsync(SocketAsyncEventArgs^ e)
{
	// TODO
	throw gcnew NotImplementedException();
	return false;
}

IAsyncResult^
SctpSocket::BeginReceive(IList<ArraySegment<unsigned char>>^ buffers, SocketFlags socketFlags, AsyncCallback^ callback, Object^ state)
{
	SocketError errorCode;
	return BeginReceive(buffers, socketFlags, errorCode, callback, state);
}

IAsyncResult^
SctpSocket::BeginReceive(IList<ArraySegment<unsigned char>>^ buffers, SocketFlags socketFlags, [OutAttribute] SocketError% errorCode, AsyncCallback^ callback, Object^ state)
{
	PER_IO_DATA *io = new PER_IO_DATA();
	ZeroMemory(io, sizeof(PER_IO_DATA));
	io->hIoProcessed = CreateEvent(NULL, FALSE, FALSE, NULL);
	io->operation = RECV;
	io->cb = Marshal::GetFunctionPointerForDelegate(callback).ToPointer();
	WSABUF *buf = new WSABUF[buffers->Count];
	DWORD flags = 0;
	io->sgBuffer = buf;

	for (int i = 0; i < buffers->Count; i++) {
		buf[i].len = buffers[i].Count;
		array<unsigned char> ^b = buffers[i].Array;
		pin_ptr<unsigned char> buf_ptr = &b[0];
		buf[i].buf = (char*)buf_ptr;
	}

	int rc = WSARecv(m_socket, buf, buffers->Count, &(io->ioByteCount), &flags, &(io->overlapped), NULL);
	if (rc == SOCKET_ERROR && GetLastError() != ERROR_IO_PENDING) {
		errorCode = GetSocketError(WSAGetLastError());
		delete io;
		throw gcnew SocketException();
	}

	delete io;
 
	return gcnew SctpSocketAsyncResult(io->hIoProcessed, state);
}



IAsyncResult^
SctpSocket::BeginReceive(array<unsigned char>^ buffer, int offset, int size, SocketFlags socketFlags, AsyncCallback^ callback, Object^ state)
{
	SocketError se;
	return BeginReceive(buffer, offset, size, socketFlags, se, callback, state);
}

IAsyncResult^
SctpSocket::BeginReceive(array<unsigned char>^ buffer, int offset, int size, SocketFlags socketFlags, [OutAttribute] SocketError% errorCode, AsyncCallback^ callback, Object^ state)
{
	ArraySegment<unsigned char> segment(buffer, offset, size);
	List<ArraySegment<unsigned char>> ^list = gcnew List<ArraySegment<unsigned char>>();
	list->Add(segment);
	return BeginReceive(list, socketFlags, errorCode, callback, state);
}

IAsyncResult^
SctpSocket::BeginReceiveFrom(array<unsigned char>^ buffer, int offset, int size, SocketFlags socketFlags, EndPoint^% remoteEP, AsyncCallback^ callback, Object^ state)
{
	if (remoteEP == nullptr)
		throw gcnew ArgumentNullException();



	throw gcnew NotImplementedException();
	return nullptr;
}

IAsyncResult^
SctpSocket::BeginReceiveMessageFrom(array<unsigned char>^ buffer, int offset, int size, SocketFlags socketFlags, EndPoint^% remoteEP, AsyncCallback^ callback, Object^ state)
{
	throw gcnew NotImplementedException();
	return nullptr;
}

IAsyncResult^
SctpSocket::BeginSend(IList<ArraySegment<unsigned char>>^ buffers, SocketFlags socketFlags, AsyncCallback^ callback, Object^ state)
{
	throw gcnew NotImplementedException();
	return nullptr;
}

IAsyncResult^
SctpSocket::BeginSend(IList<ArraySegment<unsigned char>>^ buffers, SocketFlags socketFlags, [OutAttribute] SocketError% errorCode, AsyncCallback^ callback, Object^ state)
{
	PER_IO_DATA *io = new PER_IO_DATA();
	ZeroMemory(io, sizeof(PER_IO_DATA));
	io->hIoProcessed = CreateEvent(NULL, FALSE, FALSE, NULL);
	io->operation = SEND;
	io->cb = Marshal::GetFunctionPointerForDelegate(callback).ToPointer();
	WSABUF *buf = new WSABUF[buffers->Count];
	DWORD flags = 0;
	io->sgBuffer = buf;

	for (int i = 0; i < buffers->Count; i++) {
		buf[i].len = buffers[i].Count;
		array<unsigned char> ^b = buffers[i].Array;
		pin_ptr<unsigned char> buf_ptr = &b[0];
		buf[i].buf = (char*)buf_ptr;
	}

	int rc = WSASend(m_socket, buf, buffers->Count, &(io->ioByteCount), flags, &(io->overlapped), NULL);
	if (rc == SOCKET_ERROR && GetLastError() != ERROR_IO_PENDING) {
		errorCode = GetSocketError(WSAGetLastError());
		delete io;
		throw gcnew SocketException();
	}

	delete io;
 
	return gcnew SctpSocketAsyncResult(io->hIoProcessed, state);
}

IAsyncResult^
SctpSocket::BeginSend(array<unsigned char>^ buffer, int offset, int size, SocketFlags socketFlags, AsyncCallback^ callback, Object^ state)
{
	SocketError errorCode;
	ArraySegment<unsigned char> segment(buffer, offset, size);
	List<ArraySegment<unsigned char>> ^list = gcnew List<ArraySegment<unsigned char>>();
	list->Add(segment);
	return BeginSend(list, socketFlags, errorCode, callback, state);
}

IAsyncResult^
SctpSocket::BeginSend(array<unsigned char>^ buffer, int offset, int size, SocketFlags socketFlags, [OutAttribute] SocketError% errorCode, AsyncCallback^ callback, Object^ state)
{
	ArraySegment<unsigned char> segment(buffer, offset, size);
	List<ArraySegment<unsigned char>> ^list = gcnew List<ArraySegment<unsigned char>>();
	list->Add(segment);
	return BeginSend(list, socketFlags, errorCode, callback, state);
}


IAsyncResult^
SctpSocket::BeginSendTo(array<unsigned char>^ buffer, int offset, int size, SocketFlags socketFlags, EndPoint^ remoteEP, AsyncCallback^ callback, Object^ state)
{
	throw gcnew NotImplementedException();
	return nullptr;
}

int
SctpSocket::EndReceive(IAsyncResult^ asyncResult)
{
	throw gcnew NotImplementedException();
	return -1;
}

int
SctpSocket::EndReceive(IAsyncResult^ asyncResult, [OutAttribute] SocketError% errorCode)
{
	throw gcnew NotImplementedException();
	return -1;
}

int
SctpSocket::EndReceiveFrom(IAsyncResult^ asyncResult, EndPoint^% endPoint)
{
	throw gcnew NotImplementedException();
	return -1;
}

int
SctpSocket::EndReceiveMessageFrom(IAsyncResult^ asyncResult, SocketFlags% socketFlags, EndPoint^% endPoint, [OutAttribute] IPPacketInformation% ipPacketInformation)
{
	throw gcnew NotImplementedException();
	return -1;
}

int
SctpSocket::EndSend(IAsyncResult^ asyncResult)
{
	throw gcnew NotImplementedException();
	return -1;
}

int
SctpSocket::EndOperation(IAsyncResult^ asyncResult, [OutAttribute] SocketError% errorCode)
{
	ULONG_PTR completionKey;
	BOOL ioOK;
	OVERLAPPED *ovl;
	const DWORD timeout = INFINITE;
	DWORD bytes;

	HANDLE h = (HANDLE)asyncResult->AsyncWaitHandle->SafeWaitHandle->DangerousGetHandle();
	ioOK = GetQueuedCompletionStatus(h, &bytes, &completionKey, &ovl, timeout);

	if (!ioOK)
		errorCode = GetSocketError(WSAGetLastError());

	return bytes;
}

int
SctpSocket::EndSend(IAsyncResult^ asyncResult, [OutAttribute] SocketError% errorCode)
{
	return EndOperation(asyncResult, errorCode);
}

int
SctpSocket::Receive(array<byte> ^buffer)
{
	return Receive(buffer, 0, buffer->Length, SocketFlags::None);
}

int
SctpSocket::Receive(array<unsigned char> ^buffer, int offset, int size, SocketFlags socketFlags)
{
	SocketError errorCode;
	return Receive(buffer, offset, size, socketFlags, errorCode);
}
int
SctpSocket::Receive(IList<ArraySegment<unsigned char>>^ buffers)
{
	return Receive(buffers, SocketFlags::None);
}

int
SctpSocket::Receive(IList<ArraySegment<unsigned char>>^ buffers, SocketFlags socketFlags)
{
	SocketError errorCode;
	return Receive(buffers, socketFlags, errorCode);
}

int
SctpSocket::Receive(array<unsigned char>^ buffer, SocketFlags socketFlags)
{
	return Receive(buffer, 0, buffer->Length, socketFlags);
}

int
SctpSocket::Receive(IList<ArraySegment<unsigned char>>^ buffers, SocketFlags socketFlags, [OutAttribute] SocketError% errorCode)
{
	if (buffers == nullptr)
		throw gcnew ArgumentNullException();

	DWORD flags = 0;
	DWORD bytesReceived = 0;

	if ((int)socketFlags & (int)SocketFlags::OutOfBand)
		flags |= MSG_OOB;
	if ((int)socketFlags & (int)SocketFlags::Peek)
		flags |= MSG_PEEK;

	PER_IO_DATA *io = new PER_IO_DATA();
	ZeroMemory(io, sizeof(PER_IO_DATA));
	io->hIoProcessed = CreateEvent(NULL, FALSE, FALSE, NULL);
	io->operation = RECV;
	WSABUF *wsa = new WSABUF[buffers->Count];

	io->sgBuffer = wsa;

	for (int i = 0; i < buffers->Count; i++) {
		wsa[i].len = buffers[i].Count;
		array<unsigned char> ^b = buffers[i].Array;
		pin_ptr<unsigned char> buf_ptr = &b[0];
		wsa[i].buf = (char*)buf_ptr;
	}

	int rc = WSARecv(m_socket, wsa, buffers->Count, &(io->ioByteCount), &flags, &(io->overlapped), NULL);

	if (rc == SOCKET_ERROR) {
		if (WSAGetLastError() != ERROR_IO_PENDING) {
			delete io;
			throw gcnew SocketException();
		}
	}

	if (WaitForSingleObject(io->hIoProcessed, INFINITE) != WAIT_OBJECT_0)
		throw gcnew SocketException();

	bytesReceived = io->ioByteCount;
	CloseHandle(io->hIoProcessed);

	delete io;
	
	return bytesReceived;
}

int
SctpSocket::Receive(array<unsigned char>^ buffer, int size, SocketFlags socketFlags)
{
	return Receive(buffer, 0, buffer->Length, socketFlags);
}

int
SctpSocket::Receive(array<unsigned char>^ buffer, int offset, int size, SocketFlags socketFlags, [OutAttribute] SocketError% errorCode)
{
	int adjustedSize = size;
	if (offset + size > buffer->Length)
		adjustedSize = buffer->Length - offset;

	ArraySegment<unsigned char> segment(buffer, offset, size);
	List<ArraySegment<unsigned char>> ^list = gcnew List<ArraySegment<unsigned char>>();
	list->Add(segment);

	return Receive(list, socketFlags, errorCode);
}

int
SctpSocket::Send(IList<ArraySegment<unsigned char>>^ buffers, SocketFlags socketFlags)
{
	SocketError errorCode;
	return Send(buffers, socketFlags, errorCode);
}

int
SctpSocket::Send(array<unsigned char>^ buffer, SocketFlags socketFlags)
{
	SocketError errorCode;
	return Send(buffer, 0, buffer->Length, socketFlags, errorCode);
}

int
SctpSocket::Send(array<unsigned char> ^buffer)
{
	return Send(buffer, SocketFlags::None);
}

int
SctpSocket::Send(IList<ArraySegment<unsigned char>>^ buffers, SocketFlags socketFlags, [OutAttribute] SocketError% errorCode)
{
	if (buffers == nullptr)
		throw gcnew ArgumentNullException();

	DWORD bytesSent = 0;
	PER_IO_DATA *io = new PER_IO_DATA();
	ZeroMemory(io, sizeof(PER_IO_DATA));
	io->hIoProcessed = CreateEvent(NULL, FALSE, FALSE, NULL);
	io->operation = SEND;

	WSABUF *buf = new WSABUF[buffers->Count];
	DWORD flags = 0;
	io->sgBuffer = buf;

	for (int i = 0; i < buffers->Count; i++) {
		buf[i].len = buffers[i].Count;
		array<unsigned char> ^b = buffers[i].Array;
		pin_ptr<unsigned char> buf_ptr = &b[0];
		buf[i].buf = (char*)buf_ptr;
	}

	int rc = WSASend(m_socket, buf, buffers->Count, &(io->ioByteCount), flags, &(io->overlapped), NULL);
	if (rc == SOCKET_ERROR && GetLastError() != ERROR_IO_PENDING) {
		errorCode = GetSocketError(WSAGetLastError());
		delete io;
		throw gcnew SocketException();
	}

	if (WaitForSingleObject(io->hIoProcessed, INFINITE) != WAIT_OBJECT_0)
		throw gcnew SocketException();

	bytesSent = io->ioByteCount;
	CloseHandle(io->hIoProcessed);

	delete io;
 
	return bytesSent;
}

int 
SctpSocket::Send(array<unsigned char>^ buffer, int size, SocketFlags socketFlags)
{
	return Send(buffer, 0, size, SocketFlags::None);
}

int
SctpSocket::Send(array<unsigned char>^ buffer, int offset, int size, SocketFlags socketFlags)
{
	SocketError errorCode;
	return Send(buffer, offset, size, socketFlags, errorCode);
}

int
SctpSocket::Send(array<unsigned char>^ buffer, int offset, int size, SocketFlags socketFlags, [OutAttribute] SocketError% errorCode)
{
	ArraySegment<unsigned char> segment(buffer, 0, buffer->Length);
	List<ArraySegment<unsigned char>> ^list = gcnew List<ArraySegment<unsigned char>>();
	list->Add(segment);
	return Send(list, socketFlags, errorCode);
}

int
SctpSocket::Send(IList<ArraySegment<unsigned char>>^ buffers)
{
	return Send(buffers, SocketFlags::None);
}

void
SctpSocket::Connect(array<IPAddress^> ^addresses, int port)
{
	if (addresses == nullptr)
		throw gcnew ArgumentNullException();

	if (port < 0 || port > 65535)
		throw gcnew ArgumentOutOfRangeException();

	bool succeeded = false;

	for each (IPAddress ^ip in addresses) {
		try {
			Connect(ip, port);
			succeeded = true;
		} catch (SocketException ^se) {
			(void)se;
			// that didn't work, try the next one
		}
	}

	if (!succeeded)
		throw gcnew SocketException();
}


void
SctpSocket::Select(System::Collections::IList ^checkRead, System::Collections::IList ^checkWrite, System::Collections::IList ^checkError, int microSeconds)
{
	if (checkRead == nullptr && checkWrite == nullptr && checkError == nullptr)
		throw gcnew ArgumentNullException();

	fd_set readfds;
	fd_set writefds;
	fd_set exceptfds;
	TIMEVAL timeout;
	int ready;

	FD_ZERO(&readfds);
	FD_ZERO(&writefds);
	FD_ZERO(&exceptfds);

	if (checkRead != nullptr) {
		for (int i = 0; i < checkRead->Count; i++) {
			FD_SET(((SctpSocket^)(checkRead[i]))->Handle, &readfds);
		}
	}

	if (checkWrite != nullptr) {
		for (int i = 0; i < checkWrite->Count; i++) {
			FD_SET(((SctpSocket^)(checkWrite[i]))->Handle, &writefds);
		}
	}

	if (checkError != nullptr) {
		for (int i = 0; i < checkError->Count; i++) {
			FD_SET(((SctpSocket^)(checkError[i]))->Handle, &exceptfds);
		}
	}

	timeout.tv_sec = microSeconds/1024;
	timeout.tv_usec = microSeconds % 1000;

	if (microSeconds != -1)
		ready = select(0, &readfds, &writefds, &exceptfds, NULL);
	else
		ready = select(0, &readfds, &writefds, &exceptfds, &timeout);

	if (ready == SOCKET_ERROR)
		throw gcnew SocketException();

	if (checkRead != nullptr) {
		for (int i = 0; i < checkRead->Count; i++) {
			if (!FD_ISSET(((SctpSocket^)(checkRead[i]))->Handle, &readfds))
				checkRead->Remove(checkRead[i]);
		}
	}

	if (checkWrite != nullptr) {
		for (int i = 0; i < checkWrite->Count; i++) {
			if (!FD_ISSET(((SctpSocket^)(checkWrite[i]))->Handle, &writefds))
				checkWrite->Remove(checkWrite[i]);
		}
	}

	if (checkError != nullptr) {
		for (int i = 0; i < checkError->Count; i++) {
			if (!FD_ISSET(((SctpSocket^)(checkError[i]))->Handle, &exceptfds))
				checkError->Remove(checkError[i]);
		}
	}
}

System::Collections::IList^
SctpSocket::GetPeerAddresses(int associd)
{
	return GetAddresses(associd, true);
}

System::Collections::IList^
SctpSocket::GetAddresses(int associd, bool remote)
{
	List<EndPoint^> ^EPs = gcnew List<EndPoint^>();

	struct sockaddr *sa;
	int naddrs;
	if (remote)
		naddrs = internal_sctp_getpaddrs(this->m_socket, associd, &sa);
	else
		naddrs = internal_sctp_getladdrs(this->m_socket, associd, &sa);

	if (naddrs == -1)
		throw gcnew SocketException();

	for (int i = 0; i < naddrs; i++) {
		char *addr_data;
		unsigned short port;
		int size;
		if (sa->sa_family == AF_INET) {
			struct sockaddr_in *sin = (struct sockaddr_in*)sa;
			addr_data = (char*) sin->sin_addr.s_addr;
			size = 4;
			port = sin->sin_port;
		} else {
			struct sockaddr_in6 *sin6 = (struct sockaddr_in6*)sa;
			size = 16;
			addr_data = (char*) sin6->sin6_addr.s6_addr;
			port = sin6->sin6_port;
		}

		array<byte> ^managed_bytes = gcnew array<byte>(size);
		for (int i = 0; i < size; i++) {
			managed_bytes[i] = addr_data[i];
		}

		IPAddress ^ipaddr = gcnew IPAddress(managed_bytes);
		EPs->Add(gcnew IPEndPoint(ipaddr, port));
	}

	internal_sctp_freepaddrs(sa);
	return EPs;
}

System::Collections::IList^
SctpSocket::GetLocalAddresses(int associd)
{
	return GetAddresses(associd, false);
}


int
SctpSocket::EndSendTo(IAsyncResult^ asyncResult)
{
	SocketError errorCode;
	return EndOperation(asyncResult, errorCode);
}

bool
SctpSocket::Poll(int microSeconds, SelectMode mode)
{
	throw gcnew NotImplementedException();
	return false;
}

int
SctpSocket::SendTo(array<unsigned char>^ buffer, EndPoint^ remoteEP)
{
	return SendTo(buffer, 0, buffer->Length, SocketFlags::None, remoteEP);
}

int
SctpSocket::SendTo(array<unsigned char>^ buffer, SocketFlags socketFlags, EndPoint^ remoteEP)
{
	return SendTo(buffer, 0, buffer->Length, socketFlags, remoteEP);
}

int
SctpSocket::SendTo(array<unsigned char>^ buffer, int size, SocketFlags socketFlags, EndPoint^ remoteEP)
{
	return SendTo(buffer, 0, size, socketFlags, remoteEP);
}

int
SctpSocket::SendTo(array<unsigned char>^ buffer, int offset, int size, SocketFlags socketFlags, EndPoint^ remoteEP)
{
	if (buffer == nullptr)
		throw gcnew ArgumentNullException();

	SocketError errorCode;
	DWORD bytesSent = 0;
	PER_IO_DATA *io = new PER_IO_DATA();
	ZeroMemory(io, sizeof(PER_IO_DATA));
	io->hIoProcessed = CreateEvent(NULL, FALSE, FALSE, NULL);
	io->operation = SEND;

	WSABUF buf;
	DWORD flags = 0;
	io->sgBuffer = &buf;

	pin_ptr<unsigned char> pinnedBuffer = &buffer[0];

	int adjustedSize = size;

	if ((offset + size) > buffer->Length)
		adjustedSize = buffer->Length - offset;

	buf.buf = (char*)(pinnedBuffer + offset);
	buf.len = adjustedSize;

	int sa_size;
	struct sockaddr *sa = GetSockAddr(remoteEP, sa_size);

	int rc = WSASendTo(m_socket, &buf, 1, &(io->ioByteCount), flags, sa, sa_size, &(io->overlapped), NULL);
	if (rc == SOCKET_ERROR && GetLastError() != ERROR_IO_PENDING) {
		errorCode = GetSocketError(WSAGetLastError());
		delete io;
		throw gcnew SocketException();
	}

	if (WaitForSingleObject(io->hIoProcessed, INFINITE) != WAIT_OBJECT_0)
		throw gcnew SocketException();

	bytesSent = io->ioByteCount;
	CloseHandle(io->hIoProcessed);

	FreeSockAddr(sa);
	delete io;
 
	return bytesSent;
}

int
SctpSocket::SendTo(array<unsigned char> ^msg, EndPoint ^endPoint, int ppid, int flags, int stream_no, int ttl, int context)
{
	pin_ptr<unsigned char> msgbytes = &msg[0];
	int tolen;
	sockaddr *to = GetSockAddr(endPoint, tolen);

	int rc = internal_sctp_sendmsg(m_socket, (const void*)msgbytes, msg->Length, to, (socklen_t)tolen, ppid, flags, stream_no, ttl, context);

	FreeSockAddr(to);
	return rc;
}

int
SctpSocket::SendTo(array<unsigned char> ^msg, array<EndPoint^> ^remoteEPs, SendReceiveInfo ^sinfo, int flags)
{
	pin_ptr<unsigned char> msg_ptr = &msg[0];
	pin_ptr<sctp_sndrcvinfo> sinfo_ptr = (sctp_sndrcvinfo*)&sinfo;
	int tolen;
	sockaddr *addrs = new sockaddr[remoteEPs->Length];
	sockaddr *sa;
	
	for (int i = 0; i < remoteEPs->Length; i++) {
		sa = GetSockAddr(remoteEPs[i], tolen);
		memcpy(&addrs[i], sa, tolen);
		FreeSockAddr(sa);
	}

	return internal_sctp_sendx(m_socket, (const void*)msg_ptr, msg->Length, addrs, remoteEPs->Length, sinfo_ptr, (int)flags);
}

int
SctpSocket::SendTo(array<unsigned char> ^msg, array<EndPoint^> ^remoteEPs, int ppid, int flags, int stream_no, int ttl, int context)
{
	pin_ptr<unsigned char> msg_ptr = &msg[0];
	int tolen;
	sockaddr *addrs = new sockaddr[remoteEPs->Length];
	sockaddr *sa;
	
	for (int i = 0; i < remoteEPs->Length; i++) {
		sa = GetSockAddr(remoteEPs[i], tolen);
		memcpy(&addrs[i], sa, tolen);
		FreeSockAddr(sa);
	}
	
	return internal_sctp_sendmsgx(m_socket, (const void*)msg_ptr, msg->Length, addrs, remoteEPs->Length, ppid, flags, stream_no, ttl, context);
}


SocketError
SctpSocket::GetSocketError(DWORD errCode)
{
	SocketError se = SocketError::Success;

	switch (errCode) {
	case WSAEINTR:
		se = SocketError::Interrupted;
		break;
	case WSAEACCES:
		se = SocketError::AccessDenied;
		break;
	case WSAEFAULT:
		se = SocketError::Fault;
		break;
	case WSAEINVAL:
		se = SocketError::InvalidArgument;
		break;
	case WSAEMFILE:
		se = SocketError::TooManyOpenSockets;
		break;
	case WSAEWOULDBLOCK:
		se = SocketError::WouldBlock;
		break;
	case WSAEINPROGRESS:
		se = SocketError::InProgress;
		break;
	case WSAEALREADY:
		se = SocketError::AlreadyInProgress;
		break;
	case WSAENOTSOCK:
		se = SocketError::NotSocket;
		break;
	case WSAEDESTADDRREQ:
		se = SocketError::DestinationAddressRequired;
		break;
	case WSAEMSGSIZE:
		se = SocketError::MessageSize;
		break;
	case WSAEPROTOTYPE:
		se = SocketError::ProtocolType;
		break;
	case WSAENOPROTOOPT:
		se = SocketError::ProtocolOption;
		break;
	case WSAEPROTONOSUPPORT:
		se = SocketError::ProtocolNotSupported;
		break;
	case WSAESOCKTNOSUPPORT:
		se = SocketError::SocketNotSupported;
		break;
	case WSAEOPNOTSUPP:
		se = SocketError::OperationNotSupported;
		break;
	case WSAEPFNOSUPPORT:
		se = SocketError::ProtocolFamilyNotSupported;
		break;
	case WSAEAFNOSUPPORT:
		se = SocketError::AddressFamilyNotSupported;
		break;
	case WSAEADDRINUSE:
		se = SocketError::AddressAlreadyInUse;
		break;
	case WSAEADDRNOTAVAIL:
		se = SocketError::AddressNotAvailable;
		break;
	case WSAENETDOWN:
		se = SocketError::NetworkDown;
		break;
	case WSAENETUNREACH:
		se = SocketError::NetworkUnreachable;
		break;
	case WSAENETRESET:
		se = SocketError::NetworkReset;
		break;
	case WSAECONNABORTED:
		se = SocketError::ConnectionAborted;
		break;
	case WSAECONNRESET:
		se = SocketError::ConnectionReset;
		break;
	case WSAENOBUFS:
		se = SocketError::NoBufferSpaceAvailable;
		break;
	case WSAEISCONN:
		se = SocketError::IsConnected;
		break;
	case WSAENOTCONN:
		se = SocketError::NotConnected;
		break;
	case WSAESHUTDOWN:
		se = SocketError::Shutdown;
		break;
	case WSAETIMEDOUT:
		se = SocketError::TimedOut;
		break;
	case WSAECONNREFUSED:
		se = SocketError::ConnectionRefused;
		break;
	case WSAEHOSTDOWN:
		se = SocketError::HostDown;
		break;
	case WSAEHOSTUNREACH:
		se = SocketError::HostUnreachable;
		break;
	case WSAEPROCLIM:
		se = SocketError::ProcessLimit;
		break;
	case WSASYSNOTREADY:
		se = SocketError::SystemNotReady;
		break;
	case WSAVERNOTSUPPORTED:
		se = SocketError::VersionNotSupported;
		break;
	case WSANOTINITIALISED:
		se = SocketError::NotInitialized;
		break;
	case WSAEDISCON:
		se = SocketError::Disconnecting;
		break;
	case WSAECANCELLED:
		se = SocketError::OperationAborted;
		break;
	case WSATYPE_NOT_FOUND:
		se = SocketError::TypeNotFound;
		break;
	case WSATRY_AGAIN:
		se = SocketError::TryAgain;
		break;
	case WSANO_RECOVERY:
		se = SocketError::NoRecovery;
		break;
	case WSANO_DATA:
		se = SocketError::NoData;
		break;
	case WSAHOST_NOT_FOUND:
		se = SocketError::HostNotFound;
		break;
	// TODO see if there's a more specific error we can return
	case WSAEINVALIDPROVIDER:
	case WSASYSCALLFAILURE:
	case WSA_E_CANCELLED:
	case WSAEREFUSED:
	case WSA_E_NO_MORE:
	case WSASERVICE_NOT_FOUND:
	case WSAEINVALIDPROCTABLE:
	case WSAENOMORE:
	case WSAEREMOTE:
	case WSAESTALE:
	case WSAEDQUOT:
	case WSAEUSERS:
	case WSAENOTEMPTY:
	case WSAENAMETOOLONG:
	case WSAELOOP:
	case WSAETOOMANYREFS:
		se = SocketError::SocketError;
		break;
	}

	return se;
}


