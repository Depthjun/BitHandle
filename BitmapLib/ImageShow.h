#pragma once

#include "Image.h"
using namespace ImageSpace;

#include <iostream>
#include <string>
#include <map>
#include <thread>
#include <mutex>

class ImageShow
{
public:
	//���ھ������
	static std::map<std::string,HWND> m_cWindowHwnd;

	//ͬ����
	static std::mutex m_cMutex;

	//��ȡ��Ϣ�߳�
	static void GetMsgLoop();

	//Ĭ�ϵĴ��ڹ���
	static LRESULT CALLBACK ImageDefProc(HWND, UINT, WPARAM, LPARAM);

	//��������
	static void Create(const char* szWindowName);

	//��ʾ����
	static void Show(HWND& hWnd, HDC& hDc,const BitmapStruct& cBitmap);
	static void Show(const char* szWindowName,const BitmapStruct& cBitmap);
	static void Show(const char* szWindowName,ImageSpace::Image& cImage);
};

