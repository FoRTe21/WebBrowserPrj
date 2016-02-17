#include "CMainSystem.h"

#pragma once

CMainSystem::CMainSystem()
{

}

CMainSystem::~CMainSystem()
{

}

bool CMainSystem::InitProcess(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{

}

bool CMainSystem::RunProcess()
{

}

bool CMainSystem::EndProcess()
{

}

{
	

	g_hInst = hInstance;
	drawFlag = false;
	notFoundDrawFlag = false;
	badRequestDrawFlag = false;

	if (ip.InitGraphics() == false)
	{
		return 1;
	}

	if (ws.InitSock() == false)
	{
		return 1;
	}

	InitWindow(hInstance, nCmdShow);

	while (GetMessage(&Message, NULL, 0, 0)) {
		TranslateMessage(&Message);
		DispatchMessage(&Message);
	}

	ws.Cleanup();
	ip.ShutDownGdi();

	return (int)Message.wParam;
}

LRESULT CALLBACK CMainSystem::EditSubProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam)
{
	RECT rc = { 0, };
	switch (iMessage)
	{
	case WM_KEYDOWN:
		if (wParam == VK_RETURN)						// Enter�� edit�� �ԷµǾ� �ִ� string���� ���� �� URI GET ��û
		{
			drawFlag = false;
			notFoundDrawFlag = false;
			badRequestDrawFlag = false;

			bp.Cleanup();
			ws.MemoryClear();
			hp.Cleanup();
			eventHandler.Cleanup();
			ip.Cleanup();

			memset(uriStr, 0, sizeof(uriStr));
			GetWindowText(hWnd, uriStr, URLLENGTH);		// edit control�� ���� URI �Է�
			SendMessage(g_hWnd, WM_USER + 1, 0, 0);		// main window�� event message ����
			return 0;
		}
	}

	return CallWindowProc(OldEditProc, hWnd, iMessage, wParam, lParam);
}

LRESULT CALLBACK CMainSystem::WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_CREATE:
	{
		RECT rc = { 0, };
		GetClientRect(hWnd, &rc);

		hEdit = CreateWindow("edit", NULL, WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL,		// URI �Է¿� edit control ����
			rc.left + 50, 10, rc.right - 100, 25, hWnd, (HMENU)ID_URI, g_hInst, NULL);

		SetFocus(hEdit);

		OldEditProc = (WNDPROC)SetWindowLongPtr(hEdit, GWLP_WNDPROC, (LONG_PTR)EditSubProc);		// edit control ���� enter ó���� ���ֱ� ���� 
																									// procedure�� edit control�� procedure�� intercept.
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
		ws.SetRequestFlag(0);																		// 0���� Set�� GET ��û
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
			ip.DrawImages(hdc, rc);
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

void CMainSystem::RequestProc()
{
	int code = -1;

	ws.FillUriBuffer(uriStr);					// socket instance�� URI ����
	printf("uriStr: %s\n", uriStr);
	if (ws.CheckUrlFormat() == false)			// URI format �˻�
	{
		printf("Invalid URI format\n");
		return;
	}
	if (ws.Connection() == false)				// ������ ����
	{
		ws.CloseSocket();
		return;
	}

	if (ws.GetRequestFlag() == 0)				// GET, POST Ȯ�� �� ����
	{
		ws.MakeGETHeader();
	}
	else if (ws.GetRequestFlag() == 1)
	{
		ws.MakePostHeader(eventHandler.GetBodyLength());		// POST�� body length�� header�� ����
	}

	if (ws.SendRequest() == false)								// header, body ���� �����͸� �����ϰ� response�� ����.
	{
		ws.CloseSocket();
		return;
	}

	hp.StoreResponseData(ws.GetResponseBuffer(), ws.GetTotalReceiveLength());	// socket class�κ��� html parser class�� ������ ����
	hp.SetURI(ws.GetUri());

	//printf("%s\n", hp.GetBodyString());

	hp.ExtractHttpHeaderFromResponse();			// ex) HTTP/1.1 200 OK �� ���� HTTP ��� �Ľ�

	code = hp.CheckHttpHeaderNCode();			// HTTP header �˻�

	switch (code)
	{
	case OK:
	{
		OkProc();
		RECT rc = { 0, };
		GetClientRect(g_hWnd, &rc);
		ip.SetsPoint(true, rc.left, rc.top);		// ��� ��ǥ �ʱ�ȭ
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
	ws.CloseSocket();
}

void CMainSystem::OkProc()
{
	switch (hp.ExtensionCheck())
	{
	case CGIM:
	case HTMLM:
	{
		hp.ExtractHtmlHeaderFromResponse();			// ex) <HEAD> ... 
		hp.ExtractBodyFromResponse();				// ex) <BODY> .....

		if (hp.IsBodyNull() == false)
		{
			bp.StoreData(hp.GetBodyString());
			bp.FillBodyListByParsing();				// <tag>�� �������� �Ľ��� �ϰ� list�� ����
			bp.PrintData();
			FillObjectData(bp.GetBodyList());		// list�� ����ִ� <tag>�� text���� ������� ������ ����.(control, text, image ��)

													//bp.PrintData();
													//hp.SetBodyNull();
		}
		break;
	}
	case IMAGEM:
	{
		hp.SetImageFileSize();
		hp.WriteImageFile();
		if (ip.IsOrderEmpty() == true)				// URI edit control�� ���� ���� GET�� �� �ÿ� order�� ��������Ƿ� order�� �̹��� order�� ��������
		{
			ip.SetOrder(IMAGEN);
		}

		ip.RegisterImages(ws.GetUri());				// �̹����� GDI+�� ���

		break;
	}
	default:
	{
		printf("incorrect extension\n");
		break;
	}
	}
}

void CMainSystem::SetTitle()
{
	if (hp.IsTitleNull() == false)
	{
		SetWindowText(g_hWnd, hp.GetTitleName());
	}
}

void CMainSystem::RequestImageURI(char* iter)
{
	ws.MemoryClear();
	ws.SetRequestFlag(0);						// image�� ���� GET ��û
	memset(uriStr, 0, sizeof(uriStr));
	MakeImageUri(iter);							// image name.extention�� ������ URI ����
	RequestProc();								// ��û procedure
}

void CMainSystem::MakeImageUri(char* data)
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

void CMainSystem::MakeUri(char* uri)
{
	sprintf_s(uriStr, "http://%s:%d/%s\0", ws.GetAddr(), ws.GetPortNum(), uri);
}

void CMainSystem::InitWindow(HINSTANCE hInstance, int nCmdShow)
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

	g_hWnd = CreateWindow(lpszClass, lpszClass, WS_OVERLAPPEDWINDOW | WS_HSCROLL | WS_VSCROLL | WS_CLIPCHILDREN, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, NULL, (HMENU)NULL, hInstance, NULL);
	ShowWindow(g_hWnd, nCmdShow);
}

void CMainSystem::ScrollEvent(HWND hWnd, WPARAM wParam)
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
			ip.SetsPoint(false, 0, SCROLLVALUE);
			ScrollWindow(hWnd, 0, SCROLLVALUE, &winRC, &rc);
		}
		break;
	}
	case SB_LINEDOWN:
	{
		scrollPos += SCROLLVALUE;
		ip.SetsPoint(false, 0, -SCROLLVALUE);

		ScrollWindow(hWnd, 0, -SCROLLVALUE, &winRC, &rc);
		break;
	}
	}
	SetScrollPos(hWnd, SB_VERT, scrollPos, true);
	InvalidateRect(hWnd, &winRC, true);
}

