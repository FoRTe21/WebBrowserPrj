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

	char m_uriBuffer[URLLENGTH];				// 입력받을 URL
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

	bool Connection();							// 소켓 생성과 함께 접속
	bool ConnectionByHostName();				// hostname을 이용하여 접속
	bool ConnectionByAddress();					// xxx.xxx.xxx.xxx 형태의 ip 주소로 접속
	bool SendRequest();							// Request를 보내고 response 수신
	bool CheckUrlFormat();						// Url 포맷 확인

	void FillUriBuffer(char* buffer);			// Url 버퍼를 입력으로부터 채움
	void MakeGETHeader();						// GET 헤더파일 생성
	void MakePostHeader(int bodyLength);						// POST 헤더파일 생성
	bool ParsingURI();							// URI 파싱
	

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