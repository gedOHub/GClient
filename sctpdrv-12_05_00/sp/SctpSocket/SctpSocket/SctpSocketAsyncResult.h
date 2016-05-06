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
using namespace System::Threading;
using namespace Microsoft::Win32::SafeHandles;


namespace SctpDrv {
		public ref class SctpSocketWaitHandle : public WaitHandle {
	public:
		SctpSocketWaitHandle(Microsoft::Win32::SafeHandles::SafeWaitHandle ^handle) 
			: WaitHandle()
		{
			SafeWaitHandle = handle;
		}
	};

	public ref class SctpSocketAsyncResult : public IAsyncResult
{
public:
	property bool IsCompleted {
		virtual bool get() { return m_handle->WaitOne(); }
	}

	property WaitHandle^ AsyncWaitHandle {
		virtual WaitHandle^ get() { return m_handle; }
	}

	property Object^ AsyncState {
		virtual Object^ get() { return userState; }
	}

	property bool CompletedSynchronously {
		virtual bool get() { return m_completedSynchronously; }
		void set(bool completedSynchronously) { m_completedSynchronously = completedSynchronously; }
	}

	SctpSocketAsyncResult(HANDLE handle, Object ^state) {
		SafeWaitHandle ^safeHandle = gcnew SafeWaitHandle(IntPtr(handle), true);
		m_handle = gcnew SctpSocketWaitHandle(safeHandle);
		userState = state;
	}

	private:
		SctpSocketWaitHandle ^m_handle;
		Object ^userState;
		bool m_completedSynchronously;
	};
}