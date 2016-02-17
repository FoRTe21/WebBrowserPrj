#pragma once

#include "CWebSocket.h"
#include "CHtmlParser.h"
#include "CBodySpliter.h"
#include "CGraphicProcess.h"
#include "CFormEventHandler.h"
#include <Windows.h>

#define ID_URI 100						// URI �Է¿� Edit control ID
#define URILENGTH 1024					// �ӽ� URI buffer ũ��
#define SCROLLVALUE 10					// ��ũ�� �� �̵� unit size

class CMainSystem
{
private:
	char m_uriStr[URILENGTH];					// URI �Է� ����
	HWND m_hEdit;								// URI �Է¿� Edit handle
	HWND m_hWnd;							// main window handle
	HINSTANCE m_hInst;						// main process handle
	
	
	CWebSocket ws;
	CHtmlParser hp;
	CBodySpliter bp;
	CGraphicProcess ip;
	CFormEventHandler eventHandler;

	MSG m_Message;
	WNDPROC m_OldEditProc;					// �ӽ� Procedure �Լ� ���� ����

	bool m_drawFlag;							// OK �� ȭ�鿡 �׷��ִ� flag
	bool m_notFoundDrawFlag;					// Not Found ��
	bool m_badRequestDrawFlag;				// Bad Request ��

private:
	LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);								// main window�� procedure
	LRESULT CALLBACK EditSubProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam);	// edit control�� procedure

	CMainSystem();
	~CMainSystem();
	void InitWindow(HINSTANCE hInstance, int nCmdShow);			// window ����
	void RequestProc();											// ������ request. (main function)
	void RequestImageURI(char* iter);							// Image�� request.
	void SetTitle();											// window�� title ����
	void MakeImageUri(char* data);								// Image request�� �ϱ� ���� URI ���� ä���
	void MakeUri(char* uri);									// �������� URI ���� ä���
	void OkProc();												// response�� OK�� �� procedure
	void ScrollEvent(HWND hWnd, WPARAM wParam);					// scroll event ó��
	void FillObjectData(std::list<char*>* data);				// <tag>�� ���� ������ ó�� procedure
																// Graphic ���, Form handler ���, text ��� ���
	void SubmitProc(WPARAM wParam);								// Form �±��� Submit �� ��(button down)�� ó��

public:
	bool InitProcess(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow);
	bool RunProcess();
	bool EndProcess();
};