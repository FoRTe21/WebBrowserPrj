#pragma once

#include "CWebSocket.h"
#include "CHtmlParser.h"
#include "CBodySpliter.h"
#include "CGraphicProcess.h"
#include "CFormEventHandler.h"
#include "CHyperTextHandler.h"

#include <Windows.h>

#define ID_URI 100						// URI �Է¿� Edit control ID
#define ID_STATIC -1
#define URILENGTH 1024					// �ӽ� URI buffer ũ��
#define SCROLLVALUE 10					// ��ũ�� �� �̵� unit size
// ���� ����:

// custom variable

char uriStr[URILENGTH];					// URI �Է� ����
HWND hEdit;								// URI �Է¿� Edit handle
HWND hUriStatic;
HWND g_hWnd;							// main window handle
HWND g_hClientWnd;
HINSTANCE g_hInst;						// main process handle
WNDPROC OldEditProc;					// �ӽ� Procedure �Լ� ���� ����
WNDPROC RichEditOldProc;				// rich edit�� ���� proc
CWebSocket socketProcessor;
CHtmlParser htmlParser;
CBodySpliter bodySpliter;
CGraphicProcess renderer;
CFormEventHandler eventHandler;
CHyperTextHandler htHandler;

bool drawFlag;							// OK �� ȭ�鿡 �׷��ִ� flag
bool notFoundDrawFlag;					// Not Found ��
bool badRequestDrawFlag;				// Bad Request ��
// ================

// �� �ڵ� ��⿡ ��� �ִ� �Լ��� ������ �����Դϴ�.
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);								// main window�� procedure
LRESULT CALLBACK EditSubProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam);	// edit control�� procedure

// custom function
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

void OnHyperText(HWND hWnd);
void LeavingHyperText(HWND hWnd);
void TrackMouse(HWND hwnd);

void HyperLinkEvent(HWND hWnd);
// ================

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	MSG Message;

	g_hInst = hInstance;
	drawFlag = false;
	notFoundDrawFlag = false;
	badRequestDrawFlag = false;

	if (renderer.InitGraphics() == false)
	{
		return 1;
	}

	if (socketProcessor.InitSock() == false)
	{
		return 1;
	}

	InitWindow(hInstance, nCmdShow);

	if (htHandler.InitHTHandler(g_hWnd, g_hInst) == false)
	{
		printf("hyper text init error\n");
		socketProcessor.Cleanup();
		renderer.ShutDownGdi();

		return 1;
	}
	while (GetMessage(&Message, NULL, 0, 0)) {
		TranslateMessage(&Message);
		DispatchMessage(&Message);
	}

	socketProcessor.Cleanup();
	renderer.ShutDownGdi();

	htHandler.EndModule();
	return (int)Message.wParam;
}


