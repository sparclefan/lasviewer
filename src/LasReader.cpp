#pragma execution_character_set("utf-8")

#include "LasReader.h"
#include <QElapsedTimer>
#include <QFileInfo>
#include <QDebug>
#include <QApplication>
#include <QRunnable>
#include <QThreadPool>
#include <osg/Switch>
#include "ColorUtils.h"
	
#define reversbytes32(d)	( (((d)<<24)&0xff000000)|(((d)<<8)&0xff0000)|(((d)>>8)&0xff00)|(((d)>>24)&0xff) )
#define reversbytes16(d) ( (((d)<<8)&0xff00)|(((d)>>8)&0xff) )

using namespace AsprsLasFile;
using namespace std;

QMutex LasReader::m_mutex;
AdjustCoordinate LasReader::adjCoord;

class readLodThread : public QRunnable
{
public:
	readLodThread(LasReader *pReader, int level):m_pReader(pReader),m_startLevel(level){};

protected:
	virtual void run()
	{
		m_pReader->readLodRemainLayer(m_startLevel);
	}	

private:
	LasReader *m_pReader;
	int m_startLevel;
};

LasReader::LasReader(QString &filepath, osg::Group *parentNode, osg::StateSet *stateSet)
	: m_filepath(filepath), m_parentNode(parentNode), m_stateSet(stateSet)
{
	// moveToThread(QApplication::instance()->thread());
	// test byte order
	int tmp = 1;
	uchar *pTmp = (uchar *)&tmp;
	m_bigEndian = ((*pTmp)==0);

	m_pFile = new QFile(filepath);

	m_pFile->open(QIODevice::ReadWrite);
	m_pMap = m_pFile->map(0, m_pFile->size());

	m_pHeader = (LasHeader *)m_pMap;		
	memcpy(&m_header, m_pHeader, sizeof m_header);
	adjustHeaderByteOrder();

	m_minAltitude = m_pHeader->m_minZ;
	m_maxAltitude = m_pHeader->m_maxZ;

}

LasReader::~LasReader()
{
	if (m_pFile == NULL)
		return;

	if (m_pMap != NULL)
		m_pFile->unmap(m_pMap);

	m_pFile->close();
	delete m_pFile;

	// std::map<int, osg::ref_ptr<LodGroup>> empty;
	// m_lodLayers.swap(empty);

}

double LasReader::reversBytesDouble(double d)
{
	double rt;
	uchar *pSrc = (uchar *)&d;
	uchar *pDst = (uchar *)&rt;

	int size = sizeof(double);
	for(int i=0; i<size; i++)
	{
		pDst[i] = pSrc[size-i-1];
	}

	return rt;
}

void LasReader::adjustHeaderByteOrder()
{
	if(m_bigEndian){
		m_header.m_sourceId = reversbytes16(m_header.m_sourceId);		
		m_header.m_encoding = reversbytes16(m_header.m_encoding);

		m_header.m_createDOY = reversbytes16(m_header.m_createDOY);		
		m_header.m_createYear = reversbytes16(m_header.m_createYear);
		m_header.m_headerSize = reversbytes16(m_header.m_headerSize);
		m_header.m_dataOffset = reversbytes32(m_header.m_dataOffset);	
		m_header.m_recordsCount = reversbytes32(m_header.m_recordsCount);
		m_header.m_pointDataRecordLength = reversbytes16(m_header.m_pointDataRecordLength);

		m_header.m_pointRecordsCount = reversbytes32(m_header.m_pointRecordsCount);
		for(int i=0; i<5; i++){
			m_header.m_lasHeaderNumOfReturns[i] = reversbytes32(m_header.m_lasHeaderNumOfReturns[i]);
		}						
		
		m_header.m_xScaleFactor = reversBytesDouble(m_header.m_xScaleFactor);
		m_header.m_yScaleFactor = reversBytesDouble(m_header.m_yScaleFactor);
		m_header.m_zScaleFactor = reversBytesDouble(m_header.m_zScaleFactor);
		m_header.m_xOffset = reversBytesDouble(m_header.m_xOffset);
		m_header.m_yOffset = reversBytesDouble(m_header.m_yOffset);
		m_header.m_zOffset = reversBytesDouble(m_header.m_zOffset);

		m_header.m_maxX = reversBytesDouble(m_header.m_maxX);
		m_header.m_minX = reversBytesDouble(m_header.m_minX);
		m_header.m_maxY = reversBytesDouble(m_header.m_maxY);
		m_header.m_minY = reversBytesDouble(m_header.m_minY);
		m_header.m_maxZ = reversBytesDouble(m_header.m_maxZ);
		m_header.m_minZ = reversBytesDouble(m_header.m_minZ);
		
	}
}


