#ifndef __LasViwer_LodLayer_H_SPARCLE_2020_12_10
#define __LasViwer_LodLayer_H_SPARCLE_2020_12_10

#include <osg/Point>
#include <osg/Array>
#include <osg/Geometry>
#include <osg/Geode>
#include <QColor>
#include <QMutex>

const int FILTER_FACTOR[] = {100,50,30,10,3,1};
const float LEVEL_DISTANSE[] = {FLT_MAX, 1000, 500, 350, 300, 200};
const int MAX_FACTOR_LEVEL = sizeof(FILTER_FACTOR)/sizeof(int);
const QColor LEVEL_COLORS[] = {QColor(255,0,0), QColor(0,255,0), QColor(0,0,255), QColor(128,128,0), QColor(0,128,128), QColor(128,0,128)}; 

enum RenderColorMode {
	OverAll,
	Intent,
	Altitude,
	Blend,
	Rgb
};

#define OPTIMUM_POINT_NUMBER 100000

class LodGroup;

class LodLayer : public osg::Referenced {
    friend class LodGroup;
public:
    LodLayer(osg::LOD *parentNode, int level, osg::StateSet *stateSet);
    ~LodLayer();

    osg::Geode *createGeode();

    void addPoint(double x, double y, double z, int intent);
    void addPointColor(QColor &rgb);
	void finishAddPoints(int firstLevel);

    // 根据点数匹配抽稀级别，为提升加载显示效率，抽稀后点数控制在 OPTIMUM_POINT_NUMBER 左右
	static int preferLevel(unsigned int pointNumber);


private:
    osg::ref_ptr<osg::Geode> m_geode;
	osg::ref_ptr<osg::Vec3Array> m_pointsVertices;
	osg::ref_ptr<osg::Geometry> m_pointsGeometry;
	osg::ref_ptr<osg::Vec4Array> PointsIntentColor;
	osg::ref_ptr<osg::Vec4Array> PointsAltitudeColor;
	osg::ref_ptr<osg::Vec4Array> PointsBlendColor;
	osg::ref_ptr<osg::Vec4Array> m_pointsRGB;

	osg::ref_ptr<osg::LOD> m_parentNode;

	int intentColorMode;
	int altitudeColorMode;
	int blIntentColorMode;
	int blAltitudeColorMode;

    osg::Geometry *getGeometry();

    void setOverallColor(QColor color);
    void setRGBColor();
    void setAltitudeColor(int mode, double minAltitude, double maxAltitude);
    void setIntentColor(int mode, int minIntent, int maxIntent);
    void setBlendColor(int modeIntent, int modeAltitude, int minIntent, int maxIntent, double minAltitude, double maxAltitude);

    QList<int> m_intents;

    unsigned int m_pointNumber;
    int m_level;

	RenderColorMode colorMode;
	int m_maxIntent;
	int m_minIntent;
	double m_maxAltitude;
	double m_minAltitude;

	QMutex m_colorRenderMutex;

};


#endif //__LasViwer_LodLayer_H_SPARCLE_2020_12_10