#include "ImageShow.h"
using namespace ImageSpace;

std::map<std::string, HWND> ImageShow::m_cWindowHwnd;
std::mutex ImageShow::m_cMutex;

void ImageShow::GetMsgLoop()
{
	MSG stMsg;
	while (GetMessageA(&stMsg, 0, 0, 0))
	{
		TranslateMessage(&stMsg);
		DispatchMessageA(&stMsg);
	}
}

LRESULT CALLBACK ImageShow::ImageDefProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	//���ڵ�����
	if (uMsg == WM_CLOSE)
	{
		m_cMutex.lock();
		for (std::map<std::string,HWND>::iterator it = m_cWindowHwnd.begin(); 
			it != m_cWindowHwnd.end(); it++)
		{
			if (it->second == hWnd)
			{
				m_cWindowHwnd.erase(it);
				break;
			}
		}
		m_cMutex.unlock();
	}
	return DefWindowProcA(hWnd, uMsg, wParam, lParam);
}

void ImageShow::Create(const char* szWindowName)
{
	WNDCLASSEXA stWndCls;
	ZeroMemory(&stWndCls, sizeof(stWndCls));
	stWndCls.cbSize = sizeof(WNDCLASSEXA);
	stWndCls.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	stWndCls.hInstance = GetModuleHandleA(NULL);
	stWndCls.lpfnWndProc = ImageDefProc;
	stWndCls.style = CS_HREDRAW | CS_VREDRAW;
	stWndCls.lpszClassName = szWindowName;
	if (RegisterClassExA(&stWndCls))
	{
		HWND hWnd = CreateWindowA(szWindowName, szWindowName, WS_SYSMENU,
			0, 0, 100, 100, NULL, NULL, stWndCls.hInstance, NULL);
		if (hWnd)
		{
			m_cWindowHwnd.emplace(std::pair<std::string, HWND>(std::string(szWindowName), hWnd));
			ShowWindow(hWnd, SW_SHOW);
			UpdateWindow(hWnd);
		}
	}

}

void ImageShow::Show(HWND& hWnd, HDC& hDc,const BitmapStruct& cBitmap)
{
	//���û���ģʽλ����ģʽ
	SetStretchBltMode(hDc, HALFTONE);

	//��ȡλͼ�Ŀ�Ⱥ͸߶�
	int nWidth = cBitmap.m_pInfoHeader->biWidth;
	int nHeight = cBitmap.m_pInfoHeader->biHeight;

	//���ô��ڴ�СΪλͼ��С
	RECT stRect;
	GetWindowRect(hWnd, &stRect);
	MoveWindow(hWnd, stRect.left, stRect.top, nWidth + 16 / 2, nHeight + 16 / 2, TRUE);

	//����λͼ����
	StretchDIBits(hDc, 0, 0, nWidth, nHeight, 0, 0, nWidth, nHeight,
		cBitmap.m_pBuffer, (LPBITMAPINFO)cBitmap.m_pInfoHeader, DIB_RGB_COLORS, SRCCOPY);
}

void ImageShow::Show(const char* szWindowName,const BitmapStruct& cBitmap)
{
	//��ȡ���ھ��
	HWND hWnd = m_cWindowHwnd.at(szWindowName);

	//��ȡ���ڵ�DC
	HDC hDc = GetWindowDC(hWnd);
	if (!hDc) return;

	//��ʾͼ��
	Show(hWnd, hDc, cBitmap);

	//�ͷ�DC
	ReleaseDC(hWnd, hDc);
}

void ImageShow::Show(const char* szWindowName, Image& cImage)
{
	//�������������Ҫ��Ϊ����ʾ��ȫ��ͼ��ʹ�õ�ɫ��������

	//��ȡ���ھ��
	HWND hWnd = m_cWindowHwnd.at(szWindowName);

	//��ȡ���ڵ�DC
	HDC hDc = GetWindowDC(hWnd);
	if (!hDc) return;

	//��ɫ��
	HPALETTE hPalete = NULL;

	//�滻��ɫ��
	if (cImage.m_cImage.m_pRgbQuad)
	{
		hPalete = cImage.CreateBitmapPalete();
		if (hPalete)
		{
			SelectPalette(hDc, hPalete, TRUE);
			RealizePalette(hDc);
		}
	}

	//��ʾͼ��
	Show(hWnd, hDc, cImage.m_cImage);

	//�ͷ�DC
	ReleaseDC(hWnd, hDc);
}