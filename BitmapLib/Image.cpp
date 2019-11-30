#include "Image.h"
using namespace ImageSpace;

Image::Image(){}

Image::Image(BitmapStruct& cImage)
{
	m_cImage.Release();
	m_cImage = cImage;
}

Image::Image(Image& cImage)
{
	m_cImage.Release();
	m_cImage = cImage.m_cImage;
}

Image& Image::operator=(Image& cImage)
{
	m_cImage = cImage.m_cImage;
	return *this;
}

Image& Image::operator=(BitmapStruct& cImage)
{
	m_cImage = cImage;
	return *this;
}

Image::~Image(){}

bool Image::ReadBitmap(const char* szBitmapPath)
{
	///��λͼ�ļ�
	HANDLE hFile = CreateFileA(szBitmapPath, GENERIC_READ,
		FILE_SHARE_READ, NULL, OPEN_EXISTING, NULL, NULL);
	if (hFile == INVALID_HANDLE_VALUE)return false;

	///��ȡλͼ�ļ��Ĵ�С
	int nBitmapSize = GetFileSize(hFile, NULL);
	if (!nBitmapSize)
	{
		CloseHandle(hFile);
		return false;
	}

	///�����ڴ���g����λͼ����
	m_cImage.Release();
	m_cImage.Allocate(nBitmapSize);

	///��ȡλͼ����
	DWORD dwRead = 0;
	ReadFile(hFile, m_cImage.m_pImage, nBitmapSize, &dwRead, NULL);
	if (!dwRead)
	{
		CloseHandle(hFile);
		return false;
	}

	///��ʼ��λ�Dָ�
	m_cImage.Initialize();

	///�ر��ļ����
	CloseHandle(hFile);

	return true;
}

bool Image::WriteBitmap(const char* szBitmapPath)
{
	///�����ļ�
	HANDLE hFile = CreateFileA(szBitmapPath, GENERIC_WRITE,
		FILE_SHARE_WRITE, NULL, CREATE_ALWAYS, NULL, NULL);
	if (hFile == INVALID_HANDLE_VALUE) return false;

	///��ȡλͼ���ļ��Ĵ�С
	int nBitmapSize = m_cImage.m_pFileHeader->bfOffBits + m_cImage.m_pInfoHeader->biSizeImage;

	///��λͼ����д���ļ�
	DWORD dwWrite = 0;
	WriteFile(hFile, m_cImage.m_pImage, nBitmapSize, &dwWrite, NULL);
	if (!dwWrite)
	{
		CloseHandle(hFile);
		return false;
	}

	///����ˢ�µ��ļ�
	FlushFileBuffers(hFile);

	///�P�]�ļ�
	CloseHandle(hFile);

	return true;
}

HPALETTE Image::CreateBitmapPalete()
{
	///��ȡ��ɫ����
	int nColorTableLen = 0;
	switch (m_cImage.m_pInfoHeader->biBitCount)
	{
	case 1:nColorTableLen = 2; break;
	case 4:nColorTableLen = 16; break;
	case 8:nColorTableLen = 256; break;
	}
	if (!nColorTableLen) return NULL;

	///�����ɫ���ڴ�
	LPLOGPALETTE pLogPalete = (LPLOGPALETTE)VirtualAlloc(NULL, nColorTableLen, PAGE_READWRITE, MEM_COMMIT);
	if (!pLogPalete)return NULL;

	///��ɫ�����
	pLogPalete->palVersion = 0x300;
	pLogPalete->palNumEntries = nColorTableLen;
	for (int i = 0; i < nColorTableLen; i++)
	{
		pLogPalete->palPalEntry[i].peBlue = m_cImage.m_pRgbQuad[i].rgbBlue;
		pLogPalete->palPalEntry[i].peGreen = m_cImage.m_pRgbQuad[i].rgbGreen;
		pLogPalete->palPalEntry[i].peRed = m_cImage.m_pRgbQuad[i].rgbRed;
		pLogPalete->palPalEntry[i].peFlags = m_cImage.m_pRgbQuad[i].rgbReserved;
	}

	///������ɫ����
	HPALETTE hPalete = CreatePalette(pLogPalete);

	///�ͷ��ڴ�
	VirtualFree(pLogPalete, NULL, MEM_RELEASE);

	///���ؾ��
	return hPalete;
}

bool Image::ToGray()
{
	//��ȡԴλͼ������λ
	int nSrcPiexlByte = m_cImage.m_pInfoHeader->biBitCount / 8;

	//���8λռһ�����أ���λͼ�������һ�ŻҶ�ͼ���߶�ֵͼ
	if (nSrcPiexlByte == 1) return false;

	//��ȡԴλͼ�Ŀ�Ⱥ͸߶�
	int nSrcWidth = m_cImage.m_pInfoHeader->biWidth;
	int nSrcHeight = m_cImage.m_pInfoHeader->biHeight;

	//��ȡԴλͼһ������Ҫ�Ĵ�С
	int nSrcLineByte = (nSrcWidth*nSrcPiexlByte + 3) / 4 * 4;

	//Ŀ��λͼ������λ����ΪҪת��Ϊ�Ҷ�ͼ������8λ��ʾһ�����ؾ�OK
	int nDestPiexlByte = 8;

	//Ŀ��λͼһ������Ĵ�С
	int nDestLineByte = (nSrcWidth*nDestPiexlByte / 8 + 3) / 4 * 4;

	//��ȡ��ɫ�����ɫ����
	int nColorTable = 256;

	//��ȡĿ��λͼ��С
	int nDestImageSize = (nDestLineByte * nSrcHeight);
	nDestImageSize += sizeof(BITMAPFILEHEADER);
	nDestImageSize += sizeof(BITMAPINFOHEADER);
	nDestImageSize += (nColorTable * sizeof(RGBQUAD));

	//�����ڴ�
	BitmapStruct cDestImage;
	cDestImage.Allocate(nDestImageSize);
	cDestImage.Initialize(nDestPiexlByte);

	//�ȱ���λͼ��Ϣ
	CopyMemory(cDestImage.m_pFileHeader, m_cImage.m_pFileHeader, sizeof(BITMAPFILEHEADER));
	CopyMemory(cDestImage.m_pInfoHeader, m_cImage.m_pInfoHeader, sizeof(BITMAPINFOHEADER));

	//��ɫ�����
	for (int i = 0; i < nColorTable; i++)
	{
		cDestImage.m_pRgbQuad[i].rgbBlue = i;
		cDestImage.m_pRgbQuad[i].rgbGreen = i;
		cDestImage.m_pRgbQuad[i].rgbRed = i;
		cDestImage.m_pRgbQuad[i].rgbReserved = 0;
	}

	//��λͼ�ҶȻ�
	char cRed = 0, cGreen = 0, cBule = 0;
	for (int i = 0; i < nSrcHeight; i++)
	{
		for (int j = 0; j < nSrcWidth; j++)
		{
			cRed = static_cast<char>(m_cImage.m_pBuffer[i*nSrcLineByte + j * nSrcPiexlByte + 0] * 0.11f);
			cGreen = static_cast<char>(m_cImage.m_pBuffer[i*nSrcLineByte + j * nSrcPiexlByte + 1] * 0.59f);
			cBule = static_cast<char>(m_cImage.m_pBuffer[i*nSrcLineByte + j * nSrcPiexlByte + 2] * 0.30f);
			cDestImage.m_pBuffer[i*nDestLineByte + j] = static_cast<char>(cRed + cGreen + cBule + 0.50f);
		}
	}

	//����ƫ��
	cDestImage.m_pFileHeader->bfOffBits = (nColorTable * sizeof(RGBQUAD));
	cDestImage.m_pFileHeader->bfOffBits += sizeof(BITMAPFILEHEADER);
	cDestImage.m_pFileHeader->bfOffBits += sizeof(BITMAPINFOHEADER);
	
	//�������ش�С
	cDestImage.m_pInfoHeader->biBitCount = nDestPiexlByte;

	//����λͼ�ܴ�С
	cDestImage.m_pFileHeader->bfSize = nDestImageSize;

	//����λͼ���ݴ�С
	cDestImage.m_pInfoHeader->biSizeImage = (nDestLineByte * nSrcHeight);

	//������ɫ����
	cDestImage.m_pInfoHeader->biClrUsed = nColorTable;
	cDestImage.m_pInfoHeader->biClrImportant = nColorTable;

	//��������
	m_cImage = cDestImage;

	return true;
}

bool Image::ToBinaryzation(int nThreshold)
{
	//��ȡԴλͼ����λ
	int nSrcPiexlByte = m_cImage.m_pInfoHeader->biBitCount / 8;

	//����Ҷ�ͼ���ܽ��ж�ֵ��
	if (nSrcPiexlByte != 1) return false;

	//��ȡԴλͼ�Ŀ�Ⱥ͸߶�
	int nSrcWidth = m_cImage.m_pInfoHeader->biWidth;
	int nSrcHeight = m_cImage.m_pInfoHeader->biHeight;

	//��ȡԴλͼһ�е����ش�С
	int nSrcLineByte = (nSrcWidth*nSrcPiexlByte + 3) / 4 * 4;

	//��λͼ��ֵ��
	for (int i = 0; i < nSrcHeight; i++)
	{
		for (int j = 0; j < nSrcWidth; j++)
		{
			if (m_cImage.m_pBuffer[i*nSrcLineByte + j] <= nThreshold)
				m_cImage.m_pBuffer[i*nSrcLineByte + j] = 0;
			else
				m_cImage.m_pBuffer[i*nSrcLineByte + j] = 255;
		}
	}

	return true;
}

bool Image::ColorEnhancement(int nThreshold)
{
	//��ȡԴλͼ����λ
	int nSrcPiexlByte = m_cImage.m_pInfoHeader->biBitCount / 8;

	//��ȡԴλͼ�Ŀ�Ⱥ͸߶�
	int nSrcWidth = m_cImage.m_pInfoHeader->biWidth;
	int nSrcHeight = m_cImage.m_pInfoHeader->biHeight;

	//��ȡԴλͼһ�е����ش�С
	int nSrcLineByte = (nSrcWidth*nSrcPiexlByte + 3) / 4 * 4;

	//��λͼ��ֵ��
	for (int i = 0; i < nSrcHeight; i++)
	{
		for (int j = 0; j < nSrcWidth; j++)
		{
			for (int n = 0; n < nSrcPiexlByte; n++)
			{
				if (m_cImage.m_pBuffer[i * nSrcLineByte + j * nSrcPiexlByte + n] < nThreshold) 
					m_cImage.m_pBuffer[i * nSrcLineByte + j * nSrcPiexlByte + n] = 255;
				else
					m_cImage.m_pBuffer[i * nSrcLineByte + j * nSrcPiexlByte + n] = 0;
			}
		}
	}

	return true;
}

bool Image::InvertColors()
{
	//��ȡԴλͼ������λ
	int nSrcPiexlByte = m_cImage.m_pInfoHeader->biBitCount / 8;

	//��ȡԴλͼ�Ŀ�Ⱥ͸߶�
	int nSrcWidth = m_cImage.m_pInfoHeader->biWidth;
	int nSrcHeight = m_cImage.m_pInfoHeader->biHeight;

	//��ȡԴλͼһ�е����ش�С
	int nSrcLineByte = (nSrcWidth*nSrcPiexlByte + 3) / 4 * 4;

	//��λͼ��ɫ����
	for (int i = 0; i < nSrcHeight; i++)
	{
		for (int j = 0; j < nSrcWidth; j++)
		{
			for (int n = 0; n < nSrcPiexlByte; n++)
			{
				int nValue = m_cImage.m_pBuffer[i*nSrcLineByte + j * nSrcPiexlByte + n];
				m_cImage.m_pBuffer[i*nSrcLineByte + j * nSrcPiexlByte + n] = 255 - nValue;
			}
		}
	}

	return true;
}

bool Image::SubMatrix(int nPosX, int nPosY, int nWidth, int nHeight)
{
	//��ȡԴλͼ����λ
	int nSrcPiexlByte = m_cImage.m_pInfoHeader->biBitCount / 8;

	//��ȡԴλͼ�Ŀ�Ⱥ͸߶�
	int nSrcWidth = m_cImage.m_pInfoHeader->biWidth;
	int nSrcheight = m_cImage.m_pInfoHeader->biHeight;

	//��ȡԴλͼһ�еĳ���
	int nSrcLineByte = (nSrcWidth*nSrcPiexlByte + 3) / 4 * 4;

	//��ȡĿ��λͼ�Ŀ�Ⱥ͸߶�
	int nDestWidth = nWidth;
	int nDestHeight = nHeight;

	//��ȡĿ��λͼ��λ��
	int nDestBeginX = nPosX;
	int nDestEndX = nPosX + nDestWidth;
	int nDestBeginY = nPosY;
	int nDestEndY = nPosY + nDestHeight;

	//��ȡĿ��λͼ������λ
	int nDestPiexlByte = nSrcPiexlByte;

	//��ȡĿ��λͼ��һ�����ش�С
	int nDestLineByte = (nDestWidth*nSrcPiexlByte + 3) / 4 * 4;

	//��ȡĿ��λͼ�ܴ�С
	int nDestImageSize = nDestLineByte * nDestHeight;
	nDestImageSize += m_cImage.m_pFileHeader->bfOffBits;

	//����ռ�
	BitmapStruct cDestImage;
	cDestImage.Allocate(nDestImageSize);
	cDestImage.Initialize(m_cImage.m_pInfoHeader->biBitCount);

	//����λͼ��Ϣ
	CopyMemory(cDestImage.m_pFileHeader, m_cImage.m_pFileHeader, sizeof(BITMAPFILEHEADER));
	CopyMemory(cDestImage.m_pInfoHeader, m_cImage.m_pInfoHeader, sizeof(BITMAPINFOHEADER));

	//������ڵ�ɫ��
	if (m_cImage.m_pRgbQuad)
	{
		//��ȡ��ɫ���С
		int nSize = m_cImage.m_pFileHeader->bfOffBits;
		nSize -= sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);

		//���Ƶ�ɫ��
		CopyMemory(cDestImage.m_pRgbQuad, m_cImage.m_pRgbQuad, sizeof(RGBQUAD)*nSize);
	}

	//������ȡ
	for (int i = 0; i < nSrcheight; i++)
	{
		for (int j = 0; j < nSrcWidth; j++)
		{
			for (int n = 0; n < nSrcPiexlByte; n++)
			{
				if (i >= nSrcheight - nDestBeginY - nDestHeight &&
					i < nSrcheight - nDestBeginY - nDestHeight + nDestHeight &&
					j >= nDestBeginX &&
					j < nDestEndX)
					cDestImage.m_pBuffer[(i - (nSrcheight - nDestBeginY - nDestHeight))*nDestLineByte + (j - nDestBeginX)* nDestPiexlByte + n]
					= m_cImage.m_pBuffer[i * nSrcLineByte + j * nSrcPiexlByte + n];
			}
		}
	}

	//�޸�Ŀ��ͼ���Ⱥ͸߶�
	cDestImage.m_pInfoHeader->biWidth = nDestWidth;
	cDestImage.m_pInfoHeader->biHeight = nDestHeight;

	//�޸�Ŀ��ͼ����ܴ�С
	cDestImage.m_pFileHeader->bfSize = cDestImage.m_pFileHeader->bfOffBits + (nDestLineByte*nDestHeight);

	//�޸�Ŀ��λͼ���ݴ�С
	cDestImage.m_pInfoHeader->biSizeImage = (nDestLineByte*nDestHeight);

	//����λͼ
	m_cImage = cDestImage;

	return true;
}

