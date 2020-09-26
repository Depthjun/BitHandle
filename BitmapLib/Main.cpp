#include "ImageShow.h"
#include "Image.h"

INT WINAPI WinMain(
	_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPSTR lpCmdLine,
	_In_ int nShowCmd)
{
	UNREFERENCED_PARAMETER(hInstance);
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);
	UNREFERENCED_PARAMETER(nShowCmd);

	OPENFILENAMEA ofn{ 0 };
	TCHAR strFilename[MAX_PATH]{ 0 };//���ڽ����ļ���
	ofn.lStructSize = sizeof(OPENFILENAMEA);//�ṹ���С
	ofn.hwndOwner = NULL;//ӵ���Ŵ��ھ����ΪNULL��ʾ�Ի����Ƿ�ģ̬�ģ�ʵ��Ӧ����һ�㶼Ҫ��������
	ofn.lpstrFilter = TEXT("�����ļ�\0*.bmp\0\0");//���ù���
	ofn.nFilterIndex = 1;//����������
	ofn.lpstrFile = strFilename;//���շ��ص��ļ�����ע���һ���ַ���ҪΪNULL
	ofn.nMaxFile = sizeof(strFilename);//����������
	ofn.lpstrInitialDir = NULL;//��ʼĿ¼ΪĬ��
	ofn.lpstrTitle = TEXT("��ѡ��һ���ļ�");//ʹ��ϵͳĬ�ϱ������ռ���
	ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_HIDEREADONLY;//�ļ���Ŀ¼������ڣ�����ֻ��ѡ��
	BOOL State = GetOpenFileName(&ofn);
	if (State == FALSE)
	{
		MessageBox(NULL, strFilename, TEXT("��Ҫѡ��һ��Bmp�ļ�"), 0);
		exit(-1);
	}

	//ԭʼ
	ImageSpace::Image Origin, Result;
	Origin.ReadBitmap(strFilename);
	ImageShow::PushWindow("ԭʼ", Origin);

	//�ҶȻ�
	{
		Origin.CopyTo(Result);
		Result.ToGray();
		ImageShow::PushWindow("�ҶȻ�", Result);
	}

	//��ֵ��
	{
		Origin.CopyTo(Result);
		Result.ToGray();
		Result.ToBinaryzation(100);
		ImageShow::PushWindow("��ֵ��", Result);
	}

	//��ɫ��ǿ
	{
		Origin.CopyTo(Result);
		Result.ColorEnhancement(100);
		ImageShow::PushWindow("��ɫ��ǿ", Result);
	}

	//��ɫ
	{
		Origin.CopyTo(Result);
		Result.InvertColors();
		ImageShow::PushWindow("��ɫ", Result);
	}

	//������ȡ
	{
		Origin.CopyTo(Result);
		Result.SubMatrix(0, 0, 200, 200);
		ImageShow::PushWindow("������ȡ", Result);
	}

	//λͼ�ƶ�
	{
		Origin.CopyTo(Result);
		Result.ToMove(100, 100);
		ImageShow::PushWindow("λͼ�ƶ�", Result);
	}

	//λͼ����
	{
		Origin.CopyTo(Result);
		Result.ToScale(2, 2);
		ImageShow::PushWindow("λͼ����", Result);
	}

	//ˮƽ����
	{
		Origin.CopyTo(Result);
		Result.LevelMirror();
		ImageShow::PushWindow("ˮƽ����", Result);
	}

	//��ֱ����
	{
		Origin.CopyTo(Result);
		Result.VerticalMirror();
		ImageShow::PushWindow("��ֱ����", Result);
	}

	//λͼ��ת
	{
		Origin.CopyTo(Result);
		Result.Rotate(90.0f);
		ImageShow::PushWindow("λͼ��ת", Result);
	}

	//��������
	{
		Origin.CopyTo(Result);
		Result.AddSpicedSaltNoise();
		ImageShow::PushWindow("��������", Result);
	}

	//��˹����
	{
		Origin.CopyTo(Result);
		Result.ToGray();
		Result.ToBinaryzation(100);
		Result.AddGaussNoise();
		ImageShow::PushWindow("��˹����", Result);
	}

	//��ɫ�ݶ���
	{
		Origin.CopyTo(Result);
		Result.ToGray();
		Result.ToBinaryzation(100);
		Result.WhiteGradeSharp(100);
		ImageShow::PushWindow("��ɫ�ݶ���", Result);
	}

	//�ݶ���
	{
		Origin.CopyTo(Result);
		Result.ToGray();
		Result.ToBinaryzation(100);
		Result.GradeSharp(100);
		ImageShow::PushWindow("�ݶ���", Result);
	}

	//���ڱ任
	{
		Origin.CopyTo(Result);
		Result.ThresholdWindow(50, 150);
		ImageShow::PushWindow("���ڱ任", Result);
	}

	//��������
	{
		Origin.CopyTo(Result);
		Result.LinearStrech(10, 10, 100, 100);
		ImageShow::PushWindow("��������", Result);
	}

	//Roberts��Ե��ȡ
	{
		Origin.CopyTo(Result);
		Result.Roberts();
		ImageShow::PushWindow("Roberts��Ե��ȡ", Result);
	}

	//Sobel��Ե��ȡ
	{
		Origin.CopyTo(Result);
		Result.Sobel();
		ImageShow::PushWindow("Sobel��Ե��ȡ", Result);
	}

	//Prewitt��Ե��ȡ
	{
		Origin.CopyTo(Result);
		Result.Prewitt();
		ImageShow::PushWindow("Prewitt��Ե��ȡ", Result);
	}

	//Laplacian��Ե��ȡ
	{
		Origin.CopyTo(Result);
		Result.Laplacian();
		ImageShow::PushWindow("Laplacian��Ե��ȡ", Result);
	}

	//����Ӧ��ֵ�ָ�
	{
		Origin.CopyTo(Result);
		Result.ToGray();
		Result.ToBinaryzation(100);
		Result.AdaptThreshSeg();
		ImageShow::PushWindow("����Ӧ��ֵ�ָ�", Result);
	}

	//��ֵģ��ƽ��
	{
		Origin.CopyTo(Result);
		Result.ToGray();
		Result.MeanTemplateSmooth();
		ImageShow::PushWindow("��ֵģ��ƽ��", Result);
	}

	//��ֵ�˲�
	{
		Origin.CopyTo(Result);
		Result.ToGray();
		Result.ToBinaryzation(100);
		Result.MedianFilter();
		ImageShow::PushWindow("��ֵ�˲�", Result);
	}

	//��ģƽ��
	{
		Origin.CopyTo(Result);
		Result.ToGray();
		Result.ToBinaryzation(100);
		Result.MaskSmooth();
		ImageShow::PushWindow("��ģƽ��", Result);
	}

	//������˹��
	{
		Origin.CopyTo(Result);
		Result.ToGray();
		Result.ToBinaryzation(100);
		Result.LapTemplate(3, 3, ImageSpace::LapTemplate_Avg);
		ImageShow::PushWindow("������˹��", Result);
	}

	//��ʴ
	{
		//����һ��3x3��ģ��
		int pTemplate[9]{ 0,1,0,1,1,1,0,1,0 };

		Origin.CopyTo(Result);
		Result.ToGray();
		Result.ToBinaryzation(100);
		Result.Erosion(3, 3, pTemplate);
		ImageShow::PushWindow("��ʴ", Result);
	}

	//����
	{
		//����һ��3x3��ģ��
		int pTemplate[9]{ 0,1,0,1,1,1,0,1,0 };

		Origin.CopyTo(Result);
		Result.ToGray();
		Result.ToBinaryzation(100);
		Result.Dilate(3, 3, pTemplate);
		ImageShow::PushWindow("����", Result);
	}

	//������
	{
		//����һ��3x3��ģ��
		int pTemplate[9]{ 0,1,0,1,1,1,0,1,0 };

		Origin.CopyTo(Result);
		Result.ToGray();
		Result.ToBinaryzation(100);
		Result.Dilate(3, 3, pTemplate);
		ImageShow::PushWindow("������", Result);
	}

	//������
	{
		//����һ��3x3��ģ��
		int pTemplate[9]{ 0,1,0,1,1,1,0,1,0 };

		Origin.CopyTo(Result);
		Result.ToGray();
		Result.ToBinaryzation(100);
		Result.Close(3, 3, pTemplate);
		ImageShow::PushWindow("������", Result);
	}

	//���л�����ϸ��
	{
		Origin.CopyTo(Result);
		Result.ToGray();
		Result.ToBinaryzation(100);
		Result.ImgThinning();
		ImageShow::PushWindow("���л�����ϸ��", Result);
	}

	//�ݹ�˫���˲�
	{
		Origin.CopyTo(Result);
		Result.Recursive_bilateral_filtering();
		ImageShow::PushWindow("�ݹ�˫���˲�", Result);
	}

	//���봰��ѭ��
	ImageShow::MsgHandle();
	return 0;
}