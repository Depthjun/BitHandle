#include "BitmapStruct.h"

BitmapStruct::BitmapStruct() :
	m_pImage(NULL),
	m_pFileHeader(NULL),
	m_pInfoHeader(NULL),
	m_pRgbQuad(NULL),
	m_pBuffer(NULL) {}

BitmapStruct::~BitmapStruct() { Release(); }

void BitmapStruct::operator=(BitmapStruct& pThis)
{
	//ጷ�ԭ��Ĕ���
	this->Release();

	//�޸�ָ�
	m_pImage = pThis.m_pImage;

	//�޸�ָ�ָ���
	pThis.m_pImage = NULL;
	pThis.Release();

	//��ʼ��
	Initialize();
}

void BitmapStruct::Initialize(int nPiexlByte)
{
	if (m_pImage)
	{
		//�ļ�ͷָ��
		m_pFileHeader = (PBITMAPFILEHEADER)m_pImage;

		//��Ϣͷָ��
		m_pInfoHeader = (PBITMAPINFOHEADER)(m_pImage + nFileHeadSize);

		//���ٵ�λͼ
		if (nPiexlByte == 0) nPiexlByte = m_pInfoHeader->biBitCount;

		//��ȡ��ɫ���С
		int nColorTable = 0;
		switch (nPiexlByte)
		{
		case 1:nColorTable = 2; break;
		case 4:nColorTable = 16; break;
		case 8:nColorTable = 256; break;
		}

		//����ɫ��
		if (nColorTable)
			m_pRgbQuad = (LPRGBQUAD)(m_pImage + nFileHeadSize + nInfoHeadSize);

		//λͼ����ָ��
		m_pBuffer = (m_pImage + nFileHeadSize + nInfoHeadSize + sizeof(RGBQUAD)*nColorTable);
	}
}

void BitmapStruct::Allocate(int nBitmapSize)
{
	//��Ϊ�ղ�������,û�ͷŻ�����ڴ�й¶
	if (m_pImage == NULL && nBitmapSize)
	{
		m_pImage = new BYTE[nBitmapSize];
		error(m_pImage, "�����ڴ�ʧ��");
		ZeroMemory(m_pImage, nBitmapSize);
	}
}

void BitmapStruct::Release()
{
	//�ͷ��ڴ������
	if (m_pImage)
		delete[] m_pImage;

	m_pImage = NULL;
	m_pFileHeader = NULL;
	m_pInfoHeader = NULL;
	m_pRgbQuad = NULL;
	m_pBuffer = NULL;
}

void BitmapStruct::CopyTo(BitmapStruct& pThis)
{
	//��ǰλͼ���ݶ��ǿյĻ�
	if (m_pImage == NULL) return;

	//�ȳ����ͷ�
	pThis.Release();

	//��ȡ��С
	DWORD AllSize = m_pFileHeader->bfOffBits + m_pInfoHeader->biSizeImage;

	//�����С
	pThis.Allocate(AllSize);

	//�����ڴ�
	CopyMemory(pThis.m_pImage, this->m_pImage, AllSize);

	//��ʼ���ṹָ��
	pThis.Initialize();
}