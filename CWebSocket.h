#pragma once


#include <winsock2.h>
#include <WS2tcpip.h>

#define URLLENGTH 1024
#define REQUESTHEADERLENGTH 1024
#define BODYLENGTH 32
#define ADDRLENGTH 15



class CWebSocket
{
private:
	WSADATA m_wsaData;
	SOCKET m_sock;

	int m_portNum;
	bool m_uriFlag;								// T : domain name
												// F : ipaddress:port
	int m_requestFlag;							// 0 : GET
												// 1 : POST

	char m_uriBuffer[URLLENGTH];				// �Է¹��� URL
	char m_addrBuffer[ADDRLENGTH];				// xxx.xxx.xxx.xxx
	char m_requestBuffer[REQUESTHEADERLENGTH];	// GET ./index.html HTTP/1.1\r\n ...

	char* m_uri;
	char* m_responseBuffer;
	char* m_bodyData;

	int m_responseDataLength;
	hostent* m_hosts;

public:
	CWebSocket();
	~CWebSocket();
	bool InitSock();

	bool Connection();							// ���� ������ �Բ� ����
	bool ConnectionByHostName();				// hostname�� �̿��Ͽ� ����
	bool ConnectionByAddress();					// xxx.xxx.xxx.xxx ������ ip �ּҷ� ����
	bool SendRequest();							// Request�� ������ response ����
	bool CheckUrlFormat();						// Url ���� Ȯ��

	void FillUriBuffer(char* buffer);			// Url ���۸� �Է����κ��� ä��
	void MakeGETHeader();						// GET ������� ����
	void MakePostHeader(int bodyLength);						// POST ������� ����
	bool ParsingURI();							// URI �Ľ�
	

	void CloseSocket();
	void Cleanup();
	void MemoryClear();

	char* GetResponseBuffer();
	int GetResponseBufferLength();
	char* GetUri();

	char* GetAddr();
	int GetPortNum();

	int GetTotalReceiveLength();

	void SetRequestFlag(int flag);
	int GetRequestFlag();

	void FillBodyData(char* body);
};