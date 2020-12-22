#ifndef __LasReader_H_SPARCLE_2019_01_23
#define __LasReader_H_SPARCLE_2019_01_23
	
#include <QString>
#include <QFile>
#include <QThread>
#include <QPointF>
#include <QRgb>
#include <QLineSeries>
#include <QMutex>
#include "lasFileStruct.h"
#include "PointCloudLayer.h"
#include "LodGroup.h"

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

typedef struct _AdjustCoordinate {
	bool bInit;
	double adjX;
	double adjY;
	double adjZ;
	_AdjustCoordinate()
		:bInit(false), adjX(0), adjY(0), adjZ(0)
	{}
} AdjustCoordinate;

class LasReader : public QThread{
	Q_OBJECT
public:
	LasReader(QString &filepath, osg::Group *parentNode, osg::StateSet *stateSet);
	~LasReader();

	AsprsLasFile::LasHeader *getHeader();
	// std::map<int, osg::ref_ptr<PointCloudLayer>> readPointsLayers(){ return m_layers; };
	// std::map<int, osg::ref_ptr<LodGroup>> *readPointsLodLayers(){ return &m_lodLayers; };
	int getMaxIntent(){ return m_maxInten; };
	int getMinIntent(){ return m_minInten; };
	// int getThinFactor(){ return m_thinfactor; };
	// void setThinFactor(int thinfactor){ m_thinfactor = thinfactor; };

	void setIntentRange(int max, int min, int mode);
	void setAltitudeRange(double maxAl, double minAl, int mode);

	void setOverallColor(QColor c);
    void setClassifyColor();
    void setRGBColor();
    void setIntentColor(int mode);
    void setAltitudeColor(int mode);
    void setBlendColor(int modeIntent, int modeAltitude);

	void exportLas(QString fname, ExpParam expParam);

	void statistic();
	void statisticImpl();
	QPointF *getIntentStatisticData(){ return m_InstentStatistic; };
	QPointF *getAltitudeStatisticData() { return m_AltitudeStatistic; };
	QString filePath() { return m_filepath;};

	void setStateSet(osg::StateSet *set){m_stateSet = set;};

	static void resetAdjCoord() { adjCoord.bInit = false;};

	void readLodRemainLayer(int level);
	//QLineSeries *getIntentCurve();
	//QLineSeries *getAltitudeCurve();
signals:
	void progress(int);
	void processFinished(int, int, double, double);
	void newClassifyLayerAdded(QString, osg::Switch *, int);
	void statisticFinished();

protected:
	virtual void run();

private:
	QFile *m_pFile;
	uchar *m_pMap;
	QString m_filepath;
	static QMutex m_mutex;

	bool m_bigEndian;
	// int m_thinfactor;
	int m_maxInten;
	int m_minInten;
	double m_minAltitude;
	double m_maxAltitude;

	AsprsLasFile::LasHeader *m_pHeader;
	AsprsLasFile::LasHeader m_header;

	// std::map<int, osg::ref_ptr<PointCloudLayer>> m_layers;
	std::map<int, osg::ref_ptr<LodGroup>> m_lodLayers;

	osg::ref_ptr<osg::StateSet> m_stateSet;
	osg::ref_ptr<osg::Group> m_parentNode;

	QPointF m_InstentStatistic[100];
	QPointF m_AltitudeStatistic[100];

	double reversBytesDouble(double d);
	void adjustHeaderByteOrder();

	void readPointsData(int filterLevel);

	static AdjustCoordinate adjCoord;

};


// class LasReaderPool : public QThreadPool
// {


// };
	
#endif //__LasReader_H_SPARCLE_2019_01_23