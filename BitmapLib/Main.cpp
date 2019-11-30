
#include "ImageShow.h"
#include "Image.h"

int main(int argc,char* argv[])
{
	//��ȡһ��λͼ
	ImageSpace::Image cBitmap;
	cBitmap.ReadBitmap("H://56.bmp");

	//��������������ʾλͼ�Ĵ���
	ImageShow::Create("Test");
	ImageShow::Create("Tow");

	//��λͼ�ҶȻ�
	//cBitmap.ToGray();

	//��λͼ���5��
	//cBitmap.ToScale(5, 5);

	//������ΪTest�Ĵ�����ʾλͼ
	ImageShow::Show("Test", cBitmap);

	//��λͼ��ֵ��
	//cBitmap.ToBinaryzation(128);

	//��λͼ��ɫ����
	//cBitmap.InvertColors();
	
	//�ݶ���
	//cBitmap.GradeSharp(30);

	//��ֵģ��ƽ��
	//cBitmap.MeanTemplateSmooth(5, 5, ImageSpace::MeanTemplate_Avg);

	//������˹��λͼ
	//cBitmap.LapTemplate(3, 3, ImageSpace::LapTemplate_Avg);

	//����һ��3x3��ģ��
	//int pTemplate[9]{ 0,1,0,1,1,1,0,1,0 };
	//����ͼ��ʴ
	//cBitmap.Erosion(3, 3, pTemplate);

	//��ֵ�˲�
	//cBitmap.MedianFilter(10, 10);

	//�ݹ�˫��ģ��
	cBitmap.Recursive_bilateral_filtering();

	//��ָ���Ĵ�����ʾλͼ
	ImageShow::Show("Tow", cBitmap);

	//��������λͼ����
	//cBitmap.WriteBitmap("H://data//222.bmp");

	//���봰��ѭ��
	ImageShow::GetMsgLoop();
	return 0;
}