LasHeader *LasReader::getHeader()
{
	return &m_header;
}

void LasReader::run()
{
	// QMutexLocker lock(&m_mutex);
	QElapsedTimer t;
	t.start();
	std::map<int, osg::ref_ptr<LodGroup>>().swap(m_lodLayers);

	uchar *pPoints = (((uchar *)m_pHeader) + m_header.m_dataOffset);
	PointFormat0 *pPoint = (PointFormat0 *)pPoints;

	unsigned short int intensity = pPoint->m_intensity;
	if (m_bigEndian){
		intensity = reversbytes16(intensity);
	}

	m_maxInten = intensity;
	m_minInten = intensity;

	size_t offset = 0;
	size_t ptsize = m_header.m_pointDataRecordLength;

	if( !adjCoord.bInit){
		adjCoord.adjX = m_header.m_minX;
		adjCoord.adjY = m_header.m_minY;
		adjCoord.adjZ = m_header.m_minZ;
		adjCoord.bInit = true;
	}

	int optLevel = LodLayer::preferLevel(m_header.m_pointRecordsCount);

	readPointsData(optLevel);

	if( (optLevel+1) < MAX_FACTOR_LEVEL)
	{
		readLodThread *t = new readLodThread(this, optLevel+1);
		QThreadPool::globalInstance()->start(t);
		// readPointsData(optLevel + 1);
	}
	//emit processFinished(this);
}

