#pragma once
#pragma comment(linker, "/entry:WinMainCRTStartup /subsystem:console")

#include <string.h>
#include <stdio.h>
#include "ParsingData.h"

class CHtmlParser
{
private:
	char* m_responseData;				// ���� ó�� �޴� ������(���� ������)
	char* m_pHeader;					// http ��� ������ ������
	char* m_httpHeader;					// http ���
	char* m_titleName;					// title �̸�
	char* m_uri;
	char *m_extension;

	char* m_style;						// style
	char* m_bgColor;					// window background color
	char* m_fontSize;	
	char* m_textAlign;
	char* m_color;						// text color
	
	char* m_body;						// response ������ �� ���� header�� �� body �κ�
	int m_responseDataLength;			// HTTP header�� �� �κ��� ����

private:
	bool SetExtension();

public:
	CHtmlParser();
	~CHtmlParser();

	void ExtractHttpHeaderFromResponse();			// http ��� �Ľ�
	bool ExtractHtmlHeaderFromResponse();			// html ��� �Ľ�
	bool ExtractStyleInfoFromStyle();				// style info �Ľ�
	bool ExtractBodyFromResponse();					// body �Ľ�

	void SetURI(char* uri);

	bool FillHeaderContents(char* pHead, char* pTail, char* tag);					// ��� �����͵� ����
	void StoreResponseData(char* data, int dataLength);								// �������� ���� �����͸� ��������		
	
	void PrintData();				// ������ Ȯ�ο�
	
	void Cleanup();					

	bool IsTitleNull();				// title ���� Ȯ��
	char* GetTitleName();
	int GetTitleNameLength();

	void SetBodyNull();				
	bool IsBodyNull();				// body ���� Ȯ��
	char* GetBodyString();
	int GetBodyLength();

	void SetImageFileSize();		// HTTP header�� ������ body ũ�� = ���� �̹��� ������
	void WriteImageFile();			// image data local disk�� ����
	int ExtensionCheck();			// URI extension
	int CheckHttpHeaderNCode();		// HTTP header�� ���� �� code Ȯ��
};