#pragma execution_character_set("utf-8")

#include "ColorUtils.h"

QColor ColorUtils::getIntentColor(double factor, int mode)
{
	float H = caculateColorH(factor, mode); //1.0 / 3 + (factor)*2.0 / 3; //(2.0 / 3)*(1.0-factor);
	float L = 1.0;
	float S = 0.5;
	float r, g, b;
	HSL2RGB(H, L, S, r, g, b);
	return QColor((int)(r*255),(int)(g*255),(int)(b*255));
}

QColor ColorUtils::getBlendColor(double f1, double f2, int mode1, int mode2)
{
	float H1 = caculateColorH(f1, mode1); 
	float H2 = caculateColorH(f2, mode2);
	float L = 1.0;
	float S = 0.5;
	float r1, g1, b1, r2, g2, b2;
	HSL2RGB(H1, L, S, r1, g1, b1);
	HSL2RGB(H2, L, S, r2, g2, b2);
	float r = crCalculateBlend(r1, r2);
	float g = crCalculateBlend(g2, g2);
	float b = crCalculateBlend(b1, b2);
	return QColor((int)(r * 255), (int)(g * 255), (int)(b * 255));
}

float ColorUtils::caculateColorH(float factor, int mode)
{
	switch (mode){
	case 0:
		return ((1.0 - factor)*2.0 / 3) - (1.0 / 3);    // 绿-红-蓝
	case 1:
		return (2.0 / 3)*(1.0 - factor);   // 蓝-绿-红
	case 2:
		return (1.0 / 3 + (factor)*2.0 / 3);  // 绿-蓝-红
	case 3:
		return 1.0/3 + factor/3;  // 绿-蓝
	case 4:
		return 1.0 / 3 - factor / 3;  // 绿-红
	default:
		return ((1.0 - factor)*2.0 / 3) - (1.0 / 3);    // 绿-红-蓝
	}
}

float ColorUtils::Hue_2_RGB(float v1, float v2, float vH)             //Function Hue_2_RGB
{
	if (vH < 0) vH += 1;
	if (vH > 1) vH -= 1;
	if ((6 * vH) < 1)
		return (v1 + (v2 - v1) * 6 * vH);
	if ((2 * vH) < 1)
		return (v2);

	if ((3 * vH) < 2)
		return (v1 + (v2 - v1) * ((2.0 / 3) - vH) * 6);

	return (v1);
}

void ColorUtils::HSL2RGB(float H, float S, float L, float &R, float &G, float &B)
{
	if (S == 0)                       //HSL from 0 to 1
	{
		R = L;                      //RGB results from 0 to 1
		G = L;
		B = L;
	}
	else
	{
		float var_2;
		if (L < 0.5)
			var_2 = L * (1 + S);
		else
			var_2 = (L + S) - (S * L);

		float var_1 = 2 * L - var_2;

		R = Hue_2_RGB(var_1, var_2, H + (1.0 / 3));
		G = Hue_2_RGB(var_1, var_2, H);
		B = Hue_2_RGB(var_1, var_2, H - (1.0 / 3));
	}
}

float ColorUtils::crCalculateBlend( float c1, float c2)
{
	//(c1 * a1 * (1.0 - a2) + c2 ) / (a1 + a2 - a1 * a2);
	return (c1*0.7 + c2*0.3);
}