void LasReader::readPointsData(int filterLevel)
{
	uchar *pPoints = (((uchar *)m_pHeader) + m_header.m_dataOffset);
	PointFormat0 *pPoint = (PointFormat0 *)pPoints;	
	size_t ptsize = m_header.m_pointDataRecordLength;
	size_t offset = 0;
	int percent = 0;
	PointFormat0 tmpPoint;
	int filterFactor = FILTER_FACTOR[filterLevel];
	for (unsigned long i = 0; i < m_header.m_pointRecordsCount; i += filterFactor, offset += (ptsize * filterFactor))
	{
		bool skip = false;
		for(int level=0; level<filterLevel; level++)
		{
			if( (i%FILTER_FACTOR[level]) == 0 ){
				skip = true;
				break;
			}
		}
		if(skip) continue;

		pPoint = (PointFormat0 *)(pPoints + offset);
		if(m_bigEndian)
		{
			memcpy(&tmpPoint, pPoint, sizeof tmpPoint);
			tmpPoint.m_position.X = reversbytes32(tmpPoint.m_position.X);
			tmpPoint.m_position.Y = reversbytes32(tmpPoint.m_position.Y);
			tmpPoint.m_position.Z = reversbytes32(tmpPoint.m_position.Z);
			pPoint = &tmpPoint;
		}

		double x = (m_header.m_xScaleFactor * pPoint->m_position.X + m_header.m_xOffset);
		if( x>m_header.m_maxX || x<m_header.m_minX){
			printf("*** X out of range, %f [%f - %f]\n", x, m_header.m_minX, m_header.m_maxX);
			continue;
		}
		x -= adjCoord.adjX;
		double y = (m_header.m_yScaleFactor * pPoint->m_position.Y + m_header.m_yOffset);
		if( y>m_header.m_maxY || y<m_header.m_minY){
			printf("*** Y out of range, %f [%f - %f]\n", y, m_header.m_minY, m_header.m_maxY);
			continue;
		}
		y -= adjCoord.adjY;
		double z = (m_header.m_zScaleFactor * pPoint->m_position.Z + m_header.m_zOffset);
		if( z>m_header.m_maxZ || z<m_header.m_minZ){
			printf("*** Z out of range, %f [%f - %f]\n", z, m_header.m_minZ, m_header.m_maxZ);
			continue;
		}
		// z -= adjCoord.adjZ;     //高度不需要调整

		int classify = (int)pPoint->m_classification;

		osg::ref_ptr<LodGroup> pcl;
		if (m_lodLayers.find(classify) != m_lodLayers.end())
		{
			pcl = m_lodLayers[classify];
		}
		else {
			osg::ref_ptr<osg::Switch> classifySwitch = new osg::Switch();
			m_parentNode->addChild(classifySwitch);

			pcl = new LodGroup(classify, filterLevel, classifySwitch);
			m_lodLayers[classify] = pcl;

			double cx = (m_header.m_maxX-m_header.m_minX)/2 + m_header.m_minX - adjCoord.adjX;
			double cy = (m_header.m_maxY-m_header.m_minY)/2 + m_header.m_minY - adjCoord.adjY;
			double cz = (m_header.m_maxZ-m_header.m_minZ)/2 + m_header.m_minZ - adjCoord.adjZ;

			printf("======= Center point: %f, %f, %f\n", cx, cy, cz);

			pcl->setCenter(osg::Vec3(cx, cy, cz));
			pcl->maxAltitude = m_header.m_maxZ;
			pcl->minAltitude = m_header.m_minZ;

			emit newClassifyLayerAdded(m_filepath, classifySwitch, classify);
		}

		LodLayer *lodlayer = pcl->getLayer(filterLevel, m_stateSet.get());

		int intent = pPoint->m_intensity;
		if( intent < pcl->minIntent ){
			pcl->minIntent = intent;
		}
		if( intent > pcl->maxIntent){
			pcl->maxIntent = intent;
		}
		if (intent < m_minInten) {
			m_minInten = intent;
		}
		if (intent > m_maxInten) {
			m_maxInten = intent;
		}

		lodlayer->addPoint(x, y, z,intent);

		if (m_header.m_pointDataFormatId == 3)
		{
			PointFormat3 *ptFormat3 = (PointFormat3 *)pPoint;
			
			QColor rgb;
			if (m_bigEndian){
				rgb = QColor(reversbytes16(ptFormat3->m_rgb.red), reversbytes16(ptFormat3->m_rgb.green), reversbytes16(ptFormat3->m_rgb.blue));
			}
			else{
				rgb = QColor(ptFormat3->m_rgb.red, ptFormat3->m_rgb.green, ptFormat3->m_rgb.blue);
			}
			lodlayer->addPointColor(rgb);
		}

		int newpercent = i * 100 / m_pHeader->m_pointRecordsCount;
		if (newpercent > percent){
			percent = newpercent;
			emit progress(percent);
		}
	}

	emit progress(100);

	for(auto it : m_lodLayers)
	{
		osg::ref_ptr<LodGroup> lodgroup = it.second;
		
		if( lodgroup->m_layerList.find(filterLevel) != lodgroup->m_layerList.end())
		{
			lodgroup->finishAddPoints(filterLevel);
		}
	}
	emit processFinished(m_minInten, m_maxInten, m_minAltitude, m_maxAltitude);
}

void LasReader::readLodRemainLayer(int level)
{
	for(int i=level; i<MAX_FACTOR_LEVEL; i++)
	{
		readPointsData(i);
	}
}

