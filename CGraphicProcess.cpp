#include "CGraphicProcess.h"

CGraphicProcess::CGraphicProcess()
{
	m_gpToken = NULL;
	m_order.clear();
	m_images.clear();
	m_controls.clear();
	m_hyperTexts.clear();

	InitHyperTextFonts();

	m_xScrollMax = 0;
	m_yScrollMax = 0;
	memset(m_fontD, 0, sizeof(m_fontD));
	
}
CGraphicProcess::~CGraphicProcess()
{

}

bool CGraphicProcess::InitGraphics()
{
	return (GdiplusStartup(&m_gpToken, &m_gpsi, NULL) == Gdiplus::Ok);
}

void CGraphicProcess::ShutDownGdi()
{
	if (m_gpToken != NULL)
	{
		GdiplusShutdown(m_gpToken);
	}
}

void CGraphicProcess::Cleanup()
{
	m_firstDrawing = false;
	m_xScrollMax = 0;
	m_yScrollMax = 0;

	if (m_order.empty() == false)
	{
		m_order.clear();
	}
	if (m_images.empty() == false)
	{
		m_images.clear();
	}
	if (m_controls.empty() == false)
	{
		for (std::list<HWND>::iterator iter = m_controls.begin(); iter != m_controls.end(); iter++)
		{
			DestroyWindow(*iter);
			CloseHandle(*iter);
		}
		m_controls.clear();
	}
	if (m_hyperTexts.empty() == false)
	{
		for (std::list<gHyperText>::iterator iter = m_hyperTexts.begin(); iter != m_hyperTexts.end(); iter++)
		{
			DestroyWindow(iter->_hRichEdit);
			//CloseHandle(iter->_hRichEdit);

			delete[] iter->_text;
		}
		m_hyperTexts.clear();
	}
}

void CGraphicProcess::SetOrder(eOrder order)
{
	m_order.push_back(order);
}

void CGraphicProcess::RegisterImages(char* filename)
{
	size_t len = 0;
	WCHAR* tmpName = NULL;
	tmpName = new WCHAR[strlen(filename) + 1];
	mbstowcs_s(&len, tmpName, strlen(filename) + 1, filename, strlen(filename));
	m_images.push_back(Image::FromFile(tmpName));

	delete[] tmpName;
}

void CGraphicProcess::RegisterControls(HWND hWnd)			
{
	m_controls.push_back(hWnd);								// 단순히 control handle 등록
}

void CGraphicProcess::RegisterHyperTexts(HWND hRichEdit, char* text, HFONT hFont)
{
	gHyperText tmpHt;

	tmpHt._hFont = hFont;
	tmpHt._hRichEdit = hRichEdit;
	tmpHt._text = new char[strlen(text) + 1];

	m_hyperTexts.push_back(tmpHt);
}

void CGraphicProcess::DrawImages(HDC hdc, RECT rt)
{
	int maxHeight = 0;
	std::list<eOrder>::iterator orderIter;
	std::list<Image*>::iterator imageIter;
	std::list<HWND>::iterator controlIter;
	std::list<gHyperText>::iterator hyperTextIter;

	HWND hWnd;
	RECT rc;

	PointF pf;
	SolidBrush blackBrush(Color(255, 0, 0, 0));
	HFONT hOldFont;
	char tmpBuf[BUFSIZ] = { 0, };

	pf.X = m_sPoint.X;
	pf.Y = m_sPoint.Y + 10.0f;
	Graphics graphics(hdc);

	imageIter = m_images.begin();
	controlIter = m_controls.begin();
	hyperTextIter = m_hyperTexts.begin();

	for (orderIter = m_order.begin(); orderIter != m_order.end(); orderIter++)							// order를 확인한 뒤, order에 맞춰 text, image, control을 그려준다.
	{
		if ((*orderIter == HYPERTEXTN) && (hyperTextIter != m_hyperTexts.end()))
		{
			hWnd = hyperTextIter->_hRichEdit;
			
			SetWindowPos(hWnd, 0, pf.X, pf.Y, 0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_SHOWWINDOW);				// 실제 rich edit 위치 설정해주는 부분
			memset(&rc, 0, sizeof(RECT));
			GetClientRect(hWnd, &rc);
			pf.X += rc.right;
			if (maxHeight < rc.bottom )
			{
				maxHeight = rc.bottom;
			}
			hyperTextIter++;
		}
		else if ((*orderIter == IMAGEN) && (imageIter != m_images.end()))
		{
			graphics.DrawImage(*imageIter, pf);															
			pf.X += (*imageIter)->GetWidth();

			if (maxHeight < (*imageIter)->GetHeight())
			{
				maxHeight = (*imageIter)->GetHeight();
			}
			imageIter++;
		}
		else if ((*orderIter == CONTROLN) && (controlIter != m_controls.end()))
		{
			memset(&rc, 0, sizeof(RECT));
			hWnd = *controlIter;
			
			GetClientRect(hWnd, &rc);
			SetWindowPos(hWnd, 0, pf.X, pf.Y, 0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_SHOWWINDOW);
			
			
			pf.X += rc.right;

			if (maxHeight < rc.bottom)
			{
				maxHeight = rc.bottom;
			}

			controlIter++;
		}
		else if (*orderIter == BRN)									// <BR> 태그이면 height 최대값만큼 y좌표값 증가
		{
			pf.X = m_sPoint.X;
			pf.Y += maxHeight;
			maxHeight = 0;
		}
		if ((m_firstDrawing == false) && (m_xScrollMax < pf.X))
		{
			m_xScrollMax = pf.X;
		}
	}

	if (m_firstDrawing == false)
	{
		m_yScrollMax = pf.Y + maxHeight;
	}
	graphics.ReleaseHDC(hdc);
}

