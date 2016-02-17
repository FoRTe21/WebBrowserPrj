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
		if (wParam == VK_RETURN)						// Enter시 edit에 입력되어 있는 string으로 접속 및 URI GET 요청
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
			GetWindowText(hWnd, uriStr, URLLENGTH);		// edit control로 부터 URI 입력
			SendMessage(g_hWnd, WM_USER + 1, 0, 0);		// main window에 event message 전달
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

		hEdit = CreateWindow("edit", NULL, WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL,		// URI 입력용 edit control 생성
			rc.left + 50, 10, rc.right - 100, 25, hWnd, (HMENU)ID_URI, g_hInst, NULL);

		SetFocus(hEdit);

		OldEditProc = (WNDPROC)SetWindowLongPtr(hEdit, GWLP_WNDPROC, (LONG_PTR)EditSubProc);		// edit control 상의 enter 처리를 해주기 위해 
																									// procedure를 edit control의 procedure로 intercept.
		return 0;
	}
	case WM_COMMAND:																					// <form>의 submit(button control)의 처리
	{
		if (eventHandler.IsActionPushed(wParam) == true)												// eventHandler에서 Submit 버튼이 눌려졌는지 확인
		{
			SubmitProc(wParam);																		// <form>의 action을 method를 통해 요청
		}
		return 0;
	}
	case WM_VSCROLL:																					// 상하 scroll 이벤트 처리
	{
		ScrollEvent(hWnd, wParam);
		return 0;
	}
	case WM_USER + 1:																					// custom event : edit control에서 enter를 통해 GET 요청시 이벤트 처리
	{
		ws.SetRequestFlag(0);																		// 0으로 Set시 GET 요청
		RequestProc();																				// Request 처리의 주요 procedure
		SetTitle();																					// window title 변경
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

		ExcludeClipRect(hdc, rc.left, rc.top, rc.right, editRect.bottom + 20);						// 상위의 URI 입력용 edit control이 있는 영역은 제외하고 redraw.

		if (drawFlag == true)																		// OK시에 그려줌
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

	ws.FillUriBuffer(uriStr);					// socket instance에 URI 저장
	printf("uriStr: %s\n", uriStr);
	if (ws.CheckUrlFormat() == false)			// URI format 검사
	{
		printf("Invalid URI format\n");
		return;
	}
	if (ws.Connection() == false)				// 서버로 접속
	{
		ws.CloseSocket();
		return;
	}

	if (ws.GetRequestFlag() == 0)				// GET, POST 확인 후 진행
	{
		ws.MakeGETHeader();
	}
	else if (ws.GetRequestFlag() == 1)
	{
		ws.MakePostHeader(eventHandler.GetBodyLength());		// POST시 body length를 header에 포함
	}

	if (ws.SendRequest() == false)								// header, body 등의 데이터를 전송하고 response를 수신.
	{
		ws.CloseSocket();
		return;
	}

	hp.StoreResponseData(ws.GetResponseBuffer(), ws.GetTotalReceiveLength());	// socket class로부터 html parser class로 데이터 전달
	hp.SetURI(ws.GetUri());

	//printf("%s\n", hp.GetBodyString());

	hp.ExtractHttpHeaderFromResponse();			// ex) HTTP/1.1 200 OK 와 같은 HTTP 헤더 파싱

	code = hp.CheckHttpHeaderNCode();			// HTTP header 검사

	switch (code)
	{
	case OK:
	{
		OkProc();
		RECT rc = { 0, };
		GetClientRect(g_hWnd, &rc);
		ip.SetsPoint(true, rc.left, rc.top);		// 출력 좌표 초기화
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
			bp.FillBodyListByParsing();				// <tag>를 기준으로 파싱을 하고 list에 저장
			bp.PrintData();
			FillObjectData(bp.GetBodyList());		// list에 들어있는 <tag>와 text들을 기반으로 데이터 저장.(control, text, image 등)

													//bp.PrintData();
													//hp.SetBodyNull();
		}
		break;
	}
	case IMAGEM:
	{
		hp.SetImageFileSize();
		hp.WriteImageFile();
		if (ip.IsOrderEmpty() == true)				// URI edit control을 통해 직접 GET을 할 시에 order가 비어있으므로 order에 이미지 order를 저장해줌
		{
			ip.SetOrder(IMAGEN);
		}

		ip.RegisterImages(ws.GetUri());				// 이미지를 GDI+에 등록

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
	ws.SetRequestFlag(0);						// image에 대해 GET 요청
	memset(uriStr, 0, sizeof(uriStr));
	MakeImageUri(iter);							// image name.extention를 포함한 URI 생성
	RequestProc();								// 요청 procedure
}

void CMainSystem::MakeImageUri(char* data)
{
	char* p = NULL;
	char* tmpBuf = NULL;
	int index = 0;

	p = strstr(data, "\"");
	for (index = 1; p[index] != '\"' || 0; index++);	// <img src로부터 image file name parsing

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
			else if (strstr(*iter, "<form") != NULL)	// form event 처리 시작
			{
				eventHandler.MakeEventForm(*iter);
			}
			else if (strstr(*iter, "</form>") != NULL)	// form event 처리 종료
			{
				eventHandler.SetFormFlag(false);
			}
			else if (strstr(*iter, "<input") != NULL)	// form event에서 control 등록 과 동시에 graphic에서 control 등록
			{
				ip.SetOrder(CONTROLN);
				hWnd = eventHandler.RegisterFormControls(*iter, g_hWnd, g_hInst);
				if (hWnd != (HWND)-1)
				{
					ip.RegisterControls(hWnd);
				}
			}
			else if (strstr(*iter, "<") != NULL)		// 그 외 <tag> 무시
			{
				continue;
			}
			else
			{											// tag 외의 text라면 앞선 font data와 함께 등록
				ip.SetOrder(TEXTN);
				ip.RegisterTexts(*iter, tmpFont);
			}
		}
	}
}

void CMainSystem::SubmitProc(WPARAM wParam)
{
	if (eventHandler.EventProc(wParam) == 1)				// event 확인과 동시에 Action, Method 저장
	{
		drawFlag = false;
		notFoundDrawFlag = false;
		badRequestDrawFlag = false;

		bp.Cleanup();
		hp.Cleanup();
		ip.Cleanup();

		ws.SetRequestFlag(1);							// POST로 request
		MakeUri(eventHandler.GetAction());				// Action에 따라 URI 결정
		ws.FillBodyData(eventHandler.GetBodyData());	// socket class로 body 전달
		RequestProc();									// header와 body로 request
		eventHandler.Cleanup();
		InvalidateRect(g_hWnd, NULL, true);
	}

}