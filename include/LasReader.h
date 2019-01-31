#ifndef __LasReader_H_SPARCLE_2019_01_23
#define __LasReader_H_SPARCLE_2019_01_23
	
#include <QString>
#include <QFile>
#include <QThread>
#include "lasFileStruct.h"
#include "PointCloudLayer.h"

class LasReader : public QThread{
	Q_OBJECT
public:
	LasReader(QString &filepath);
	~LasReader();

	YupontLasFile::LasHeader *getHeader();
	std::map<int, osg::ref_ptr<PointCloudLayer>> readPointsLayers(){ return m_layers; };
	int getMaxIntent(){ return m_maxInten; };
	int getMinIntent(){ return m_minInten; };
	int getThinFactor(){ return m_thinfactor; };
	void setThinFactor(int thinfactor){ m_thinfactor = thinfactor; };

	void splitFile(unsigned long ptNums, QString prefix);

signals:
	void progress(int percent);
	void processFinished();

protected:
	virtual void run();

private:
	QFile *m_pFile;
	uchar *m_pMap;

	int m_thinfactor;
	double m_maxInten;
	double m_minInten;
	YupontLasFile::LasHeader *m_pHeader;

	std::map<int, osg::ref_ptr<PointCloudLayer>> m_layers;

};
	
#endif //__LasReader_H_SPARCLE_2019_01_23