bool Image::ToMove(int nPosX, int nPosY, int nColor)
{
	//��ȡԴλͼ������λ��С
	int nSrcPiexlByte = m_cImage.m_pInfoHeader->biBitCount / 8;

	//��ȡԴλͼ�Ŀ�Ⱥ͸߶�
	int nSrcWidth = m_cImage.m_pInfoHeader->biWidth;
	int nSrcHeight = m_cImage.m_pInfoHeader->biHeight;

	//��ȡԴλͼһ�����ش�С
	int nSrcLineByte = (nSrcWidth*nSrcPiexlByte + 3) / 4 * 4;

	//Ŀ��λͼ
	BitmapStruct cDestImage;
	cDestImage.Allocate(m_cImage.m_pFileHeader->bfSize);
	cDestImage.Initialize(m_cImage.m_pInfoHeader->biBitCount);

	//����λͼ��Ϣ
	CopyMemory(cDestImage.m_pFileHeader, m_cImage.m_pFileHeader, sizeof(BITMAPFILEHEADER));
	CopyMemory(cDestImage.m_pInfoHeader, m_cImage.m_pInfoHeader, sizeof(BITMAPINFOHEADER));

	//������ڵ�ɫ��
	if (m_cImage.m_pRgbQuad)
	{
		//��ȡ��ɫ���С
		int nSize = m_cImage.m_pFileHeader->bfOffBits;
		nSize -= sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);

		//���Ƶ�ɫ������
		CopyMemory(cDestImage.m_pRgbQuad, m_cImage.m_pRgbQuad, nSize);
	}

	//��λͼƽ������
	for (int i = 0; i < nSrcHeight; i++)
	{
		for (int j = 0; j < nSrcWidth; j++)
		{
			if (i - nPosY >= 0
				&& i - nPosY <= nSrcHeight
				&& j - nPosX >= 0
				&& j - nPosX <= nSrcWidth)
			{
				for (int n = 0; n < nSrcPiexlByte; n++)
				{
					cDestImage.m_pBuffer[i * nSrcLineByte + j * nSrcPiexlByte + n] =
						m_cImage.m_pBuffer[(i - nPosY) * nSrcLineByte + (j - nPosX) * nSrcPiexlByte + n];
				}
			}
			else
			{
				for (int n = 0; n < nSrcPiexlByte; n++)
				{
					cDestImage.m_pBuffer[i * nSrcLineByte + j * nSrcPiexlByte + n] = nColor;
				}
			}
		}
	}

	//����λͼ����
	m_cImage = cDestImage;

	return true;
}

bool Image::ToScale(float fMultipleX, float fMultipleY)
{
	//��ȡԴλͼ������λ
	int nSrcPiexlByte = m_cImage.m_pInfoHeader->biBitCount / 8;

	//��ȡԴλͼ�Ŀ�Ⱥ͸߶�
	int nSrcWidth = m_cImage.m_pInfoHeader->biWidth;
	int nSrcHeight = m_cImage.m_pInfoHeader->biHeight;

	//��ȡԴλͼ��һ�е����ش�С
	int nSrcLineByte = (nSrcWidth*nSrcPiexlByte + 3) / 4 * 4;

	//��ȡĿ��λͼ�Ŀ�Ⱥ͸߶�
	int nDestWidth = static_cast<int>(nSrcWidth*fMultipleX + 0.5f);
	int nDestHeight = static_cast<int>(nSrcHeight*fMultipleY + 0.5f);

	//��ȡĿ��λͼ��һ�е����ش�С
	int nDestLineByte = (nDestWidth*nSrcPiexlByte + 3) / 4 * 4;

	//Ŀ��λͼ��С
	int nDestImageSize = nDestLineByte * nDestHeight;
	nDestImageSize += m_cImage.m_pFileHeader->bfOffBits;

	//�����ڴ�
	BitmapStruct cDestImage;
	cDestImage.Allocate(nDestImageSize);
	cDestImage.Initialize(m_cImage.m_pInfoHeader->biBitCount);

	//��ֵλͼ����
	CopyMemory(cDestImage.m_pFileHeader, m_cImage.m_pFileHeader, sizeof(BITMAPFILEHEADER));
	CopyMemory(cDestImage.m_pInfoHeader, m_cImage.m_pInfoHeader, sizeof(BITMAPINFOHEADER));

	//������ڵ�ɫ��
	if (m_cImage.m_pRgbQuad)
	{
		//��ȡ��ɫ���С
		int nSize = m_cImage.m_pFileHeader->bfOffBits;
		nSize -= sizeof(BITMAPFILEHEADER)+ sizeof(BITMAPINFOHEADER);

		//���Ƶ�ɫ������
		CopyMemory(cDestImage.m_pRgbQuad, m_cImage.m_pRgbQuad, nSize);
	}

	//Ŀ��λͼ��Ⱥ͸߶ȵĲ�ֵ
	int nCoordinateX = 0, nCoordinateY = 0;

	for (int i = 0; i < nDestHeight; i++)
	{
		for (int j = 0; j < nDestWidth; j++)
		{
			//����ӳ��õ���ֵλ��
			nCoordinateX = static_cast<int>(j / fMultipleX + 0.5f);
			nCoordinateY = static_cast<int>(i / fMultipleY + 0.5f);

			if (nCoordinateX >= 0
				&& nCoordinateX < nSrcWidth
				&& nCoordinateY >= 0
				&& nCoordinateY < nSrcHeight)
			{
				for (int n = 0; n < nSrcPiexlByte; n++)
				{
					cDestImage.m_pBuffer[i * nDestLineByte + j * nSrcPiexlByte + n] =
						m_cImage.m_pBuffer[nCoordinateY * nSrcLineByte + nCoordinateX * nSrcPiexlByte + n];
				}
			}
		}
	}

	//����Ŀ��λͼ��Ⱥ͸߶�
	cDestImage.m_pInfoHeader->biWidth = nDestWidth;
	cDestImage.m_pInfoHeader->biHeight = nDestHeight;

	//����Ŀ��λͼ�ܴ�С
	cDestImage.m_pFileHeader->bfSize = nDestImageSize;

	//��ϵĿ��λͼ���ݴ�С
	cDestImage.m_pInfoHeader->biSizeImage = nDestImageSize - m_cImage.m_pFileHeader->bfOffBits;

	//��������
	m_cImage = cDestImage;

	return true;
}

bool Image::LevelMirror()
{
	//��ȡԴλͼ������λ
	int nSrcPiexlByte = m_cImage.m_pInfoHeader->biBitCount / 8;

	//��ȡԴλͼ�Ŀ�Ⱥ͸߶�
	int nSrcWidth = m_cImage.m_pInfoHeader->biWidth;
	int nSrcHeight = m_cImage.m_pInfoHeader->biHeight;

	//��ȡԴλͼһ�е����ش�С
	int nSrcLineByte = (nSrcWidth*nSrcPiexlByte + 3) / 4 * 4;

	//��ȡĿ��λͼ������λ
	int nDestPiexlByte = nSrcPiexlByte;

	//��ȡĿ��λͼ��Ⱥ͸߶�
	int nDestWidth = nSrcWidth;
	int nDestHeight = nSrcHeight;

	//��ȡĿ��λͼһ�е����ش�С
	int nDestLineByte = nSrcLineByte;

	//��ȡĿ��λͼ�ܴ�С
	int nDestImageSize = m_cImage.m_pFileHeader->bfSize;

	//����ռ�
	BitmapStruct cDestImage;
	cDestImage.Allocate(nDestImageSize);
	cDestImage.Initialize(m_cImage.m_pInfoHeader->biBitCount);

	//����λͼ��Ϣ
	CopyMemory(cDestImage.m_pFileHeader, m_cImage.m_pFileHeader, sizeof(BITMAPFILEHEADER));
	CopyMemory(cDestImage.m_pInfoHeader, m_cImage.m_pInfoHeader, sizeof(BITMAPINFOHEADER));

	//������ڵ�ɫ��
	if (m_cImage.m_pRgbQuad)
	{
		//��ȡ��ɫ���С
		int nSize = m_cImage.m_pFileHeader->bfOffBits;
		nSize -= sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);

		//���Ƶ�ɫ����Ϣ
		CopyMemory(cDestImage.m_pRgbQuad, m_cImage.m_pRgbQuad, nSize);
	}

	//ˮƽ����
	for (int i = 0; i < nDestHeight; i++)
	{
		for (int j = 0; j < nDestWidth; j++)
		{
			for (int n = 0; n < nDestPiexlByte; n++)
				cDestImage.m_pBuffer[i*nDestLineByte + j * nDestPiexlByte + n] =
				m_cImage.m_pBuffer[i*nSrcLineByte + (nSrcWidth - 1 - j)*nSrcPiexlByte + n];
		}
	}

	//��������
	m_cImage = cDestImage;

	return true;
}

bool Image::VerticalMirror()
{
	//��ȡԴλͼ������λ
	int nSrcPiexlByte = m_cImage.m_pInfoHeader->biBitCount / 8;

	//��ȡԴλͼ�Ŀ�Ⱥ͸߶�
	int nSrcWidth = m_cImage.m_pInfoHeader->biWidth;
	int nSrcHeight = m_cImage.m_pInfoHeader->biHeight;

	//��ȡԴλͼһ�е����ش�С
	int nSrcLineByte = (nSrcWidth*nSrcPiexlByte + 3) / 4 * 4;

	//��ȡĿ��λͼ������λ
	int nDestPiexlByte = nSrcPiexlByte;

	//��ȡĿ��λͼ��Ⱥ͸߶�
	int nDestWidth = nSrcWidth;
	int nDestHeight = nSrcHeight;

	//��ȡĿ��λͼһ�е����ش�С
	int nDestLineByte = nSrcLineByte;

	//��ȡĿ��λͼ�ܴ�С
	int nDestImageSize = m_cImage.m_pFileHeader->bfSize;

	//����ռ�
	BitmapStruct cDestImage;
	cDestImage.Allocate(nDestImageSize);
	cDestImage.Initialize(m_cImage.m_pInfoHeader->biBitCount);

	//����λͼ��Ϣ
	CopyMemory(cDestImage.m_pFileHeader, m_cImage.m_pFileHeader, sizeof(BITMAPFILEHEADER));
	CopyMemory(cDestImage.m_pInfoHeader, m_cImage.m_pInfoHeader, sizeof(BITMAPINFOHEADER));

	//������ڵ�ɫ��
	if (m_cImage.m_pRgbQuad)
	{
		//��ȡ��ɫ���С
		int nSize = m_cImage.m_pFileHeader->bfOffBits;
		nSize -= sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);

		//���Ƶ�ɫ����Ϣ
		CopyMemory(cDestImage.m_pRgbQuad, m_cImage.m_pRgbQuad, nSize);
	}

	//ˮƽ����
	for (int i = 0; i < nDestHeight; i++)
	{
		for (int j = 0; j < nDestWidth; j++)
		{
			for (int n = 0; n < nDestPiexlByte; n++)
				cDestImage.m_pBuffer[i*nDestLineByte + j * nDestPiexlByte + n] =
				m_cImage.m_pBuffer[(nSrcHeight - 1 - i)*nSrcLineByte + j * nSrcPiexlByte + n];
		}
	}

	//��������
	m_cImage = cDestImage;

	return true;
}

