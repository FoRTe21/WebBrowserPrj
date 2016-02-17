#include "CWebSocket.h"

#include <stdio.h>

CWebSocket::CWebSocket()
{
	m_sock = INVALID_SOCKET;
	m_uri = NULL;
	m_responseBuffer = NULL;
	m_uriFlag = false;
	m_responseDataLength = 0;
	m_requestFlag = -1;
	m_bodyData = NULL;
}

CWebSocket::~CWebSocket()
{

}

bool CWebSocket::InitSock()
{
	if (WSAStartup(MAKEWORD(2, 2), &m_wsaData) != NO_ERROR)
	{
		return false;
	}

	return true;
}

void CWebSocket::FillUriBuffer(char* buffer)
{
	strcpy_s(m_uriBuffer, URLLENGTH, buffer);
}

bool CWebSocket::Connection()
{
	if (m_uriFlag == false)					// domain name인지, ip:port 형식인지 확인
	{
		if (ParsingURI() == false)
		{
			return false;
		}
		if (ConnectionByAddress() == false)		// ip address, port number로 접속
		{
			return false;
		}
	}
	else
	{
		if (ConnectionByHostName() == false)	// domain name으로 접속
		{
			return false;
		}
	}

	return true;
}

bool CWebSocket::ConnectionByHostName()
{
	struct addrinfo hints;
	struct addrinfo* result;
	struct addrinfo* ptr = NULL;

	int iResult;

	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;

	iResult = getaddrinfo(m_uriBuffer, "http", &hints, &result);

	if (iResult != 0)
	{
		return false;
	}

	for (ptr = result; ptr != NULL; ptr = ptr->ai_next)
	{
		m_sock = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
		if (m_sock == INVALID_SOCKET)
		{
			return false;
		}

		iResult = connect(m_sock, ptr->ai_addr, (int)ptr->ai_addrlen);
		if (iResult == SOCKET_ERROR)
		{
			CloseSocket();
			m_sock = INVALID_SOCKET;
			continue;
		}
		break;
	}

	freeaddrinfo(result);

	if (m_sock == INVALID_SOCKET)
	{
		return false;
	}

	return true;
}

