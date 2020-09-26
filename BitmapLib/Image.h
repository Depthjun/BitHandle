#pragma once

#include "BitmapStruct.h"
#include <math.h>

namespace ImageSpace
{
	//��ֵģ���ƽ��ģ��
	static double MeanTemplate_Avg[25] =
	{
		0.0,1.0,2.0,1.0,0.0,
		1.0,2.0,4.0,2.0,1.0,
		2.0,4.0,8.0,4.0,2.0,
		1.0,2.0,4.0,2.0,1.0,
		0.0,1.0,2.0,1.0,0.0,
	};

	//������˹������
	static double LapTemplate_Four[9] =
	{
		0.0,-1.0,0.0,
		-1.0,4.0,-1.0,
		0.0,-1.0,0.0
	};

	//������˹������
	static double LapTemplate_Eight[9] =
	{
		-1.0,-1.0,-1.0,
		-1.0,8.0,-1.0,
		-1.0,-1.0,-1.0,
	};

	//������˹ƽ��ģ��
	static double LapTemplate_Avg[9] =
	{
		-1.0,-1.0,-1.0,
		-1.0,9.0,-1.0,
		-1.0,-1.0,-1.0,
	};

	class Image
	{
	public:
		BitmapStruct m_cImage;		//λͼ��Ϣ

	public:
		Image();
		Image(BitmapStruct& cImage);
		Image(Image& cImage);
		~Image();

		Image& operator=(Image& cImage);
		Image& operator=(BitmapStruct& cImage);

		//��ȸ���
		void CopyTo(Image& cImage);

	public:
		//��ȡλͼ��Ϣ
		bool ReadBitmap(const char* szBitmapPath);

		//д��λͼ��Ϣ
		bool WriteBitmap(const char* szBitmapPath);

		//�������ݵ�ɫ��
		HPALETTE CreateBitmapPalete();

		//ת��Ϊ�Ҷ�ͼ
		bool ToGray();

		//ת��λ��ֵ��ͼ
		bool ToBinaryzation(int nThreshold);				//��ֵ

		//��ɫ��ǿ
		bool ColorEnhancement(int nThreshold = 255 / 2);	//��ֵ

		//ת��λ��ɫͼ
		bool InvertColors();

		//������ȡ
		bool SubMatrix(int nPosX,		//��ʼλ��X
			int nPosY,								//��ʼλ��Y
			int nWidth,							//���ο��
			int nHeight);						//���θ߶�

		//λͼ�ƶ�
		bool ToMove(int nPosX,			//��ʼλ��X
			int nPosY,								//��ʼλ��Y
			int nColor = 255);				//�����ɫ

		//λͼ����
		bool ToScale(float fMultipleX,		//�������
			float fMultipleY);						//�߶�����

		//ˮƽ����
		bool LevelMirror();

		//��ֱ����
		bool VerticalMirror();

		//λͼ��ת
		bool Rotate(float fAngle,		//ѡ��Ƕ�
			int nColor = 255);			//�����ɫ

		//��ӽ�������
		bool AddSpicedSaltNoise();

		//��Ӹ�˹����
		bool AddGaussNoise();

		//��ɫ�ݶ���-����
		bool WhiteGradeSharp(int nThresh);	//��ֵ

		//�ݶ���
		bool GradeSharp(int nThresh);		//��ֵ

		//���ڱ任
		bool ThresholdWindow(int nTop,		//���ֵ
			int nButtom);									//���ֵ

		//��������
		bool LinearStrech(int nPosX1,		//��һ��X
			int nPosY1,								//��һ��Y
			int nPosX2,								//�ڶ���X
			int nPosY2);								//�ڶ���Y

		//Roberts��Ե��ȡ
		bool Roberts();

		//Sobel��Ե��ȡ
		bool Sobel();

		//Prewitt��Ե��ȡ
		bool Prewitt();

		//Laplacian��Ե��ȡ
		bool Laplacian();

		//����Ӧ��ֵ�ָ�
		bool AdaptThreshSeg();

		//��ֵģ��ƽ��
		bool MeanTemplateSmooth(int nTemplateWidth = 7,	//ģ����
			int nTemplateHeight = 7,											//ģ��߶�
			double *pTemplate = NULL,										//ģ������
			int nIgnoreX = 0,															//���Զ����У�����ȡģ���ȵ�һ��
			int nIgnoreY = 0,															//���Զ����У�����ȡģ��߶ȵ�һ��
			double dCoef = 0.0);													//���ʣ����ƹ�����

		//��ֵ�˲�
		bool MedianFilter(int nTemplateWidth = 5,	//ģ����
			int nTemplateHeight = 5,							//ģ��߶�
			int nIgnoreX = 0,											//���Զ����У�����ȡģ���ȵ�һ��
			int nIgnoreY = 0);										//���Զ����У�����ȡģ��߶ȵ�һ��

		//��ģƽ��
		bool MaskSmooth();

		//������˹��
		bool LapTemplate(int nTemplateWidth,		//ģ����
			int nTemplateHeight,								//ģ��߶�
			double *pTemplate,								//ģ������
			int nIgnoreX = 0,										//���Զ����У�����ȡģ���ȵ�һ��
			int nIgnoreY = 0,										//���Զ����У�����ȡģ��߶ȵ�һ��
			double dCoef = 0.0);

		//��ʴ
		bool Erosion(int nTemplateWidth,			//ģ����
			int nTemplateHeight,							//ģ��߶�
			const int *pTemplate);							//ģ������

		//����
		bool Dilate(int nTemplateWidth,				//ģ����
			int nTemplateHeight,								//ģ��߶�
			const int *pTemplate);								//ģ������

		//������
		bool Open(int nTemplateWidth,				//ģ����
			int nTemplateHeight,							//ģ��߶�
			const int *pTemplate);							//ģ������

		//������
		bool Close(int nTemplateWidth,				//ģ����
			int nTemplateHeight,							//ģ��߶�
			const int *pTemplate);							//ģ������

		//���л�����ϸ��
		bool ImgThinning();

		//�ݹ�˫���˲�
		bool Recursive_bilateral_filtering(float sigma_spatial = 0.03, float sigma_range = 0.1, float * buffer = 0);
	};
}