#pragma execution_character_set("utf-8")

#include "LodLayer.h"
#include "ColorUtils.h"
#include <osg/LOD>

LodLayer::LodLayer(osg::LOD *parentNode, int level, osg::StateSet *stateSet)
    :m_pointNumber(0), m_level(level), intentColorMode(-1), altitudeColorMode(-1),
	blIntentColorMode(-1), blAltitudeColorMode(-1),	colorMode(OverAll), 
	m_minIntent(0), m_maxIntent(0), m_minAltitude(0), m_maxAltitude(0),
	m_parentNode(parentNode)
{
    m_pointsVertices = new osg::Vec3Array;
    m_geode = new osg::Geode;
    m_geode->setStateSet(stateSet);

	// printf("LodLayer construct level %d\n", m_level);

}

LodLayer::~LodLayer()
{
	// printf("******LodLayer desctruct level %d\n", m_level);
}

int LodLayer::preferLevel(unsigned int pointNumber){

    int factor = pointNumber/OPTIMUM_POINT_NUMBER;

    for(int level=0; level<6; level++){
        if( factor>=FILTER_FACTOR[level] )	
            return level;		
    }
    return MAX_FACTOR_LEVEL;
};

void LodLayer::addPoint(double x, double y, double z, int intent)
{
    m_pointsVertices->push_back(osg::Vec3d(x,y,z));
    m_intents.push_back(intent);
    m_pointNumber++;
}

void LodLayer::addPointColor(QColor &rgb)
{
    if( !m_pointsRGB.valid()){
        m_pointsRGB = new osg::Vec4Array;
    }

    m_pointsRGB->push_back(osg::Vec4(rgb.redF(), rgb.greenF(), rgb.blueF(), 1.0));
}

void LodLayer::finishAddPoints(int firstLevel)
{
    m_geode->addDrawable(getGeometry());
    m_parentNode->addChild( m_geode.get(),0, LEVEL_DISTANSE[m_level- firstLevel]);
	osg::BoundingSphere bs = m_geode->getBound();
	osg::Vec3 c = bs.center();

	// printf("boundspere: center %f, %f, %f, rad: %f\n", c.x(), c.y(), c.z(), bs.radius());
}

osg::Geometry *LodLayer::getGeometry()
{
    if( m_pointsGeometry.valid())
        return m_pointsGeometry.get();

    m_pointsGeometry = new osg::Geometry;
    m_pointsGeometry->setVertexArray(m_pointsVertices);
    m_pointsGeometry->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::POINTS, 0, m_pointsVertices->size()));

	// osg::ref_ptr<osg::Vec4Array> tmpColor = new osg::Vec4Array;
    // QColor color = LEVEL_COLORS[m_level];
	// tmpColor->push_back(osg::Vec4(color.redF(), color.greenF(), color.blueF(), 1.0));
	// m_pointsGeometry->setColorArray(tmpColor.get());
	// m_pointsGeometry->setColorBinding(osg::Geometry::BIND_OVERALL);

    return m_pointsGeometry.get();

}

void LodLayer::setOverallColor(QColor color)
{
	if (!m_pointsGeometry.valid())
		return;

	osg::ref_ptr<osg::Vec4Array> tmpColor = new osg::Vec4Array;
	tmpColor->push_back(osg::Vec4(color.redF(), color.greenF(), color.blueF(), 1.0));
	m_pointsGeometry->setColorArray(tmpColor.get());
	m_pointsGeometry->setColorBinding(osg::Geometry::BIND_OVERALL);
	colorMode = OverAll;
}

void LodLayer::setRGBColor()
{
	if (!m_pointsGeometry.valid())
		return;

	if (!m_pointsRGB.valid())
	{
		setOverallColor(QColor(100, 100, 100));
		return;
	}
	m_pointsGeometry->setColorArray(m_pointsRGB.get());
	m_pointsGeometry->setColorBinding(osg::Geometry::BIND_PER_VERTEX);
	colorMode = Rgb;
}

void LodLayer::setIntentColor(int mode, int minIntent, int maxIntent)
{
	QMutexLocker lock(&m_colorRenderMutex);

	if (!m_pointsGeometry.valid())
		return;

	if (!PointsIntentColor.valid())
		PointsIntentColor = new osg::Vec4Array;

	if (intentColorMode != mode || m_minIntent != minIntent || m_maxIntent != maxIntent )
	{
		PointsIntentColor->clear();
		for (int intent : m_intents)
		{
			double factor = ((double)intent-minIntent)/(maxIntent-minIntent);
			QColor rgb = ColorUtils::getIntentColor(factor, mode);
			PointsIntentColor->push_back(osg::Vec4(rgb.redF(), rgb.greenF(), rgb.blueF(), 1.0));
		}
		intentColorMode = mode;
		m_minIntent = minIntent;
		m_maxIntent = maxIntent;
	}

	m_pointsGeometry->setColorArray(PointsIntentColor.get());
	m_pointsGeometry->setColorBinding(osg::Geometry::BIND_PER_VERTEX);
	colorMode = Intent;
}

void LodLayer::setAltitudeColor(int mode, double minAltitude, double maxAltitude)
{
	QMutexLocker lock(&m_colorRenderMutex);

	if (!m_pointsGeometry.valid())
		return;

	if (!PointsAltitudeColor.valid())
		PointsAltitudeColor = new osg::Vec4Array;

	if (altitudeColorMode != mode)
	{
		PointsAltitudeColor->clear();
		for( osg::Vec3d vec : m_pointsVertices->asVector() )
		{
			double factor = (vec.z()-minAltitude)/(maxAltitude - minAltitude);
			QColor rgb = ColorUtils::getIntentColor(factor, mode);
			PointsAltitudeColor->push_back(osg::Vec4(rgb.redF(), rgb.greenF(), rgb.blueF(), 1.0));
		}

		altitudeColorMode = mode;
	}

	m_pointsGeometry->setColorArray(PointsAltitudeColor.get());
	m_pointsGeometry->setColorBinding(osg::Geometry::BIND_PER_VERTEX);

	colorMode = Altitude;

}

void LodLayer::setBlendColor(int modeIntent, int modeAltitude, int minIntent, int maxIntent, double minAltitude, double maxAltitude)
{
	if (!m_pointsGeometry.valid())
		return;

	if (!PointsBlendColor.valid())
		PointsBlendColor = new osg::Vec4Array;

	if ((blAltitudeColorMode != modeAltitude) || (blIntentColorMode != modeIntent))
	{
		PointsBlendColor->clear();
		int n = m_intents.size();

		for (int i = 0; i < n; i++)
		{	
			double factor1 = ((double)m_intents[i]-minIntent)/(maxIntent-minIntent);
			osg::Vec3d vec = m_pointsVertices->asVector().at(i);
			double factor2 = (vec.z()-minAltitude)/(maxAltitude - minAltitude);
			QColor rgb = ColorUtils::getBlendColor(factor1, factor2, modeIntent, modeAltitude);
			PointsBlendColor->push_back(osg::Vec4(rgb.redF(), rgb.greenF(), rgb.blueF(), 1.0));
		}
		blAltitudeColorMode = modeAltitude;
		blIntentColorMode = modeIntent;
	}

	m_pointsGeometry->setColorArray(PointsBlendColor.get());
	m_pointsGeometry->setColorBinding(osg::Geometry::BIND_PER_VERTEX);

	colorMode = Blend;

}