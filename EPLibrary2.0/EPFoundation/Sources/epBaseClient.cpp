/*! 
BaseClient for the EpLibrary
Copyright (C) 2012  Woong Gyu La <juhgiyo@gmail.com>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#include "epBaseClient.h"
using namespace epl;

BaseClient::BaseClient(const TCHAR * hostName, const TCHAR * port,LockPolicy lockPolicyType)
{
	SetHostName(hostName);
	SetPort(port);
	m_connectSocket=NULL;
	m_result=0;
	m_ptr=0;
	m_isConnected=false;
	m_lockPolicy=lockPolicyType;
	switch(lockPolicyType)
	{
	case LOCK_POLICY_CRITICALSECTION:
		m_sendLock=EP_NEW CriticalSectionEx();
		m_generalLock=EP_NEW CriticalSectionEx();
		break;
	case LOCK_POLICY_MUTEX:
		m_sendLock=EP_NEW Mutex();
		m_generalLock=EP_NEW Mutex();
		break;
	case LOCK_POLICY_NONE:
		m_sendLock=EP_NEW NoLock();
		m_generalLock=EP_NEW NoLock();
		break;
	default:
		m_sendLock=NULL;
		m_generalLock=NULL;
		break;
	}
}

BaseClient::BaseClient(const BaseClient& b)
{
	m_connectSocket=NULL;
	m_result=0;
	m_ptr=0;
	m_isConnected=false;
	m_hostName=b.m_hostName;
	m_port=b.m_port;
	m_lockPolicy=b.m_lockPolicy;
	switch(m_lockPolicy)
	{
	case LOCK_POLICY_CRITICALSECTION:
		m_sendLock=EP_NEW CriticalSectionEx();
		m_generalLock=EP_NEW CriticalSectionEx();
		break;
	case LOCK_POLICY_MUTEX:
		m_sendLock=EP_NEW Mutex();
		m_generalLock=EP_NEW Mutex();
		break;
	case LOCK_POLICY_NONE:
		m_sendLock=EP_NEW NoLock();
		m_generalLock=EP_NEW NoLock();
		break;
	default:
		m_sendLock=NULL;
		m_generalLock=NULL;
		break;
	}
}
BaseClient::~BaseClient()
{
	Disconnect();
	if(m_sendLock)
		EP_DELETE m_sendLock;
	if(m_generalLock)
		EP_DELETE m_generalLock;
}

bool BaseClient::SetHostName(const TCHAR * hostName)
{
	LockObj lock(m_generalLock);
	if(m_isConnected)
		return false;

	unsigned int strLength=System::TcsLen(hostName);
	if(strLength==0)
		m_hostName=DEFAULT_HOSTNAME;
	else
	{
		
		char *tmpString=EP_NEW char[strLength+1];
		System::WideCharToMultiByte(hostName,tmpString);
		m_hostName=tmpString;
		EP_DELETE[] tmpString;
	}
	return true;
}

bool BaseClient::SetPort(const TCHAR *port)
{
	LockObj lock(m_generalLock);
	if(m_isConnected)
		return false;

	unsigned int strLength=System::TcsLen(port);
	if(strLength==0)
		m_port=DEFAULT_PORT;
	else
	{
		char *tmpString=EP_NEW char[strLength+1];
		System::WideCharToMultiByte(port,tmpString);
		m_port=tmpString;
		EP_DELETE[] tmpString;
	}
	return true;

}
EpTString BaseClient::GetHostName() const
{
	if(!m_hostName.length())
		return _T("");
	EpTString retString;
	TCHAR *hostName=EP_NEW TCHAR[m_hostName.length()+1];
	System::MultiByteToWideChar(m_hostName.c_str(),m_hostName.length(),hostName);
	retString=hostName;
	EP_DELETE[] hostName;
	return retString;
}
EpTString BaseClient::GetPort() const
{
	if(!m_port.length())
		return _T("");
	EpTString retString;
	TCHAR *port=EP_NEW TCHAR[m_port.length()+1];
	System::MultiByteToWideChar(m_port.c_str(),m_port.length(),port);
	retString=port;
	EP_DELETE[] port;
	return retString;
}

int BaseClient::Send(const Packet &packet)
{
	LockObj lock(m_sendLock);
	int writeLength=0;
	const char *packetData=packet.GetPacket();
	int length=packet.GetPacketByteSize();
	if(length>0)
	{
		int sentLength=send(m_connectSocket,reinterpret_cast<char*>(&length),4,0);
		if(sentLength<=0)
			return false;
	}
	while(length>0)
	{
		int sentLength=send(m_connectSocket,packetData,length,0);
		writeLength+=sentLength;
		if(sentLength<=0)
		{
			Disconnect();
			return writeLength;
		}
		length-=sentLength;
		packetData+=sentLength;
	}
	return writeLength;
}

int BaseClient::receive(Packet &packet)
{
	
	int readLength=0;
	int length=packet.GetPacketByteSize();
	char *packetData=const_cast<char*>(packet.GetPacket());
	while(length>0)
	{
		int recvLength=recv(m_connectSocket,packetData, length, 0);
		readLength+=recvLength;
		if(recvLength<=0)
			break;
		length-=recvLength;
		packetData+=recvLength;
	}
	return readLength;
}

bool BaseClient::Connect()
{
	LockObj lock(m_generalLock);
	if(m_isConnected)
		return true;
	if(!m_port.length())
	{
		m_port=DEFAULT_PORT;
	}

	if(!m_hostName.length())
	{
		m_hostName=DEFAULT_HOSTNAME;
	}


	WSADATA wsaData;
	m_connectSocket = INVALID_SOCKET;
	struct addrinfo hints;
	int iResult;

	// Initialize Winsock
	iResult = WSAStartup(MAKEWORD(2,2), &wsaData);
	if (iResult != 0) {
		EP_NOTICEBOX(_T("WSAStartup failed with error\n"));
		return false;
	}

	ZeroMemory( &hints, sizeof(hints) );
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;

	// Resolve the server address and port
	iResult = getaddrinfo(m_hostName.c_str(), m_port.c_str(), &hints, &m_result);
	if ( iResult != 0 ) {
		EP_NOTICEBOX(_T("getaddrinfo failed with error\n"));
		WSACleanup();
		return false;
	}

	// Attempt to connect to an address until one succeeds
	for(m_ptr=m_result; m_ptr != NULL ;m_ptr=m_ptr->ai_next) {

		// Create a SOCKET for connecting to server
		m_connectSocket = socket(m_ptr->ai_family, m_ptr->ai_socktype, 
			m_ptr->ai_protocol);
		if (m_connectSocket == INVALID_SOCKET) {
			EP_NOTICEBOX(_T("socket failed with error\n"));
			Disconnect();
			return false;
		}

		// Connect to server.
		iResult = connect( m_connectSocket, m_ptr->ai_addr, static_cast<int>(m_ptr->ai_addrlen));
		if (iResult == SOCKET_ERROR) {
			closesocket(m_connectSocket);
			m_connectSocket = INVALID_SOCKET;
			continue;
		}
		break;
	}
	if (m_connectSocket == INVALID_SOCKET) {
		EP_NOTICEBOX(_T("Unable to connect to server!\n"));
		Disconnect();
		return false;
	}
	m_isConnected=true;

	m_clientThreadHandle = CreateThread( NULL, 0, reinterpret_cast<LPTHREAD_START_ROUTINE>(BaseClient::ClientThread), this, 0, NULL);  
	if(m_clientThreadHandle)
		return true;
	return true;
}


unsigned long BaseClient::ClientThread( LPVOID lpParam ) 
{
	BaseClient *pMainClass=reinterpret_cast<BaseClient*>(lpParam);
	int iResult;
	/// Receive buffer
	Packet recvSizePacket(NULL,4);
	// Receive until the peer shuts down the connection
	do {
		int size =pMainClass->receive(recvSizePacket);
		if(size>0)
		{
			unsigned int shouldReceive=(reinterpret_cast<unsigned int*>(const_cast<char*>(recvSizePacket.GetPacket())))[0];
			Packet recvPacket(NULL,shouldReceive);
			iResult = pMainClass->receive(recvPacket);

			if (iResult == shouldReceive) {
				// Process Received Packet and Send Result to Client
				if(pMainClass->parsePacket(recvPacket)<=0)
				{
					break;
				}
			}
			else if (iResult == 0)
			{
				LOG_THIS_MSG(_T("Connection closing...\n"));
				break;
			}
			else  {
				LOG_THIS_MSG(_T("recv failed with error\n"));
				break;
			}
		}
		else
		{
			break;
		}

	} while (iResult > 0);
	return 0; 
}

bool BaseClient::IsConnected() const
{
	return m_isConnected;
}


void BaseClient::Disconnect()
{
	LockObj lock(m_generalLock);
	// No longer need server socket
	if(m_result)
		freeaddrinfo(m_result);

	if(m_isConnected)
	{
		// shutdown the connection since no more data will be sent
		int iResult = shutdown(m_connectSocket, SD_SEND);
		if (iResult == SOCKET_ERROR) {
			LOG_THIS_MSG(_T("shutdown failed with error: %d\n"), WSAGetLastError());
		}
		closesocket(m_connectSocket);
	}
	m_isConnected=false;
	m_connectSocket = INVALID_SOCKET;
	m_result=NULL;
	WSACleanup();
}


