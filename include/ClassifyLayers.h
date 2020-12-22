#ifndef __LASVIWER_ClassifyLayers_H_SPARCLE_2020_11_19
#define __LASVIWER_ClassifyLayers_H_SPARCLE_2020_11_19

#include <osg/Switch>
#include <QString>

class ClassifyLayers : public osg::Referenced
{
public:
    ClassifyLayers(int classify);
    ~ClassifyLayers();

    void addLayer(QString filename, osg::ref_ptr<osg::Switch> layer);
    bool removeLayer(QString filename);
    void showLayers(bool show);

    inline int classify() const { return m_classify;};

private:
    int m_classify;
    std::map<QString, osg::ref_ptr<osg::Switch>> m_layers;
};


#endif //__LASVIWER_ClassifyLayers_H_SPARCLE_2020_11_19