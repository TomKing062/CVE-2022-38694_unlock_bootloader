#include "BMPlatform.h"
#include <iostream>

CBMPlatformApp::CBMPlatformApp()
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
	m_pfCreateChannel = NULL;
	m_pfReleaseChannel = NULL;
	m_hChannelLib = NULL;
}

BOOL CBMPlatformApp::InitInstance()
{
	m_hChannelLib = LoadLibrary(_T("Channel9.dll"));

	if(m_hChannelLib == NULL)
	{
		std::cout << GetLastError() << std::endl;
		return FALSE;
	}

	m_pfCreateChannel = (pfCreateChannel)GetProcAddress(m_hChannelLib,"CreateChannel");
	m_pfReleaseChannel = (pfReleaseChannel)GetProcAddress(m_hChannelLib,"ReleaseChannel");

	if(m_pfCreateChannel == NULL || m_pfReleaseChannel == NULL )
	{
		return FALSE;
	}

	return TRUE;
}

int CBMPlatformApp::ExitInstance()
{
	// TODO: Add your specialized code here and/or call the base class
	if(m_hChannelLib)
	{
		FreeLibrary(m_hChannelLib);
		m_hChannelLib = NULL;

		//m_pfCreateChannel = NULL;
		//m_pfReleaseChannel = NULL;
	}

	return TRUE;
}

CBMPlatformApp g_theApp;

CBootModeOpr::CBootModeOpr()
{
	m_bOpened = FALSE;
	m_pChannel = NULL;
	g_theApp.InitInstance();
}

CBootModeOpr::~CBootModeOpr()
{
	g_theApp.ExitInstance();
}

BOOL CBootModeOpr::Initialize(DWORD dwPort)
{
	m_bOpened = FALSE;


	if (!g_theApp.m_pfCreateChannel)
	{
		return FALSE;
	}
	if (!g_theApp.m_pfCreateChannel((ICommChannel**)&m_pChannel, CHANNEL_TYPE_COM))
	{
		return FALSE;
	}
	m_bOpened = ConnectChannel(dwPort);
	return m_bOpened;
}

void CBootModeOpr::Uninitialize()
{
	DisconnectChannel();
	if (m_pChannel)
	{
		if (g_theApp.m_pfReleaseChannel)
			g_theApp.m_pfReleaseChannel(m_pChannel);
		m_pChannel = NULL;
	}
}

int CBootModeOpr::Read(UCHAR * m_RecvData, int max_len, int dwTimeout)
{
	DWORD dwBegin = GetTickCount();
	DWORD dwCur = GetTickCount();
	do
	{
		dwCur = GetTickCount();
		DWORD dwRead = m_pChannel->Read(m_RecvData, max_len, dwTimeout);

		if (dwRead)
		{
			return dwRead;
		}
	} while ((dwCur - dwBegin) < (DWORD)dwTimeout);
	return 0;
}

int CBootModeOpr::Write(UCHAR* lpData, int iDataSize)
{
	return (int)(m_pChannel->Write(lpData, iDataSize));
}

BOOL CBootModeOpr::ConnectChannel(DWORD dwPort)
{
	if(!dwPort) return FALSE;
	DWORD dwBaud = 115200;

	CHANNEL_ATTRIBUTE ca;
	ca.ChannelType = CHANNEL_TYPE_COM;
	ca.Com.dwPortNum = dwPort;
	ca.Com.dwBaudRate = dwBaud;

	//Log(_T("Connect Channel +++"));
	BOOL bOK = m_pChannel->Open(&ca);
	//Log(_T("Connect Channel ---"));
	if (!bOK)
	{
		//Log(_T("Open Channel fail."));
		return FALSE;
	}
	return TRUE;
}

BOOL CBootModeOpr::DisconnectChannel()
{
	if (m_pChannel == NULL)
	{
		return FALSE;
	}

	if (m_bOpened)
	{
		m_bOpened = FALSE;
		//Log(_T("Disconnect Channel +++"));
		m_pChannel->Close();
		//Log(_T("Disconnect Channel ---"));
	}
	return TRUE;
}
