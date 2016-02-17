#pragma once

#include "CWebSocket.h"
#include "CHtmlParser.h"
#include "CBodySpliter.h"
#include "CGraphicProcess.h"
#include "CFormEventHandler.h"
#include <Windows.h>

#define ID_URI 100						// URI 입력용 Edit control ID
#define URILENGTH 1024					// 임시 URI buffer 크기
#define SCROLLVALUE 10					// 스크롤 시 이동 unit size

class CMainSystem
{
private:
	char m_uriStr[URILENGTH];					// URI 입력 버퍼
	HWND m_hEdit;								// URI 입력용 Edit handle
	HWND m_hWnd;							// main window handle
	HINSTANCE m_hInst;						// main process handle
	
	
	CWebSocket ws;
	CHtmlParser hp;
	CBodySpliter bp;
	CGraphicProcess ip;
	CFormEventHandler eventHandler;

	MSG m_Message;
	WNDPROC m_OldEditProc;					// 임시 Procedure 함수 저장 변수

	bool m_drawFlag;							// OK 시 화면에 그려주는 flag
	bool m_notFoundDrawFlag;					// Not Found 시
	bool m_badRequestDrawFlag;				// Bad Request 시

private:
	LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);								// main window의 procedure
	LRESULT CALLBACK EditSubProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam);	// edit control의 procedure

	CMainSystem();
	~CMainSystem();
	void InitWindow(HINSTANCE hInstance, int nCmdShow);			// window 생성
	void RequestProc();											// 서버에 request. (main function)
	void RequestImageURI(char* iter);							// Image를 request.
	void SetTitle();											// window의 title 설정
	void MakeImageUri(char* data);								// Image request를 하기 위한 URI 버퍼 채우기
	void MakeUri(char* uri);									// 범용적인 URI 버퍼 채우기
	void OkProc();												// response가 OK일 때 procedure
	void ScrollEvent(HWND hWnd, WPARAM wParam);					// scroll event 처리
	void FillObjectData(std::list<char*>* data);				// <tag>에 따른 데이터 처리 procedure
																// Graphic 등록, Form handler 등록, text 등록 등등
	void SubmitProc(WPARAM wParam);								// Form 태그의 Submit 할 시(button down)의 처리

public:
	bool InitProcess(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow);
	bool RunProcess();
	bool EndProcess();
};