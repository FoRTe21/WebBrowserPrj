#pragma once

#include "CGraphicProcess.h"
#include <Windows.h>
#include <Richedit.h>
#include <list>

typedef struct hyperText
{
	HWND _hRichEdit;
	char* _linkedUri;
	char* _linkedStr;
	HFONT _hFont;
} hyperText;

class CHyperTextHandler
{
private:
	HMODULE m_hMod;
	HWND m_hParentWnd;
	HINSTANCE m_hInst;
	std::list<hyperText> m_hyperTexts;
	int m_numHT;							// HyperText °³¼ö

	bool m_aTagFlag;
	char* m_tempUri;

private:
	char* ConvertToOneLine(char* text);

public:
	CHyperTextHandler();
	~CHyperTextHandler();

	bool InitHTHandler(HWND hParentWnd, HINSTANCE hInst);
	void Cleanup();
	void EndModule();

	void SetATagFlag(bool flag);
	bool GetATagFlag();

	bool SaveTempUri(char* aTag);
	hyperText* MakeRichEdit(char* text, HFONT hFont, bool isHyperText);

	char* GetLinkedUri(HWND hWnd);
};