bool Image::Rotate(float fAngle, int nColor /*= 255*/)
{
	//��ȡԴλͼ������λ
	int nSrcPiexlByte = m_cImage.m_pInfoHeader->biBitCount / 8;

	//��ȡԴλͼ�Ŀ�Ⱥ͸߶�
	int nSrcWidth = m_cImage.m_pInfoHeader->biWidth;
	int nSrcHeight = m_cImage.m_pInfoHeader->biHeight;

	//��ȡԴλͼһ�����صĴ�С
	int nSrcLineByte = (nSrcPiexlByte*nSrcWidth + 3) / 4 * 4;

	//��ȡĿ��λͼ������λ
	int nDestPiexlByte = nSrcPiexlByte;

	//����ת�Ƕ�ת��Ϊ����
	float fRotateAngle = 2.0f * 3.1415926f*fAngle / 360.0f;

	//Դλͼ���ĸ���λ��
	float fSrcX1, fSrcY1, fSrcX2, fSrcY2, fSrcX3, fSrcY3, fSrcX4, fSrcY4;

	//Ŀ��λͼ����ת����ĸ���
	float fDestX1, fDestY1, fDestX2, fDestY2, fDestX3, fDestY3, fDestX4, fDestY4;

	//������ת�Ƕȵ�����
	float fSin = static_cast<float>(sin(fRotateAngle));

	//������ת�Ƕȵ�����
	float fCos = static_cast<float>(cos(fRotateAngle));

	// ����ԭͼ���ĸ��ǵ����꣬��ͼ������Ϊ����ϵԭ��
	fSrcX1 = static_cast<float>(-(nSrcWidth - 1.0f) / 2.0f);
	fSrcY1 = static_cast<float>((nSrcHeight - 1.0f) / 2.0f);
	fSrcX2 = static_cast<float>((nSrcWidth - 1.0f) / 2.0f);
	fSrcY2 = static_cast<float>((nSrcHeight - 1.0f) / 2.0f);
	fSrcX3 = static_cast<float>(-(nSrcWidth - 1.0f) / 2.0f);
	fSrcY3 = static_cast<float>(-(nSrcHeight - 1.0f) / 2.0f);
	fSrcX4 = static_cast<float>((nSrcWidth - 1.0f) / 2.0f);
	fSrcY4 = static_cast<float>(-(nSrcHeight - 1.0f) / 2.0f);

	// ������ͼ�ĸ��ǵ����꣬��ͼ������Ϊ����ϵԭ��
	fDestX1 = fCos * fSrcX1 + fSin * fSrcY1;
	fDestY1 = -fSin * fSrcX1 + fCos * fSrcY1;
	fDestX2 = fCos * fSrcX2 + fSin * fSrcY2;
	fDestY2 = -fSin * fSrcX2 + fCos * fSrcY2;
	fDestX3 = fCos * fSrcX3 + fSin * fSrcY3;
	fDestY3 = -fSin * fSrcX3 + fCos * fSrcY3;
	fDestX4 = fCos * fSrcX4 + fSin * fSrcY4;
	fDestY4 = -fSin * fSrcX4 + fCos * fSrcY4;

	//��ȡĿ��λͼ�Ŀ�Ⱥ͸߶�
	int nDestWidth = static_cast<int>(max(fabs(fDestX4 - fDestX1), fabs(fDestX3 - fDestX2)) + 0.5f);
	int nDestHeight = static_cast<int>(max(fabs(fDestY4 - fDestY1), fabs(fDestY3 - fDestY2)) + 0.5f);

	//��ȡĿ��λͼ��һ�е����ش�С
	int nDestLineByte = (nDestWidth * nDestPiexlByte + 3) / 4 * 4;

	//��ȡĿ��λͼ�Ĵ�С
	int nDestImageSize = nDestLineByte * nDestHeight;
	nDestImageSize += m_cImage.m_pFileHeader->bfOffBits;

	//�����ڴ�
	BitmapStruct cDestImage;
	cDestImage.Allocate(nDestImageSize);
	cDestImage.Initialize(m_cImage.m_pInfoHeader->biBitCount);

	//����λͼ��Ϣ
	CopyMemory(cDestImage.m_pFileHeader, m_cImage.m_pFileHeader, sizeof(BITMAPFILEHEADER));
	CopyMemory(cDestImage.m_pInfoHeader, m_cImage.m_pInfoHeader, sizeof(BITMAPINFOHEADER));

	//������ڵ�ɫ��
	if (m_cImage.m_pRgbQuad)
	{
		//��ȡ��ɫ���С
		int nSize = m_cImage.m_pFileHeader->bfOffBits;
		nSize -= sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);

		//���Ƶ�ɫ������
		CopyMemory(cDestImage.m_pRgbQuad, m_cImage.m_pRgbQuad, nSize);
	}

	//����
	float fConstData1 = (-0.5f * (nDestWidth - 1.0f) * fCos
		+ 0.5f * (nDestHeight - 1.0f) * fSin + 0.5f * (nSrcWidth - 1.0f));
	float fConstData2 = (-0.5f * (nDestWidth - 1.0f) * fSin
		- 0.5f * (nDestHeight - 1.0f) * fCos + 0.5f * (nSrcHeight - 1.0f));

	for (int i = 0; i < nDestHeight; i++)
	{
		for (int j = 0; j < nDestWidth; j++)
		{

			//��ȡ�ٽ���ֵ
			int nCorrdinateX = static_cast<int>(j * fCos - i * fSin + fConstData1 + 0.5f);
			int nCoordinateY = static_cast<int>(j * fSin + i * fCos + fConstData2 + 0.5f);

			//����ڷ�Χ��
			if (nCorrdinateX >= 0
				&& nCorrdinateX < nSrcWidth
				&& nCoordinateY >= 0
				&& nCoordinateY < nSrcHeight)
			{
				for (int n = 0; n < nDestPiexlByte; n++)
				{
					cDestImage.m_pBuffer[i*nDestLineByte + j * nDestPiexlByte + n] =
						m_cImage.m_pBuffer[nCoordinateY*nSrcLineByte + nCorrdinateX * nSrcPiexlByte + n];
				}
			}
			else
			{
				for (int n = 0; n < nDestPiexlByte; n++)
				{
					cDestImage.m_pBuffer[i*nDestLineByte + j * nDestPiexlByte + n] = nColor;
				}
			}
		}
	}

	//����λͼ��С
	cDestImage.m_pInfoHeader->biWidth = nDestWidth;
	cDestImage.m_pInfoHeader->biHeight = nDestHeight;

	//����λͼ�ܴ�С
	cDestImage.m_pFileHeader->bfSize = nDestImageSize;

	//����λͼ���ݴ�С
	cDestImage.m_pInfoHeader->biSizeImage = nDestImageSize - m_cImage.m_pFileHeader->bfOffBits;

	//���ݸ���
	m_cImage = cDestImage;

	return true;
}

bool Image::AddSpicedSaltNoise()
{
	//��ȡԴλͼ������λ
	int nSrcPiexlByte = m_cImage.m_pInfoHeader->biBitCount;

	//��ȡԴλͼ�Ŀ�Ⱥ͸߶�
	int nSrcWidth = m_cImage.m_pInfoHeader->biWidth;
	int nSrcHeight = m_cImage.m_pInfoHeader->biHeight;

	//��ȡ�������ֽ���
	int nDataCount = nSrcWidth * nSrcHeight*nSrcPiexlByte / 8;

	//�����8λ
	if (nSrcPiexlByte == 8)
	{
		for (int i = 0; i < nDataCount; i++)
		{
			if (rand() > 32000) m_cImage.m_pBuffer[i] = 0;
			if (rand() < 200) m_cImage.m_pBuffer[i] = 255;
		}
	}
	else if (nSrcPiexlByte == 24)
	{
		for (int i = 0; i < nDataCount; i += 3)
		{
			int nTemp = rand();
			if (nTemp > 32000 || nTemp < 200)
			{
				m_cImage.m_pBuffer[i] = rand() % 255;
				m_cImage.m_pBuffer[i + 1] = rand() % 255;
				m_cImage.m_pBuffer[i + 2] = rand() % 255;
			}
		}
	}
	return true;
}

bool Image::AddGaussNoise()
{
	//��ȡԴλͼ������λ
	int nSrcPiexlByte = m_cImage.m_pInfoHeader->biBitCount;

	//ֻ����8λ
	if (nSrcPiexlByte != 8)return false;

	//��ȡԴλͼ�Ŀ�Ⱥ͸߶�
	int nSrcWidth = m_cImage.m_pInfoHeader->biWidth;
	int nSrcHeight = m_cImage.m_pInfoHeader->biHeight;

	//��ȡ�������ֽ���
	int nDataCount = nSrcWidth * nSrcHeight*nSrcPiexlByte / 8;

	//�������
	for (int i = 0; i < nDataCount; i++)
	{
		int nTemp = rand();
		nTemp = m_cImage.m_pBuffer[i] * 224 / 256 + nTemp / 512;
		m_cImage.m_pBuffer[i] = nTemp >= 255 ? 255 : nTemp;
	}

	return true;
}

bool Image::WhiteGradeSharp(int nThresh)
{
	//��ȡԴλͼ������λ
	int nSrcPiexlByte = m_cImage.m_pInfoHeader->biBitCount;

	//ֻ����8λ
	if (nSrcPiexlByte != 8)return false;

	//��ȡԴλͼ�Ŀ�Ⱥ͸߶�
	int nSrcWidth = m_cImage.m_pInfoHeader->biWidth;
	int nSrcHeight = m_cImage.m_pInfoHeader->biHeight;

	//��ȡԴλͼһ�е����ش�С
	int nSrcLineByte = (nSrcWidth*nSrcPiexlByte / 8 + 3) / 4 * 4;

	//��λͼ
	BitmapStruct cDestImage;
	cDestImage.Allocate(m_cImage.m_pFileHeader->bfSize);
	cDestImage.Initialize(m_cImage.m_pInfoHeader->biBitCount);

	//����λͼ��Ϣ
	CopyMemory(cDestImage.m_pFileHeader, m_cImage.m_pFileHeader, sizeof(BITMAPFILEHEADER));
	CopyMemory(cDestImage.m_pInfoHeader, m_cImage.m_pInfoHeader, sizeof(BITMAPINFOHEADER));

	//������ڵ�ɫ��
	if (m_cImage.m_pRgbQuad)
	{
		//��ȡ��ɫ���С
		int nSize = m_cImage.m_pFileHeader->bfOffBits;
		nSize -= sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);

		//���Ƶ�ɫ������
		CopyMemory(cDestImage.m_pRgbQuad, m_cImage.m_pRgbQuad, nSize);
	}

	for (int i = 0; i < nSrcHeight - 1; i++)
	{
		for (int j = 0; j < nSrcWidth; j++)
		{
			//�����ݶ�����
			char cTemp = m_cImage.m_pBuffer[(nSrcHeight - 1 - i)*nSrcLineByte + j];
			char cTemp1 = m_cImage.m_pBuffer[(nSrcHeight - 2 - i)*nSrcLineByte + j];
			char cTemp2 = m_cImage.m_pBuffer[(nSrcHeight - 1 - i)*nSrcLineByte + j + 1];

			int nTemp = abs(cTemp - cTemp1) + abs(cTemp - cTemp2);
			int nSave = 255;

			if ((nTemp + 120) < 255)
			{
				if (nTemp >= nThresh)
				{
					nSave = nTemp + 120;
				}
			}

			m_cImage.m_pBuffer[(nSrcHeight - 1 - i)*nSrcLineByte + j] = nSave;
			cDestImage.m_pBuffer[(nSrcHeight - 1 - i)*nSrcLineByte + j] = nSave;
		}
	}

	//��������
	m_cImage = cDestImage;

	return true;
}

bool Image::GradeSharp(int nThresh)
{
	//��ȡԴλͼ������λ
	int nSrcPiexlByte = m_cImage.m_pInfoHeader->biBitCount;

	//ֻ����8λ
	if (nSrcPiexlByte != 8)return false;

	//��ȡԴλͼ�Ŀ�Ⱥ͸߶�
	int nSrcWidth = m_cImage.m_pInfoHeader->biWidth;
	int nSrcHeight = m_cImage.m_pInfoHeader->biHeight;

	//��ȡԴλͼһ�е����ش�С
	int nSrcLineByte = (nSrcWidth*nSrcPiexlByte / 8 + 3) / 4 * 4;

	//��λͼ
	BitmapStruct cDestImage;
	cDestImage.Allocate(m_cImage.m_pFileHeader->bfSize);
	cDestImage.Initialize(m_cImage.m_pInfoHeader->biBitCount);

	//����λͼ��Ϣ
	CopyMemory(cDestImage.m_pFileHeader, m_cImage.m_pFileHeader, sizeof(BITMAPFILEHEADER));
	CopyMemory(cDestImage.m_pInfoHeader, m_cImage.m_pInfoHeader, sizeof(BITMAPINFOHEADER));

	//������ڵ�ɫ��
	if (m_cImage.m_pRgbQuad)
	{
		//��ȡ��ɫ���С
		int nSize = m_cImage.m_pFileHeader->bfOffBits;
		nSize -= sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);

		//���Ƶ�ɫ������
		CopyMemory(cDestImage.m_pRgbQuad, m_cImage.m_pRgbQuad, nSize);
	}

	for (int i = 0; i < nSrcHeight - 1; i++)
	{
		for (int j = 0; j < nSrcWidth; j++)
		{
			//�����ݶ�����
			char cTemp = m_cImage.m_pBuffer[(nSrcHeight - 1 - i)*nSrcLineByte + j];
			char cTemp1 = m_cImage.m_pBuffer[(nSrcHeight - 2 - i)*nSrcLineByte + j];
			char cTemp2 = m_cImage.m_pBuffer[(nSrcHeight - 1 - i)*nSrcLineByte + j + 1];

			int nTemp = abs(cTemp - cTemp1) + abs(cTemp - cTemp2);

			if ((nTemp + 120) < 255)
			{
				if (nTemp >= nThresh)
				{
					m_cImage.m_pBuffer[(nSrcHeight - 1 - i)*nSrcLineByte + j] = nTemp + 120;

				}
			}
			else
				m_cImage.m_pBuffer[(nSrcHeight - 1 - i)*nSrcLineByte + j] = 255;

			cDestImage.m_pBuffer[(nSrcHeight - 1 - i)*nSrcLineByte + j] = m_cImage.m_pBuffer[(nSrcHeight - 1 - i)*nSrcLineByte + j];
		}
	}

	//��������
	m_cImage = cDestImage;

	return true;
}

