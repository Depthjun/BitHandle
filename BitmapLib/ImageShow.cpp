#include "ImageShow.h"
using namespace ImageSpace;

std::vector<ShowHelp*> ImageShow::m_vWindowList;
std::mutex ImageShow::m_cMutex;

void ImageShow::MsgHandle()
{
	MSG stMsg{ 0 };
	while (GetMessageA(&stMsg, 0, 0, 0))
	{
		TranslateMessage(&stMsg);
		DispatchMessageA(&stMsg);
	}
}

LRESULT CALLBACK ImageShow::ImageDefProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	//���ڵĻ���
	if (uMsg == WM_PAINT) ShowAll();

	//���ڵ�����
	if (uMsg == WM_CLOSE)
	{
		//����
		m_cMutex.lock();

		for (auto it = m_vWindowList.begin(); it != m_vWindowList.end(); it++)
		{
			//���һ��
			if ((*it)->WindowHwnd == hWnd)
			{
				(*it)->BmpData.Release();
				DestroyWindow((*it)->WindowHwnd);
				delete (*it);
				m_vWindowList.erase(it);
				break;
			}
		}

		//��������һ�������˵Ļ�,ֱ�ӽ�������
		if (m_vWindowList.empty()) PostQuitMessage(0);

		//����
		m_cMutex.unlock();

		return 1;
	}

	return DefWindowProcA(hWnd, uMsg, wParam, lParam);
}

void ImageShow::PushWindow(std::string WindowName, BitmapStruct& cBitmap)
{
	//�Ȳ����ж�
	if (WindowName.empty() || cBitmap.Empty()) return;

	//����
	m_cMutex.lock();

	//�����б�������û���������
	for (auto& it : m_vWindowList)
	{
		//�����������,ֱ���滻λͼ��OK
		if ((*it).WindowName == WindowName)
		{
			cBitmap.CopyTo((*it).BmpData);
			return;
		}
	}

	//����
	m_cMutex.unlock();

	//����һ������
	WNDCLASSEXA stWndCls{ 0 };
	stWndCls.cbSize = sizeof(WNDCLASSEXA);
	stWndCls.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	stWndCls.hInstance = GetModuleHandleA(NULL);
	stWndCls.lpfnWndProc = ImageDefProc;
	stWndCls.style = CS_HREDRAW | CS_VREDRAW;
	stWndCls.lpszClassName = WindowName.c_str();
	if (RegisterClassExA(&stWndCls))
	{
		HWND hWnd = CreateWindowA(WindowName.c_str(), WindowName.c_str(), WS_SYSMENU,
			0, 0, 0, 0, NULL, NULL, stWndCls.hInstance, NULL);
		if (hWnd)
		{
			//����
			m_cMutex.lock();

			//�����б�
			ShowHelp* Temp = new ShowHelp;
			cBitmap.CopyTo(Temp->BmpData);
			Temp->WindowName = WindowName;
			Temp->WindowHwnd = hWnd;
			m_vWindowList.push_back(std::move(Temp));

			//����
			m_cMutex.unlock();

			ShowWindow(hWnd, SW_SHOW);
			UpdateWindow(hWnd);
		}
	}
}

void ImageShow::PushWindow(std::string WindowName, ImageSpace::Image& cImage)
{
	PushWindow(WindowName, cImage.m_cImage);
}

void ImageShow::PopWindow(std::string WindowName)
{
	HWND Target = NULL;

	//����
	m_cMutex.lock();

	//��������
	for (auto it = m_vWindowList.begin(); it != m_vWindowList.end(); it++)
	{
		if ((*it)->WindowName == WindowName)
		{
			//�õ����ڵľ��
			Target = (*it)->WindowHwnd;
			break;
		}
	}

	//����
	m_cMutex.unlock();

	//������ھ����Ϊ��,�ͷ��ͽ���������Ϣ
	if (Target) SendMessageA(Target, WM_CLOSE, 0, 0);
}

HPALETTE ImageShow::CreateBitPalette(const BitmapStruct& Bit)
{
	///��ȡ��ɫ����
	int nColorTableLen = 0;
	switch (Bit.m_pInfoHeader->biBitCount)
	{
	case 1:nColorTableLen = 2; break;
	case 4:nColorTableLen = 16; break;
	case 8:nColorTableLen = 256; break;
	}
	if (nColorTableLen == 0) return NULL;

	///�����ɫ���ڴ�
	LPLOGPALETTE pLogPalete = (LPLOGPALETTE)VirtualAlloc(NULL, nColorTableLen, PAGE_READWRITE, MEM_COMMIT);
	if (pLogPalete == NULL) return NULL;

	///��ɫ�����
	pLogPalete->palVersion = 0x300;
	pLogPalete->palNumEntries = nColorTableLen;
	for (int i = 0; i < nColorTableLen; i++)
	{
		pLogPalete->palPalEntry[i].peBlue = Bit.m_pRgbQuad[i].rgbBlue;
		pLogPalete->palPalEntry[i].peGreen = Bit.m_pRgbQuad[i].rgbGreen;
		pLogPalete->palPalEntry[i].peRed = Bit.m_pRgbQuad[i].rgbRed;
		pLogPalete->palPalEntry[i].peFlags = Bit.m_pRgbQuad[i].rgbReserved;
	}

	///������ɫ����
	HPALETTE hPalete = CreatePalette(pLogPalete);

	///�ͷ��ڴ�
	VirtualFree(pLogPalete, NULL, MEM_RELEASE);

	///���ؾ��
	return hPalete;
}

void ImageShow::Show(HWND& hWnd, HDC& hDc, const BitmapStruct& cBitmap)
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

void ImageShow::ShowAll()
{
	//����
	m_cMutex.lock();

	//����ÿһ������
	for (const auto& it : m_vWindowList)
	{
		//��ȡ���ھ��
		HWND hWnd = (*it).WindowHwnd;

		//���㴰�ڲŻ���
		if (hWnd == GetForegroundWindow())
		{
			//��ȡ���ڵ�DC
			HDC hDc = GetWindowDC(hWnd);
			if (!hDc) continue;

			//��ɫ��
			HPALETTE hPalete = NULL;

			//�滻��ɫ��
			if ((*it).BmpData.m_pRgbQuad)
			{
				hPalete = CreateBitPalette((*it).BmpData);
				if (hPalete)
				{
					SelectPalette(hDc, hPalete, TRUE);
					RealizePalette(hDc);
				}
			}

			//��ʾͼ��
			Show(hWnd, hDc, (*it).BmpData);

			//�ͷ�DC
			ReleaseDC(hWnd, hDc);

			break;
		}
	}

	//����
	m_cMutex.unlock();
}