void CMainSystem::FillObjectData(std::list<char*>* data)
{
	HWND hWnd;
	std::list<char*>::iterator iter;
	HFONT tmpFont;
	ip.MakeFonts();
	tmpFont = ip.GetHfont(0);
	for (iter = data->begin(); iter != data->end(); iter++)
	{
		if (strcmp(*iter, H1H) == 0)					// <h1> : font set
		{
			tmpFont = ip.GetHfont(1);
		}
		else if (strcmp(*iter, H1T) == 0)				// </h1> : font unset.
		{
			tmpFont = ip.GetHfont(0);
		}
		else if (strcmp(*iter, BRS) == 0 || strcmp(*iter, BRE) == 0 || strcmp(*iter, BBRS) == 0 || strcmp(*iter, BBRE) == 0)	// <BR> : enter
		{
			ip.SetOrder(BRN);
		}
		else
		{
			if (strstr(*iter, "<img") != NULL)			// image request
			{
				ws.CloseSocket();
				ip.SetOrder(IMAGEN);
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
				ip.SetOrder(CONTROLN);
				hWnd = eventHandler.RegisterFormControls(*iter, g_hWnd, g_hInst);
				if (hWnd != (HWND)-1)
				{
					ip.RegisterControls(hWnd);
				}
			}
			else if (strstr(*iter, "<") != NULL)		// �� �� <tag> ����
			{
				continue;
			}
			else
			{											// tag ���� text��� �ռ� font data�� �Բ� ���
				ip.SetOrder(TEXTN);
				ip.RegisterTexts(*iter, tmpFont);
			}
		}
	}
}

void CMainSystem::SubmitProc(WPARAM wParam)
{
	if (eventHandler.EventProc(wParam) == 1)				// event Ȯ�ΰ� ���ÿ� Action, Method ����
	{
		drawFlag = false;
		notFoundDrawFlag = false;
		badRequestDrawFlag = false;

		bp.Cleanup();
		hp.Cleanup();
		ip.Cleanup();

		ws.SetRequestFlag(1);							// POST�� request
		MakeUri(eventHandler.GetAction());				// Action�� ���� URI ����
		ws.FillBodyData(eventHandler.GetBodyData());	// socket class�� body ����
		RequestProc();									// header�� body�� request
		eventHandler.Cleanup();
		InvalidateRect(g_hWnd, NULL, true);
	}

}