bool Image::ThresholdWindow(int nTop, int nButtom)
{
	//��ȡԴλͼ������λ
	int nSrcPiexlByte = m_cImage.m_pInfoHeader->biBitCount / 8;

	//��ȡԴλͼ�Ŀ�Ⱥ͸߶�
	int nSrcWidth = m_cImage.m_pInfoHeader->biWidth;
	int nSrcHeight = m_cImage.m_pInfoHeader->biHeight;

	//��ȡԴλͼһ�е����ش�С
	int nSrcLineByte = (nSrcWidth*nSrcPiexlByte + 3) / 4 * 4;

	for (int i = 0; i < nSrcHeight; i++)
	{
		for (int j = 0; j < nSrcWidth; j++)
		{
			for (int n = 0; n < nSrcPiexlByte; n++)
			{
				if (m_cImage.m_pBuffer[i*nSrcLineByte + j * nSrcPiexlByte + n] < nButtom)
					m_cImage.m_pBuffer[i*nSrcLineByte + j * nSrcPiexlByte + n] = 0;
				else if (m_cImage.m_pBuffer[i*nSrcLineByte + j * nSrcPiexlByte + n] > nTop)
					m_cImage.m_pBuffer[i*nSrcLineByte + j * nSrcPiexlByte + n] = 255;
			}
		}
	}

	return true;
}

bool Image::LinearStrech(int nPosX1, int nPosY1, int nPosX2, int nPosY2)
{
	//��ȡԴλͼ������λ
	int nSrcPiexlByte = m_cImage.m_pInfoHeader->biBitCount / 8;

	//��ȡԴλͼ�Ŀ�Ⱥ͸߶�
	int nSrcWidth = m_cImage.m_pInfoHeader->biWidth;
	int nSrcHeight = m_cImage.m_pInfoHeader->biHeight;

	//��ȡԴλͼһ�е����ش�С
	int nSrcLineByte = (nSrcWidth*nSrcPiexlByte + 3) / 4 * 4;

	//��ȡ�����ֶε�ֱ��б��
	double dStraightSlope1 = nPosY1 / nPosX1;
	double dStraightSlope2 = (nPosY2 - nPosY1) / (nPosX2 - nPosX1);
	double dStraightSlope3 = (255 - nPosY2) / (255 - nPosX2);

	//��ȡ��������б��
	double dCoordinatesSlope1 = 0;
	double dCoordinatesSlope2 = nPosY1 - dStraightSlope2 * nPosX1;
	double dCoordinatesSlope3 = nPosY2 - dStraightSlope3 * nPosX2;

	//ӳ���
	int nMap[256];

	//���ӳ���
	for (int i = 0; i < 256; i++)
	{
		if (i < nPosX1)
			nMap[i] = static_cast<int>(dStraightSlope1*i + dCoordinatesSlope1 + 0.5f);
		else if (i < nPosX2)
			nMap[i] = static_cast<int>(dStraightSlope2*i + dCoordinatesSlope2 + 0.5f);
		else
			nMap[i] = static_cast<int>(dStraightSlope3*i + dCoordinatesSlope3 + 0.5f);
	}

	if (nSrcPiexlByte == 1)//��ͨ��
	{
		//һ�����س���
		int nLineByte = (nSrcWidth + 3) / 4 * 4;

		for (int i = 0; i < nSrcHeight; i++)
		{
			for (int j = 0; j < nSrcWidth; j++)
			{
				char cIndex = m_cImage.m_pBuffer[i*nLineByte + j];
				m_cImage.m_pBuffer[i*nLineByte + j] = nMap[cIndex];
			}
		}

	}
	else if (nSrcPiexlByte == 3)//��ͨ��
	{
		for (int i = 0; i < nSrcHeight; i++)
		{
			for (int j = 0; j < nSrcWidth; j++)
			{
				for (int n = 0; n < nSrcPiexlByte; n++)
				{
					char cIndex = m_cImage.m_pBuffer[i*nSrcLineByte + j * nSrcPiexlByte + n];
					m_cImage.m_pBuffer[i*nSrcLineByte + j*nSrcPiexlByte +n] = nMap[cIndex];
				}
			}
		}
	}

	return true;
}

bool Image::Roberts()
{
	//��ȡԴλͼ������λ
	int nSrcPiexlByte = m_cImage.m_pInfoHeader->biBitCount / 8;

	//��ȡԴλͼ�Ŀ�Ⱥ͸߶�
	int nSrcWidth = m_cImage.m_pInfoHeader->biWidth;
	int nSrcHeight = m_cImage.m_pInfoHeader->biHeight;

	//��ȡԴλͼ��һ������λ
	int nSrcLineByte = (nSrcWidth*nSrcPiexlByte + 3) / 4 * 4;

	for (int i = 1; i < nSrcHeight - 1; i++)
	{
		for (int j = 1; j < nSrcWidth - 1; j++)
		{
			for (int n = 0; n < nSrcPiexlByte; n++)
			{
				//X������ݶ�
				int nDirectX = m_cImage.m_pBuffer[i*nSrcLineByte + j * nSrcPiexlByte + n] 
					- m_cImage.m_pBuffer[(i + 1)*nSrcLineByte + j * nSrcPiexlByte + n];

				//Y������ݶ�
				int nDirectY = m_cImage.m_pBuffer[i*nSrcLineByte + j * nSrcPiexlByte + n]
					- m_cImage.m_pBuffer[i*nSrcLineByte + (j + 1)*nSrcPiexlByte + n];

				int nValue = static_cast<int>(sqrt(nDirectX * nDirectX + nDirectY * nDirectY) + 0.5);
				if (nValue > 255) nValue = 255;

				m_cImage.m_pBuffer[i*nSrcLineByte + j * nSrcPiexlByte + n] = nValue;
			}
		}
	}

	return true;
}

bool Image::Sobel()
{
	//��ȡԴλͼ������λ
	int nSrcPiexlByte = m_cImage.m_pInfoHeader->biBitCount / 8;

	//��ȡԴλͼ�Ŀ�Ⱥ͸߶�
	int nSrcWidth = m_cImage.m_pInfoHeader->biWidth;
	int nSrcHeight = m_cImage.m_pInfoHeader->biHeight;

	//��ȡԴλͼ��һ������λ
	int nSrcLineByte = (nSrcWidth*nSrcPiexlByte + 3) / 4 * 4;

	//��ȡĿ��λͼ������λ
	int nDestPiexlByte = nSrcPiexlByte;

	//��ȡĿ��λͼ�Ŀ�Ⱥ͸߶�
	int nDestWidth = nSrcWidth;
	int nDestHeight = nSrcHeight;

	//��ȡĿ��λͼ��һ�е�����λ
	int nDestLineByte = nSrcLineByte;

	//Ŀ��λͼ
	BitmapStruct cDestImage;
	cDestImage.Allocate(m_cImage.m_pFileHeader->bfSize);
	cDestImage.Initialize(m_cImage.m_pInfoHeader->biBitCount);

	//����λͼ��Ϣ
	CopyMemory(cDestImage.m_pFileHeader, m_cImage.m_pFileHeader, sizeof(BITMAPFILEHEADER));
	CopyMemory(cDestImage.m_pInfoHeader, m_cImage.m_pInfoHeader, sizeof(BITMAPINFOHEADER));

	//������ڵ�ɫ������
	if (m_cImage.m_pRgbQuad)
	{
		//��ȡ��ɫ���С
		int nSize = m_cImage.m_pFileHeader->bfOffBits;
		nSize -= sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);

		//���Ƶ�ɫ������
		CopyMemory(cDestImage.m_pRgbQuad, m_cImage.m_pRgbQuad, nSize);
	}

	for (int i = 1; i < nSrcHeight - 1; i++)
	{
		for (int j = 1; j < nSrcWidth - 1; j++)
		{
			for (int n = 0; n < nSrcPiexlByte; n++)
			{
				//X������ݶ�
				int nDirectX = m_cImage.m_pBuffer[(i - 1)*nSrcLineByte + (j + 1)*nSrcPiexlByte + n]
					+ 2 * m_cImage.m_pBuffer[i*nSrcLineByte + (j + 1)*nSrcPiexlByte + n]
					+ m_cImage.m_pBuffer[(i + 1)*nSrcLineByte + (j + 1)*nSrcPiexlByte + n]
					- m_cImage.m_pBuffer[(i - 1)*nSrcLineByte + (j - 1)*nSrcPiexlByte + n]
					- 2 * m_cImage.m_pBuffer[i*nSrcLineByte + (j - 1)*nSrcPiexlByte + n]
					- m_cImage.m_pBuffer[(i + 1)*nSrcLineByte + (j - 1)*nSrcPiexlByte + n];

				//Y������ݶ�
				int nDirectY = m_cImage.m_pBuffer[(i - 1)*nSrcLineByte + (j - 1)*nSrcPiexlByte + n]
					+ 2 * m_cImage.m_pBuffer[(i - 1)*nSrcLineByte + j * nSrcPiexlByte + n]
					+ m_cImage.m_pBuffer[(i - 1)*nSrcLineByte + (j + 1)*nSrcPiexlByte + n]
					- m_cImage.m_pBuffer[(i + 1)*nSrcLineByte + (j - 1)*nSrcPiexlByte + n]
					- 2 * m_cImage.m_pBuffer[(i + 1)*nSrcLineByte + j * nSrcPiexlByte + n]
					- m_cImage.m_pBuffer[(i + 1)*nSrcLineByte + (j + 1)*nSrcPiexlByte + n];

				int nValue = static_cast<int>(sqrt(nDirectX*nDirectX + nDirectY * nDirectY) + 0.5f);
				if (nValue > 255) nValue = 255;

				cDestImage.m_pBuffer[i*nSrcLineByte + j * nSrcPiexlByte + n] = nValue;
			}
		}
	}

	//��������
	m_cImage = cDestImage;

	return true;
}

bool Image::Prewitt()
{
	//��ȡԴλͼ������λ
	int nSrcPiexlByte = m_cImage.m_pInfoHeader->biBitCount / 8;

	//��ȡԴλͼ�Ŀ�Ⱥ͸߶�
	int nSrcWidth = m_cImage.m_pInfoHeader->biWidth;
	int nSrcHeight = m_cImage.m_pInfoHeader->biHeight;

	//��ȡԴλͼ��һ������λ
	int nSrcLineByte = (nSrcWidth*nSrcPiexlByte + 3) / 4 * 4;

	//��ȡĿ��λͼ������λ
	int nDestPiexlByte = nSrcPiexlByte;

	//��ȡĿ��λͼ�Ŀ�Ⱥ͸߶�
	int nDestWidth = nSrcWidth;
	int nDestHeight = nSrcHeight;

	//��ȡĿ��λͼ��һ�е�����λ
	int nDestLineByte = nSrcLineByte;

	//Ŀ��λͼ
	BitmapStruct cDestImage;
	cDestImage.Allocate(m_cImage.m_pFileHeader->bfSize);
	cDestImage.Initialize(m_cImage.m_pInfoHeader->biBitCount);

	//����λͼ��Ϣ
	CopyMemory(cDestImage.m_pFileHeader, m_cImage.m_pFileHeader, sizeof(BITMAPFILEHEADER));
	CopyMemory(cDestImage.m_pInfoHeader, m_cImage.m_pInfoHeader, sizeof(BITMAPINFOHEADER));

	//������ڵ�ɫ������
	if (m_cImage.m_pRgbQuad)
	{
		//��ȡ��ɫ���С
		int nSize = m_cImage.m_pFileHeader->bfOffBits;
		nSize -= sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);

		//���Ƶ�ɫ������
		CopyMemory(cDestImage.m_pRgbQuad, m_cImage.m_pRgbQuad, nSize);
	}

	for (int i = 1; i < nSrcHeight - 1; i++)
	{
		for (int j = 1; j < nSrcWidth - 1; j++)
		{
			for (int n = 0; n < nSrcPiexlByte; n++)
			{
				//X������ݶ�
				int nDirectX = m_cImage.m_pBuffer[(i - 1)*nSrcLineByte + (j + 1)*nSrcPiexlByte + n]
					+ m_cImage.m_pBuffer[i*nSrcLineByte + (j + 1)*nSrcPiexlByte + n]
					+ m_cImage.m_pBuffer[(i + 1)*nSrcLineByte + (j + 1)*nSrcPiexlByte + n]
					- m_cImage.m_pBuffer[(i - 1)*nSrcLineByte + (j - 1)*nSrcPiexlByte + n]
					- m_cImage.m_pBuffer[i*nSrcLineByte + (j - 1)*nSrcPiexlByte + n]
					- m_cImage.m_pBuffer[(i + 1)*nSrcLineByte + (j - 1)*nSrcPiexlByte + n];

				//Y������ݶ�
				int nDirectY = m_cImage.m_pBuffer[(i - 1)*nSrcLineByte + (j - 1)*nSrcPiexlByte + n]
					+ m_cImage.m_pBuffer[(i - 1)*nSrcLineByte + j * nSrcPiexlByte + n]
					+ m_cImage.m_pBuffer[(i - 1)*nSrcLineByte + (j + 1)*nSrcPiexlByte + n]
					- m_cImage.m_pBuffer[(i + 1)*nSrcLineByte + (j - 1)*nSrcPiexlByte + n]
					- m_cImage.m_pBuffer[(i + 1)*nSrcLineByte + j * nSrcPiexlByte + n]
					- m_cImage.m_pBuffer[(i + 1)*nSrcLineByte + (j + 1)*nSrcPiexlByte + n];

				int nValue = static_cast<int>(sqrt(nDirectX*nDirectX + nDirectY * nDirectY) + 0.5f);
				if (nValue > 255) nValue = 255;

				cDestImage.m_pBuffer[i*nSrcLineByte + j * nSrcPiexlByte + n] = nValue;
			}
		}
	}

	//λͼ���ݸ���
	m_cImage = cDestImage;

	return true;
}

