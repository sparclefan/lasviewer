#ifndef __PointCloudLayer_h_SPAECLE_2019_1_28
#define __PointCloudLayer_h_SPAECLE_2019_1_28

#include <osg/Point>
#include <osg/Array>
#include <osg/Geometry>
#include <QColor>

class LasReader;

typedef QColor(*IntentColorCallBack)(double intentfactor, int mode);

class PointCloudLayer : public osg::Referenced{
	friend class LasReader;
public:
	PointCloudLayer(int classify);
	osg::Geometry *getGeometry();
	int getPointNumber(){ return pointNumber; };

	void setOverallColor(QColor color);
	void setIntentColor(IntentColorCallBack callback, int mode);

private:

	osg::ref_ptr<osg::Vec3Array> PointsVertices;
	osg::ref_ptr<osg::Geometry> PointsGeometry;
	osg::ref_ptr<osg::Vec4Array> PointsIntentColor;
	std::vector<double> m_intentFactors;
	int pointNumber;
	int intentColorMode;
	int classifier;
};
	
#endif //__PointCloudLayer_h_SPAECLE_2019_1_28