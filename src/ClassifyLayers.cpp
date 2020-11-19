#include "ClassifyLayers.h"

ClassifyLayers::ClassifyLayers(int classify)
:m_classify(classify)
{

}

ClassifyLayers::~ClassifyLayers()
{

}

void ClassifyLayers::addLayer(QString filename, osg::ref_ptr<osg::Switch> layer)
{
    m_layers[filename] = layer;
}

bool ClassifyLayers::removeLayer(QString filename)
{
    if( m_layers.find(filename) != m_layers.end() )
    {
        m_layers.erase(filename);
    }
    return m_layers.empty();
}

void ClassifyLayers::showLayers(bool show)
{
    for( auto it : m_layers )
    {
        osg::ref_ptr<osg::Switch> sw = it.second;
        sw->setValue(0, show);
    }
}