void LasReader::setIntentRange(int max, int min, int mode)
{
	for(auto it : m_lodLayers)
	{
		LodGroup *layer = it.second;
		layer->setIntentRange(max, min, mode);
	}
}

void LasReader::setAltitudeRange(double maxAl, double minAl, int mode)
{
	for(auto it : m_lodLayers)
	{
		LodGroup *layer = it.second;
		layer->setAltitudeRange(maxAl, minAl, mode);
	}
}

void LasReader::setClassifyColor()
{
	for(auto it : m_lodLayers)
	{
		int classify = it.first;
		LodGroup *layer = it.second;
		layer->setOverallColor(classifyColor[classify].color);
	}
}

void LasReader::setOverallColor(QColor c)
{
	for(auto it : m_lodLayers)
	{
		LodGroup *layer = it.second;
		layer->setOverallColor(c);
	}
}

void LasReader::setRGBColor()
{
	for(auto it : m_lodLayers)
	{
		LodGroup *layer = it.second;
		layer->setRGBColor();
	}
}

void LasReader::setIntentColor(int mode)
{
	for(auto it : m_lodLayers)
	{
		LodGroup *layer = it.second;
		layer->setIntentColor(mode);
	}
}

void LasReader::setAltitudeColor(int mode)
{
	for(auto it : m_lodLayers)
	{
		LodGroup *layer = it.second;
		layer->setAltitudeColor(mode);
	}
}

void LasReader::setBlendColor(int modeIntent, int modeAltitude)
{
	for(auto it : m_lodLayers)
	{
		LodGroup *layer = it.second;
		layer->setBlendColor(modeIntent, modeAltitude);
	}
}

