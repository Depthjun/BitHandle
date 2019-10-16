#pragma once

#include <Windows.h>
#include <assert.h>

class BitmapStruct
{
public:
	PBYTE m_pImage;							///λͼ������ָ��
	PBITMAPFILEHEADER m_pFileHeader;		///λͼ�ļ�ͷָ��
	PBITMAPINFOHEADER m_pInfoHeader;		///λͼ��Ϣͷָ��
	LPRGBQUAD m_pRgbQuad;					///λͼ��ɫ��ָ��
	PBYTE m_pBuffer;						///λͼRGB����ָ��

	const int nFileHeadSize = sizeof(BITMAPFILEHEADER);		///λ�D�ļ��^��С
	const int nInfoHeadSize = sizeof(BITMAPINFOHEADER);		///λ�D��Ϣ�^��С

public:
	BitmapStruct();
	~BitmapStruct();

	///���ظ�ֵ�����
	void operator=(BitmapStruct& pThis);

public:
	///��ʼ��λͼָ��
	void Initialize(int nPiexlByte = 0);

	///����λͼ�ڴ�
	void Allocate(int nBitmapSize);

	///�ͷ�λͼ�ڴ�
	void Release();

};

