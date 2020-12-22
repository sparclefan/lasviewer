#ifndef __PointCloudLayer_h_SPAECLE_2019_1_28
#define __PointCloudLayer_h_SPAECLE_2019_1_28

#include <osg/Point>
#include <osg/Array>
#include <osg/Geometry>
#include <osg/LOD>
#include <QColor>
#include <QList>

class LasReader;

class PointCloudLayer : public osg::Referenced{
	friend class LasReader;
public:
	PointCloudLayer(int classify);
	osg::Geometry *getGeometry();
	int getPointNumber(){ return pointNumber; };

	void setOverallColor(QColor color);
	void setIntentColor(int mode);
	void setAltitudeColor(int mode);
	void setAltitudeRange(double maxAl, double minAl, int mode);
	void setIntentRange(int max, int min, int mode);
	void setBlendColor(int modeIntent, int modeAltitude);
	void setRGBColor();

	inline int getMaxIntent() const { return maxIntent;};
	inline int getMinIntent() const { return minIntent;};

private:

	osg::ref_ptr<osg::Vec3Array> PointsVertices;
	osg::ref_ptr<osg::Geometry> PointsGeometry;
	osg::ref_ptr<osg::Vec4Array> PointsIntentColor;
	osg::ref_ptr<osg::Vec4Array> PointsAltitudeColor;
	osg::ref_ptr<osg::Vec4Array> PointsBlendColor;
	osg::ref_ptr<osg::Vec4Array> PointsRGB;
	QList<int> m_intents;
	// QList<double> m_altitudeFactors;
	int pointNumber;
	int intentColorMode;
	int altitudeColorMode;
	int blIntentColorMode;
	int blAltitudeColorMode;
	int classifier;

	int maxIntent;
	int minIntent;
	double maxAltitude;
	double minAltitude;
};

	
#endif //__PointCloudLayer_h_SPAECLE_2019_1_28