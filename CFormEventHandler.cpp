#include "CFormEventHandler.h"

CFormEventHandler::CFormEventHandler()
{
	m_formFlag = false;
	m_numControl = 0;
	m_bodyData = NULL;
	m_queryString = NULL;
	m_action = NULL;
}

CFormEventHandler::~CFormEventHandler()
{

}

void CFormEventHandler::Cleanup()
{
	m_formFlag = false;
	m_numControl = 0;

	if(m_submitIds.empty() == false)
	{
		m_submitIds.clear();
	}
	if (m_forms.empty() == false)
	{
		for (std::list<customForm>::iterator iter = m_forms.begin(); iter != m_forms.end(); iter++)
		{
			if (iter->_hEdits.empty() == false)
			{
				for (std::list<char*>::iterator iterNames = iter->_names.begin(); iterNames != iter->_names.end(); iterNames++)
				{
					delete[](*iterNames);
				}
				for (std::list<HWND>::iterator iterHandle = iter->_hEdits.begin(); iterHandle != iter->_hEdits.end(); iterHandle++)
				{
					DestroyWindow(*iterHandle);
					CloseHandle(*iterHandle);
				}

				iter->_names.clear();
				iter->_hEdits.clear();
			}

			CloseHandle(iter->_submitButton);
		}
		m_forms.clear();
	}

	if(m_bodyData != NULL)
	{
		delete[] m_bodyData;
		m_bodyData = NULL;
	}
	if(m_queryString != NULL)
	{
		delete[] m_queryString;
		m_queryString = NULL;
	}
	if(m_action != NULL)
	{
		delete[] m_action;
		m_action = NULL;
	}
}

void CFormEventHandler::SetFormFlag(bool flag)
{
	m_formFlag = flag;
}

bool CFormEventHandler::MakeEventForm(char* formTag)
{
	char* p = NULL;
	int index = 0;

	customForm cf;

	if (m_formFlag == true)							// �̹� <form> �ȿ� ���ͼ� 
	{												// �ϳ��� form struct�� �����Ǿ� 
		return false;								// ������ �ʾ����� �������� ����
	}

	p = strstr(formTag, "action=");					// action ������ ����
	if (p == NULL)
	{
		return false;
	}

	p = p + strlen("action=") + 1;
	for (index = 0; p[index] != '\"'; index++);
	strncpy_s(cf._action, index + 1, p, index);

	p = NULL;

	p = strstr(formTag, "method=");					// method ������ ����
	if (p == NULL)
	{
		return false;
	}

	p = p + strlen("method=") + 1;
	for (index = 0; p[index] != '\"'; index++);
	strncpy_s(cf._method, index + 1, p, index);
	
	m_forms.push_back(cf);							// form struct ������ ����
	m_formFlag = true;

	return true;
}

HWND CFormEventHandler::RegisterFormControls(char* inputTag, HWND hParent, HINSTANCE hInstance)
{
	char typeBuf[BUFSIZE] = { 0, };
	char nameBuf[BUFSIZE] = { 0, };
	char* nameP = NULL;
	HWND hControl;

	std::list<customForm>::reference cf(m_forms.back());						// <form> �ȿ� form�� ������ �ʵ��� �߱� ������ ���� ������
	if (GetControlInfo(inputTag, typeBuf, nameBuf) == false)					// �����Ͱ� ���� fill �������� form struct.
	{
		return (HWND)-1;
	}

	if (strcmp(typeBuf, "text") == 0)											// text��� edit control ���
	{
		nameP = new char[strlen(nameBuf) + 1];									// edit control�� ����԰� ���ÿ� ���� �������� ���� ������ ���
		strcpy_s(nameP, strlen(nameBuf) + 1, nameBuf);
		cf._names.push_back(nameP);

		hControl = CreateWindow("edit", NULL, WS_CHILD | WS_BORDER, 0, 0, 100, 20, hParent, (HMENU)m_numControl, hInstance, NULL);
		cf._hEdits.push_back(hControl);
		
		m_numControl++;
	}
	else if (strcmp(typeBuf, "submit") == 0)									// submit�̸� button control ���
	{
		hControl = CreateWindow("button", nameBuf, WS_CHILD | BS_PUSHBUTTON, 0, 0, 50, 50, hParent, (HMENU)m_numControl, hInstance, NULL);
		cf._submitId = m_numControl;
		cf._submitButton = hControl;
		
		m_submitIds.push_back(m_numControl);
		
		m_numControl++;
		
	}
	else
	{
		return (HWND)-1;
	}

	return hControl;															//����� ��ģ control�� handle ��ȯ
}

