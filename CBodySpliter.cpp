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
	char* sPoint = NULL; // �Ľ��Ϸ��� ���ڿ��� ù ��ġ �ּ�
	char* tmpPoint = NULL;
	int sIndex = 0;		// �Ľ��Ϸ��� ���ڿ��� ù ��ġ �ε���
	int eIndex = 0;		// �Ľ��Ϸ��� ���ڿ��� ������ �ε����� ��.
	int totalLength = 0;		// ���� �������� ����
	int strLength = 0;
	bool bracketFlag = false;	// <, > ���̸� true, ���̸� false.
	
	totalLength = strlen(m_storedData);
	
	while (eIndex <= totalLength)
	{
		if ((bracketFlag == false) && (m_storedData[eIndex] == '<')) // �Ϲ����� body���ٰ� <tag>�� ����
		{
			bracketFlag = true;
			if (eIndex != sIndex)
			{
				sPoint = &m_storedData[sIndex]; // �Ľ��Ϸ��� ���ڿ��� ���� ù��° ��ġ�� ������ ��ġ��Ŵ.
				strLength = eIndex  - sIndex;		// �Ľ��Ϸ��� ���ڿ��� ����(�� - ó��)
				tmpPoint = new char[strLength + 1];	// �Ľ��Ϸ��� ���ڿ� ���̸�ŭ �޸𸮸� �Ҵ��ؼ�
				strncpy_s(tmpPoint, strLength + 1, sPoint, strLength);	// �� ���̸�ŭ ����
				m_bodyListData.push_back(tmpPoint);

				tmpPoint = NULL;
				sIndex = eIndex;
			}
		}
		else if ((bracketFlag == true) && (m_storedData[eIndex] == '>'))	// <tag>���� ����������.
		{
			bracketFlag = false;
			if (eIndex != sIndex)
			{
				sPoint = &m_storedData[sIndex]; // �Ľ��Ϸ��� ���ڿ��� ���� ù��° ��ġ�� ������ ��ġ��Ŵ.
				strLength = eIndex - sIndex + 1;		// �Ľ��Ϸ��� ���ڿ��� ����(�� - ó��)
				tmpPoint = new char[strLength + 1];	// �Ľ��Ϸ��� ���ڿ� ���̸�ŭ �޸𸮸� �Ҵ��ؼ�
				strncpy_s(tmpPoint, strLength + 1, sPoint, strLength);	// �� ���̸�ŭ ����
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
		sPoint = &m_storedData[sIndex]; // �Ľ��Ϸ��� ���ڿ��� ���� ù��° ��ġ�� ������ ��ġ��Ŵ.
		strLength = eIndex - sIndex + 1;		// �Ľ��Ϸ��� ���ڿ��� ����(�� - ó��)
		tmpPoint = new char[strLength + 1];	// �Ľ��Ϸ��� ���ڿ� ���̸�ŭ �޸𸮸� �Ҵ��ؼ�
		strncpy_s(tmpPoint, strLength + 1, sPoint, strLength);	// �� ���̸�ŭ ����
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