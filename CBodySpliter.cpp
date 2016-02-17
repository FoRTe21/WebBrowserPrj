#include "CBodySpliter.h"

CBodySpliter::CBodySpliter()
{
	m_storedData = NULL;
	m_bodyListData.clear();
}
CBodySpliter::~CBodySpliter()
{
}

void CBodySpliter::StoreData(char* data)
{
	m_storedData = new char[strlen(data) + 1];
	strcpy_s(m_storedData, strlen(data) + 1, data);
}

void CBodySpliter::FillBodyListByParsing()
{
	char* sPoint = NULL; // 파싱하려는 문자열의 첫 위치 주소
	char* tmpPoint = NULL;
	int sIndex = 0;		// 파싱하려는 문자열의 첫 위치 인덱스
	int eIndex = 0;		// 파싱하려는 문자열의 마지막 인덱스의 뒤.
	int totalLength = 0;		// 원본 데이터의 길이
	int strLength = 0;
	bool bracketFlag = false;	// <, > 안이면 true, 밖이면 false.
	
	totalLength = strlen(m_storedData);
	
	while (eIndex <= totalLength)
	{
		if ((bracketFlag == false) && (m_storedData[eIndex] == '<')) // 일반적인 body였다가 <tag>를 만남
		{
			bracketFlag = true;
			if (eIndex != sIndex)
			{
				sPoint = &m_storedData[sIndex]; // 파싱하려는 문자열의 가장 첫번째 위치로 포인터 위치시킴.
				strLength = eIndex  - sIndex;		// 파싱하려는 문자열의 길이(끝 - 처음)
				tmpPoint = new char[strLength + 1];	// 파싱하려는 문자열 길이만큼 메모리를 할당해서
				strncpy_s(tmpPoint, strLength + 1, sPoint, strLength);	// 그 길이만큼 복사
				m_bodyListData.push_back(tmpPoint);

				tmpPoint = NULL;
				sIndex = eIndex;
			}
		}
		else if ((bracketFlag == true) && (m_storedData[eIndex] == '>'))	// <tag>에서 빠져나갈때.
		{
			bracketFlag = false;
			if (eIndex != sIndex)
			{
				sPoint = &m_storedData[sIndex]; // 파싱하려는 문자열의 가장 첫번째 위치로 포인터 위치시킴.
				strLength = eIndex - sIndex + 1;		// 파싱하려는 문자열의 길이(끝 - 처음)
				tmpPoint = new char[strLength + 1];	// 파싱하려는 문자열 길이만큼 메모리를 할당해서
				strncpy_s(tmpPoint, strLength + 1, sPoint, strLength);	// 그 길이만큼 복사
				m_bodyListData.push_back(tmpPoint);

				tmpPoint = NULL;
				eIndex++;
				sIndex = eIndex;
			}
		}
		else
		{
			eIndex++;
		}
	}
	if (sIndex != eIndex)
	{
		sPoint = &m_storedData[sIndex]; // 파싱하려는 문자열의 가장 첫번째 위치로 포인터 위치시킴.
		strLength = eIndex - sIndex + 1;		// 파싱하려는 문자열의 길이(끝 - 처음)
		tmpPoint = new char[strLength + 1];	// 파싱하려는 문자열 길이만큼 메모리를 할당해서
		strncpy_s(tmpPoint, strLength + 1, sPoint, strLength);	// 그 길이만큼 복사
		m_bodyListData.push_back(tmpPoint);
	}
}

bool CBodySpliter::IsBodyNull()
{
	if (m_storedData == NULL)
	{
		return true;
	}

	return false;
}

char* CBodySpliter::GetBodyString()
{
	return m_storedData;
}

std::list<char*>* CBodySpliter::GetBodyList()
{
	return &m_bodyListData;
}

int CBodySpliter::GetBodyLength()
{
	return strlen(m_storedData);
}

void CBodySpliter::PrintData()
{
	int index = 0;

	//printf("storedData : %s\n\n\n", m_storedData);
	
	std::list<char*>::iterator iter;
	for (iter = m_bodyListData.begin(); iter != m_bodyListData.end(); iter++)
	{
		printf("[%d] : %s\n", index, (*iter));
		index++;
	}
}

void CBodySpliter::Cleanup()
{
	std::list<char*>::iterator iter;
	if(m_bodyListData.empty() == false)
	{
		m_bodyListData.clear();
		for (iter = m_bodyListData.begin(); iter != m_bodyListData.end(); iter++)
		{
			delete[](*iter);
		}
		
	}
	
}