bool CFormEventHandler::GetControlInfo(char* inputTag, char* typeBuf, char* nameBuf)
{
	char* p = NULL;
	char* tmpP = NULL;
	
	int index = 0;

	p = strstr(inputTag, "type=");
	if (p == NULL)
	{
		return false;
	}

	p = p + strlen("type=");
	for (index = 0; p[index] != ' '; index++);

	strncpy_s(typeBuf, index + 1, p, index);

	if (strlen(typeBuf) <= 0)
	{
		return false;
	}

	tmpP = p;
	p = strstr(p, "name=");
	if (p == NULL)
	{
		p = tmpP;
		p = strstr(p, "value=");
		if (p == NULL)
		{
			return false;
		}
		else
		{
			p = p + strlen("value=") + 1;
		}
	}
	else
	{
		p = p + strlen("name=") + 1;
	}

	for (index = 0; p[index] != '\"'; index++);
	strncpy_s(nameBuf, index + 1, p, index);

	if (strlen(nameBuf) <= 0)
	{
		return false;
	}
	return true;
}

int CFormEventHandler::GetNumForms()
{
	return m_numControl;
}

bool CFormEventHandler::MakePostBody(customForm iter)					// form struct ���� name�� edit�� ���������� �����ϸ鼭 body �����͸� ä����
{
	
	std::list<HWND>::iterator iterHwnd;
	std::list<char*>::iterator iterName;

	
	char* tmpBuf = NULL;
	char wndText[BUFSIZE] = {0,};
	int length = 0;

	for(iterHwnd = iter._hEdits.begin(), iterName = iter._names.begin(); iterHwnd != iter._hEdits.end(); iterHwnd++, iterName++)
	{
		memset(wndText, 0, sizeof(wndText));
		if(m_bodyData == NULL)
		{
			GetWindowText(*iterHwnd, wndText, BUFSIZE);
			length = strlen(*iterName) + strlen("=") + strlen(wndText);
			m_bodyData = new char[length + 1];
			sprintf_s(m_bodyData, length + 1, "%s=%s", *iterName, wndText);				// ���´� name=value
			continue;
		}

		tmpBuf = m_bodyData;
		GetWindowText(*iterHwnd, wndText, BUFSIZE);
		length += strlen(*iterName) + strlen("=&") + strlen(wndText);
		m_bodyData = new char[length + 1];
		sprintf_s(m_bodyData, length + 1, "%s&%s=%s", tmpBuf, *iterName, wndText);		// �����Ϳ� ������ ���̴� &�� ���� ex) name1=value1&name2=value2

		delete[] tmpBuf;
		tmpBuf = NULL;
	}

	return true;
}

char* CFormEventHandler::GetBodyData()
{
	if(m_bodyData != NULL)
	{
		return m_bodyData;
	}
	else
	{
		return NULL;
	}
}

int CFormEventHandler::EventProc(WPARAM wParam)
{
	std::list<customForm>::iterator iter;
	bool flag = false;

	for (iter = m_forms.begin(); iter != m_forms.end(); iter++)				// submit button�� �����ִ� form struct�� ã�� ����.
	{
		if (iter->_submitId == LOWORD(wParam))
		{
			flag = true;
			break;
		}
	}

	if (flag == false)
	{
		return -1;
	}

	if(m_action != NULL)
	{
		delete[] m_action;
		m_action = NULL;
	}
	m_action = new char[strlen(iter->_action) + 1];
	strcpy_s(m_action, strlen(iter->_action) + 1, iter->_action);

	if(strcmp(iter->_method, "POST") == 0)
	{
		MakePostBody(*iter);
		return 1;
	}
	else if(strcmp(iter->_method, "GET") == 0)							// GET�� �̱���
	{
		return 0;
	}
	else
	{
		return -1;
	}
}

char* CFormEventHandler::GetAction()
{
	return m_action;
}

bool CFormEventHandler::IsActionPushed(WPARAM wParam)
{
	std::list<int>::iterator iter;
	for(iter = m_submitIds.begin(); iter != m_submitIds.end(); iter++)
	{
		if(LOWORD(wParam) == *iter)
		{
			return true;
		}
	}
	
	return false;
}

int CFormEventHandler::GetBodyLength()
{
	return strlen(m_bodyData);
}