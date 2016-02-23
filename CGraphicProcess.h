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
	ULONG_PTR m_gpToken;						// GDI+ lib 초기화를 위한 변수
	GdiplusStartupInput m_gpsi;

	
												// TEXTN : 텍스트, IMAGEN : 이미지, BRN : <br> 태그, CONTROLN : control
	std::list<eOrder> m_order;
	std::list<Image*> m_images;					// image 데이터
	std::list<HWND> m_controls;					// control handles.
	std::list<gHyperText> m_hyperTexts;				// hyper text(rich edit) handles

	HFONT m_fontD[FONTSNUM];					// fonts.

	PointF m_sPoint;							// 출력 초기 위치
	bool m_firstDrawing;
	
	CHARFORMAT2 m_hyperTextFormat[3];			// 마우스가 위에 있을 때, 없을 때의 font.

	int m_xScrollMax;
	int m_yScrollMax;

private:
	void InitHyperTextFonts();

public:
	CGraphicProcess();
	~CGraphicProcess();
	bool InitGraphics();						// GDI+ lib 초기화
	void ShutDownGdi();							// GDI+ lib 정리

	void Cleanup();								// 출력 데이터들 정리

	void MakeFonts();							// font 등록
	void SetOrder(eOrder eOrder);
	bool IsOrderEmpty();						// order empty 여부
	void RegisterImages(char* filename);		// Image 데이터 등록
	void RegisterControls(HWND hWnd);			// Control handle 데이터 등록
	void RegisterHyperTexts(HWND hRichEdit, char* text, HFONT hFont);			// Hyper text(rich edit) 데이터 등록

	void DrawImages(HDC hdc, RECT rt);			// order순에 맞춰서 각종 데이터들 출력
	HFONT GetHFont(int order);					// m_fontD 멤버 변수에 들어있는 font 데이터 가져옴
	void SetsPoint(bool init, float x, float y);// 출력할 데이터의 초기 위치 지정.

	CHARFORMAT2 GetHyperTextFonts(int state);
	
	int GetXScrollMax();
	int GetYScrollMax();

	void SetFirstFlag(bool flag);
	int GetFirstFlag();
};