LRESULT CALLBACK EditSubProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam)
{
	static bool flag = false;
	RECT rc = { 0, };
	char className[BUFSIZE] = { 0, };
	GetClassName(hWnd, className, BUFSIZE);
	switch (iMessage)
	{
	case WM_LBUTTONDOWN:
	{
		if (strcmp(className, RICHEDIT_CLASS) == 0)
		{
			flag = false;
			HyperLinkEvent(hWnd);
			return 0;
		}
		break;
	}
	case WM_MOUSEMOVE:
	{
		if (strcmp(className, RICHEDIT_CLASS) == 0)
		{
			SetCursor(LoadCursor(NULL, IDC_HAND));
			if (flag == false)
			{
				printf("ENTERED\n");
				TrackMouse(hWnd);
				OnHyperText(hWnd);
				flag = true;
			}
			break;
		}
	}
	case WM_MOUSELEAVE:
	{
		if (strcmp(className, RICHEDIT_CLASS) == 0)
		{
			if (flag == true)
			{
				printf("mouseLeave\n");
				LeavingHyperText(hWnd);
				flag = false;
			}
			break;
		}
	}
	case WM_KEYDOWN:
		if (wParam == VK_RETURN)						// Enter�� edit�� �ԷµǾ� �ִ� string���� ���� �� URI GET ��û
		{
			drawFlag = false;
			notFoundDrawFlag = false;
			badRequestDrawFlag = false;

			bodySpliter.Cleanup();
			socketProcessor.MemoryClear();
			htmlParser.Cleanup();
			eventHandler.Cleanup();
			renderer.Cleanup();
			htHandler.Cleanup();

			memset(uriStr, 0, sizeof(uriStr));
			GetWindowText(hWnd, uriStr, URLLENGTH);		// edit control�� ���� URI �Է�
			SendMessage(g_hWnd, WM_USER + 1, 0, 0);		// main window�� event message ����
			return 0;
		}
	}

	if (strcmp(className, RICHEDIT_CLASS) == 0)
	{
		return CallWindowProc(RichEditOldProc, hWnd, iMessage, wParam, lParam);
	}
	else
	{
		return CallWindowProc(OldEditProc, hWnd, iMessage, wParam, lParam);
	}
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_CREATE:
	{
		RECT rc = { 0, };
		GetClientRect(hWnd, &rc);

		hEdit = CreateWindow("edit", NULL, WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL,		// URI �Է¿� edit control ����
			rc.left + 50, 10, rc.right - 100, 25, hWnd, (HMENU)ID_URI, g_hInst, NULL);

		hUriStatic = CreateWindow("static", "URI", WS_CHILD | WS_VISIBLE | WS_BORDER | SS_CENTER, 0, 10, 50, 25, hWnd, (HMENU)ID_STATIC, g_hInst, NULL);
		SetFocus(hEdit);

		OldEditProc = (WNDPROC)SetWindowLongPtr(hEdit, GWLP_WNDPROC, (LONG_PTR)EditSubProc);		// edit control ���� enter ó���� ���ֱ� ���� 
																									// procedure�� edit control�� procedure�� intercept.
		return 0;
	}
	case WM_LBUTTONDOWN:
	{
		SetFocus(hWnd);
		return 0;
	}
	case WM_COMMAND:																					// <form>�� submit(button control)�� ó��
	{
		if (eventHandler.IsActionPushed(wParam) == true)												// eventHandler���� Submit ��ư�� ���������� Ȯ��
		{
			SubmitProc(wParam);																		// <form>�� action�� method�� ���� ��û
		}
		return 0;
	}
	case WM_VSCROLL:																					// ���� scroll �̺�Ʈ ó��
	{
		ScrollEvent(hWnd, wParam);
		return 0;
	}
	case WM_USER + 1:																					// custom event : edit control���� enter�� ���� GET ��û�� �̺�Ʈ ó��
	{
		socketProcessor.SetRequestFlag(0);																		// 0���� Set�� GET ��û
		RequestProc();																				// Request ó���� �ֿ� procedure
		SetTitle();																					// window title ����
		InvalidateRect(hWnd, NULL, true);
		return 0;
	}
	case WM_PAINT:
	{
		RECT editRect = { 0, };
		PAINTSTRUCT ps;
		RECT rc = { 0, };

		GetClientRect(hEdit, &editRect);
		GetClientRect(hWnd, &rc);


		HDC hdc = BeginPaint(hWnd, &ps);

		ExcludeClipRect(hdc, rc.left, rc.top, rc.right, editRect.bottom + 20);						// ������ URI �Է¿� edit control�� �ִ� ������ �����ϰ� redraw.

		if (drawFlag == true)																		// OK�ÿ� �׷���
		{
			renderer.DrawImages(hdc, rc);
		}
		else if (notFoundDrawFlag == true)
		{
			TextOut(hdc, 50, 50, "Not Found", strlen("Not Found"));
		}
		else if (badRequestDrawFlag == true)
		{
			TextOut(hdc, 50, 50, "Bad Request", strlen("Bad Request"));
		}
		EndPaint(hWnd, &ps);
		return 0;
	}
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}

	return 0;
}

