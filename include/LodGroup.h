#ifndef __LasViewer_LodGroup_H_SPARCLE_2020_12_10
#define __LasViewer_LodGroup_H_SPARCLE_2020_12_10

#include "LodLayer.h"
#include <osg/LOD>
#include <QList>

class LasReader;

class LodGroup : public osg::LOD
{
    friend class LasReader;
public:
    LodGroup(int classify, int firstLevel, osg::Group *parentNode);
    ~LodGroup();

    virtual void traverse(osg::NodeVisitor& nv);

    LodLayer * getLayer(int level, osg::StateSet *stateSet);

    inline int getMaxIntent() const { return maxIntent; };
    inline int getMinIntent() const { return minIntent; };
    inline double getMaxAltitude() const { return maxAltitude; };
    inline double getMinAltitude() const { return minAltitude; };

private:

	std::map<int, osg::ref_ptr<LodLayer> > m_layerList;
    osg::ref_ptr<Group> m_parentNode;

    void setOverallColor(QColor color);
    void setRGBColor();
    void setIntentColor(int mode);
    void setIntentRange(int max, int min, int mode);
    void setAltitudeColor(int mode);
    void setAltitudeRange(double maxAl, double minAl, int mode);
    void setBlendColor(int modeIntent, int modeAltitude);

    void finishAddPoints(int level);

    int m_classifier;
    int m_firstLevel;

	int maxIntent;
	int minIntent;
	double maxAltitude;
	double minAltitude;
    RenderColorMode colorMode;
    int intentColorMode;
    int altitudeColorMode;
};


#endif //__LasViewer_LodGroup_H_SPARCLE_2020_12_10