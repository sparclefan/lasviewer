#include "PointCloudLayer.h"

PointCloudLayer::PointCloudLayer(int classify)
	:classifier(classify), pointNumber(0), intentColorMode(-1), altitudeColorMode(-1),
	blIntentColorMode(-1), blAltitudeColorMode(-1)
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

void PointCloudLayer::setIntentColor(IntentColorCallBack callback, int mode)
{
	if (!PointsGeometry.valid())
		return;

	if (!PointsIntentColor.valid())
		PointsIntentColor = new osg::Vec4Array;

	if (intentColorMode != mode)
	{
		PointsIntentColor->clear();
		for (double factor : m_intentFactors)
		{
			QColor rgb = callback(factor, mode);
			PointsIntentColor->push_back(osg::Vec4(rgb.redF(), rgb.greenF(), rgb.blueF(), 1.0));
		}
		intentColorMode = mode;
	}

	PointsGeometry->setColorArray(PointsIntentColor.get());
	PointsGeometry->setColorBinding(osg::Geometry::BIND_PER_VERTEX);
}

void PointCloudLayer::setAltitudeColor(IntentColorCallBack callback, int mode)
{
	if (!PointsGeometry.valid())
		return;

	if (!PointsAltitudeColor.valid())
		PointsAltitudeColor = new osg::Vec4Array;

	if (altitudeColorMode != mode)
	{
		PointsAltitudeColor->clear();
		for (double factor : m_altitudeFactors)
		{
			QColor rgb = callback(factor, mode);
			PointsAltitudeColor->push_back(osg::Vec4(rgb.redF(), rgb.greenF(), rgb.blueF(), 1.0));
		}
		altitudeColorMode = mode;
	}

	PointsGeometry->setColorArray(PointsAltitudeColor.get());
	PointsGeometry->setColorBinding(osg::Geometry::BIND_PER_VERTEX);

}

void PointCloudLayer::setBlendColor(BlendColorCallBack callback, int modeIntent, int modeAltitude)
{
	if (!PointsGeometry.valid())
		return;

	if (!PointsBlendColor.valid())
		PointsBlendColor = new osg::Vec4Array;

	if ((blAltitudeColorMode != modeAltitude) || (blIntentColorMode != modeIntent))
	{
		PointsBlendColor->clear();
		int n = m_intentFactors.size();
		for (int i = 0; i < n; i++)
		{	
			double factor1 = m_intentFactors[i];
			double factor2 = m_altitudeFactors[i];
			QColor rgb = callback(factor1, factor2, modeIntent, modeAltitude);
			PointsBlendColor->push_back(osg::Vec4(rgb.redF(), rgb.greenF(), rgb.blueF(), 1.0));
		}
		blAltitudeColorMode = modeAltitude;
		blIntentColorMode = modeIntent;
	}

	PointsGeometry->setColorArray(PointsBlendColor.get());
	PointsGeometry->setColorBinding(osg::Geometry::BIND_PER_VERTEX);

}