void RequestProc()
{
	int code = -1;

	socketProcessor.FillUriBuffer(uriStr);					// socket instance�� URI ����
	printf("uriStr: %s\n", uriStr);
	if (socketProcessor.CheckUrlFormat() == false)			// URI format �˻�
	{
		printf("Invalid URI format\n");
		return;
	}
	if (socketProcessor.Connection() == false)				// ������ ����
	{
		socketProcessor.CloseSocket();
		return;
	}

	if (socketProcessor.GetRequestFlag() == 0)				// GET, POST Ȯ�� �� ����
	{
		socketProcessor.MakeGETHeader();
	}
	else if (socketProcessor.GetRequestFlag() == 1)
	{
		socketProcessor.MakePostHeader(eventHandler.GetBodyLength());		// POST�� body length�� header�� ����
	}

	if (socketProcessor.SendRequest() == false)								// header, body ���� �����͸� �����ϰ� response�� ����.
	{
		socketProcessor.CloseSocket();
		return;
	}

	htmlParser.StoreResponseData(socketProcessor.GetResponseBuffer(), socketProcessor.GetTotalReceiveLength());	// socket class�κ��� html parser class�� ������ ����
	htmlParser.SetURI(socketProcessor.GetUri());

	//printf("%s\n", hp.GetBodyString());

	htmlParser.ExtractHttpHeaderFromResponse();			// ex) HTTP/1.1 200 OK �� ���� HTTP ��� �Ľ�

	code = htmlParser.CheckHttpHeaderNCode();			// HTTP header �˻�

	switch (code)
	{
	case OK:
	{
		OkProc();
		RECT rc = { 0, };
		GetClientRect(g_hWnd, &rc);
		renderer.SetsPoint(true, rc.left, rc.top);		// ��� ��ǥ �ʱ�ȭ
		drawFlag = true;
		break;
	}
	case NotFound:
	{
		notFoundDrawFlag = true;
		break;
	}
	case BadRequest:
	{
		badRequestDrawFlag = true;
		break;
	}
	}
	socketProcessor.CloseSocket();
}

void OkProc()
{
	switch (htmlParser.ExtensionCheck())
	{
	case CGIM:
	case HTMLM:
	{
		htmlParser.ExtractHtmlHeaderFromResponse();			// ex) <HEAD> ... 
		htmlParser.ExtractBodyFromResponse();				// ex) <BODY> .....

		if (htmlParser.IsBodyNull() == false)
		{
			bodySpliter.StoreData(htmlParser.GetBodyString());
			bodySpliter.FillBodyListByParsing();				// <tag>�� �������� �Ľ��� �ϰ� list�� ����
			bodySpliter.PrintData();
			FillObjectData(bodySpliter.GetBodyList());		// list�� ����ִ� <tag>�� text���� ������� ������ ����.(control, text, image ��)

			//bp.PrintData();
			//hp.SetBodyNull();
		}
		break;
	}
	case IMAGEM:
	{
		htmlParser.SetImageFileSize();
		htmlParser.WriteImageFile();
		if (renderer.IsOrderEmpty() == true)				// URI edit control�� ���� ���� GET�� �� �ÿ� order�� ��������Ƿ� order�� �̹��� order�� ��������
		{
			renderer.SetOrder(IMAGEN);
		}

		renderer.RegisterImages(socketProcessor.GetUri());				// �̹����� GDI+�� ���

		break;
	}
	default:
	{
		printf("incorrect extension\n");
		break;
	}
	}
}

void SetTitle()
{
	if (htmlParser.IsTitleNull() == false)
	{
		SetWindowText(g_hWnd, htmlParser.GetTitleName());
	}
}

void RequestImageURI(char* iter)
{
	socketProcessor.MemoryClear();
	socketProcessor.SetRequestFlag(0);						// image�� ���� GET ��û
	memset(uriStr, 0, sizeof(uriStr));
	MakeImageUri(iter);							// image name.extention�� ������ URI ����
	RequestProc();								// ��û procedure
}

void MakeImageUri(char* data)
{
	char* p = NULL;
	char* tmpBuf = NULL;
	int index = 0;

	p = strstr(data, "\"");
	for (index = 1; p[index] != '\"' || 0; index++);	// <img src�κ��� image file name parsing

	tmpBuf = new char[index];
	strncpy_s(tmpBuf, index, (p + 1), index - 1);
	if ((p = strstr(tmpBuf, "./")) != NULL)
	{
		p = tmpBuf + strlen("./");
	}
	else
	{
		p = tmpBuf;
	}

	MakeUri(p);
	delete[] tmpBuf;
	tmpBuf = NULL;
}

void MakeUri(char* uri)
{
	sprintf_s(uriStr, "http://%s:%d/%s\0", socketProcessor.GetAddr(), socketProcessor.GetPortNum(), uri);
}