bool CWebSocket::ConnectionByAddress()
{
	struct sockaddr_in addr;

	if ((m_sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		return false;
	}

	memset((void *)&addr, 0x00, sizeof(addr));
	addr.sin_family = AF_INET;
	inet_pton(AF_INET, m_addrBuffer, &addr.sin_addr.s_addr);
	addr.sin_port = htons(m_portNum);

	if (connect(m_sock, (struct sockaddr *)&addr, sizeof(addr)) == SOCKET_ERROR)
	{
		CloseSocket();
		return false;
	}

	return true;
}

bool CWebSocket::SendRequest()
{
	char tmpBuf[BODYLENGTH] = { 0, };
	char* tmpPoint = NULL;
	m_responseDataLength = 0;
	int totalLength = 0;
	int prevTotalLen = 0;
	int len = 0;
	int i = 0;

	send(m_sock, m_requestBuffer, strlen(m_requestBuffer), 0);

	if(m_requestFlag == 1)										// POST request라면 body data를 더 전송
	{
		send(m_sock, m_bodyData, strlen(m_bodyData), 0);
	}

	if (m_responseBuffer != NULL)
	{
		delete[] m_responseBuffer;
		m_responseBuffer = NULL;
	}

	while ((len = recv(m_sock, tmpBuf, BODYLENGTH, 0)) > 0)		// 버퍼의 크기를 점점 크게 하면서 response data 전체를 받음.
	{
		//printf("tmpBuf : %s\n", tmpBuf);
		prevTotalLen = totalLength;
		totalLength = totalLength + len;
		if (m_responseBuffer == NULL)							
		{
			m_responseBuffer = new char[totalLength + 1];
			memset(m_responseBuffer, 0, totalLength + 1);
			memcpy_s(m_responseBuffer, totalLength + 1, tmpBuf, totalLength + 1);
			continue;
		}
		tmpPoint = m_responseBuffer;
		m_responseBuffer = new char[totalLength + 1];
		memset(m_responseBuffer, 0, totalLength + 1);
		memcpy_s(m_responseBuffer, prevTotalLen + 1, tmpPoint, prevTotalLen + 1);
		memcpy_s(m_responseBuffer + prevTotalLen, len + 1, tmpBuf, len + 1);

		delete[] tmpPoint;
		tmpPoint = NULL;
		memset(tmpBuf, 0, sizeof(char) * BODYLENGTH);
	}

	m_responseDataLength = totalLength;
	printf("\nresponseBuffer : %s\n", m_responseBuffer);
	return true;
}

int CWebSocket::GetTotalReceiveLength()
{
	return m_responseDataLength;
}

void CWebSocket::MakeGETHeader()
{
	if (m_uri == NULL)
	{
		wsprintf(m_requestBuffer, "GET /index.html HTTP/1.1\r\n\r\n", m_uri);
	}
	else
	{
		wsprintf(m_requestBuffer, "GET /%s HTTP/1.1\r\n\r\n", m_uri);
	}
}

void CWebSocket::MakePostHeader(int bodyLength)
{
	wsprintf(m_requestBuffer, "POST /%s HTTP/1.1\r\nContent-Length: %d\r\n\r\n", m_uri, bodyLength);
}
bool CWebSocket::ParsingURI()
{
	char* tok = NULL;
	char* context = NULL;
	int length = 0;
	tok = strtok_s(m_uriBuffer, "/", &context);
	if (tok == NULL)
	{
		return false;
	}

	tok = strtok_s(context, "/", &context);
	if (context == NULL)
	{
		m_uri = NULL;
	}
	else
	{
		length = strlen(context);
		m_uri = new char[length + 1];
		strcpy_s(m_uri, strlen(context) + 1, context);
	}

	tok = strtok_s(tok, ":", &context);
	if (tok == NULL)
	{
		return false;
	}

	strcpy_s(m_addrBuffer, ADDRLENGTH, tok);

	if (context == NULL)
	{
		return false;
	}

	m_portNum = atoi(context);
	if (m_portNum == 0)
	{
		return false;
	}

	return true;
}

void CWebSocket::MemoryClear()
{
	m_requestFlag = -1;
	if (m_responseBuffer != NULL)
	{
		delete[] m_responseBuffer;
		m_responseBuffer = NULL;
	}

	if (m_uri != NULL)
	{
		delete[] m_uri;
		m_uri = NULL;
	}

	if(m_bodyData != NULL)
	{
		delete[] m_bodyData;
		m_bodyData = NULL;
	}
}

void CWebSocket::FillBodyData(char* body)
{
	if(m_bodyData != NULL)
	{
		delete[] m_bodyData;
		m_bodyData = NULL;
	}
	m_bodyData = new char[strlen(body) + 1];
	strcpy_s(m_bodyData, strlen(body) + 1, body);
}

char* CWebSocket::GetResponseBuffer()
{
	return m_responseBuffer;
}

int CWebSocket::GetResponseBufferLength()
{
	return strlen(m_responseBuffer);
}

bool CWebSocket::CheckUrlFormat()
{
	if (m_uriBuffer[0] == 'h')
	{
		m_uriFlag = false;
		return true;
	}
	else
	{
		m_uriFlag = true;
		return true;
	}

	return false;
}

void CWebSocket::Cleanup()
{
	WSACleanup();
}

void CWebSocket::CloseSocket()
{
	closesocket(m_sock);
	m_sock = INVALID_SOCKET;
}

char* CWebSocket::GetUri()
{
	if(m_uri != NULL)
	{
		return m_uri;
	}
	else
	{
		return NULL;
	}
}

char* CWebSocket::GetAddr()
{
	return m_addrBuffer;
}

int CWebSocket::GetPortNum()
{
	return m_portNum;
}

void CWebSocket::SetRequestFlag(int flag)
{
	m_requestFlag = flag;
}
int CWebSocket::GetRequestFlag()
{
	return m_requestFlag;
}