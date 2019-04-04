#ifndef __LasReader_H_SPARCLE_2019_01_23
#define __LasReader_H_SPARCLE_2019_01_23
	
#include <QString>
#include <QFile>
#include <QThread>
#include <QPointF>
#include <QRgb>
#include <QLineSeries>
#include "lasFileStruct.h"
#include "PointCloudLayer.h"

typedef struct _ExpParam{
	int intentMax;
	int intentMin;
	double altitudeMax;
	double altitudeMin;
	uint32_t splitPointsNum;
	bool bFiltColor;
	QRgb filtColor;
	_ExpParam() 
		:intentMax(-1), intentMin(-1), altitudeMax(NAN), altitudeMin(NAN), 
		splitPointsNum(0), bFiltColor(false)
	{}
} ExpParam;

class LasReader : public QThread{
	Q_OBJECT
public:
	LasReader(QString &filepath);
	~LasReader();

	AsprsLasFile::LasHeader *getHeader();
	std::map<int, osg::ref_ptr<PointCloudLayer>> readPointsLayers(){ return m_layers; };
	int getMaxIntent(){ return m_maxInten; };
	int getMinIntent(){ return m_minInten; };
	int getThinFactor(){ return m_thinfactor; };
	void setThinFactor(int thinfactor){ m_thinfactor = thinfactor; };

	void exportLas(QString fname, ExpParam expParam);

	void statistic();
	void statisticImpl();
	QPointF *getIntentStatisticData(){ return m_InstentStatistic; }
	QPointF *getAltitudeStatisticData() { return m_AltitudeStatistic; }
	//QLineSeries *getIntentCurve();
	//QLineSeries *getAltitudeCurve();

signals:
	void progress(int percent);
	void processFinished();
	void statisticFinished();

protected:
	virtual void run();

private:
	QFile *m_pFile;
	uchar *m_pMap;

	bool m_bigEndian;
	int m_thinfactor;
	double m_maxInten;
	double m_minInten;
	AsprsLasFile::LasHeader *m_pHeader;
	AsprsLasFile::LasHeader m_header;

	std::map<int, osg::ref_ptr<PointCloudLayer>> m_layers;

	QPointF m_InstentStatistic[100];
	QPointF m_AltitudeStatistic[100];

	double reversBytesDouble(double d);
	void adjustHeaderByteOrder();

};

	
#endif //__LasReader_H_SPARCLE_2019_01_23