void LasReader::exportLas(QString fname, ExpParam expParam)
{
	bool bMinAlt = !isnan(expParam.altitudeMin);
	bool bMaxAlt = !isnan(expParam.altitudeMax);
	int32_t altMax, altMin;
	if (bMinAlt){
		altMin = (int)((expParam.altitudeMin - m_header.m_zOffset) / m_header.m_zScaleFactor);
	}
	if (bMaxAlt){
		altMax = (int)((expParam.altitudeMax - m_header.m_zOffset) / m_header.m_zScaleFactor);
	}

	int splitFileId = 0;
	uint32_t totalnum = 0;
	QFileInfo fi = QFileInfo(fname);

	QFile *pFile = new QFile(fname);
	pFile->open(QIODevice::ReadWrite);
	size_t filelen = m_header.m_dataOffset + m_header.m_pointRecordsCount * m_header.m_pointDataRecordLength;
	pFile->resize(filelen);

	uchar *pMem = pFile->map(0, filelen);
	LasHeader *pHeader = (LasHeader *)pMem;

	memcpy(pMem, m_pHeader, m_header.m_dataOffset);
	uchar *pSrc = (uchar *)m_pHeader + m_header.m_dataOffset;

	uchar *pData = pMem + m_header.m_dataOffset;
	int32_t maxAlt, minAlt, maxX, maxY, minX, minY;
	bool minmaxInit = false;
	int returnNumber[5];
	memset(returnNumber, 0, sizeof returnNumber);

	for (uint32_t i = 0; i < m_header.m_pointRecordsCount; i++, pSrc += m_header.m_pointDataRecordLength)
	{
		PointFormat0 *pPoint = (PointFormat0 *)pSrc;
		uint16_t intent = pPoint->m_intensity;
		int32_t altitude = pPoint->m_position.Z;
		int32_t px = pPoint->m_position.X;
		int32_t py = pPoint->m_position.Y;
		if (m_bigEndian){
			intent = reversbytes16(intent);
			altitude = reversbytes32(altitude);
			px = reversbytes32(px);
			py = reversbytes32(py);
		}
		if (((expParam.intentMin > 0) && (intent < expParam.intentMin))
			|| ((expParam.intentMax>0) && (intent > expParam.intentMax)))
			continue;

		if((bMinAlt && (altitude<altMin)) || (bMaxAlt && (altitude>altMax)))
			continue;

		if (expParam.bFiltColor && (m_header.m_pointDataFormatId==3))
		{
			PointFormat3 *pF3 = (PointFormat3 *)pPoint;
			uint16_t r = pF3->m_rgb.red;
			uint16_t g = pF3->m_rgb.green;
			uint16_t b = pF3->m_rgb.blue;
			if (m_bigEndian){
				r = reversbytes16(r);
				g = reversbytes16(g);
				b = reversbytes16(b);
			}
			QRgb rgb = QColor(r, g, b).rgb();
			if (rgb == expParam.filtColor)
				continue;
		}

		if (minmaxInit){
			if (altitude < minAlt) minAlt = altitude;
			if (altitude > maxAlt) maxAlt = altitude;
			if (px < minX) minX = px;
			if (px > maxX) maxX = px;
			if (py < minY) minY = py;
			if (py > maxY) maxY = py;
		}
		else{
			minAlt = maxAlt = altitude;
			minX = maxX = px;
			minY = maxY = py;
			minmaxInit = true;
		}

		if (pPoint->m_returnNumber > 0 && pPoint->m_returnNumber < 6){
			returnNumber[pPoint->m_returnNumber - 1]++;
		}

		memcpy(pData, pSrc, m_header.m_pointDataRecordLength);
		pData += m_header.m_pointDataRecordLength;
		totalnum++;

		if ((expParam.splitPointsNum > 0) && (totalnum == expParam.splitPointsNum)
			&& (i<(m_header.m_pointRecordsCount-1)))
		{
			filelen = m_header.m_dataOffset + totalnum * m_header.m_pointDataRecordLength;
			if (m_bigEndian){
				totalnum = reversbytes32(totalnum);
			}
			pHeader->m_pointRecordsCount = totalnum;
			pHeader->m_minX = m_header.m_xScaleFactor*minX + m_header.m_xOffset;
			pHeader->m_maxX = m_header.m_xScaleFactor*maxX + m_header.m_xOffset;
			pHeader->m_minY = m_header.m_yScaleFactor*minY + m_header.m_yOffset;
			pHeader->m_maxY = m_header.m_yScaleFactor*maxY + m_header.m_yOffset;
			pHeader->m_minZ = m_header.m_zScaleFactor*minAlt + m_header.m_zOffset;
			pHeader->m_maxZ = m_header.m_zScaleFactor*maxAlt + m_header.m_zOffset;

			for (int j = 0; j < 5; j++){
				pHeader->m_lasHeaderNumOfReturns[j] = returnNumber[j];
				returnNumber[j] = 0;
			}

			pFile->unmap(pMem);
			pFile->resize(filelen);
			pFile->close();
			delete pFile;

			QString nextfile = QString("%1/%2_%3.las").arg(fi.canonicalPath())
				.arg(fi.completeBaseName()).arg(splitFileId++, 3, 10, QLatin1Char('0'));
			//qDebug() << nextfile;
			pFile = new QFile(nextfile);
			pFile->open(QIODevice::ReadWrite);
			filelen = m_header.m_dataOffset + m_header.m_pointRecordsCount * m_header.m_pointDataRecordLength;
			pFile->resize(filelen);

			pMem = pFile->map(0, filelen);
			pHeader = (LasHeader *)pMem;
			memcpy(pMem, m_pHeader, m_header.m_dataOffset);
			pData = pMem + m_header.m_dataOffset;
			totalnum = 0;
			minmaxInit = false;
		}
	}

	filelen = m_header.m_dataOffset + totalnum * m_header.m_pointDataRecordLength;
	if (m_bigEndian){
		totalnum = reversbytes32(totalnum);
	}
	pHeader->m_pointRecordsCount = totalnum;
	pHeader->m_minX = m_header.m_xScaleFactor*minX + m_header.m_xOffset;
	pHeader->m_maxX = m_header.m_xScaleFactor*maxX + m_header.m_xOffset;
	pHeader->m_minY = m_header.m_yScaleFactor*minY + m_header.m_yOffset;
	pHeader->m_maxY = m_header.m_yScaleFactor*maxY + m_header.m_yOffset;
	pHeader->m_minZ = m_header.m_zScaleFactor*minAlt + m_header.m_zOffset;
	pHeader->m_maxZ = m_header.m_zScaleFactor*maxAlt + m_header.m_zOffset;

	for (int j = 0; j < 5; j++){
		pHeader->m_lasHeaderNumOfReturns[j] = returnNumber[j];
		returnNumber[j] = 0;
	}

	pFile->unmap(pMem);
	pFile->resize(filelen);
	pFile->close();
	delete pFile;

}

