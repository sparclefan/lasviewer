#include "PointCloudLayer.h"
#include "ColorUtils.h"

PointCloudLayer::PointCloudLayer(int classify)
	:classifier(classify), pointNumber(0), intentColorMode(-1), altitudeColorMode(-1),
	blIntentColorMode(-1), blAltitudeColorMode(-1), minIntent(100), maxIntent(0)
{
	PointsVertices = new osg::Vec3Array;
}

osg::Geometry *PointCloudLayer::getGeometry()
{
	if (PointsGeometry.valid())
		return PointsGeometry.get();

	PointsGeometry = new osg::Geometry;
	PointsGeometry->setVertexArray(PointsVertices.get());
	PointsGeometry->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::POINTS, 0, PointsVertices->size()));
	return PointsGeometry.get();
}

void PointCloudLayer::setOverallColor(QColor color)
{
	if (!PointsGeometry.valid())
		return;

	osg::ref_ptr<osg::Vec4Array> tmpColor = new osg::Vec4Array;
	tmpColor->push_back(osg::Vec4(color.redF(), color.greenF(), color.blueF(), 1.0));
	PointsGeometry->setColorArray(tmpColor.get());
	PointsGeometry->setColorBinding(osg::Geometry::BIND_OVERALL);
}

void PointCloudLayer::setIntentColor(int mode)
{
	if (!PointsGeometry.valid())
		return;

	if (!PointsIntentColor.valid())
		PointsIntentColor = new osg::Vec4Array;

	if (intentColorMode != mode)
	{
		PointsIntentColor->clear();
		for (int intent : m_intents)
		{
			double factor = ((double)intent-minIntent)/(maxIntent-minIntent);
			QColor rgb = ColorUtils::getIntentColor(factor, mode);
			PointsIntentColor->push_back(osg::Vec4(rgb.redF(), rgb.greenF(), rgb.blueF(), 1.0));
		}
		intentColorMode = mode;
	}

	PointsGeometry->setColorArray(PointsIntentColor.get());
	PointsGeometry->setColorBinding(osg::Geometry::BIND_PER_VERTEX);
}

void PointCloudLayer::setAltitudeRange(double maxAl, double minAl, int mode)
{
	maxAltitude = maxAl;
	minAltitude = minAl;

	if (!PointsGeometry.valid())
		return;

	if (!PointsAltitudeColor.valid())
		return;

	PointsAltitudeColor->clear();
	for( osg::Vec3d vec : PointsVertices->asVector() )
	{
		double factor = (vec.z()-minAltitude)/(maxAltitude - minAltitude);
		QColor rgb = ColorUtils::getIntentColor(factor, mode);
		PointsAltitudeColor->push_back(osg::Vec4(rgb.redF(), rgb.greenF(), rgb.blueF(), 1.0));
	}

	altitudeColorMode = mode;
}

void PointCloudLayer::setIntentRange(int max, int min, int mode)
{
	maxIntent = max;
	minIntent = min;

	setIntentColor(mode);

}


void PointCloudLayer::setAltitudeColor(int mode)
{
	if (!PointsGeometry.valid())
		return;

	if (!PointsAltitudeColor.valid())
		PointsAltitudeColor = new osg::Vec4Array;

	if (altitudeColorMode != mode)
	{
		PointsAltitudeColor->clear();
		// for (double factor : m_altitudeFactors)
		// {
		// 	QColor rgb = callback(factor, mode);
		// 	PointsAltitudeColor->push_back(osg::Vec4(rgb.redF(), rgb.greenF(), rgb.blueF(), 1.0));
		// }
		for( osg::Vec3d vec : PointsVertices->asVector() )
		{
			double factor = (vec.z()-minAltitude)/(maxAltitude - minAltitude);
			QColor rgb = ColorUtils::getIntentColor(factor, mode);
			PointsAltitudeColor->push_back(osg::Vec4(rgb.redF(), rgb.greenF(), rgb.blueF(), 1.0));
		}

		altitudeColorMode = mode;
	}

	PointsGeometry->setColorArray(PointsAltitudeColor.get());
	PointsGeometry->setColorBinding(osg::Geometry::BIND_PER_VERTEX);

}

void PointCloudLayer::setBlendColor(int modeIntent, int modeAltitude)
{
	if (!PointsGeometry.valid())
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
			// double factor2 = m_altitudeFactors[i];
			osg::Vec3d vec = PointsVertices->asVector().at(i);
			double factor2 = (vec.z()-minAltitude)/(maxAltitude - minAltitude);
			QColor rgb = ColorUtils::getBlendColor(factor1, factor2, modeIntent, modeAltitude);
			PointsBlendColor->push_back(osg::Vec4(rgb.redF(), rgb.greenF(), rgb.blueF(), 1.0));
		}
		blAltitudeColorMode = modeAltitude;
		blIntentColorMode = modeIntent;
	}

	PointsGeometry->setColorArray(PointsBlendColor.get());
	PointsGeometry->setColorBinding(osg::Geometry::BIND_PER_VERTEX);

}

void PointCloudLayer::setRGBColor()
{
	if (!PointsRGB.valid())
	{
		setOverallColor(QColor(100, 100, 100));
		return;
	}
	PointsGeometry->setColorArray(PointsRGB.get());
	PointsGeometry->setColorBinding(osg::Geometry::BIND_PER_VERTEX);
}

