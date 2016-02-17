#pragma once

#include "BodyParsingData.h"
#include <list>

class CBodySpliter
{
private:
	char* m_storedData;								// split �� ���� data
	std::list<char*> m_bodyListData;				// split�� data
public:
	CBodySpliter();
	~CBodySpliter();

	void StoreData(char* data);						// �ܺηκ��� data �Է�
	void FillBodyListByParsing();					// ���� data�� split ���Ѽ� list�� ����
	std::list<char*>* GetBodyList();				// list�� ���� ����

	bool IsBodyNull();
	char* GetBodyString();
	int GetBodyLength();

	void PrintData();
	void Cleanup();
};