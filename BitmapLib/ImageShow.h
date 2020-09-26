#pragma once

#include "Image.h"
using namespace ImageSpace;

#include <iostream>
#include <string>
#include <vector>
#include <thread>
#include <mutex>

//������ʾ�ṹ
struct ShowHelp
{
	std::string WindowName;			//��������
	HWND WindowHwnd;				//���ھ��
	BitmapStruct BmpData;				//����λͼ
};

class ImageShow
{
private:
	//�б�
	static std::vector<ShowHelp*> m_vWindowList;

	//ͬ����
	static std::mutex m_cMutex;

	//Ĭ�ϵĴ��ڹ���
	static LRESULT CALLBACK ImageDefProc(HWND, UINT, WPARAM, LPARAM);

	//����һ����ɫ��
	static HPALETTE CreateBitPalette(const BitmapStruct& Bit);

	//��ʾ����
	static void Show(HWND& hWnd, HDC& hDc, const BitmapStruct& cBitmap);

	//����Ϊ����ӦWM_PAINT��Ϣ,��ʾ���д��ڵ�ͼƬ
	static void ShowAll();

public:
	//��ȡ��Ϣ�߳�
	static void MsgHandle();

	//��λͼ���봰�ڽ�����ʾ
	static void PushWindow(std::string WindowName, ImageSpace::Image& cImage);
	static void PushWindow(std::string WindowName, BitmapStruct& cBitmap);

	//�����ڵĴ��ں�λͼ�����Ƴ�
	static void PopWindow(std::string WindowName);
};
