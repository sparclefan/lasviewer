#ifndef __PointCloudLayer_h_SPAECLE_2019_1_28
#define __PointCloudLayer_h_SPAECLE_2019_1_28

#include <osg/Point>
#include <osg/Array>
#include <osg/Geometry>
#include <QColor>

class LasReader;

typedef QColor(*IntentColorCallBack)(double intentfactor, int mode);
typedef QColor(*BlendColorCallBack)(double factorIntent, double factorAltitude, int modeIntent, int modeAltitude);

class PointCloudLayer : public osg::Referenced{
	friend class LasReader;
public:
	PointCloudLayer(int classify);
	osg::Geometry *getGeometry();
	int getPointNumber(){ return pointNumber; };

	void setOverallColor(QColor color);
	void setIntentColor(IntentColorCallBack callback, int mode);
	void setAltitudeColor(IntentColorCallBack callback, int mode);
	void setBlendColor(BlendColorCallBack callback, int modeIntent, int modeAltitude);
	void setRGBColor();

private:

	osg::ref_ptr<osg::Vec3Array> PointsVertices;
	osg::ref_ptr<osg::Geometry> PointsGeometry;
	osg::ref_ptr<osg::Vec4Array> PointsIntentColor;
	osg::ref_ptr<osg::Vec4Array> PointsAltitudeColor;
	osg::ref_ptr<osg::Vec4Array> PointsBlendColor;
	osg::ref_ptr<osg::Vec4Array> PointsRGB;
	std::vector<double> m_intentFactors;
	std::vector<double> m_altitudeFactors;
	int pointNumber;
	int intentColorMode;
	int altitudeColorMode;
	int blIntentColorMode;
	int blAltitudeColorMode;
	int classifier;
};
	
#endif //__PointCloudLayer_h_SPAECLE_2019_1_28