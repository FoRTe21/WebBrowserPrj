#include "CHtmlParser.h"
#include <windows.h>

CHtmlParser::CHtmlParser()
{
	m_responseData = NULL;
	m_httpHeader = NULL;
	m_fontSize = NULL;
	m_textAlign = NULL;
	m_color = NULL;
	m_body = NULL;
	m_titleName = NULL;
	m_style = NULL;
	m_bgColor = NULL;
	m_uri = NULL;

	m_responseDataLength = 0;
}

CHtmlParser::~CHtmlParser()
{

}

void CHtmlParser::StoreResponseData(char* data, int dataLength)
{
	m_responseDataLength = dataLength;
	m_responseData = new char[dataLength + 1];
	memcpy_s(m_responseData, dataLength + 1, data, dataLength + 1);
}

bool CHtmlParser::ExtractHtmlHeaderFromResponse()
{
	char* pH = NULL;
	char* pT = NULL;

	pH = strstr(m_responseData, TITLEH);				// <TITLE> parsing
	pT = strstr(m_responseData, TITLET);
	if (FillHeaderContents(pH, pT, TITLEH) == false)
	{
		return false;
	}

	pH = NULL;
	pT = NULL;
	pH = strstr(m_responseData, STYLEH);				// <STYLE> parsing
	pT = strstr(m_responseData, STYLET);
	if (FillHeaderContents(pH, pT, STYLEH) == true)		// STYLE이 존재하면 STYLE data 저장
	{
		ExtractStyleInfoFromStyle();
	}

	return true;
}

bool CHtmlParser::FillHeaderContents(char* pHead, char* pTail, char* tag)		// pHead부터 pTail 사이에 있는 tag에 해당하는 데이터를 저장
{																				// ex) <tag>... </tag> 에서 ...를 저장
	char* pH = NULL;
	char* pT = NULL;
	char* tmpBuf = NULL;
	int index = 0;

	if (pHead == NULL || pTail == NULL)
	{
		return false;
	}
	pH = pHead + strlen(tag);
	pT = pTail;

	for (index = 0; (&pH[index] != pT) && (index < (int)strlen(m_responseData)); index++);

	tmpBuf = new char[index + 1];
	strncpy_s(tmpBuf, index + 1, pH, index);
	if (strcmp(tag, TITLEH) == 0)
	{
		m_titleName = tmpBuf;
	}
	else if (strcmp(tag, STYLEH) == 0)
	{
		m_style = tmpBuf;
	}
	else
	{
		return false;
	}

	return true;
}

bool CHtmlParser::ExtractStyleInfoFromStyle()				// style data를 parsing 하기 위한 예비로 만들어 놓은 function
{
	return true;
}

bool CHtmlParser::ExtractBodyFromResponse()					// response data로부터 Header를 떼고 Body를 parsing
{
	char* pH = NULL;
	char* pT = NULL;
	int index = 0;

	pH = strstr(m_responseData, BODYH);
	if (pH == NULL)
	{
		if((pH = strstr(m_responseData, BBODYH)) == NULL)
		{
			return false;
		}
		else
		{
			pH = pH + strlen(BBODYH);
		}
	}
	else
	{
		pH = pH + strlen(BODYH);
	}
	pT = strstr(m_responseData, BODYT);
	if (pT == NULL)
	{
		if((pT = strstr(m_responseData, BBODYT)) == NULL)
		{
			return false;
		}
	}
	for (index = 0; (&pH[index] != pT) && index < strlen(m_responseData); index++);

	m_body = new char[index + 1];
	strncpy_s(m_body, index + 1, pH, index);

	return true;
}

void CHtmlParser::ExtractHttpHeaderFromResponse()					// HTTP 헤더 parsing
{
	int index = 0;
	while (true)
	{
		if (m_responseData[index] == '\r')
		{
			break;
		}
		index++;
	}

	m_httpHeader = new char[index + 1];
	strncpy_s(m_httpHeader, index + 1, m_responseData, index);

	m_pHeader = strstr(m_responseData, HTMLH);						// <HTML> 태그가 없는 경우 HTML 텍스트 파일 외의 파일로 인식하여 헤더를 제외한부분을 body로 저장.

	if (m_pHeader == NULL)
	{
		m_pHeader = strstr(m_responseData, "\r\n\r\n");
		m_pHeader = m_pHeader + strlen("\r\n\r\n");
	}

}

int CHtmlParser::CheckHttpHeaderNCode()
{
	char* p = NULL;

	if(m_httpHeader == NULL)
	{
		return -1;
	}
	
	if((p = strstr(m_httpHeader, "HTTP/1.1")) == NULL)
	{
		return -1;
	}

	if((p = strstr(m_httpHeader, "200 OK")) != NULL)
	{
		return OK;
	}

	if((p = strstr(m_httpHeader, "404 Not Found")) != NULL)
	{
		return NotFound;
	}

	if((p = strstr(m_httpHeader, "400 Bad Request")) != NULL)
	{
		return BadRequest;
	}

	return -1;

}

