#pragma once

#include "BodyParsingData.h"
#include <list>

class CBodySpliter
{
private:
	char* m_storedData;								// split 할 원본 data
	std::list<char*> m_bodyListData;				// split된 data
public:
	CBodySpliter();
	~CBodySpliter();

	void StoreData(char* data);						// 외부로부터 data 입력
	void FillBodyListByParsing();					// 원본 data를 split 시켜서 list에 저장
	std::list<char*>* GetBodyList();				// list에 대해 접근

	bool IsBodyNull();
	char* GetBodyString();
	int GetBodyLength();

	void PrintData();
	void Cleanup();
};