void ScrollEvent(HWND hWnd, WPARAM wParam)
{
	static int scrollPos = 0;
	RECT rc = { 0, };
	RECT winRC = { 0, };

	GetClientRect(hEdit, &rc);
	GetClientRect(hWnd, &winRC);

	winRC.top = winRC.top + rc.bottom + 10;
	switch (LOWORD(wParam))
	{

	case SB_LINEUP:
	{
		if (scrollPos > 0)
		{
			scrollPos -= SCROLLVALUE;
			renderer.SetsPoint(false, 0, SCROLLVALUE);
			ScrollWindow(hWnd, 0, SCROLLVALUE, &winRC, &rc);
		}
		break;
	}
	case SB_LINEDOWN:
	{
		scrollPos += SCROLLVALUE;
		renderer.SetsPoint(false, 0, -SCROLLVALUE);

		ScrollWindow(hWnd, 0, -SCROLLVALUE, &winRC, &rc);
		break;
	}
	}
	SetScrollPos(hWnd, SB_VERT, scrollPos, true);
	InvalidateRect(hWnd, &winRC, true);
}

void FillObjectData(std::list<char*>* data)
{
	HWND hWnd;
	hyperText* tmpHyperTxt;
	std::list<char*>::iterator iter;
	HFONT tmpFont;
	renderer.MakeFonts();
	tmpFont = renderer.GetHFont(0);
	for (iter = data->begin(); iter != data->end(); iter++)
	{
		if (strcmp(*iter, H1H) == 0)					// <h1> : font set
		{
			tmpFont = renderer.GetHFont(1);
		}
		else if (strcmp(*iter, H1T) == 0)				// </h1> : font unset.
		{
			tmpFont = renderer.GetHFont(0);
		}
		else if (strcmp(*iter, BRS) == 0 || strcmp(*iter, BRE) == 0 || strcmp(*iter, BBRS) == 0 || strcmp(*iter, BBRE) == 0)	// <BR> : enter
		{
			renderer.SetOrder(BRN);
		}
		else
		{
			if (strstr(*iter, "<img") != NULL)			// image request
			{
				socketProcessor.CloseSocket();
				renderer.SetOrder(IMAGEN);
				RequestImageURI(*iter);
			}
			else if (strstr(*iter, "<form") != NULL)	// form event ó�� ����
			{
				eventHandler.MakeEventForm(*iter);
			}
			else if (strstr(*iter, "</form>") != NULL)	// form event ó�� ����
			{
				eventHandler.SetFormFlag(false);
			}
			else if (strstr(*iter, "<input") != NULL)	// form event���� control ��� �� ���ÿ� graphic���� control ���
			{
				renderer.SetOrder(CONTROLN);
				hWnd = eventHandler.RegisterFormControls(*iter, g_hWnd, g_hInst);
				if (hWnd != (HWND)-1)
				{
					renderer.RegisterControls(hWnd);
				}
			}
			else if (strstr(*iter, "<a") != NULL)		// <a href=... > </a>
			{
				if (htHandler.SaveTempUri(*iter) == true)
				{
					renderer.SetOrder(HYPERTEXTN);
					htHandler.SetATagFlag(true);
					tmpFont = renderer.GetHFont(2);
				}
			}
			else if (strstr(*iter, "</a") != NULL)
			{
				htHandler.SetATagFlag(false);
			}
			else if (strstr(*iter, "<") != NULL)		// �� �� <tag> ����
			{
				continue;
			}
			else
			{
				if (htHandler.GetATagFlag() == true)				// hyper text�� text ����.
				{
					tmpHyperTxt = htHandler.MakeRichEdit(*iter, tmpFont, true);
					SendMessage(tmpHyperTxt->_hRichEdit, EM_SETCHARFORMAT, (WPARAM)SCF_ALL, (LPARAM)&renderer.GetHyperTextFonts(0));
					RichEditOldProc = (WNDPROC)SetWindowLongPtr(tmpHyperTxt->_hRichEdit, GWLP_WNDPROC, (LONG_PTR)EditSubProc);
				}
				else
				{
					renderer.SetOrder(HYPERTEXTN);
					tmpHyperTxt = htHandler.MakeRichEdit(*iter, tmpFont, false);
				}
				renderer.RegisterHyperTexts(tmpHyperTxt->_hRichEdit, tmpHyperTxt->_linkedStr, tmpHyperTxt->_hFont);
				delete tmpHyperTxt;
				tmpHyperTxt = NULL;

			}											// tag ���� text��� �ռ� font data�� �Բ� ���
		}
	}
}

