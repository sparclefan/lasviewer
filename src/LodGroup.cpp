#include "LodGroup.h"
#include <osg/Geode>
#include <osg/Switch>
#include <QDebug>

LodGroup::LodGroup(int classify, int firstLevel, osg::Group *parentNode)
    :m_classifier(classify), m_firstLevel(firstLevel),maxIntent(0),minIntent(100),
    colorMode(OverAll), m_parentNode(parentNode)
{
    // osg::ref_ptr<osg::Switch> classifySwitch = new osg::Switch();
    m_parentNode->addChild(this);
    // classifySwitch->addChild(this);

    setRangeMode(osg::LOD::DISTANCE_FROM_EYE_POINT);
    // setCenterMode(osg::LOD::USE_BOUNDING_SPHERE_CENTER);
    setCenterMode(osg::LOD::USER_DEFINED_CENTER);
    printf("=====LodGroup construct classifier %d\n", m_classifier);
}

LodGroup::~LodGroup()
{
    printf("#####LodGroup destruct classifier %d\n", m_classifier);
}

void LodGroup::traverse(osg::NodeVisitor& nv)
{
    switch (nv.getTraversalMode())    
    {
    case(osg::NodeVisitor::TRAVERSE_ALL_CHILDREN):
        // qDebug()<<"LodGroup traverse TRAVERSE_ALL_CHILDREN";
        break;
    case(osg::NodeVisitor::TRAVERSE_ACTIVE_CHILDREN):
        // qDebug()<<"LodGroup traverse TRAVERSE_ACTIVE_CHILDREN";
        break;
    default:
        // qDebug()<<"LodGroup traverse default";
        break;
    }

    // osg::Vec3 eye = nv.getEyePoint();
    // osg::Vec3 vp = nv.getViewPoint();
    // osg::Vec3 ct = getCenter();
    // printf("**pageLod traverse mode:%d,type:%d, dis1:%f,dis2:%f,dis3:%f\n", nv.getTraversalMode(), nv.getVisitorType(),
    //     nv.getDistanceFromEyePoint(getCenter(), true),
    //     nv.getDistanceToEyePoint(getCenter(), true),
    //     nv.getDistanceToViewPoint(getCenter(), true));
    // printf("**eye x:%f,y:%f,z:%f\n",eye.x(),eye.y(),eye.z());
    // printf("**viewPoint x:%f,y:%f,z:%f\n", vp.x(), vp.y(), vp.z());
    // printf("**Center x:%f,y:%f,z:%f\n",ct.x(),ct.y(),ct.z());

    osg::LOD::traverse(nv);
}

LodLayer *LodGroup::getLayer(int level, osg::StateSet *stateSet)
{
    if (m_layerList.find(level) != m_layerList.end())
    {
        return m_layerList[level];
    }

    // osg::ref_ptr<LodLayer> layer = new LodLayer(level, stateSet);
    LodLayer *layer = new LodLayer(this, level, stateSet);
    m_layerList[level] = layer;

    return layer;
}

void LodGroup::finishAddPoints(int level)
{
    osg::ref_ptr<LodLayer> layer = m_layerList[level];
    layer->finishAddPoints(m_firstLevel);

    // layer->m_geode->addDrawable(layer->getGeometry());
    // addChild(layer->m_geode.get(),0, LEVEL_DISTANSE[level-m_firstLevel]);

    switch(colorMode)
    {
    case Intent:
        layer->setIntentColor(intentColorMode, minIntent, maxIntent );
        break;
    case Altitude:
        layer->setAltitudeColor(altitudeColorMode, minAltitude, maxAltitude );
        break;
    case Rgb:
        layer->setRGBColor();
        break;
    default:
        break;
    }
}

void LodGroup::setOverallColor(QColor color)
{
    for(auto it : m_layerList)
    {
        osg::ref_ptr<LodLayer> layer = it.second;
        layer->setOverallColor(color);
    }
    colorMode = OverAll;    
}

void LodGroup::setIntentColor(int mode)
{
    for(auto it : m_layerList)
    {
        osg::ref_ptr<LodLayer> layer = it.second;
        layer->setIntentColor(mode, minIntent, maxIntent );
    }
    colorMode = Intent;
    intentColorMode = mode;
}

void LodGroup::setIntentRange(int max, int min, int mode)
{
	maxIntent = max;
	minIntent = min;

    if( colorMode == Intent ){
    	setIntentColor(mode);
    }
}

void LodGroup::setAltitudeColor(int mode)
{
    for(auto it : m_layerList)
    {
        osg::ref_ptr<LodLayer> layer = it.second;
        layer->setAltitudeColor(mode, minAltitude, maxAltitude );
    }
    colorMode = Altitude;
    altitudeColorMode = mode;
}

void LodGroup::setAltitudeRange(double maxAl, double minAl, int mode)
{
    maxAltitude = maxAl;
    minAltitude = minAl;

    if( colorMode == Altitude )
    {
        setAltitudeColor(mode);
    }
}

void LodGroup::setRGBColor()
{
    for(auto it : m_layerList)
    {
        osg::ref_ptr<LodLayer> layer = it.second;
        layer->setRGBColor();
    }
    colorMode = Rgb;
}

void LodGroup::setBlendColor(int modeIntent, int modeAltitude)
{
    for(auto it : m_layerList)
    {
        osg::ref_ptr<LodLayer> layer = it.second;
        layer->setBlendColor(modeIntent, modeAltitude, minIntent, maxIntent, minAltitude, maxAltitude);
    }
    colorMode = Blend;    
}