bool Image::Laplacian()
{
	//��ȡԴλͼ������λ
	int nSrcPiexlByte = m_cImage.m_pInfoHeader->biBitCount / 8;

	//��ȡԴλͼ�Ŀ�Ⱥ͸߶�
	int nSrcWidth = m_cImage.m_pInfoHeader->biWidth;
	int nSrcHeight = m_cImage.m_pInfoHeader->biHeight;

	//��ȡԴλͼ��һ������λ
	int nSrcLineByte = (nSrcWidth*nSrcPiexlByte + 3) / 4 * 4;

	//��ȡĿ��λͼ������λ
	int nDestPiexlByte = nSrcPiexlByte;

	//��ȡĿ��λͼ�Ŀ�Ⱥ͸߶�
	int nDestWidth = nSrcWidth;
	int nDestHeight = nSrcHeight;

	//��ȡĿ��λͼ��һ�е�����λ
	int nDestLineByte = nSrcLineByte;

	//Ŀ��λͼ
	BitmapStruct cDestImage;
	cDestImage.Allocate(m_cImage.m_pFileHeader->bfSize);
	cDestImage.Initialize(m_cImage.m_pInfoHeader->biBitCount);

	//����λͼ��Ϣ
	CopyMemory(cDestImage.m_pFileHeader, m_cImage.m_pFileHeader, sizeof(BITMAPFILEHEADER));
	CopyMemory(cDestImage.m_pInfoHeader, m_cImage.m_pInfoHeader, sizeof(BITMAPINFOHEADER));

	//������ڵ�ɫ������
	if (m_cImage.m_pRgbQuad)
	{
		//��ȡ��ɫ���С
		int nSize = m_cImage.m_pFileHeader->bfOffBits;
		nSize -= sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);

		//���Ƶ�ɫ������
		CopyMemory(cDestImage.m_pRgbQuad, m_cImage.m_pRgbQuad, nSize);
	}

	for (int i = 1; i < nSrcHeight - 1; i++)
	{
		for (int j = 1; j < nSrcWidth - 1; j++)
		{
			for (int n = 0; n < nSrcPiexlByte; n++)
			{
				int nValue = 4 * m_cImage.m_pBuffer[i*nSrcLineByte + j * nSrcPiexlByte + n]
					- m_cImage.m_pBuffer[(i - 1)*nSrcLineByte + j * nSrcPiexlByte + n]
					- m_cImage.m_pBuffer[(i + 1)*nSrcLineByte + j * nSrcPiexlByte + n]
					- m_cImage.m_pBuffer[i*nSrcLineByte + (j - 1)*nSrcPiexlByte + n]
					- m_cImage.m_pBuffer[i*nSrcLineByte + (j + 1)*nSrcPiexlByte + n];

				nValue = static_cast<int>(abs(nValue) + 0.5f);
				if (nValue > 255) nValue = 255;

				cDestImage.m_pBuffer[i*nSrcLineByte + j * nSrcPiexlByte + n] = nValue;
			}
		}
	}

	//����λͼ����
	m_cImage = cDestImage;

	return true;
}

bool Image::AdaptThreshSeg()
{
	//��ȡԴλͼ������λ
	int nSrcPiexlByte = m_cImage.m_pInfoHeader->biBitCount / 8;

	//ֻ����8λ
	if (nSrcPiexlByte != 1)return false;

	//��ȡԴλͼ�Ŀ�Ⱥ͸߶�
	int nSrcWidth = m_cImage.m_pInfoHeader->biWidth;
	int nSrcHeight = m_cImage.m_pInfoHeader->biHeight;

	//��ȡԴλͼ��һ������λ
	int nSrcLineByte = (nSrcWidth*nSrcPiexlByte + 3) / 4 * 4;

	//��ͼ��ƽ��ֵ
	int nSubAvg = 0;

	//���Ͻ�ͼ����
	for (int i = 0; i < nSrcHeight / 2; i++)
	{
		for (int j = 0; j < nSrcWidth / 2; j++)
		{
			nSubAvg += m_cImage.m_pBuffer[i*nSrcLineByte + j];
		}
	}

	//����ƽ��ֵ
	nSubAvg /= ((nSrcHeight / 2) * (nSrcWidth / 2));

	//�����Ͻ����ؽ����и�
	for (int i = 0; i < nSrcHeight / 2; i++)
	{
		for (int j = 0; j < nSrcWidth / 2; j++)
		{
			if (nSubAvg > m_cImage.m_pBuffer[i*nSrcLineByte + j])
				m_cImage.m_pBuffer[i*nSrcLineByte + j] = 255;
			else
				m_cImage.m_pBuffer[i*nSrcLineByte + j] = 0;
		}
	}

	nSubAvg = 0;

	//���½�ͼ����
	for (int i = nSrcHeight / 2; i < nSrcHeight; i++)
	{
		for (int j = 0; j < nSrcWidth / 2; j++)
		{
			nSubAvg += m_cImage.m_pBuffer[i*nSrcLineByte + j];
		}
	}

	//����ƽ��ֵ
	nSubAvg /= ((nSrcHeight - nSrcHeight / 2) * (nSrcWidth / 2));

	//���½��и�
	for (int i = nSrcHeight / 2; i < nSrcHeight; i++)
	{
		for (int j = 0; j < nSrcWidth / 2; j++)
		{
			if (nSubAvg > m_cImage.m_pBuffer[i*nSrcLineByte + j])
				m_cImage.m_pBuffer[i*nSrcLineByte + j] = 255;
			else
				m_cImage.m_pBuffer[i*nSrcLineByte + j] = 0;
		}
	}

	nSubAvg = 0;

	//���ϽǴ���
	for (int i = 0; i < nSrcHeight / 2; i++)
	{
		for (int j = nSrcWidth / 2; j < nSrcWidth; j++)
		{
			nSubAvg += m_cImage.m_pBuffer[i*nSrcLineByte + j];
		}
	}

	//����ƽ��ֵ
	nSubAvg /= ((nSrcHeight / 2) * (nSrcWidth - nSrcWidth / 2));

	//���Ͻ��и�
	for (int i = 0; i < nSrcHeight / 2; i++)
	{
		for (int j = nSrcWidth / 2; j < nSrcWidth; j++)
		{
			if (nSubAvg > m_cImage.m_pBuffer[i*nSrcLineByte + j])
				m_cImage.m_pBuffer[i*nSrcLineByte + j] = 255;
			else
				m_cImage.m_pBuffer[i*nSrcLineByte + j] = 0;
		}
	}

	nSubAvg = 0;

	//���½�ͼ����
	for (int i = nSrcHeight / 2; i < nSrcHeight; i++)
	{
		for (int j = nSrcWidth / 2; j < nSrcWidth; j++)
		{
			nSubAvg += m_cImage.m_pBuffer[i*nSrcLineByte + j];
		}
	}

	//����ƽ��ֵ
	nSubAvg /= ((nSrcHeight - nSrcHeight / 2) * (nSrcWidth - nSrcWidth / 2));

	//���½��и�
	for (int i = nSrcHeight / 2; i < nSrcHeight; i++)
	{
		for (int j = nSrcWidth / 2; j < nSrcWidth; j++)
		{
			if (nSubAvg > m_cImage.m_pBuffer[i*nSrcLineByte + j])
				m_cImage.m_pBuffer[i*nSrcLineByte + j] = 255;
			else
				m_cImage.m_pBuffer[i*nSrcLineByte + j] = 0;
		}
	}

	//��ɫ������ɫ�ͺ�ɫ����
	for (int i = 0; i < nSrcHeight; i++)
	{
		for (int j = 0; j < nSrcWidth; j++)
		{
			if (!m_cImage.m_pBuffer[i*nSrcLineByte + j])
				m_cImage.m_pBuffer[i*nSrcLineByte + j] = 255;
			else
				m_cImage.m_pBuffer[i*nSrcLineByte + j] = 0;
		}
	}

	return true;
}

bool Image::MeanTemplateSmooth(int nTemplateWidth,	
	int nTemplateHeight,					
	double *pTemplate,
	int nIgnoreX ,						
	int nIgnoreY ,						
	double dCoef )
{
	//��ȡԴλͼ������λ
	int nSrcPiexlByte = m_cImage.m_pInfoHeader->biBitCount / 8;

	//�����ǻҶ�λͼ
	if (nSrcPiexlByte != 1) return false;

	//��ȡԴλͼ�Ŀ�Ⱥ͸߶�
	int nSrcWidth = m_cImage.m_pInfoHeader->biWidth;
	int nSrcHeight = m_cImage.m_pInfoHeader->biHeight;

	//��ȡԴλͼ��һ������λ
	int nSrcLineByte = (nSrcWidth * nSrcPiexlByte + 3) / 4 * 4;

	//Ŀ��λͼ
	BitmapStruct cDestImage;
	cDestImage.Allocate(m_cImage.m_pFileHeader->bfSize);
	cDestImage.Initialize(m_cImage.m_pInfoHeader->biBitCount);

	//����λͼ��Ϣ
	CopyMemory(cDestImage.m_pFileHeader, m_cImage.m_pFileHeader, sizeof(BITMAPFILEHEADER));
	CopyMemory(cDestImage.m_pInfoHeader, m_cImage.m_pInfoHeader, sizeof(BITMAPINFOHEADER));

	//������ڵ�ɫ������
	if (m_cImage.m_pRgbQuad)
	{
		//��ȡ��ɫ���С
		int nSize = m_cImage.m_pFileHeader->bfOffBits;
		nSize -= sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);

		//���Ƶ�ɫ������
		CopyMemory(cDestImage.m_pRgbQuad, m_cImage.m_pRgbQuad, nSize);
	}

	if (nIgnoreX <= 0 || nIgnoreX > nTemplateWidth)
		nIgnoreX = nTemplateWidth / 2;
	if (nIgnoreY <= 0 || nIgnoreY > nTemplateHeight)
		nIgnoreY = nTemplateHeight / 2;
	if (dCoef <= 0.001) dCoef = 1.0 / static_cast<double>(nTemplateWidth * nTemplateHeight);

	//ģ���Ƿ��ǵ�ǰ���������
	bool bSelfAlloc = false;
	if (pTemplate == NULL)
	{
		int nArrayLen = nTemplateWidth * nTemplateHeight;
		pTemplate = new double[nArrayLen];
		for (int i = 0; i < nArrayLen; i++)pTemplate[i] = 1.0;
		bSelfAlloc = true;
	}

	//ȥ����Ե������
	for (int i = nIgnoreY; i < nSrcHeight - nTemplateHeight + nIgnoreY + 1; i++)
	{
		//ȥ����Ե������
		for (int j = nIgnoreX; j < nSrcWidth - nTemplateWidth  + nIgnoreX + 1; j++)
		{

			double dValue = 0;
			for (int n = 0; n < nTemplateHeight; n++)
			{
				for (int m = 0; m < nTemplateWidth; m++)
				{
					dValue += m_cImage.m_pBuffer[(nSrcHeight - 1 - i + nIgnoreY - n)*nSrcLineByte + j - nIgnoreX + m]
						* pTemplate[n*nTemplateWidth + m];
				}
			}
			dValue = fabs(dValue*dCoef);
			if (dValue > 255.0) dValue = 255.0;
			else dValue += 0.5;
			cDestImage.m_pBuffer[(nSrcHeight - 1 - i)*nSrcLineByte + j] = static_cast<char>(dValue);
		}
	}

	//λͼ����
	m_cImage = cDestImage;

	if (bSelfAlloc)delete[] pTemplate;

	return true;
}

bool Image::MedianFilter(int nTemplateWidth,
	int nTemplateHeight,
	int nIgnoreX ,
	int nIgnoreY ) 
{
	//��ȡԴλͼ������λ
	int nSrcPiexlByte = m_cImage.m_pInfoHeader->biBitCount / 8;

	//�����Ƕ�ֵ��λͼ
	if (nSrcPiexlByte != 1) return false;

	//��ȡԴλͼ�Ŀ�Ⱥ͸߶�
	int nSrcWidth = m_cImage.m_pInfoHeader->biWidth;
	int nSrcHeight = m_cImage.m_pInfoHeader->biHeight;

	//��ȡԴλͼ��һ������λ
	int nSrcLineByte = (nSrcWidth * nSrcPiexlByte + 3) / 4 * 4;

	//Ŀ��λͼ
	BitmapStruct cDestImage;
	cDestImage.Allocate(m_cImage.m_pFileHeader->bfSize);
	cDestImage.Initialize(m_cImage.m_pInfoHeader->biBitCount);

	//����λͼ��Ϣ
	CopyMemory(cDestImage.m_pFileHeader, m_cImage.m_pFileHeader, sizeof(BITMAPFILEHEADER));
	CopyMemory(cDestImage.m_pInfoHeader, m_cImage.m_pInfoHeader, sizeof(BITMAPINFOHEADER));

	//������ڵ�ɫ������
	if (m_cImage.m_pRgbQuad)
	{
		//��ȡ��ɫ���С
		int nSize = m_cImage.m_pFileHeader->bfOffBits;
		nSize -= sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);

		//���Ƶ�ɫ������
		CopyMemory(cDestImage.m_pRgbQuad, m_cImage.m_pRgbQuad, nSize);
	}

	if (nIgnoreX <= 0 || nIgnoreX > nTemplateWidth)
		nIgnoreX = nTemplateWidth / 2;
	if (nIgnoreY <= 0 || nIgnoreY > nTemplateHeight)
		nIgnoreY = nTemplateHeight / 2;

	//�����˲�������
	char* pFilter = new char[nTemplateWidth*nTemplateHeight];

	//��ȡ��ֵ
	auto GetMedianValue = [&]() -> char
	{
		int nFilterSize = nTemplateHeight * nTemplateWidth;

		char cValue = 0;

		for (int i = 0; i < nFilterSize; i++)
		{
			for (int j = 0; j < nFilterSize - i - 1; j++)
			{
				if (pFilter[j] > pFilter[j + 1])
				{
					cValue = pFilter[j];
					pFilter[j] = pFilter[j + 1];
					pFilter[j + 1] = cValue;
				}
			}
		}

		if ((nFilterSize & 1) > 0)
			cValue = pFilter[(nFilterSize + 1) / 2];
		else
			cValue = (pFilter[nFilterSize / 2] + pFilter[nFilterSize / 2 + 1]) / 2;

		return cValue;
	};

	for (int i = nIgnoreY; i < nSrcHeight - nTemplateHeight; i++)
	{
		for (int j = nIgnoreX; j < nSrcWidth - nTemplateWidth; j++)
		{
			for (int n = 0; n < nTemplateHeight; n++)
			{
				for (int m = 0; m < nTemplateWidth; m++)
				{
					int nRow = abs(nSrcHeight - m - i + nIgnoreY - n);
					char cTemp = m_cImage.m_pBuffer[nRow*nSrcLineByte + j - nIgnoreX + m];
					pFilter[n*nTemplateWidth + m] = cTemp;
				}
			}
			cDestImage.m_pBuffer[(nSrcHeight - 1 - i)*nSrcLineByte + j] = GetMedianValue();
		}
	}

	//λͼ����
	m_cImage = cDestImage;

	//�ͷ��ڴ�
	delete[] pFilter;

	return true;
}

