#ifndef __LASVIEWER_ColorUtil_H_SPARCLE_2020_11_06
#define __LASVIEWER_ColorUtil_H_SPARCLE_2020_11_06

#include <QColor>

typedef struct{
	QColor color;
	char name[32];
} ClassifyCfg;

const ClassifyCfg classifyColor[] = {
	{ QColor(255, 100, 100), "导线" },
	{ QColor(160, 160, 160), "下面的导线" },
	{ QColor(239, 250, 19), "其他导线" },
	{ QColor(170, 0, 127), "地线" },
	{ QColor(0, 180, 0), "低植被" },
	{ QColor(0, 255, 0), "高植被" },
	{ QColor(0, 255, 0), "植被" },
	{ QColor(139, 133, 30), "地面" },

	{ QColor(128, 64, 64), "地面关键点" },
	{ QColor(0, 0, 255), "建筑物" },
	{ QColor(80, 80, 255), "临时建筑物" },
	{ QColor(60, 60, 60), "道路" },
	{ QColor(170, 170, 170), "桥梁" },
	{ QColor(100, 60, 100), "铁路" },
	{ QColor(255, 128, 128), "上跨越" },
	{ QColor(255, 128, 128), "下跨越" },

	{ QColor(234, 236, 255), "杆塔" },
	{ QColor(190, 190, 190), "承力索/接触线" },
	{ QColor(151, 219, 242), "河流" },
	{ QColor(151, 219, 242), "通航河流" },
	{ QColor(151, 219, 242), "不通航河流" },
	{ QColor(255, 0, 0), "危险点" },
	{ QColor(255, 200, 50), "绝缘子" },
	{ QColor(19, 1, 0), "其他" },

	{ QColor(128, 128, 128), "噪声" },
	{ QColor(236, 236, 236), "变电站" },
	{ QColor(236, 236, 236), "汽车船舶" },
	{ QColor(151, 219, 242), "湖泊" },
	{ QColor(255, 123, 0), "引流线" },
	{ QColor(255, 0, 255), "平断面图地面线1" },
	{ QColor(128, 0, 255), "平断面图地面线2" },
	{ QColor(160, 160, 160), "无类别" }
};

const int classifyColorNum = sizeof(classifyColor) / sizeof(ClassifyCfg);

class ColorUtils 
{
public:
	static QColor getIntentColor(double factor, int mode);
	static QColor getBlendColor(double f1, double f2, int mode1, int mode2);
	static float caculateColorH(float factor, int mode);
	static float Hue_2_RGB(float v1, float v2, float vH);
	static void HSL2RGB(float H, float S, float L, float &R, float &G, float &B);

private:
	static float crCalculateBlend( float c1, float c2);

};

#endif //__LASVIEWER_ColorUtil_H_SPARCLE_2020_11_06