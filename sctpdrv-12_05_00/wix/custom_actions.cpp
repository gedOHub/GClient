/*
 * Copyright (c) 2008 CO-CONV, Corp.
 * Copyright (c) 2011 Bruce Cran.
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
#include <winsock2.h>
#include <ws2spi.h>
#include <windows.h>
#include <stdlib.h>
#include <comutil.h>
#include <atlcomcli.h>
#include <netfw.h>
#include <strsafe.h>
#include <msi.h>
#include <msiquery.h>
#include <wcautil.h>
#include <shlobj.h>
#include <atlbase.h>

#import "netfw.tlb"

#include <sctpsp.h>

extern "C" {
UINT WINAPI SctpInstallProvider(__in MSIHANDLE hInstall);
UINT WINAPI SctpUninstallProvider(__in MSIHANDLE hInstall);
UINT WINAPI SctpAddFirewallRule(__in MSIHANDLE hInstall);
BOOL WINAPI DllMain(__in HINSTANCE hInstDLL, __in DWORD fdwReason, __in LPVOID lpvReserved);
}

DWORD InstallDriver(LPWSTR fromDirectory);
DWORD UninstallDriver(LPWSTR fromDirectory);

UINT
WINAPI
SctpInstallProvider(
    __in MSIHANDLE hInstall)
{
	int ret = 0;
	int iError = 0;
	HRESULT hr;
	WSADATA wsd;
	LPWSTR providerPath;
	UINT rc = ERROR_SUCCESS;
	int i,j;

	hr = WcaInitialize(hInstall, "SctpInstallProvider");
	if (FAILED(hr))
		return ERROR_INSTALL_FAILURE;

	providerPath = (LPWSTR) malloc(MAX_PATH*sizeof(WCHAR));
	if (providerPath == NULL)
		return WcaFinalize(ERROR_INSTALL_FAILURE);

	if (ExpandEnvironmentStrings(SCTP_SERVICE_PROVIDER_PATH, providerPath, MAX_PATH) == 0) {
		free(providerPath);
		char msg[128];
		StringCchPrintfA(msg, 128, "Warning: ExpandEnvironmentStrings failed\n");
		WcaLog(LOGMSG_STANDARD, msg);
		return WcaFinalize(ERROR_INSTALL_FAILURE);
	}

	ret = WSAStartup(MAKEWORD(2, 2), &wsd);

	if (ret != 0) {
		char msg[128];
		free(providerPath);
		StringCchPrintfA(msg, 128, "Error: WSAStartup failed with code %u\n", WSAGetLastError());
		WcaLog(LOGMSG_STANDARD, msg);
		return WcaFinalize(ERROR_INSTALL_FAILURE);
	}

	if (WSCInstallProvider(&SctpProviderGuid,
				providerPath,
				SctpProtocolInfos,
				NUM_SCTP_PROTOCOL_INFOS,
				&iError) != 0)
	{
		char msg[128];
		StringCchPrintfA(msg, 128, "Warning: WSCInstallProvider failed with code %d\n", iError);
		WcaLog(LOGMSG_STANDARD, msg);
		j = 0;
		for (i = 0; i < MAX_PATH; i+=2) {
			msg[j] = (char)providerPath[i];
			if (msg[j] == 0)
				break;
		}
		WcaLog(LOGMSG_STANDARD, msg);
	}

	WSACleanup();
	free(providerPath);
	return WcaFinalize(rc);
}

UINT
WINAPI
SctpUninstallProvider(
    IN MSIHANDLE hInstall)
{
	int iError = 0;
	int ret = 0;
	WSADATA wsd;
	(void)hInstall;

	ret = WSAStartup(MAKEWORD(2, 2), &wsd);

	if (ret != 0)
		return ERROR_INSTALL_FAILURE;

	/* Ignore any errors when deinstalling the entry -
	 * there's nothing we can do to fix it */
	WSCDeinstallProvider(&SctpProviderGuid, &iError);
	WSACleanup();

	return ERROR_SUCCESS;
}

