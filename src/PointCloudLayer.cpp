#include "PointCloudLayer.h"

PointCloudLayer::PointCloudLayer(int classify)
	:classifier(classify), pointNumber(0), intentColorMode(-1)
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
	}

	PointsGeometry->setColorArray(PointsIntentColor.get());
	PointsGeometry->setColorBinding(osg::Geometry::BIND_PER_VERTEX);
}
