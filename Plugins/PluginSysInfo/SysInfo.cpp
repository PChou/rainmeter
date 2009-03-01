/*
  Copyright (C) 2004 Kimmo Pekkola

  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  as published by the Free Software Foundation; either version 2
  of the License, or (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#pragma warning(disable: 4786)
#pragma warning(disable: 4996)

#include <windows.h>
#include <math.h>
#include <string>
#include <map>
#include <Ras.h>
#include <Iphlpapi.h>
#include "..\..\Library\Export.h"	// Rainmeter's exported functions

/* The exported functions */
extern "C"
{
__declspec( dllexport ) UINT Initialize(HMODULE instance, LPCTSTR iniFile, LPCTSTR section, UINT id);
__declspec( dllexport ) LPCTSTR GetString(UINT id, UINT flags);
__declspec( dllexport ) void Finalize(HMODULE instance, UINT id);
__declspec( dllexport ) UINT GetPluginVersion();
__declspec( dllexport ) LPCTSTR GetPluginAuthor();
}

BOOL CheckConnection();
void GetOSVersion(WCHAR* buffer);

enum TYPE
{
	COMPUTER_NAME,
	USER_NAME,
	WORK_AREA,
	SCREEN_SIZE,
	RAS_STATUS,
	OS_VERSION,
	ADAPTER_DESCRIPTION,
	NET_MASK,
	IP_ADDRESS,
	GATEWAY_ADDRESS,
	HOST_NAME,
	DOMAIN_NAME,
	DNS_SERVER,
};

static std::map<UINT, TYPE> g_Types;
static std::map<UINT, UINT> g_Datas;

/*
  This function is called when the measure is initialized.
  The function must return the maximum value that can be measured. 
  The return value can also be 0, which means that Rainmeter will
  track the maximum value automatically. The parameters for this
  function are:

  instance  The instance of this DLL
  iniFile   The name of the ini-file (usually Rainmeter.ini)
  section   The name of the section in the ini-file for this measure
  id        The identifier for the measure. This is used to identify the measures that use the same plugin.
*/
UINT Initialize(HMODULE instance, LPCTSTR iniFile, LPCTSTR section, UINT id)
{
	/* Read our own settings from the ini-file */
	LPCTSTR type = ReadConfigString(section, L"SysInfoType", L"");
	if(type) 
	{
		if (wcsicmp(L"COMPUTER_NAME", type) == 0)
		{
			g_Types[id] = COMPUTER_NAME;
		} 
		else if (wcsicmp(L"USER_NAME", type) == 0)
		{
			g_Types[id] = USER_NAME;
		} 
		else if (wcsicmp(L"WORK_AREA", type) == 0)
		{
			g_Types[id] = WORK_AREA;
		} 
		else if (wcsicmp(L"SCREEN_SIZE", type) == 0)
		{
			g_Types[id] = SCREEN_SIZE;
		} 
		else if (wcsicmp(L"RAS_STATUS", type) == 0)
		{
			g_Types[id] = RAS_STATUS;
		} 
		else if (wcsicmp(L"OS_VERSION", type) == 0)
		{
			g_Types[id] = OS_VERSION;
		} 
		else if (wcsicmp(L"ADAPTER_DESCRIPTION", type) == 0)
		{
			g_Types[id] = ADAPTER_DESCRIPTION;
		} 
		else if (wcsicmp(L"NET_MASK", type) == 0)
		{
			g_Types[id] = NET_MASK;
		} 
		else if (wcsicmp(L"IP_ADDRESS", type) == 0)
		{
			g_Types[id] = IP_ADDRESS;
		} 
		else if (wcsicmp(L"GATEWAY_ADDRESS", type) == 0)
		{
			g_Types[id] = GATEWAY_ADDRESS;
		} 
		else if (wcsicmp(L"HOST_NAME", type) == 0)
		{
			g_Types[id] = HOST_NAME;
		} 
		else if (wcsicmp(L"DOMAIN_NAME", type) == 0)
		{
			g_Types[id] = DOMAIN_NAME;
		} 
		else if (wcsicmp(L"DNS_SERVER", type) == 0)
		{
			g_Types[id] = DNS_SERVER;
		} 
		else
		{
			std::wstring error = L"No such SysInfoType: ";
			error += type;
			MessageBox(NULL, error.c_str(), L"Rainmeter", MB_OK);
		}
	}

	LPCTSTR data = ReadConfigString(section, L"SysInfoData", L"0");
	if (data)
	{
		g_Datas[id] = _wtoi(data);
	}

	return 0;
}