void CHtmlParser::PrintData()
{
	printf("responseData : %s\n", m_responseData);
	printf("httpHeader : %s\n", m_httpHeader);
	printf("titleName : %s\n", m_titleName);
	printf("style : %s\n", m_style);
	printf("body : %s\n", m_body);
}

void CHtmlParser::Cleanup()
{

	if (m_responseData != NULL)
	{
		delete[] m_responseData;
		m_responseData = NULL;
	}
	if (m_httpHeader != NULL)
	{
		delete[] m_httpHeader;
		m_httpHeader = NULL;
	}
	if (m_titleName != NULL)
	{
		delete[] m_titleName;
		m_titleName = NULL;
	}
	if (m_style != NULL)
	{
		delete[] m_style;
		m_style = NULL;
	}
	if (m_bgColor != NULL)
	{
		delete[] m_bgColor;
		m_bgColor = NULL;
	}
	if (m_fontSize != NULL)
	{
		delete[] m_fontSize;
		m_fontSize = NULL;
	}
	if (m_textAlign != NULL)
	{
		delete[] m_textAlign;
		m_textAlign = NULL;
	}
	if (m_color != NULL)
	{
		delete[] m_color;
		m_color = NULL;
	}
	if (m_body != NULL)
	{
		delete[] m_body;
		m_body = NULL;
	}
	if (m_uri != NULL)
	{
		delete[] m_uri;
		m_uri = NULL;
	}
	if (m_extension != NULL)
	{
		delete[] m_extension;
		m_extension = NULL;
	}

	m_responseDataLength = 0;
}

void CHtmlParser::SetBodyNull()
{
	if (m_body != NULL)
	{
		delete[] m_body;
		m_body = NULL;
	}
}

bool CHtmlParser::IsBodyNull()
{
	if (m_body == NULL)
	{
		return true;
	}

	return false;
}

char* CHtmlParser::GetBodyString()
{
	return m_body;
}

int CHtmlParser::GetBodyLength()
{
	return strlen(m_body);
}

char* CHtmlParser::GetTitleName()
{
	return m_titleName;
}

int CHtmlParser::GetTitleNameLength()
{
	return strlen(m_titleName);
}

bool CHtmlParser::IsTitleNull()
{
	if (m_titleName == NULL)
	{
		return true;
	}

	return false;
}

void CHtmlParser::SetURI(char* uri)
{
	if(uri == NULL)
	{
		return ;
	}
	if (m_uri != NULL)
	{
		delete[] m_uri;
		m_uri = NULL;
	}

	m_uri = new char[strlen(uri) + 1];

	strcpy_s(m_uri, strlen(uri) + 1, uri);

	if (m_uri != NULL)
	{
		SetExtension();
	}
}

bool CHtmlParser::SetExtension()			// filename으로부터 extension 얻어옴
{
	char* ext = NULL;

	ext = strstr(m_uri, ".");

	if(ext == NULL)
	{
		return false;
	}
	ext = ext + 1;

	if (m_extension != NULL)
	{
		delete[] m_extension;
		m_extension = NULL;
	}
	m_extension = new char[strlen(ext) + 1];
	strcpy_s(m_extension, strlen(ext) + 1, ext);
	
	return true;
}

void CHtmlParser::SetImageFileSize()			// response data length = response length - http header length - length of (\r\n\r\n)
{
	m_responseDataLength = m_responseDataLength - strlen(m_httpHeader) - strlen("\r\n\r\n");
}

void CHtmlParser::WriteImageFile()				// image 파일 local disk에 저장
{
	HANDLE hFile;
	DWORD result = 0;

	hFile = CreateFile(m_uri, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, 0, NULL);

	//printf("\n%s\n", m_pHeader);
	if (WriteFile(hFile, m_pHeader, m_responseDataLength, &result, NULL) == 0)
	{
		printf("WriteError\n");
	}

	CloseHandle(hFile);
}

int CHtmlParser::ExtensionCheck()
{
	if (m_extension == NULL)
	{
		return -1;
	}
	if (strcmp(m_extension, HTMLE) == 0)
	{
		return HTMLM;
	}
	else if ((strcmp(m_extension, JPGE) == 0) || 
			 (strcmp(m_extension, JPEGE) == 0) ||
			 (strcmp(m_extension, BMPE) == 0))
	{
		return IMAGEM;
	}
	else if(strcmp(m_extension, CGIE) == 0)
	{
		return CGIM;
	}
	else
	{
		return -1;
	}
}