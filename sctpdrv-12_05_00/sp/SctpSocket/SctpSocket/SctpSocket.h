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

#pragma once

using namespace System;
using namespace System::Net;
using namespace System::Collections::Generic;
using namespace System::Net::Sockets;
using namespace System::Security::Permissions;
using namespace System::Runtime::InteropServices;

namespace SctpDrv {
public ref class SctpSocket
{
public:
	ref class SctpOptions
	{
		literal int SCTP_RTOINFO = 0x00000001;
		literal int SCTP_ASSOCINFO = 0x00000002;
		literal int SCTP_INITMSG = 0x00000003;
		literal int SCTP_NODELAY = 0x00000004;
		literal int SCTP_AUTOCLOSE = 0x00000005;
		literal int SCTP_SET_PEER_PRIMARY_ADDR = 0x00000006;
		literal int SCTP_PRIMARY_ADDR = 0x00000007;
		literal int SCTP_ADAPTATION_LAYER = 0x00000008;
		/* same as above */
		literal int SCTP_ADAPTION_LAYER = 0x00000008;
		literal int SCTP_DISABLE_FRAGMENTS = 0x00000009;
		literal int SCTP_PEER_ADDR_PARAMS = 0x0000000a;
		literal int SCTP_DEFAULT_SEND_PARAM = 0x0000000b;
		/* ancillary data/notification interest options */
		literal int SCTP_EVENTS = 0x0000000c;
		/* Without this applied we will give V4 and V6 addresses on a V6 socket */
		literal int SCTP_I_WANT_MAPPED_V4_ADDR = 0x0000000d;
		literal int SCTP_MAXSEG = 0x0000000e;
		literal int SCTP_DELAYED_SACK = 0x0000000f;
		literal int SCTP_FRAGMENT_INTERLEAVE = 0x00000010;
		literal int SCTP_PARTIAL_DELIVERY_POINT = 0x00000011;
		/* authentication support */
		literal int SCTP_AUTH_CHUNK = 0x00000012;
		literal int SCTP_AUTH_KEY = 0x00000013;
		literal int SCTP_HMAC_IDENT = 0x00000014;
		literal int SCTP_AUTH_ACTIVE_KEY = 0x00000015;
		literal int SCTP_AUTH_DELETE_KEY = 0x00000016;
		literal int SCTP_USE_EXT_RCVINFO = 0x00000017;
		literal int SCTP_AUTO_ASCONF = 0x00000018; /* rw */
		literal int SCTP_MAXBURST = 0x00000019; /* rw */
		literal int SCTP_MAX_BURST = 0x00000019; /* rw */
		/* assoc level context */
		literal int SCTP_CONTEXT = 0x0000001a; /* rw */
		/* explict EOR signalling */
		literal int SCTP_EXPLICIT_EOR = 0x0000001b;
		literal int SCTP_REUSE_PORT = 0x0000001c; /* rw */
		literal int SCTP_AUTH_DEACTIVATE_KEY = 0x0000001d;

		/*
			* read-only options
			*/
		literal int SCTP_STATUS = 0x00000100;
		literal int SCTP_GET_PEER_ADDR_INFO = 0x00000101;
		/* authentication support */
		literal int SCTP_PEER_AUTH_CHUNKS = 0x00000102;
		literal int SCTP_LOCAL_AUTH_CHUNKS = 0x00000103;
		literal int SCTP_GET_ASSOC_NUMBER = 0x00000104; /* ro */
		literal int SCTP_GET_ASSOC_ID_LIST = 0x00000105; /* ro */

		literal int SCTP_RESET_STREAMS = 0x00001004; /* wo */

		/* here on down are more implementation specific */
		literal int SCTP_SET_DEBUG_LEVEL = 0x00001005;
		literal int SCTP_CLR_STAT_LOG = 0x00001007;
		/* CMT ON/OFF socket option */
		literal int SCTP_CMT_ON_OFF = 0x00001200;
		literal int SCTP_CMT_USE_DAC = 0x00001201;
		/* EY - NR_SACK on/off socket option */
		literal int SCTP_NR_SACK_ON_OFF = 0x00001300;
		/* JRS - Pluggable Congestion Control Socket option */
		literal int SCTP_PLUGGABLE_CC = 0x00001202;

		/* read only */
		literal int SCTP_GET_SNDBUF_USE = 0x00001101;
		literal int SCTP_GET_STAT_LOG = 0x00001103;
		literal int SCTP_PCB_STATUS = 0x00001104;
		literal int SCTP_GET_NONCE_VALUES = 0x00001105;

		literal int SCTP_SET_DYNAMIC_PRIMARY = 0x00002001;

		literal int SCTP_VRF_ID = 0x00003001;
		literal int SCTP_ADD_VRF_ID = 0x00003002;
		literal int SCTP_GET_VRF_IDS = 0x00003003;
		literal int SCTP_GET_ASOC_VRF = 0x00003004;
		literal int SCTP_DEL_VRF_ID = 0x00003005;

		literal int SCTP_GET_PACKET_LOG = 0x00004001;

		/* sctp_bindx() flags as hidden socket options */
		literal int SCTP_BINDX_ADD_ADDR = 0x00008001;
		literal int SCTP_BINDX_REM_ADDR = 0x00008002;
	};

	[StructLayout(LayoutKind::Sequential, Pack=8)]
	ref struct SendReceiveInfo {
		unsigned short sinfo_stream;
		unsigned short sinfo_ssn;
		unsigned short sinfo_flags;
		unsigned int sinfo_ppid;
		unsigned int sinfo_context;
		unsigned int sinfo_timetolive;
		unsigned int sinfo_tsn;
		unsigned int sinfo_cumtsn;
		sctp_assoc_t sinfo_assoc_id;
		array<byte> ^__reserve_pad;

		SendReceiveInfo() { __reserve_pad = gcnew array<byte>(SCTP_ALIGN_RESV_PAD); }
	};

	SctpSocket(AddressFamily ^addressFamily, SocketType ^socketType);
	SctpSocket(SOCKET sock);
	~SctpSocket();
	!SctpSocket();

	SctpSocket^ Accept();
	bool AcceptAsync(SocketAsyncEventArgs^ e);
	[HostProtectionAttribute(SecurityAction::LinkDemand, ExternalThreading = true)]
	IAsyncResult^ BeginReceive(IList<ArraySegment<unsigned char>>^ buffers, SocketFlags socketFlags, AsyncCallback^ callback, Object^ state);
	[HostProtectionAttribute(SecurityAction::LinkDemand, ExternalThreading = true)]
	IAsyncResult^ BeginReceive(IList<ArraySegment<unsigned char>>^ buffers, SocketFlags socketFlags, [OutAttribute] SocketError% errorCode, AsyncCallback^ callback, Object^ state);
	[HostProtectionAttribute(SecurityAction::LinkDemand, ExternalThreading = true)]
	IAsyncResult^ BeginReceive(array<unsigned char>^ buffer, int offset, int size, SocketFlags socketFlags, AsyncCallback^ callback, Object^ state);
	[HostProtectionAttribute(SecurityAction::LinkDemand, ExternalThreading = true)]
	IAsyncResult^ BeginReceive(array<unsigned char>^ buffer, int offset, int size, SocketFlags socketFlags, [OutAttribute] SocketError% errorCode, AsyncCallback^ callback, Object^ state);
	[HostProtectionAttribute(SecurityAction::LinkDemand, ExternalThreading = true)]
	IAsyncResult^ BeginReceiveFrom(array<unsigned char>^ buffer, int offset, int size, SocketFlags socketFlags, EndPoint^% remoteEP, AsyncCallback^ callback, Object^ state);
	IAsyncResult^ BeginReceiveMessageFrom(array<unsigned char>^ buffer, int offset, int size, SocketFlags socketFlags, EndPoint^% remoteEP, AsyncCallback^ callback, Object^ state);
	[HostProtectionAttribute(SecurityAction::LinkDemand, ExternalThreading = true)]
	IAsyncResult^ BeginSend(IList<ArraySegment<unsigned char>>^ buffers, SocketFlags socketFlags, AsyncCallback^ callback, Object^ state);
	[HostProtectionAttribute(SecurityAction::LinkDemand, ExternalThreading = true)]
	IAsyncResult^ BeginSend(IList<ArraySegment<unsigned char>>^ buffers, SocketFlags socketFlags, [OutAttribute] SocketError% errorCode, AsyncCallback^ callback, Object^ state);
	[HostProtectionAttribute(SecurityAction::LinkDemand, ExternalThreading = true)]
	IAsyncResult^ BeginSend(array<unsigned char>^ buffer, int offset, int size, SocketFlags socketFlags, AsyncCallback^ callback, Object^ state);
	[HostProtectionAttribute(SecurityAction::LinkDemand, ExternalThreading = true)]
	IAsyncResult^ BeginSend(array<unsigned char>^ buffer, int offset, int size, SocketFlags socketFlags, [OutAttribute] SocketError% errorCode, AsyncCallback^ callback, Object^ state);
	[HostProtectionAttribute(SecurityAction::LinkDemand, ExternalThreading = true)]
	IAsyncResult^ BeginSendTo(array<unsigned char>^ buffer, int offset, int size, SocketFlags socketFlags, EndPoint^ remoteEP, AsyncCallback^ callback, Object^ state);
	void Bind(EndPoint ^localEP);
	void Close();
	void Close(int timeout);
	void Connect(EndPoint ^remoteEP);
	void Connect(IPAddress ^address, int port);
	void Connect(array<IPAddress^>^ addresses, int port);
	void Connect(String ^host, int port);
	void Disconnect(bool reuseSocket);
	int EndReceive(IAsyncResult^ asyncResult);
	int EndReceive(IAsyncResult^ asyncResult, [OutAttribute] SocketError% errorCode);
	int EndReceiveFrom(IAsyncResult^ asyncResult, EndPoint^% endPoint);
	int EndReceiveMessageFrom(IAsyncResult^ asyncResult, SocketFlags% socketFlags, EndPoint^% endPoint, [OutAttribute] IPPacketInformation% ipPacketInformation);
	int EndSend(IAsyncResult^ asyncResult);
	int EndSend(IAsyncResult^ asyncResult, [OutAttribute] SocketError% errorCode);
	int EndSendTo(IAsyncResult^ asyncResult);
	void Listen(int backlog);
	bool Poll(int microSeconds, SelectMode mode);
	int Receive(IList<ArraySegment<unsigned char>>^ buffers);
	int Receive(array<unsigned char>^ buffer);
	int Receive(IList<ArraySegment<unsigned char>>^ buffers, SocketFlags socketFlags);
	int Receive(array<unsigned char>^ buffer, SocketFlags socketFlags);
	int Receive(IList<ArraySegment<unsigned char>>^ buffers, SocketFlags socketFlags, [OutAttribute] SocketError% errorCode);
	int Receive(array<unsigned char>^ buffer, int size, SocketFlags socketFlags);
	int Receive(array<unsigned char>^ buffer, int offset, int size, SocketFlags socketFlags);
	int Receive(array<unsigned char>^ buffer, int offset, int size, SocketFlags socketFlags, [OutAttribute] SocketError% errorCode);
	
	static void Select(System::Collections::IList^ checkRead, System::Collections::IList^ checkWrite, System::Collections::IList^ checkError, int microSeconds);
	int Send(IList<ArraySegment<unsigned char>>^ buffers);
	int Send(array<byte>^ buffer);
	int Send(IList<ArraySegment<unsigned char>>^ buffers, SocketFlags socketFlags);
	int Send(array<unsigned char>^ buffer, SocketFlags socketFlags);
	int Send(IList<ArraySegment<unsigned char>>^ buffers, SocketFlags socketFlags, [OutAttribute] SocketError% errorCode);
	int Send(array<unsigned char>^ buffer, int size, SocketFlags socketFlags);
	int Send(array<unsigned char>^ buffer, int offset, int size, SocketFlags socketFlags);
	int Send(array<unsigned char>^ buffer, int offset, int size, SocketFlags socketFlags, [OutAttribute] SocketError% errorCode);
	int SendTo(array<unsigned char>^ buffer, EndPoint^ remoteEP);
	int SendTo(array<unsigned char>^ buffer, SocketFlags socketFlags, EndPoint^ remoteEP);
	int SendTo(array<unsigned char>^ buffer, int size, SocketFlags socketFlags, EndPoint^ remoteEP);
	int SendTo(array<unsigned char>^ buffer, int offset, int size, SocketFlags socketFlags, EndPoint^ remoteEP);
	void Shutdown(SocketShutdown how);

	property SOCKET Handle
	{
		SOCKET get() {
			return (SOCKET)m_socket;
		}
	}

	SctpSocket^ PeelOff(SctpSocket ^socket, int associd);
	int  Bind(array<IPEndPoint^> ^endPoints, int flags);
	int  Connect(array<IPEndPoint^> ^endPoints, unsigned int %associd);
	System::Collections::IList^  GetPeerAddresses (int associd);
	System::Collections::IList^  GetLocalAddresses (int associd);
	int  GetSocketOption(int associd, int option, array<byte> ^arg, int %len);
	int SendTo( array<byte> ^msg, EndPoint ^endPoint, int ppid, int flags, int stream_no, int ttl, int context );
	int SendTo( array<byte> ^msg, array<EndPoint^> ^remoteEPs, int ppid, int flags, int stream_no, int ttl, int context );
	int SendTo( array<byte> ^msg, array<EndPoint^> ^remoteEP, SendReceiveInfo ^sinfo, int flags );
	int Send( array<byte> ^msg, SendReceiveInfo ^sinfo, int flags );
	int GetAssociationId( EndPoint ^endPoint );
	int ReceiveFrom( array<byte> ^msg, int len, IPEndPoint ^remoteEP, SendReceiveInfo ^sinfo, int %flags );

protected:

private:
	SOCKET m_socket;
	static bool m_winsock_initialized = false;
	static int m_instances = 0;
	bool m_listening;
	HANDLE m_completionPort;
	bool m_stopping;
	bool m_stopped;
	DWORD m_recvd;

	struct sockaddr* GetSockAddr(IPAddress ^address, int port, int& size);
	struct sockaddr* GetSockAddr(EndPoint ^endPoint, int& size);
	IPEndPoint^ GetIPEndPoint(sockaddr* sa);
	void FreeSockAddr(sockaddr *sa);
	void IoCompletionThread();
	SocketError GetSocketError(DWORD errCode);
	int EndOperation(IAsyncResult^ asyncResult, [OutAttribute] SocketError% errorCode);
	System::Collections::IList^ GetAddresses(int associd, bool remote);

	literal int SCTP_ALIGN_RESV_PAD = 96;
	literal int SCTP_ALIGN_RESV_PAD_SHORT = 80;
};
}