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
	int _length;	// text 길이
}customText;

class CGraphicProcess
{
private:
	ULONG_PTR m_gpToken;						// GDI+ lib 초기화를 위한 변수
	GdiplusStartupInput m_gpsi;

	std::list<int> m_order;						// 출력할 데이터들의 순서.
												// TEXTN : 텍스트, IMAGEN : 이미지, BRN : <br> 태그, CONTROLN : control
	std::list<Image*> m_images;					// image 데이터
	std::list<customText> m_texts;				// text 데이터
	std::list<HWND> m_controls;					// control handles.

	HFONT m_fontD[FONTSNUM];					// fonts.

	PointF m_sPoint;							// 출력 초기 위치

public:
	CGraphicProcess();
	~CGraphicProcess();
	bool InitGraphics();						// GDI+ lib 초기화
	void ShutDownGdi();							// GDI+ lib 정리

	void Cleanup();								// 출력 데이터들 정리

	void MakeFonts();							// font 등록
	void SetOrder(int order);					// 출력 데이터들의 order 저장
	bool IsOrderEmpty();						// order empty 여부
	void RegisterTexts(char* texts, HFONT font);// Text 데이터 등록
	void RegisterImages(char* filename);		// Image 데이터 등록
	void RegisterControls(HWND hWnd);			// Control handle 데이터 등록
	
	void DrawImages(HDC hdc, RECT rt);			// order순에 맞춰서 각종 데이터들 출력
	HFONT GetHfont(int order);					// m_fontD 멤버 변수에 들어있는 font 데이터 가져옴

	void SetsPoint(bool init, float x, float y);// 출력할 데이터의 초기 위치 지정.
};