std::wstring ConvertToWide(LPCSTR str)
{
	std::wstring szWide;

	if (str)
	{
		size_t len = strlen(str) + 1;
		WCHAR* wideSz = new WCHAR[len * 2];
		MultiByteToWideChar(CP_ACP, 0, str, (int)len, wideSz, (int)len * 2);
		szWide = wideSz;
		delete wideSz;
	}
	return szWide;
}

/*
  This function is called when the value should be
  returned as a string.
*/
LPCTSTR GetString(UINT id, UINT flags) 
{
	static WCHAR buffer[4096];
	UINT data;
	DWORD len = 4095;
	std::map<UINT, TYPE>::iterator typeIter = g_Types.find(id);
	std::map<UINT, UINT>::iterator dataIter = g_Datas.find(id);

	if(typeIter == g_Types.end()) return NULL;
	if(dataIter == g_Datas.end())
	{
		data = 0;
	}
	else
	{
		data = (*dataIter).second;
	}

	switch((*typeIter).second)
	{
	case COMPUTER_NAME:
		GetComputerName(buffer, &len);
		return buffer;

	case USER_NAME:
		GetUserName(buffer, &len);
		return buffer;

	case WORK_AREA:
		wsprintf(buffer, L"%i x %i", GetSystemMetrics(SM_CXFULLSCREEN), GetSystemMetrics(SM_CYFULLSCREEN));
		return buffer;

	case SCREEN_SIZE:
		wsprintf(buffer, L"%i x %i", GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN));
		return buffer;

	case RAS_STATUS:
		wsprintf(buffer, L"%s", CheckConnection()?"Online":"Offline");
		return buffer;

	case OS_VERSION:
		GetOSVersion(buffer);
		return buffer;

	case ADAPTER_DESCRIPTION:
		if (ERROR_SUCCESS == GetAdaptersInfo((IP_ADAPTER_INFO*)buffer, &len))
		{
			PIP_ADAPTER_INFO info = (IP_ADAPTER_INFO*)buffer;
			int i = 0;
			while (info)
			{
				if (i == data)
				{
					wcscpy(buffer, ConvertToWide(info->Description).c_str());
					return buffer;
				}
				info = info->Next;
				i++;
			}
		}
		break;

	case IP_ADDRESS:
		if (NO_ERROR == GetIpAddrTable((PMIB_IPADDRTABLE)buffer, &len, FALSE))
		{
			PMIB_IPADDRTABLE ipTable = (PMIB_IPADDRTABLE)buffer;
			if (data < ipTable->dwNumEntries)
			{
				DWORD ip = ipTable->table[data].dwAddr;
				wsprintf(buffer, L"%i.%i.%i.%i", ip%256, (ip>>8)%256, (ip>>16)%256, (ip>>24)%256);
				return buffer;
			}
		}
		break;

	case NET_MASK:
		if (NO_ERROR == GetIpAddrTable((PMIB_IPADDRTABLE)buffer, &len, FALSE))
		{
			PMIB_IPADDRTABLE ipTable = (PMIB_IPADDRTABLE)buffer;
			if (data < ipTable->dwNumEntries)
			{
				DWORD ip = ipTable->table[data].dwMask;
				wsprintf(buffer, L"%i.%i.%i.%i", ip%256, (ip>>8)%256, (ip>>16)%256, (ip>>24)%256);
				return buffer;
			}
		}
		break;

	case GATEWAY_ADDRESS:
		if (ERROR_SUCCESS == GetAdaptersInfo((IP_ADAPTER_INFO*)buffer, &len))
		{
			PIP_ADAPTER_INFO info = (IP_ADAPTER_INFO*)buffer;
			int i = 0;
			while (info)
			{
				if (i == data)
				{
					wcscpy(buffer, ConvertToWide(info->GatewayList.IpAddress.String).c_str());
					return buffer;
				}
				info = info->Next;
				i++;
			}
		}
		break;

	case HOST_NAME:
		if (ERROR_SUCCESS == GetNetworkParams((PFIXED_INFO)buffer, &len))
		{
			PFIXED_INFO info = (PFIXED_INFO)buffer;
			wcscpy(buffer, ConvertToWide(info->HostName).c_str());
			return buffer;
		}
		break;

	case DOMAIN_NAME:
		if (ERROR_SUCCESS == GetNetworkParams((PFIXED_INFO)buffer, &len))
		{
			PFIXED_INFO info = (PFIXED_INFO)buffer;
			wcscpy(buffer, ConvertToWide(info->DomainName).c_str());
			return buffer;
		}
		break;

	case DNS_SERVER:
		if (ERROR_SUCCESS == GetNetworkParams((PFIXED_INFO)buffer, &len))
		{
			PFIXED_INFO info = (PFIXED_INFO)buffer;
			if (info->CurrentDnsServer)
			{
				wcscpy(buffer, ConvertToWide(info->CurrentDnsServer->IpAddress.String).c_str());
			}
			else
			{
				wcscpy(buffer, ConvertToWide(info->DnsServerList.IpAddress.String).c_str());
			}
			return buffer;
		}
		break;
	}

	return NULL;
}

