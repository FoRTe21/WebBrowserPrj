#include "CHyperTextHandler.h"

CHyperTextHandler::CHyperTextHandler()
{
	m_tempUri = NULL;
}

CHyperTextHandler::~CHyperTextHandler()
{

}

bool CHyperTextHandler::InitHTHandler(HWND hParentWnd, HINSTANCE hInst)
{
	if ((m_hMod = LoadLibrary("Riched20.dll")) == (HMODULE)-1)
	{
		return false;
	}

	m_hParentWnd = hParentWnd;
	m_hInst = hInst;
	return true;
}

void CHyperTextHandler::EndModule()
{
	FreeLibrary(m_hMod);
}

void CHyperTextHandler::Cleanup()
{
	std::list<hyperText>::iterator htIter;
	if (m_hyperTexts.empty() == false)
	{
		for (htIter = m_hyperTexts.begin(); htIter != m_hyperTexts.end(); htIter++)
		{
			delete[] htIter->_linkedStr;
			delete[] htIter->_linkedUri;
			//CloseHandle(htIter->_hRichEdit);
			DestroyWindow(htIter->_hRichEdit);
			
		}

		m_hyperTexts.clear();
	}

	if (m_tempUri != NULL)
	{
		delete[] m_tempUri;
		m_tempUri = NULL;
	}
}

void CHyperTextHandler::SetATagFlag(bool flag)
{
	m_aTagFlag = flag;
}

bool CHyperTextHandler::GetATagFlag()
{
	return m_aTagFlag;
}

bool CHyperTextHandler::SaveTempUri(char* aTag)
{
	char* p = NULL;
	char* ep = NULL;
	int index = 0;

	p = strstr(aTag, "href=");
	if (p == NULL)
	{
		return false;
	}

	p = p + strlen("href=") + 1;

	ep = strstr(p, "\"");

	if (ep == NULL)
	{
		return false;
	}

	for (index = 0; p[index] != ep[0]; index++);

	m_tempUri = new char[index + 1];
	strncpy_s(m_tempUri, index + 1, p, index);

	return true;
}

hyperText* CHyperTextHandler::MakeRichEdit(char* text, HFONT hFont, bool isHyperText)
{
	hyperText* tmpHt;
	hyperText forPushStruct;
	char tmpBuf[BUFSIZ] = { 0, };
	HWND hRichEdit;
	char* refinedText = NULL;
	
	HDC hdc;
	SIZE sz;
	RECT rc;
	HFONT hOldFont;

	refinedText = ConvertToOneLine(text);
	tmpHt = new hyperText;

	hdc = GetDC(m_hParentWnd);
	hOldFont = (HFONT)SelectObject(hdc, hFont);
	memset(&rc, 0, sizeof(RECT));
	hRichEdit = CreateWindow(RICHEDIT_CLASS, refinedText, WS_CHILD | ES_READONLY , 0, 0, 0, 0, m_hParentWnd, (HMENU)m_numHT, m_hInst, NULL); // rich edit 생성
	GetWindowText(hRichEdit, refinedText, strlen(refinedText) + 1);
	DrawText(hdc, refinedText, strlen(refinedText), &rc, DT_CALCRECT | DT_EDITCONTROL | DT_SINGLELINE | DT_CENTER); 	// text를 찍기 위해서가 아니라 text의 범위(rect)를 얻어오기 위함.

	SetWindowPos(hRichEdit, 0, 0, 0, rc.right + 5, rc.bottom, SWP_NOZORDER);				// 실제 rich edit 위치 설정해주는 부분
	SendMessage(hRichEdit, WM_SETFONT, (WPARAM)hFont,TRUE);

	SelectObject(hdc, hOldFont);
	tmpHt->_hFont = hFont;
	tmpHt->_linkedUri = m_tempUri;	
	m_tempUri = NULL;

	tmpHt->_linkedStr = new char[strlen(text) + 1];
	strcpy_s(tmpHt->_linkedStr, strlen(text) + 1, text);
	tmpHt->_hRichEdit = hRichEdit;

	forPushStruct._hRichEdit = hRichEdit;
	forPushStruct._hFont = hFont;
	forPushStruct._linkedStr = tmpHt->_linkedStr;
	forPushStruct._linkedUri = tmpHt->_linkedUri;
	if (isHyperText == true)							// link가 들어있는 hyper text인 경우 등록.
	{													// 단순 텍스트의 경우는 저장안함.
		m_hyperTexts.push_back(forPushStruct);
	}

	delete[] refinedText;
	return tmpHt;

}

char* CHyperTextHandler::ConvertToOneLine(char* text)
{
	char* result = NULL;
	char* tmp = NULL;

	int i = 0;
	int j = 0;
	tmp = new char[strlen(text) + 1];
	
	for (i = 0; i < strlen(text); i++)
	{
		if (text[i] == '\n' || text[i] == '\r' || text[i] == '\t')
		{
			continue;
		}
		else
		{
			tmp[j] = text[i];
			j++;
		}
	}

	tmp[j] = '\0';
	result = new char[strlen(tmp) + 1];
	strcpy_s(result, strlen(tmp) + 1, tmp);

	delete[] tmp;

	return result;
}

char* CHyperTextHandler::GetLinkedUri(HWND hWnd)
{
	std::list<hyperText>::iterator iter;

	for (iter = m_hyperTexts.begin(); iter != m_hyperTexts.end(); iter++)
	{
		if (iter->_hRichEdit == hWnd)
		{
			return iter->_linkedUri;
		}
	}

	return NULL;
}