bool Image::MaskSmooth()
{
	//��ȡԴλͼ������λ
	int nSrcPiexlByte = m_cImage.m_pInfoHeader->biBitCount / 8;

	//�����Ƕ�ֵ��λͼ
	if (nSrcPiexlByte != 1) return false;

	//��ȡԴλͼ�Ŀ�Ⱥ͸߶�
	int nSrcWidth = m_cImage.m_pInfoHeader->biWidth;
	int nSrcHeight = m_cImage.m_pInfoHeader->biHeight;

	//��ȡԴλͼ��һ������λ
	int nSrcLineByte = (nSrcWidth * nSrcPiexlByte + 3) / 4 * 4;

	//Ŀ��λͼ
	BitmapStruct cDestImage;
	cDestImage.Allocate(m_cImage.m_pFileHeader->bfSize);
	cDestImage.Initialize(m_cImage.m_pInfoHeader->biBitCount);

	//����λͼ��Ϣ
	CopyMemory(cDestImage.m_pFileHeader, m_cImage.m_pFileHeader, sizeof(BITMAPFILEHEADER));
	CopyMemory(cDestImage.m_pInfoHeader, m_cImage.m_pInfoHeader, sizeof(BITMAPINFOHEADER));

	//������ڵ�ɫ������
	if (m_cImage.m_pRgbQuad)
	{
		//��ȡ��ɫ���С
		int nSize = m_cImage.m_pFileHeader->bfOffBits;
		nSize -= sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);

		//���Ƶ�ɫ������
		CopyMemory(cDestImage.m_pRgbQuad, m_cImage.m_pRgbQuad, nSize);
	}

	for (int j = 2; j <= nSrcHeight - 3; j++)
	{
		for (int i = 2; i < nSrcWidth - 3; i++)
		{
			//������ֵ
			int nValue[9]{ 0 };

			//����ƽ��ֵ����ֵ
			double dAvg[9]{ 0.0 }, dSum[9]{ 0.0 };

			//��9�ֽ�������ľ�ֵ���䷽��
			//��1��������
			nValue[0] = m_cImage.m_pBuffer[(j - 1)*nSrcLineByte + (i - 1)];
			nValue[1] = m_cImage.m_pBuffer[(j - 1)*nSrcLineByte + i];
			nValue[2] = m_cImage.m_pBuffer[(j - 1)*nSrcLineByte + (i + 1)];
			nValue[3] = m_cImage.m_pBuffer[j*nSrcLineByte + (i - 1)];
			nValue[4] = m_cImage.m_pBuffer[j*nSrcLineByte + i];
			nValue[5] = m_cImage.m_pBuffer[j*nSrcLineByte + (i + 1)];
			nValue[6] = m_cImage.m_pBuffer[(j + 1)*nSrcLineByte + (i - 1)];
			nValue[7] = m_cImage.m_pBuffer[(j + 1)*nSrcLineByte + i];
			nValue[8] = m_cImage.m_pBuffer[(j + 1)*nSrcLineByte + (i + 1)];
			dAvg[0] = static_cast<double>(nValue[0] + nValue[1] + nValue[2] 
				+ nValue[3] + nValue[4] + nValue[5] + nValue[6] + nValue[7] + nValue[8]) / 9;
			for (int n = 0; n <= 8; n++)
				dSum[0] += nValue[n] * nValue[n] - dAvg[0] * dAvg[0];

			//��2��������
			nValue[0] = m_cImage.m_pBuffer[(j - 2)*nSrcLineByte + (i - 1)];
			nValue[1] = m_cImage.m_pBuffer[(j - 2)*nSrcLineByte + i];
			nValue[2] = m_cImage.m_pBuffer[(j - 2)*nSrcLineByte + (i + 1)];
			nValue[3] = m_cImage.m_pBuffer[(j - 1)*nSrcLineByte + (i - 1)];
			nValue[4] = m_cImage.m_pBuffer[(j - 1)*nSrcLineByte + i];
			nValue[5] = m_cImage.m_pBuffer[(j - 1)*nSrcLineByte + (i + 1)];
			nValue[6] = m_cImage.m_pBuffer[j*nSrcLineByte + i];
			dAvg[1] = static_cast<double>(nValue[0] + nValue[1] + nValue[2] 
				+ nValue[3] + nValue[4] + nValue[5] + nValue[6]) / 7;
			for (int n = 0; n <= 6; n++)
				dSum[1] += nValue[n] * nValue[n] - dAvg[1] * dAvg[1];

			//��3��������
			nValue[0] = m_cImage.m_pBuffer[(j - 1)*nSrcLineByte + (i - 2)];
			nValue[1] = m_cImage.m_pBuffer[(j - 1)*nSrcLineByte + (i - 1)];
			nValue[2] = m_cImage.m_pBuffer[j*nSrcLineByte + (i - 2)];
			nValue[3] = m_cImage.m_pBuffer[j*nSrcLineByte + (i - 1)];
			nValue[4] = m_cImage.m_pBuffer[j*nSrcLineByte + i];
			nValue[5] = m_cImage.m_pBuffer[(j + 1)*nSrcLineByte + (i - 2)];
			nValue[6] = m_cImage.m_pBuffer[(j + 1)*nSrcLineByte + (i - 1)];
			dAvg[2] = static_cast<double>(nValue[0] + nValue[1] + nValue[2]
				+ nValue[3] + nValue[4] + nValue[5] + nValue[6]) / 7;
			for (int n = 0; n <= 6; n++)
				dSum[2] += nValue[n] * nValue[n] - dAvg[2] * dAvg[2];

			//��4��������
			nValue[0] = m_cImage.m_pBuffer[j*nSrcLineByte + i];
			nValue[1] = m_cImage.m_pBuffer[(j + 1)*nSrcLineByte + (i - 1)];
			nValue[2] = m_cImage.m_pBuffer[(j + 1)*nSrcLineByte + i];
			nValue[3] = m_cImage.m_pBuffer[(j + 1)*nSrcLineByte + (i + 1)];
			nValue[4] = m_cImage.m_pBuffer[(j + 2)*nSrcLineByte + (i - 1)];
			nValue[5] = m_cImage.m_pBuffer[(j + 2)*nSrcLineByte + i];
			nValue[6] = m_cImage.m_pBuffer[(j + 2)*nSrcLineByte + (i + 1)];
			dAvg[3] = static_cast<double>(nValue[0] + nValue[1] + nValue[2]
				+ nValue[3] + nValue[4] + nValue[5] + nValue[6]) / 7;
			for (int n = 0; n <= 6; n++)
				dSum[3] += nValue[n] * nValue[n] - dAvg[3] * dAvg[3];

			//��5��������
			nValue[0] = m_cImage.m_pBuffer[(j - 1)*nSrcLineByte + (i + 1)];
			nValue[1] = m_cImage.m_pBuffer[(j - 1)*nSrcLineByte + (i + 2)];
			nValue[2] = m_cImage.m_pBuffer[j*nSrcLineByte + i];
			nValue[3] = m_cImage.m_pBuffer[j*nSrcLineByte + (i + 1)];
			nValue[4] = m_cImage.m_pBuffer[j*nSrcLineByte + (i + 2)];
			nValue[5] = m_cImage.m_pBuffer[(j + 1)*nSrcLineByte + (i + 1)];
			nValue[6] = m_cImage.m_pBuffer[(j + 1)*nSrcLineByte + (i + 2)];
			dAvg[4] = static_cast<double>(nValue[0] + nValue[1] + nValue[2]
				+ nValue[3] + nValue[4] + nValue[5] + nValue[6]) / 7;
			for (int n = 0; n <= 6; n++)
				dSum[4] += nValue[n] * nValue[n] - dAvg[4] * dAvg[4];

			//��6��������
			nValue[0] = m_cImage.m_pBuffer[(j - 2)*nSrcLineByte + (i + 1)];
			nValue[1] = m_cImage.m_pBuffer[(j - 2)*nSrcLineByte + (i + 2)];
			nValue[2] = m_cImage.m_pBuffer[(j - 1)*nSrcLineByte + i];
			nValue[3] = m_cImage.m_pBuffer[(j - 1)*nSrcLineByte + (i + 1)];
			nValue[4] = m_cImage.m_pBuffer[(j - 1)*nSrcLineByte + (i + 2)];
			nValue[5] = m_cImage.m_pBuffer[j*nSrcLineByte + i];
			nValue[6] = m_cImage.m_pBuffer[j*nSrcLineByte + (i + 1)];
			dAvg[5] = static_cast<double>(nValue[0] + nValue[1] + nValue[2] 
				+ nValue[3] + nValue[4] + nValue[5] + nValue[6]) / 7;
			for (int n = 0; n <= 6; n++)
				dSum[5] += nValue[n] * nValue[n] - dAvg[5] * dAvg[5];

			//��7��������
			nValue[0] = m_cImage.m_pBuffer[(j - 2)*nSrcLineByte + (i - 2)];
			nValue[1] = m_cImage.m_pBuffer[(j - 2)*nSrcLineByte + (i - 1)];
			nValue[2] = m_cImage.m_pBuffer[(j - 1)*nSrcLineByte + (i - 2)];
			nValue[3] = m_cImage.m_pBuffer[(j - 1)*nSrcLineByte + (i - 1)];
			nValue[4] = m_cImage.m_pBuffer[(j - 1)*nSrcLineByte + i];
			nValue[5] = m_cImage.m_pBuffer[j*nSrcLineByte + (i - 1)];
			nValue[6] = m_cImage.m_pBuffer[j*nSrcLineByte + i];
			dAvg[6] = static_cast<double>(nValue[0] + nValue[1] + nValue[2]
				+ nValue[3] + nValue[4] + nValue[5] + nValue[6]) / 7;
			for (int n = 0; n <= 6; n++)
				dSum[6] += nValue[n] * nValue[n] - dAvg[6] * dAvg[6];

			//��8��������
			nValue[0] = m_cImage.m_pBuffer[j*nSrcLineByte + (i - 1)];
			nValue[1] = m_cImage.m_pBuffer[j*nSrcLineByte + i];
			nValue[2] = m_cImage.m_pBuffer[(j + 1)*nSrcLineByte + (i - 2)];
			nValue[3] = m_cImage.m_pBuffer[(j + 1)*nSrcLineByte + (i - 1)];
			nValue[4] = m_cImage.m_pBuffer[(j + 1)*nSrcLineByte + i];
			nValue[5] = m_cImage.m_pBuffer[(j + 2)*nSrcLineByte + (i - 2)];
			nValue[6] = m_cImage.m_pBuffer[(j + 2)*nSrcLineByte + (i - 1)];
			dAvg[7] = static_cast<double>(nValue[0] + nValue[1] + nValue[2] 
				+ nValue[3] + nValue[4] + nValue[5] + nValue[6]) / 7;
			for (int n = 0; n <= 6; n++)
				dSum[7] += nValue[n] * nValue[n] - dAvg[7] * dAvg[7];

			//��9��������
			nValue[0] = m_cImage.m_pBuffer[j*nSrcLineByte + i];
			nValue[1] = m_cImage.m_pBuffer[j*nSrcLineByte + (i + 1)];
			nValue[2] = m_cImage.m_pBuffer[(j + 1)*nSrcLineByte + i];
			nValue[3] = m_cImage.m_pBuffer[(j + 1)*nSrcLineByte + (i + 1)];
			nValue[4] = m_cImage.m_pBuffer[(j + 1)*nSrcLineByte + (i + 2)];
			nValue[5] = m_cImage.m_pBuffer[(j + 2)*nSrcLineByte + (i + 1)];
			nValue[6] = m_cImage.m_pBuffer[(j + 2)*nSrcLineByte + (i + 2)];
			dAvg[8] = static_cast<double>(nValue[0] + nValue[1] + nValue[2]
				+ nValue[3] + nValue[4] + nValue[5] + nValue[6]) / 7;
			for (int n = 0; n <= 6; n++)
				dSum[8] += nValue[n] * nValue[n] - dAvg[8] * dAvg[8];

			//�󷽲���С�Ľ�������
			double dMin = dSum[0];
			int  nIndex = 0;
			for (int n = 1; n < 9; n++)
			{
				if (dMin > dSum[n])
				{
					dMin = dSum[n];
					nIndex = n;
				}

				//��ֵ�����������Ϊ��ʾͼ���ֵ
				cDestImage.m_pBuffer[j*nSrcLineByte + i] = static_cast<char>(dAvg[nIndex] + 0.5);
			}
		}
	}

	//����λͼ����
	m_cImage = cDestImage;

	return true;
}

