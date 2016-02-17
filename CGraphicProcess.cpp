#include "CGraphicProcess.h"

CGraphicProcess::CGraphicProcess()
{
	m_gpToken = NULL;
	m_order.clear();
	m_images.clear();
	m_texts.clear();
	m_controls.clear();
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
	if (m_texts.empty() == false)
	{
		for (std::list<customText>::iterator iter = m_texts.begin(); iter != m_texts.end(); iter++)
		{
			delete[](iter->_text);
		}
		m_texts.clear();
	}
}

void CGraphicProcess::SetOrder(int order)
{
	m_order.push_back(order);
}

void CGraphicProcess::RegisterTexts(char* texts, HFONT font)
{
	customText ct;
	ct._length = strlen(texts);
	ct._text = new WCHAR[ct._length + 1];
	mbstowcs(ct._text, texts, ct._length + 1);				// GDI+ �� text�� ����� �� WCHAR*�� ��ϵǱ� ������ char* -> WCHAR*�� ��ȯ
	ct._font = font;
	m_texts.push_back(ct);
}

void CGraphicProcess::RegisterImages(char* filename)
{
	WCHAR* tmpName = NULL;
	tmpName = new WCHAR[strlen(filename) + 1];
	mbstowcs(tmpName, filename, strlen(filename) + 1);		// GDI+ �� text�� ����� �� WCHAR*�� ��ϵǱ� ������ char* -> WCHAR*�� ��ȯ
	m_images.push_back(Image::FromFile(tmpName));

	delete[] tmpName;
}

void CGraphicProcess::RegisterControls(HWND hWnd)			
{
	m_controls.push_back(hWnd);								// �ܼ��� control handle ���
}

void CGraphicProcess::DrawImages(HDC hdc, RECT rt)
{
	int maxHeight = 0;
	std::list<int>::iterator orderIter;
	std::list<customText>::iterator textIter;
	std::list<Image*>::iterator imageIter;
	std::list<HWND>::iterator controlIter;

	HWND hWnd;
	HWND preHwnd;
	RECT rc;

	PointF pf;
	SolidBrush blackBrush(Color(255, 0, 0, 0));

	pf.X = m_sPoint.X;
	pf.Y = m_sPoint.Y + 50.0f;
	Graphics graphics(hdc);

	textIter = m_texts.begin();
	imageIter = m_images.begin();
	controlIter = m_controls.begin();

	for (orderIter = m_order.begin(); orderIter != m_order.end(); orderIter++)							// order�� Ȯ���� ��, order�� ���� text, image, control�� �׷��ش�.
	{
		if ((*orderIter == TEXTN) && (textIter != m_texts.end()))
		{
			Font tmpFont(hdc, textIter->_font);
			graphics.DrawString((textIter->_text), textIter->_length, &tmpFont, pf, &blackBrush);

			pf.X += textIter->_length * (tmpFont.GetSize() + 1)/ 2;										// �׷��� �� ��ǥ ����
			if (maxHeight < tmpFont.GetHeight(&graphics))
			{
				maxHeight = tmpFont.GetHeight(&graphics);
			}
			textIter++;
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

			hWnd = *controlIter;
			GetClientRect(hWnd, &rc);
			SetWindowPos(hWnd, 0, pf.X + rc.right / 2, pf.Y + rc.bottom, 0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_SHOWWINDOW);
			
			pf.X += rc.right + 10;

			if (maxHeight < rc.bottom)
			{
				maxHeight = rc.bottom;
			}

			controlIter++;
		}
		else if (*orderIter == BRN)									// <BR> �±��̸� height �ִ밪��ŭ y��ǥ�� ����
		{
			pf.X = rt.left;
			pf.Y += maxHeight;
			maxHeight = 0;
		}
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
}

HFONT CGraphicProcess::GetHfont(int order)
{
	return m_fontD[order];
}

void CGraphicProcess::SetsPoint(bool init, float x, float y)
{
	if (init == true)
	{
		m_sPoint.X = x;
		m_sPoint.Y = y;
	}
	else
	{
		m_sPoint.X += x;
		m_sPoint.Y += y;
	}
}