UINT
WINAPI
SctpAddFirewallRule(
	IN MSIHANDLE hInstall)
{
	HRESULT hr;
	IUnknown *pFwRuleEnumerator = NULL;
	IEnumVARIANT *pVariant = NULL;
	CComVariant var;
	ULONG cFetched;
	BOOL foundSctpFwRule = FALSE;

	NetFwPublicTypeLib::INetFwRulePtr pFwRule = NULL;
	NetFwPublicTypeLib::INetFwRulesPtr pFwRules = NULL;
	NetFwPublicTypeLib::INetFwPolicy2Ptr pFwPolicy = NULL;

	(void)hInstall;

	try
	{
		hr = CoInitializeEx(0, COINIT_APARTMENTTHREADED);

		if (hr != RPC_E_CHANGED_MODE && FAILED(hr))
			return ERROR_INSTALL_FAILURE;

		CoCreateInstance(
			__uuidof(NetFwPublicTypeLib::NetFwPolicy2), NULL, CLSCTX_ALL,
			IID_PPV_ARGS(&pFwPolicy));

		pFwRules = pFwPolicy->GetRules();
		pFwRules->get__NewEnum(&pFwRuleEnumerator);

		hr = pFwRuleEnumerator->QueryInterface(__uuidof(IEnumVARIANT), (void **) &pVariant);

		while(SUCCEEDED(hr) && hr != S_FALSE)
		{
			var.Clear();
			hr = pVariant->Next(1, &var, &cFetched);
			if (hr == S_FALSE)
				break;

			hr = var.ChangeType(VT_DISPATCH);
			if (!SUCCEEDED(hr))
				break;

			hr = (V_DISPATCH(&var))->QueryInterface(__uuidof(INetFwRule), reinterpret_cast<void**>(&pFwRule));
			if (!SUCCEEDED(hr))
				break;

			long proto = pFwRule->Protocol;
			pFwRule = NULL;
			var = NULL;

			if (proto == IPPROTO_SCTP) {
				foundSctpFwRule = true;
				break;
			}
		}

		if (!foundSctpFwRule) {
			pFwRule.CreateInstance("HNetCfg.FwRule");

			pFwRule->Name = L"SCTP";
			pFwRule->Protocol = IPPROTO_SCTP;
			pFwRule->RemoteAddresses = L"*";
			pFwRule->Profiles = NetFwPublicTypeLib::NET_FW_PROFILE2_ALL;
			pFwRule->Action = NetFwPublicTypeLib::NET_FW_ACTION_ALLOW;
			pFwRule->Enabled = VARIANT_TRUE;
			pFwRule->Description = L"Inbound rule to allow SCTP (Stream Control Transmission Protocol) traffic";
			pFwPolicy->Rules->Add(pFwRule);
			pFwRule = NULL;
		}
	}
	catch(_com_error& e)
	{
		hr = e.Error();
	}

	pVariant = NULL;
	pFwRule = NULL;
	pFwRuleEnumerator = NULL;
	pFwRules = NULL;
	pFwPolicy = NULL;
	CoUninitialize();

	/*
	   Ignore any errors. If the user disables the firewall then
	   configuration fails but shouldn't be considered an error.
	 */

	return ERROR_SUCCESS;
}

BOOL
WINAPI
DllMain(
	IN HINSTANCE hInstDLL,
	IN DWORD fdwReason,
	IN LPVOID lpvReserved)
{
	(void)lpvReserved;

	switch (fdwReason) {
	case DLL_PROCESS_ATTACH:
		DisableThreadLibraryCalls(hInstDLL);
		WcaGlobalInitialize(hInstDLL);
		break;

	case DLL_PROCESS_DETACH:
		WcaGlobalFinalize();
		break;

	default:
		break;
	}

	return TRUE;
}