bool Image::LapTemplate(int nTemplateWidth,
	int nTemplateHeight,
	double *pTemplate,
	int nIgnoreX,
	int nIgnoreY,
	double dCoef)
{
	//��ȡԴλͼ������λ
	int nSrcPiexlByte = m_cImage.m_pInfoHeader->biBitCount / 8;

	//�����Ƕ�ֵ��λͼ
	if (nSrcPiexlByte != 1) return false;

	//��ȡԴλͼ�Ŀ�Ⱥ͸߶�
	int nSrcWidth = m_cImage.m_pInfoHeader->biWidth;
	int nSrcHeight = m_cImage.m_pInfoHeader->biHeight;

	//��ȡԴλͼ��һ������λ
	int nSrcLineByte = (nSrcWidth * nSrcPiexlByte + 3) / 4 * 4;

	//Ŀ��λͼ
	BitmapStruct cDestImage;
	cDestImage.Allocate(m_cImage.m_pFileHeader->bfSize);
	cDestImage.Initialize(m_cImage.m_pInfoHeader->biBitCount);

	//����λͼ��Ϣ
	CopyMemory(cDestImage.m_pFileHeader, m_cImage.m_pFileHeader, sizeof(BITMAPFILEHEADER));
	CopyMemory(cDestImage.m_pInfoHeader, m_cImage.m_pInfoHeader, sizeof(BITMAPINFOHEADER));

	//������ڵ�ɫ������
	if (m_cImage.m_pRgbQuad)
	{
		//��ȡ��ɫ���С
		int nSize = m_cImage.m_pFileHeader->bfOffBits;
		nSize -= sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);

		//���Ƶ�ɫ������
		CopyMemory(cDestImage.m_pRgbQuad, m_cImage.m_pRgbQuad, nSize);
	}

	if (nIgnoreX <= 0 || nIgnoreX > nTemplateWidth)
		nIgnoreX = nTemplateWidth / 2;
	if (nIgnoreY <= 0 || nIgnoreY > nTemplateHeight)
		nIgnoreY = nTemplateHeight / 2;
	if (dCoef <= 0.00) dCoef = 1.0;

	for (int i = nIgnoreY; i < nSrcHeight - nTemplateHeight + nIgnoreY + 1; i++)
	{
		for (int j = nIgnoreY; j < nSrcWidth - nTemplateWidth + nIgnoreX + 1; j++)
		{
			double dSum = 0;
			for (int n = 0; n < nTemplateHeight; n++)
			{
				for (int m = 0; m < nTemplateWidth; m++)
				{
					dSum += m_cImage.m_pBuffer[(nSrcHeight - 1 - i + nIgnoreY - n)*nSrcLineByte + j - nIgnoreX + m]
						* pTemplate[n*nTemplateHeight + m];
				}
			}

			dSum = fabs(dSum*dCoef);
			if (dSum > 255)
				cDestImage.m_pBuffer[(nSrcHeight - 1 - i)*nSrcLineByte + j] = 255;
			else
				cDestImage.m_pBuffer[(nSrcHeight - 1 - i)*nSrcLineByte + j] = static_cast<char>(dSum + 0.5);
		}
	}

	//λͼ����
	m_cImage = cDestImage;

	return true;
}

bool Image::Erosion(int nTemplateWidth, int nTemplateHeight,const int *pTemplate)
{
	//��ȡԴλͼ������λ
	int nSrcPiexlByte = m_cImage.m_pInfoHeader->biBitCount / 8;

	//�����Ƕ�ֵ��λͼ
	if (nSrcPiexlByte != 1) return false;

	//��ȡԴλͼ�Ŀ�Ⱥ͸߶�
	int nSrcWidth = m_cImage.m_pInfoHeader->biWidth;
	int nSrcHeight = m_cImage.m_pInfoHeader->biHeight;

	//��ȡԴλͼ��һ������λ
	int nSrcLineByte = (nSrcWidth * nSrcPiexlByte + 3) / 4 * 4;

	//Ŀ��λͼ
	BitmapStruct cDestImage;
	cDestImage.Allocate(m_cImage.m_pFileHeader->bfSize);
	cDestImage.Initialize(m_cImage.m_pInfoHeader->biBitCount);

	//����λͼ��Ϣ
	CopyMemory(cDestImage.m_pFileHeader, m_cImage.m_pFileHeader, sizeof(BITMAPFILEHEADER));
	CopyMemory(cDestImage.m_pInfoHeader, m_cImage.m_pInfoHeader, sizeof(BITMAPINFOHEADER));

	//������ڵ�ɫ������
	if (m_cImage.m_pRgbQuad)
	{
		//��ȡ��ɫ���С
		int nSize = m_cImage.m_pFileHeader->bfOffBits;
		nSize -= sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);

		//���Ƶ�ɫ������
		CopyMemory(cDestImage.m_pRgbQuad, m_cImage.m_pRgbQuad, nSize);
	}

	//��־
	bool bFlag = true;

	for (int i = nTemplateHeight / 2; i < nSrcHeight - nTemplateHeight / 2; i++)
	{
		for (int j = nTemplateWidth / 2; j < nSrcWidth - nTemplateWidth / 2; j++)
		{
			bFlag = true;

			for (int n = -nTemplateHeight / 2; n <= nTemplateHeight / 2; n++)
			{
				for (int m = -nTemplateWidth / 2; m <= nTemplateWidth / 2; m++)
				{
					if (pTemplate[(n + nTemplateHeight / 2)*nTemplateWidth + 1 + nTemplateWidth / 2])
					{
						if (!m_cImage.m_pBuffer[(i + n)*nSrcLineByte + j + m])
						{
							bFlag = false;
							break;
						}
					}
				}
				if (!bFlag)break;
			}

			if (bFlag)
				cDestImage.m_pBuffer[i*nSrcLineByte + j] = 255;
			else
				cDestImage.m_pBuffer[i*nSrcLineByte + j] = 0;

		}
	}

	//����λͼ����
	m_cImage = cDestImage;

	return true;
}

bool Image::Dilate(int nTemplateWidth, int nTemplateHeight, const int *pTemplate)
{
	//��ȡԴλͼ������λ
	int nSrcPiexlByte = m_cImage.m_pInfoHeader->biBitCount / 8;

	//�����Ƕ�ֵ��λͼ
	if (nSrcPiexlByte != 1) return false;

	//��ȡԴλͼ�Ŀ�Ⱥ͸߶�
	int nSrcWidth = m_cImage.m_pInfoHeader->biWidth;
	int nSrcHeight = m_cImage.m_pInfoHeader->biHeight;

	//��ȡԴλͼ��һ������λ
	int nSrcLineByte = (nSrcWidth * nSrcPiexlByte + 3) / 4 * 4;

	//Ŀ��λͼ
	BitmapStruct cDestImage;
	cDestImage.Allocate(m_cImage.m_pFileHeader->bfSize);
	cDestImage.Initialize(m_cImage.m_pInfoHeader->biBitCount);

	//����λͼ��Ϣ
	CopyMemory(cDestImage.m_pFileHeader, m_cImage.m_pFileHeader, sizeof(BITMAPFILEHEADER));
	CopyMemory(cDestImage.m_pInfoHeader, m_cImage.m_pInfoHeader, sizeof(BITMAPINFOHEADER));

	//������ڵ�ɫ������
	if (m_cImage.m_pRgbQuad)
	{
		//��ȡ��ɫ���С
		int nSize = m_cImage.m_pFileHeader->bfOffBits;
		nSize -= sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);

		//���Ƶ�ɫ������
		CopyMemory(cDestImage.m_pRgbQuad, m_cImage.m_pRgbQuad, nSize);
	}

	//�ڰ�ת��
	for (int i = 0; i < nSrcHeight; i++)
	{
		for (int j = 0; j < nSrcWidth; j++)
		{
			if (!m_cImage.m_pBuffer[i*nSrcLineByte + j])
				m_cImage.m_pBuffer[i*nSrcLineByte + j] = 255;
			else
				m_cImage.m_pBuffer[i*nSrcLineByte + j] = 0;
		}
	}

	//����ԳƼ��ռ�
	int* pTemplateMask = new int[nTemplateHeight*nTemplateWidth];
	for (int i = 0; i < nTemplateHeight; i++) 
	{
		for (int j = 0; j < nTemplateWidth; j++)
		{
			pTemplateMask[i*nTemplateWidth + j] =
				pTemplate[(nTemplateHeight - 1 - i)*nTemplateWidth + nTemplateWidth - 1 - j];
		}
	}

	//��־
	bool bFlag = true;

	for (int i = nTemplateHeight / 2; i < nSrcHeight - nTemplateHeight / 2; i++)
	{
		for (int j = nTemplateWidth / 2; j < nSrcWidth - nTemplateWidth / 2; j++)
		{
			bFlag = true;

			for (int n = -nTemplateHeight / 2; n <= nTemplateHeight / 2; n++)
			{
				for (int m = -nTemplateWidth / 2; m <= nTemplateWidth / 2; m++)
				{
					if (pTemplateMask[(n + nTemplateHeight / 2)*nTemplateWidth + 1 + nTemplateWidth / 2])
					{
						if (!m_cImage.m_pBuffer[(i + n)*nSrcLineByte + j + m])
						{
							bFlag = false;
							break;
						}
					}
				}
				if (!bFlag)break;
			}

			if (bFlag)
				cDestImage.m_pBuffer[i*nSrcLineByte + j] = 255;
			else
				cDestImage.m_pBuffer[i*nSrcLineByte + j] = 0;

		}
	}

	//�ڰ�ת��
	for (int i = 0; i < nSrcHeight; i++)
	{
		for (int j = 0; j < nSrcWidth; j++)
		{
			if (!cDestImage.m_pBuffer[i*nSrcLineByte + j])
				cDestImage.m_pBuffer[i*nSrcLineByte + j] = 255;
			else
				cDestImage.m_pBuffer[i*nSrcLineByte + j] = 0;
		}
	}

	//λͼ���ݸ���
	m_cImage = cDestImage;

	//�ͷ��ڴ�
	delete[] pTemplateMask;

	return true;
}

bool Image::Open(int nTemplateWidth, int nTemplateHeight, const int *pTemplate)
{
	//�ȸ�ʴ ������
	if (!Erosion(nTemplateWidth, nTemplateHeight, pTemplate))return false;
	if (!Dilate(nTemplateWidth, nTemplateHeight, pTemplate))return false;

	return true;
}

bool Image::Close(int nTemplateWidth, int nTemplateHeight, const int *pTemplate)
{
	//������ ��ʴ
	if (!Dilate(nTemplateWidth, nTemplateHeight, pTemplate))return false;
	if (!Erosion(nTemplateWidth, nTemplateHeight, pTemplate))return false;

	return true;
}

