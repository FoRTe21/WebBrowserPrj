#pragma once
#include "CWebSocket.h"
#include "CBodySpliter.h"

#include <Windows.h>
#include <GdiPlus.h>
#include <CommCtrl.h>
#include <Richedit.h>
#include <list>

#define FONTSNUM 2

typedef struct gHyperText
{
	HWND _hRichEdit;
	char* _text;
	HFONT _hFont;
} gHyperText;

enum eOrder {HYPERTEXTN, IMAGEN, BRN, CONTROLN};

using namespace Gdiplus; 

class CGraphicProcess
{
private:
	ULONG_PTR m_gpToken;						// GDI+ lib �ʱ�ȭ�� ���� ����
	GdiplusStartupInput m_gpsi;

	
												// TEXTN : �ؽ�Ʈ, IMAGEN : �̹���, BRN : <br> �±�, CONTROLN : control
	std::list<eOrder> m_order;
	std::list<Image*> m_images;					// image ������
	std::list<HWND> m_controls;					// control handles.
	std::list<gHyperText> m_hyperTexts;				// hyper text(rich edit) handles

	HFONT m_fontD[FONTSNUM];					// fonts.

	PointF m_sPoint;							// ��� �ʱ� ��ġ
	bool m_firstDrawing;
	
	CHARFORMAT2 m_hyperTextFormat[3];			// ���콺�� ���� ���� ��, ���� ���� font.

	int m_xScrollMax;
	int m_yScrollMax;

private:
	void InitHyperTextFonts();

public:
	CGraphicProcess();
	~CGraphicProcess();
	bool InitGraphics();						// GDI+ lib �ʱ�ȭ
	void ShutDownGdi();							// GDI+ lib ����

	void Cleanup();								// ��� �����͵� ����

	void MakeFonts();							// font ���
	void SetOrder(eOrder eOrder);
	bool IsOrderEmpty();						// order empty ����
	void RegisterImages(char* filename);		// Image ������ ���
	void RegisterControls(HWND hWnd);			// Control handle ������ ���
	void RegisterHyperTexts(HWND hRichEdit, char* text, HFONT hFont);			// Hyper text(rich edit) ������ ���

	void DrawImages(HDC hdc, RECT rt);			// order���� ���缭 ���� �����͵� ���
	HFONT GetHFont(int order);					// m_fontD ��� ������ ����ִ� font ������ ������
	void SetsPoint(bool init, float x, float y);// ����� �������� �ʱ� ��ġ ����.

	CHARFORMAT2 GetHyperTextFonts(int state);
	
	int GetXScrollMax();
	int GetYScrollMax();

	void SetFirstFlag(bool flag);
	int GetFirstFlag();
};