/*
  If the measure needs to free resources before quitting.
  The plugin can export Finalize function, which is called
  when Rainmeter quits (or refreshes).
*/
void Finalize(HMODULE instance, UINT id)
{
	std::map<UINT, TYPE>::iterator i1 = g_Types.find(id);
	if (i1 != g_Types.end())
	{
		g_Types.erase(i1);
	}

	std::map<UINT, UINT>::iterator i2 = g_Datas.find(id);
	if (i2 != g_Datas.end())
	{
		g_Datas.erase(i2);
	}
}

/*
  Fills the buffer with OS version
*/
void GetOSVersion(WCHAR* buffer)
{
	OSVERSIONINFO version;
	version.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
	GetVersionEx(&version);

	if (version.dwPlatformId == VER_PLATFORM_WIN32_NT)
	{
		if (version.dwMajorVersion <= 4)
		{
			wcscpy(buffer, L"Windows NT");
		}
		else
		{
			if (version.dwMinorVersion == 2)
			{
				wcscpy(buffer, L"Windows 2003");
			}
			else if (version.dwMinorVersion == 1)
			{
				wcscpy(buffer, L"Windows XP");
			}
			else if (version.dwMinorVersion == 0)
			{
				wcscpy(buffer, L"Windows 2000");
			}
			else
			{
				wcscpy(buffer, L"Unknown");
			}
		}
	}
	else
	{
		if (version.dwMinorVersion < 10)
		{
			wcscpy(buffer, L"Windows 95");
		}
		else if (version.dwMinorVersion < 90)
		{
			wcscpy(buffer, L"Windows 98");
		}
		else
		{
			wcscpy(buffer, L"Windows ME");
		}
	}
}


/*
  Tests if there is a RAS connection or not. Don't know
  If this works or not (especially on Win9x):-(
*/
BOOL CheckConnection()
{
	static HRASCONN g_hRasConn=NULL;
	RASCONNSTATUS rasStatus;
	LPRASCONN lpRasConn=NULL;
    DWORD cbBuf=0;
    DWORD cConn=1;
    DWORD dwRet=0;

	if(g_hRasConn==NULL) {
	    // Enumerate connections
		cbBuf=sizeof(RASCONN);
		if(((lpRasConn=(LPRASCONN)malloc((UINT)cbBuf))!= NULL)) {            
			lpRasConn->dwSize=sizeof(RASCONN);
			if(0==RasEnumConnections(lpRasConn, &cbBuf, &cConn)) {
				if(cConn!=0) {
					g_hRasConn=lpRasConn->hrasconn;
				}
			}
			free(lpRasConn);
		}
	}

	if(g_hRasConn!=NULL) {
		// get connection status
		rasStatus.dwSize=sizeof(RASCONNSTATUS);
		dwRet=RasGetConnectStatus(g_hRasConn, &rasStatus );
		if(dwRet==0) {
			// Check for connection
			if(rasStatus.rasconnstate==RASCS_Connected) return TRUE;
		} else {
			g_hRasConn=NULL;
		}
	}

    return FALSE;
}

UINT GetPluginVersion()
{
	return 1003;
}

LPCTSTR GetPluginAuthor()
{
	return L"Rainy (rainy@iki.fi)";
}