bool Image::ImgThinning()
{
	//��ȡԴλͼ������λ
	int nSrcPiexlByte = m_cImage.m_pInfoHeader->biBitCount / 8;

	//�����Ƕ�ֵ��λͼ
	if (nSrcPiexlByte != 1) return false;

	//��ȡԴλͼ�Ŀ�Ⱥ͸߶�
	int nSrcWidth = m_cImage.m_pInfoHeader->biWidth;
	int nSrcHeight = m_cImage.m_pInfoHeader->biHeight;

	//��ȡԴλͼ��һ������λ
	int nSrcLineByte = (nSrcWidth * nSrcPiexlByte + 3) / 4 * 4;

	//Ŀ��λͼ
	BitmapStruct cDestImage;
	cDestImage.Allocate(m_cImage.m_pFileHeader->bfSize);
	cDestImage.Initialize(m_cImage.m_pInfoHeader->biBitCount);

	//����λͼ��Ϣ
	CopyMemory(cDestImage.m_pFileHeader, m_cImage.m_pFileHeader, sizeof(BITMAPFILEHEADER));
	CopyMemory(cDestImage.m_pInfoHeader, m_cImage.m_pInfoHeader, sizeof(BITMAPINFOHEADER));

	//������ڵ�ɫ������
	if (m_cImage.m_pRgbQuad)
	{
		//��ȡ��ɫ���С
		int nSize = m_cImage.m_pFileHeader->bfOffBits;
		nSize -= sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);

		//���Ƶ�ɫ������
		CopyMemory(cDestImage.m_pRgbQuad, m_cImage.m_pRgbQuad, nSize);
	}

	struct ElementPair
	{
		int nHitElement[9];
		int nMissElement[9];
		ElementPair()
		{
			memset(nHitElement, 0, sizeof(int) * 9);
			memset(nMissElement, 0, sizeof(int) * 9);
		}
	};

	ElementPair stElement[8];

	stElement[0].nHitElement[0] = 1;
	stElement[0].nHitElement[1] = 1;
	stElement[0].nHitElement[2] = 1;
	stElement[0].nHitElement[4] = 1;
	stElement[0].nMissElement[6] = 1;
	stElement[0].nMissElement[7] = 1;
	stElement[0].nMissElement[8] = 1;

	stElement[1].nHitElement[6] = 1;
	stElement[1].nHitElement[7] = 1;
	stElement[1].nHitElement[8] = 1;
	stElement[1].nHitElement[4] = 1;
	stElement[1].nMissElement[0] = 1;
	stElement[1].nMissElement[1] = 1;
	stElement[1].nMissElement[2] = 1;

	stElement[2].nHitElement[2] = 1;
	stElement[2].nHitElement[5] = 1;
	stElement[2].nHitElement[8] = 1;
	stElement[2].nHitElement[4] = 1;
	stElement[2].nMissElement[0] = 1;
	stElement[2].nMissElement[3] = 1;
	stElement[2].nMissElement[6] = 1;

	stElement[3].nHitElement[0] = 1;
	stElement[3].nHitElement[3] = 1;
	stElement[3].nHitElement[6] = 1;
	stElement[3].nHitElement[4] = 1;
	stElement[3].nMissElement[2] = 1;
	stElement[3].nMissElement[5] = 1;
	stElement[3].nMissElement[8] = 1;

	stElement[4].nHitElement[0] = 1;
	stElement[4].nHitElement[1] = 1;
	stElement[4].nHitElement[3] = 1;
	stElement[4].nHitElement[4] = 1;
	stElement[4].nMissElement[5] = 1;
	stElement[4].nMissElement[7] = 1;
	stElement[4].nMissElement[8] = 1;

	stElement[5].nHitElement[5] = 1;
	stElement[5].nHitElement[7] = 1;
	stElement[5].nHitElement[8] = 1;
	stElement[5].nHitElement[4] = 1;
	stElement[5].nMissElement[0] = 1;
	stElement[5].nMissElement[1] = 1;
	stElement[5].nMissElement[3] = 1;

	stElement[6].nHitElement[1] = 1;
	stElement[6].nHitElement[2] = 1;
	stElement[6].nHitElement[5] = 1;
	stElement[6].nHitElement[4] = 1;
	stElement[6].nMissElement[3] = 1;
	stElement[6].nMissElement[6] = 1;
	stElement[6].nMissElement[7] = 1;

	stElement[7].nHitElement[3] = 1;
	stElement[7].nHitElement[6] = 1;
	stElement[7].nHitElement[7] = 1;
	stElement[7].nHitElement[4] = 1;
	stElement[7].nMissElement[1] = 1;
	stElement[7].nMissElement[2] = 1;
	stElement[7].nMissElement[5] = 1;

	//���кͲ����д���
	auto HitMissHandle = [&](const ElementPair& stMask) -> void
	{
		bool bValidate = true;

		for (int i = 0; i < 3; i++)
		{
			for (int j = 0; j < 3; j++)
			{
				if (stMask.nHitElement[i * 3 + j] && stMask.nMissElement[i * 3 + j])
				{
					bValidate = false;
					break;
				}
			}
		}

		if (!bValidate) return;

		bool bHitFlag, bMissFlag;

		for (int i = 1; i < nSrcHeight - 1; i++)
		{
			for (int j = 1; j < nSrcWidth - 1; j++)
			{
				bHitFlag = true;
				bMissFlag = true;
				for (int n = -1; n <= 1; n++)
				{
					for (int m = -1; m <= 1; m++)
					{
						//������нṹԪ�ص�ǰλ��Ϊ1
						if (stMask.nHitElement[(n + 1) * 3 + m + 1])
						{
							//�ж�ͼ���Ӧ���Ƿ�Ϊ0,�����,��û�л���ͼ��ǰ��
							if (!m_cImage.m_pBuffer[(i + n)*nSrcLineByte + j + m])
							{
								bHitFlag = false;
								break;
							}
						}

						//��������нṹԪ�ص�ǰλ��Ϊ1
						if (stMask.nMissElement[(n + 1) * 3 + m + 1])
						{
							//�ж�ͼ���Ӧ���Ƿ�Ϊ0,�����,��û�л���ͼ��ǰ��
							if (m_cImage.m_pBuffer[(i + n)*nSrcLineByte + j + m])
							{
								bMissFlag = false;
								break;
							}
						}
					}
					if (!bHitFlag || !bMissFlag) break;
				}
				if (bHitFlag && bMissFlag)
					cDestImage.m_pBuffer[i*nSrcLineByte + j] = 255;
				else
					cDestImage.m_pBuffer[i*nSrcLineByte + j] = 0;
			}
		}
	};

	//ϸ����ֹ��ʶ
	bool bFlag = true;
	while (bFlag)
	{
		bFlag = false;

		//��8��������д���
		for (int p = 0; p < 8; p++)
		{
			HitMissHandle(stElement[p]);

			//�����л����б任����������ԭ������ȥ��
			for (int i = 0; i < nSrcHeight; i++)
			{
				for (int j = 0; j < nSrcWidth; j++)
				{
					if (cDestImage.m_pBuffer[i*nSrcLineByte + j] == 255)
					{
						m_cImage.m_pBuffer[i*nSrcLineByte + j] = 0;
						bFlag = true;
					}
				}
			}
		}
	}

	return true;
}

bool Image::Recursive_bilateral_filtering(float sigma_spatial, float sigma_range, float * buffer)
{
	unsigned char * img = m_cImage.m_pBuffer;
	int width = m_cImage.m_pInfoHeader->biWidth;
	int height = m_cImage.m_pInfoHeader->biHeight;
	int channel = m_cImage.m_pInfoHeader->biBitCount / 8;

	const int width_height = width * height;
	const int width_channel = width * channel;
	const int width_height_channel = width * height * channel;

	bool is_buffer_internal = (buffer == 0);
	if (is_buffer_internal)
		buffer = new float[(width_height_channel + width_height
			+ width_channel + width) * 2];

	float * img_out_f = buffer;
	float * img_temp = &img_out_f[width_height_channel];
	float * map_factor_a = &img_temp[width_height_channel];
	float * map_factor_b = &map_factor_a[width_height];
	float * slice_factor_a = &map_factor_b[width_height];
	float * slice_factor_b = &slice_factor_a[width_channel];
	float * line_factor_a = &slice_factor_b[width_channel];
	float * line_factor_b = &line_factor_a[width];

	//compute a lookup table
	const int QX_DEF_CHAR_MAX = 255;
	float range_table[QX_DEF_CHAR_MAX + 1];
	float inv_sigma_range = 1.0f / (sigma_range * QX_DEF_CHAR_MAX);
	for (int i = 0; i <= QX_DEF_CHAR_MAX; i++)
		range_table[i] = static_cast<float>(exp(-i * inv_sigma_range));

	float alpha = static_cast<float>(exp(-sqrt(2.0) / (sigma_spatial * width)));
	float ypr, ypg, ypb, ycr, ycg, ycb;
	float fp, fc;
	float inv_alpha_ = 1 - alpha;
	for (int y = 0; y < height; y++)
	{
		float * temp_x = &img_temp[y * width_channel];
		unsigned char * in_x = &img[y * width_channel];
		unsigned char * texture_x = &img[y * width_channel];
		*temp_x++ = ypr = *in_x++;
		*temp_x++ = ypg = *in_x++;
		*temp_x++ = ypb = *in_x++;
		unsigned char tpr = *texture_x++;
		unsigned char tpg = *texture_x++;
		unsigned char tpb = *texture_x++;

		float * temp_factor_x = &map_factor_a[y * width];
		*temp_factor_x++ = fp = 1;

		// from left to right
		for (int x = 1; x < width; x++)
		{
			unsigned char tcr = *texture_x++;
			unsigned char tcg = *texture_x++;
			unsigned char tcb = *texture_x++;
			unsigned char dr = abs(tcr - tpr);
			unsigned char dg = abs(tcg - tpg);
			unsigned char db = abs(tcb - tpb);
			int range_dist = (((dr << 1) + dg + db) >> 2);
			float weight = range_table[range_dist];
			float alpha_ = weight * alpha;
			*temp_x++ = ycr = inv_alpha_ * (*in_x++) + alpha_ * ypr;
			*temp_x++ = ycg = inv_alpha_ * (*in_x++) + alpha_ * ypg;
			*temp_x++ = ycb = inv_alpha_ * (*in_x++) + alpha_ * ypb;
			tpr = tcr; tpg = tcg; tpb = tcb;
			ypr = ycr; ypg = ycg; ypb = ycb;
			*temp_factor_x++ = fc = inv_alpha_ + alpha_ * fp;
			fp = fc;
		}
		*--temp_x; *temp_x = 0.5f*((*temp_x) + (*--in_x));
		*--temp_x; *temp_x = 0.5f*((*temp_x) + (*--in_x));
		*--temp_x; *temp_x = 0.5f*((*temp_x) + (*--in_x));
		tpr = *--texture_x;
		tpg = *--texture_x;
		tpb = *--texture_x;
		ypr = *in_x; ypg = *in_x; ypb = *in_x;

		*--temp_factor_x; *temp_factor_x = 0.5f*((*temp_factor_x) + 1);
		fp = 1;

		// from right to left
		for (int x = width - 2; x >= 0; x--)
		{
			unsigned char tcr = *--texture_x;
			unsigned char tcg = *--texture_x;
			unsigned char tcb = *--texture_x;
			unsigned char dr = abs(tcr - tpr);
			unsigned char dg = abs(tcg - tpg);
			unsigned char db = abs(tcb - tpb);
			int range_dist = (((dr << 1) + dg + db) >> 2);
			float weight = range_table[range_dist];
			float alpha_ = weight * alpha;

			ycr = inv_alpha_ * (*--in_x) + alpha_ * ypr;
			ycg = inv_alpha_ * (*--in_x) + alpha_ * ypg;
			ycb = inv_alpha_ * (*--in_x) + alpha_ * ypb;
			*--temp_x; *temp_x = 0.5f*((*temp_x) + ycr);
			*--temp_x; *temp_x = 0.5f*((*temp_x) + ycg);
			*--temp_x; *temp_x = 0.5f*((*temp_x) + ycb);
			tpr = tcr; tpg = tcg; tpb = tcb;
			ypr = ycr; ypg = ycg; ypb = ycb;

			fc = inv_alpha_ + alpha_ * fp;
			*--temp_factor_x;
			*temp_factor_x = 0.5f*((*temp_factor_x) + fc);
			fp = fc;
		}
	}
	alpha = static_cast<float>(exp(-sqrt(2.0) / (sigma_spatial * height)));
	inv_alpha_ = 1 - alpha;
	float * ycy, *ypy, *xcy;
	unsigned char * tcy, *tpy;
	memcpy(img_out_f, img_temp, sizeof(float)* width_channel);

	float * in_factor = map_factor_a;
	float*ycf, *ypf, *xcf;
	memcpy(map_factor_b, in_factor, sizeof(float) * width);
	for (int y = 1; y < height; y++)
	{
		tpy = &img[(y - 1) * width_channel];
		tcy = &img[y * width_channel];
		xcy = &img_temp[y * width_channel];
		ypy = &img_out_f[(y - 1) * width_channel];
		ycy = &img_out_f[y * width_channel];

		xcf = &in_factor[y * width];
		ypf = &map_factor_b[(y - 1) * width];
		ycf = &map_factor_b[y * width];
		for (int x = 0; x < width; x++)
		{
			unsigned char dr = abs((*tcy++) - (*tpy++));
			unsigned char dg = abs((*tcy++) - (*tpy++));
			unsigned char db = abs((*tcy++) - (*tpy++));
			int range_dist = (((dr << 1) + dg + db) >> 2);
			float weight = range_table[range_dist];
			float alpha_ = weight * alpha;
			for (int c = 0; c < channel; c++)
				*ycy++ = inv_alpha_ * (*xcy++) + alpha_ * (*ypy++);
			*ycf++ = inv_alpha_ * (*xcf++) + alpha_ * (*ypf++);
		}
	}
	int h1 = height - 1;
	ycf = line_factor_a;
	ypf = line_factor_b;
	memcpy(ypf, &in_factor[h1 * width], sizeof(float) * width);
	for (int x = 0; x < width; x++)
		map_factor_b[h1 * width + x] = 0.5f*(map_factor_b[h1 * width + x] + ypf[x]);

	ycy = slice_factor_a;
	ypy = slice_factor_b;
	memcpy(ypy, &img_temp[h1 * width_channel], sizeof(float)* width_channel);
	int k = 0;
	for (int x = 0; x < width; x++) {
		for (int c = 0; c < channel; c++) {
			int idx = (h1 * width + x) * channel + c;
			img_out_f[idx] = 0.5f*(img_out_f[idx] + ypy[k++]) / map_factor_b[h1 * width + x];
		}
	}

	for (int y = h1 - 1; y >= 0; y--)
	{
		tpy = &img[(y + 1) * width_channel];
		tcy = &img[y * width_channel];
		xcy = &img_temp[y * width_channel];
		float*ycy_ = ycy;
		float*ypy_ = ypy;
		float*out_ = &img_out_f[y * width_channel];

		xcf = &in_factor[y * width];
		float*ycf_ = ycf;
		float*ypf_ = ypf;
		float*factor_ = &map_factor_b[y * width];
		for (int x = 0; x < width; x++)
		{
			unsigned char dr = abs((*tcy++) - (*tpy++));
			unsigned char dg = abs((*tcy++) - (*tpy++));
			unsigned char db = abs((*tcy++) - (*tpy++));
			int range_dist = (((dr << 1) + dg + db) >> 2);
			float weight = range_table[range_dist];
			float alpha_ = weight * alpha;

			float fcc = inv_alpha_ * (*xcf++) + alpha_ * (*ypf_++);
			*ycf_++ = fcc;
			*factor_ = 0.5f * (*factor_ + fcc);

			for (int c = 0; c < channel; c++)
			{
				float ycc = inv_alpha_ * (*xcy++) + alpha_ * (*ypy_++);
				*ycy_++ = ycc;
				*out_ = 0.5f * (*out_ + ycc) / (*factor_);
				*out_++;
			}
			*factor_++;
		}
		memcpy(ypy, ycy, sizeof(float) * width_channel);
		memcpy(ypf, ycf, sizeof(float) * width);
	}

	for (int i = 0; i < width_height_channel; ++i)
		img[i] = static_cast<unsigned char>(img_out_f[i]);

	if (is_buffer_internal)
		delete[] buffer;

	return true;
}

