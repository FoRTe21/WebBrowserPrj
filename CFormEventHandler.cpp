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

	if (m_formFlag == true)							// 이미 <form> 안에 들어와서 
	{												// 하나의 form struct가 생성되어 
		return false;								// 끝나지 않았으면 진행하지 않음
	}

	p = strstr(formTag, "action=");					// action 데이터 저장
	if (p == NULL)
	{
		return false;
	}

	p = p + strlen("action=") + 1;
	for (index = 0; p[index] != '\"'; index++);
	strncpy_s(cf._action, index + 1, p, index);

	p = NULL;

	p = strstr(formTag, "method=");					// method 데이터 저장
	if (p == NULL)
	{
		return false;
	}

	p = p + strlen("method=") + 1;
	for (index = 0; p[index] != '\"'; index++);
	strncpy_s(cf._method, index + 1, p, index);
	
	m_forms.push_back(cf);							// form struct 데이터 저장
	m_formFlag = true;

	return true;
}

HWND CFormEventHandler::RegisterFormControls(char* inputTag, HWND hParent, HINSTANCE hInstance)
{
	char typeBuf[BUFSIZE] = { 0, };
	char nameBuf[BUFSIZE] = { 0, };
	char* nameP = NULL;
	HWND hControl;

	std::list<customForm>::reference cf(m_forms.back());						// <form> 안에 form이 들어오지 않도록 했기 때문에 제일 마지막
	if (GetControlInfo(inputTag, typeBuf, nameBuf) == false)					// 데이터가 현재 fill 진행중인 form struct.
	{
		return (HWND)-1;
	}

	if (strcmp(typeBuf, "text") == 0)											// text라면 edit control 등록
	{
		nameP = new char[strlen(nameBuf) + 1];									// edit control을 등록함과 동시에 관련 변수명을 같은 순서로 등록
		strcpy_s(nameP, strlen(nameBuf) + 1, nameBuf);
		cf._names.push_back(nameP);

		hControl = CreateWindow("edit", NULL, WS_CHILD | WS_BORDER, 0, 0, 100, 20, hParent, (HMENU)m_numControl, hInstance, NULL);
		cf._hEdits.push_back(hControl);
		
		m_numControl++;
	}
	else if (strcmp(typeBuf, "submit") == 0)									// submit이면 button control 등록
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

	return hControl;															//등록을 마친 control의 handle 반환
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

bool CFormEventHandler::MakePostBody(customForm iter)					// form struct 내의 name과 edit을 순차적으로 접근하면서 body 데이터를 채워감
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
			sprintf_s(m_bodyData, length + 1, "%s=%s", *iterName, wndText);				// 형태는 name=value
			continue;
		}

		tmpBuf = m_bodyData;
		GetWindowText(*iterHwnd, wndText, BUFSIZE);
		length += strlen(*iterName) + strlen("=&") + strlen(wndText);
		m_bodyData = new char[length + 1];
		sprintf_s(m_bodyData, length + 1, "%s&%s=%s", tmpBuf, *iterName, wndText);		// 데이터와 데이터 사이는 &로 구분 ex) name1=value1&name2=value2

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

	for (iter = m_forms.begin(); iter != m_forms.end(); iter++)				// submit button이 속해있는 form struct를 찾기 위함.
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
	else if(strcmp(iter->_method, "GET") == 0)							// GET은 미구현
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