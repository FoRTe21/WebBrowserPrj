#pragma once
#pragma comment(linker, "/entry:WinMainCRTStartup /subsystem:console")

#include <string.h>
#include <stdio.h>
#include "ParsingData.h"

class CHtmlParser
{
private:
	char* m_responseData;				// 제일 처음 받는 데이터(원본 데이터)
	char* m_pHeader;					// http 헤더 제외한 데이터
	char* m_httpHeader;					// http 헤더
	char* m_titleName;					// title 이름
	char* m_uri;
	char *m_extension;

	char* m_style;						// style
	char* m_bgColor;					// window background color
	char* m_fontSize;	
	char* m_textAlign;
	char* m_color;						// text color
	
	char* m_body;						// response 데이터 중 각종 header를 뗀 body 부분
	int m_responseDataLength;			// HTTP header를 뗀 부분의 길이

private:
	bool SetExtension();

public:
	CHtmlParser();
	~CHtmlParser();

	void ExtractHttpHeaderFromResponse();			// http 헤더 파싱
	bool ExtractHtmlHeaderFromResponse();			// html 헤더 파싱
	bool ExtractStyleInfoFromStyle();				// style info 파싱
	bool ExtractBodyFromResponse();					// body 파싱

	void SetURI(char* uri);

	bool FillHeaderContents(char* pHead, char* pTail, char* tag);					// 헤더 데이터들 저장
	void StoreResponseData(char* data, int dataLength);								// 소켓으로 받은 데이터를 원본저장		
	
	void PrintData();				// 데이터 확인용
	
	void Cleanup();					

	bool IsTitleNull();				// title 유무 확인
	char* GetTitleName();
	int GetTitleNameLength();

	void SetBodyNull();				
	bool IsBodyNull();				// body 유무 확인
	char* GetBodyString();
	int GetBodyLength();

	void SetImageFileSize();		// HTTP header를 제외한 body 크기 = 순수 이미지 데이터
	void WriteImageFile();			// image data local disk에 쓰기
	int ExtensionCheck();			// URI extension
	int CheckHttpHeaderNCode();		// HTTP header의 형식 및 code 확인
};