class statisticThread : public QThread
{
public:
	statisticThread(LasReader *pReader) :m_pReader(pReader){};

protected:
	virtual void run()
	{
		m_pReader->statisticImpl();
	}

private:
	LasReader *m_pReader;
};

void LasReader::statistic()
{
	statisticThread *t = new statisticThread(this);
	t->start();
}

void LasReader::statisticImpl()
{
	size_t offset = 0;
	size_t ptsize = m_header.m_pointDataRecordLength;
	uchar *pPoints = (((uchar *)m_pHeader) + m_header.m_dataOffset);
	PointFormat0 *pPoint = (PointFormat0 *)pPoints;
	uint16_t intensity = pPoint->m_intensity;
	if (m_bigEndian){
		intensity = reversbytes16(intensity);
	}
	int maxInten = intensity;
	int minInten = intensity;

	for (unsigned long i = 0; i < m_header.m_pointRecordsCount; i ++, offset += ptsize)
	{
		pPoint = (PointFormat0 *)(pPoints + offset);
		intensity = pPoint->m_intensity;
		if (m_bigEndian){
			intensity = reversbytes16(intensity);
		}		
		if(maxInten<intensity) maxInten = intensity;
		if(minInten>intensity) minInten = intensity;
	}

	double stepIntent = (maxInten - minInten) / 100.0;
	double stepAlt = (m_pHeader->m_maxZ - m_pHeader->m_minZ) / 100.0;
	for (int i = 0; i < 100; i++){
		m_InstentStatistic[i].setX(minInten + stepIntent*i);
		m_InstentStatistic[i].setY(0);
		m_AltitudeStatistic[i].setX(m_pHeader->m_minZ + stepAlt*i);
		m_AltitudeStatistic[i].setY(0);
	}

	pPoints = (((uchar *)m_pHeader) + m_header.m_dataOffset);
	offset = 0;
	for (unsigned long i = 0; i < m_header.m_pointRecordsCount; i ++, offset += ptsize)
	{
		pPoint = (PointFormat0 *)(pPoints + offset);
		int32_t altitude = pPoint->m_position.Z;
		uint16_t intensity = pPoint->m_intensity;
		if (m_bigEndian)
		{
			altitude = reversbytes32(altitude);
			intensity = reversbytes16(intensity);
		}
		double z = (m_header.m_zScaleFactor * altitude + m_header.m_zOffset);

		int idIntent = (intensity - minInten) * 100 / (maxInten - minInten);
		if (idIntent < 0) idIntent = 0;
		if (idIntent > 99) idIntent = 99;

		int idAlt = (z - m_pHeader->m_minZ) * 100 / (m_pHeader->m_maxZ - m_pHeader->m_minZ);
		if (idAlt < 0) idAlt = 0;
		if (idAlt > 99) idAlt = 99;

		m_InstentStatistic[idIntent].setY(m_InstentStatistic[idIntent].y() + 1);
		m_AltitudeStatistic[idAlt].setY(m_AltitudeStatistic[idAlt].y() + 1);
	}

	emit statisticFinished();
}