bool CGraphicProcess::IsOrderEmpty()
{
	if (m_order.empty() == true)
	{
		return true;
	}
	else
	{
		return false;
	}
}

void CGraphicProcess::MakeFonts()
{
	m_fontD[0] = CreateFont(15, 0, 0, 0, 0, 0, 0, 0, HANGEUL_CHARSET, 3, 2, 1,
		VARIABLE_PITCH | FF_ROMAN, "default");
	

	m_fontD[1] = CreateFont(30, 0, 0, 0, 0, 0, 0, 0, HANGEUL_CHARSET, 3, 2, 1,
		VARIABLE_PITCH | FF_ROMAN, "second");

	m_fontD[2] = CreateFont(20, 0, 0, 0, FW_BOLD, 0, 0, 0, HANGEUL_CHARSET, 3, 2, 1,
		VARIABLE_PITCH | FF_ROMAN, "linked");
}

HFONT CGraphicProcess::GetHFont(int order)
{
	return m_fontD[order];
}

void CGraphicProcess::SetsPoint(bool init, float x, float y)
{
	if (init == true)
	{
		m_sPoint.X = x;					// 초기 위치 정함
		m_sPoint.Y = y;
	}
	else
	{
		m_sPoint.X += x;				// scroll 움직일때
		m_sPoint.Y += y;
	}
}

void CGraphicProcess::InitHyperTextFonts()
{
	memset(&m_hyperTextFormat[0], 0, sizeof(m_hyperTextFormat[1]));
	m_hyperTextFormat[0].cbSize = sizeof(CHARFORMAT2);
	m_hyperTextFormat[0].dwMask = CFM_COLOR | CFM_UNDERLINE | CFM_BOLD;
	m_hyperTextFormat[0].dwEffects = CFE_BOLD;
	m_hyperTextFormat[0].crTextColor = COLORREF(RGB(255, 0, 0));
	m_hyperTextFormat[0].bUnderlineType = CFU_UNDERLINENONE;

	memset(&m_hyperTextFormat[1], 0, sizeof(m_hyperTextFormat[0]));
	m_hyperTextFormat[1].cbSize = sizeof(CHARFORMAT2);
	m_hyperTextFormat[1].dwMask = CFM_COLOR | CFM_UNDERLINE | CFM_BOLD;
	m_hyperTextFormat[1].crTextColor = COLORREF(RGB(0, 0, 255));
	m_hyperTextFormat[1].dwEffects = CFE_BOLD | CFE_UNDERLINE;
	m_hyperTextFormat[1].bUnderlineColor = COLORREF(RGB(0, 0, 255));
	m_hyperTextFormat[1].bUnderlineType = CFU_UNDERLINE;
}

CHARFORMAT2 CGraphicProcess::GetHyperTextFonts(int state)
{
	switch (state)
	{
	case 0:
		return m_hyperTextFormat[0];
	case 1:
		return m_hyperTextFormat[1];
	}
}

int CGraphicProcess::GetXScrollMax()
{
	return m_xScrollMax;
}

int CGraphicProcess::GetYScrollMax()
{
	return m_yScrollMax;
}

int CGraphicProcess::GetFirstFlag()
{
	return m_firstDrawing;
}

void CGraphicProcess::SetFirstFlag(bool flag)
{
	if (flag == false)
	{
		m_xScrollMax = 0;
		m_yScrollMax = 0;
	}
	m_firstDrawing = flag;
}