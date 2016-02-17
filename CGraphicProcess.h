#pragma once
#include "CWebSocket.h"
#include "CBodySpliter.h"

#include <Windows.h>
#include <GdiPlus.h>
#include <CommCtrl.h>
#include <list>

#define TEXTN 0
#define IMAGEN 1
#define BRN 2
#define CONTROLN 3

#define FONTSNUM 2


using namespace Gdiplus; 

typedef struct customText
{
	WCHAR* _text;
	HFONT _font;
	int _length;	// text ����
}customText;

class CGraphicProcess
{
private:
	ULONG_PTR m_gpToken;						// GDI+ lib �ʱ�ȭ�� ���� ����
	GdiplusStartupInput m_gpsi;

	std::list<int> m_order;						// ����� �����͵��� ����.
												// TEXTN : �ؽ�Ʈ, IMAGEN : �̹���, BRN : <br> �±�, CONTROLN : control
	std::list<Image*> m_images;					// image ������
	std::list<customText> m_texts;				// text ������
	std::list<HWND> m_controls;					// control handles.

	HFONT m_fontD[FONTSNUM];					// fonts.

	PointF m_sPoint;							// ��� �ʱ� ��ġ

public:
	CGraphicProcess();
	~CGraphicProcess();
	bool InitGraphics();						// GDI+ lib �ʱ�ȭ
	void ShutDownGdi();							// GDI+ lib ����

	void Cleanup();								// ��� �����͵� ����

	void MakeFonts();							// font ���
	void SetOrder(int order);					// ��� �����͵��� order ����
	bool IsOrderEmpty();						// order empty ����
	void RegisterTexts(char* texts, HFONT font);// Text ������ ���
	void RegisterImages(char* filename);		// Image ������ ���
	void RegisterControls(HWND hWnd);			// Control handle ������ ���
	
	void DrawImages(HDC hdc, RECT rt);			// order���� ���缭 ���� �����͵� ���
	HFONT GetHfont(int order);					// m_fontD ��� ������ ����ִ� font ������ ������

	void SetsPoint(bool init, float x, float y);// ����� �������� �ʱ� ��ġ ����.
};