void SubmitProc(WPARAM wParam)
{
	if (eventHandler.EventProc(wParam) == 1)				// event Ȯ�ΰ� ���ÿ� Action, Method ����
	{
		drawFlag = false;
		notFoundDrawFlag = false;
		badRequestDrawFlag = false;

		bodySpliter.Cleanup();
		htmlParser.Cleanup();
		renderer.Cleanup();
		htHandler.Cleanup();

		socketProcessor.SetRequestFlag(1);							// POST�� request
		MakeUri(eventHandler.GetAction());				// Action�� ���� URI ����
		socketProcessor.FillBodyData(eventHandler.GetBodyData());	// socket class�� body ����
		SetWindowText(hEdit, uriStr);
		RequestProc();									// header�� body�� request
		eventHandler.Cleanup();
		InvalidateRect(g_hWnd, NULL, true);
	}

}

void OnHyperText(HWND hWnd)
{
	CHARFORMAT2 cf2;

	cf2 = renderer.GetHyperTextFonts(1);

	SendMessage(hWnd, EM_SETCHARFORMAT, (WPARAM)SCF_ALL, (LPARAM)&cf2);
	InvalidateRect(hWnd, NULL, false);
}

void LeavingHyperText(HWND hWnd)
{
	CHARFORMAT2 cf2;

	cf2 = renderer.GetHyperTextFonts(0);
	SendMessage(hWnd, EM_SETCHARFORMAT, (WPARAM)SCF_ALL, (LPARAM)&cf2);
	
	InvalidateRect(hWnd, NULL, false);
}

void InitWindow(HINSTANCE hInstance, int nCmdShow)
{
	LPCSTR lpszClass = "MainWindow";
	WNDCLASS WndClass;

	WndClass.cbClsExtra = 0;
	WndClass.cbWndExtra = 0;
	WndClass.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	WndClass.hCursor = LoadCursor(NULL, IDC_ARROW);
	WndClass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	WndClass.hInstance = hInstance;
	WndClass.lpfnWndProc = WndProc;
	WndClass.lpszClassName = lpszClass;
	WndClass.lpszMenuName = NULL;
	WndClass.style = CS_HREDRAW | CS_VREDRAW;
	RegisterClass(&WndClass);

	g_hWnd = CreateWindow(lpszClass, lpszClass, WS_OVERLAPPEDWINDOW | WS_HSCROLL | WS_VSCROLL /*WS_CLIPCHILDREN*/, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, NULL, (HMENU)NULL, hInstance, NULL);
	ShowWindow(g_hWnd, nCmdShow);
}

void TrackMouse(HWND hwnd)
{
	TRACKMOUSEEVENT tme;
	tme.cbSize = sizeof(TRACKMOUSEEVENT);
	tme.dwFlags = TME_HOVER | TME_LEAVE; //Type of events to track & trigger.
	tme.dwHoverTime = 1; //How long the mouse has to be in the window to trigger a hover event.
	tme.hwndTrack = hwnd;
	TrackMouseEvent(&tme);
}

void HyperLinkEvent(HWND hWnd)
{
	char* uri = NULL;
	drawFlag = false;
	notFoundDrawFlag = false;
	badRequestDrawFlag = false;

	
	uri = htHandler.GetLinkedUri(hWnd);
	if (uri == NULL)
	{
		sprintf(uriStr, "http://%s:%d/index.html", socketProcessor.GetAddr(), socketProcessor.GetPortNum());
	}
	else
	{
		strcpy_s(uriStr, sizeof(uriStr), uri);
	}

	SetWindowText(hEdit, uriStr);
	bodySpliter.Cleanup();
	htmlParser.Cleanup();
	renderer.Cleanup();
	eventHandler.Cleanup();
	htHandler.Cleanup();

	socketProcessor.SetRequestFlag(0);
	
	RequestProc();									// header�� body�� request
	
	InvalidateRect(g_hWnd, NULL, true);
}