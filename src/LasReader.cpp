#include "LasReader.h"
#include <QTime>
	
#define reversbytes32(d)	( (((d)<<24)&0xff000000)|(((d)<<8)&0xff0000)|(((d)>>8)&0xff00)|(((d)>>24)&0xff) )
#define reversbytes16(d) ( (((d)<<8)&0xff00)|(((d)>>8)&0xff) )

using namespace YupontLasFile;

LasReader::LasReader(QString &filepath)
	: m_thinfactor(0), m_maxInten(0), m_minInten(0)
{
	// test byte order
	int tmp = 1;
	uchar *pTmp = (uchar *)&tmp;
	m_bigEndian = ((*pTmp)==0);

	m_pFile = new QFile(filepath);

	m_pFile->open(QIODevice::ReadWrite);
	m_pMap = m_pFile->map(0, m_pFile->size());

	m_pHeader = (YupontLasFile::LasHeader *)m_pMap;		
	memcpy(&m_header, m_pHeader, sizeof m_header);
	adjustHeaderByteOrder();

}

LasReader::~LasReader()
{
	if (m_pFile == NULL)
		return;

	if (m_pMap != NULL)
		m_pFile->unmap(m_pMap);

	m_pFile->close();
	delete m_pFile;
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


YupontLasFile::LasHeader *LasReader::getHeader()
{
	return &m_header;
}

void LasReader::run()
{
	QTime t;
	t.start();
	std::map<int, osg::ref_ptr<PointCloudLayer>>().swap(m_layers);

	uchar *pPoints = (((uchar *)m_pHeader) + m_header.m_dataOffset);
	YupontLasFile::PointFormat0 *pPoint = (YupontLasFile::PointFormat0 *)pPoints;

	unsigned short int intensity = pPoint->m_intensity;
	if (m_bigEndian){
		intensity = reversbytes16(intensity);
	}

	m_maxInten = intensity;
	m_minInten = intensity;

	size_t offset = 0;
	size_t ptsize = m_header.m_pointDataRecordLength;
	for (unsigned long i = 0; i < m_header.m_pointRecordsCount; i += m_thinfactor, offset += (ptsize*m_thinfactor))
	{
		pPoint = (YupontLasFile::PointFormat0 *)(pPoints + offset);
		intensity = pPoint->m_intensity;
		if(m_bigEndian){
			intensity = reversbytes16(intensity);
		}

		if (intensity > m_maxInten)
			m_maxInten = intensity;
		if (intensity < m_minInten)
			m_minInten = intensity;			
	}

	//qDebug("Time elapsed: %d ms", t.elapsed());

	offset = 0;
	int percent = 0;
	YupontLasFile::PointFormat0 tmpPoint;
	for (unsigned long i = 0; i < m_header.m_pointRecordsCount; i += m_thinfactor, offset += (ptsize * m_thinfactor))
	{
		pPoint = (YupontLasFile::PointFormat0 *)(pPoints + offset);
		if(m_bigEndian)
		{
			memcpy(&tmpPoint, pPoint, sizeof tmpPoint);
			tmpPoint.m_position.X = reversbytes32(tmpPoint.m_position.X);
			tmpPoint.m_position.Y = reversbytes32(tmpPoint.m_position.Y);
			tmpPoint.m_position.Z = reversbytes32(tmpPoint.m_position.Z);
			pPoint = &tmpPoint;
		}

		double x = (m_header.m_xScaleFactor * pPoint->m_position.X + m_header.m_xOffset) - m_header.m_minX;
		double y = (m_header.m_yScaleFactor * pPoint->m_position.Y + m_header.m_yOffset) - m_header.m_minY;
		double z = (m_header.m_zScaleFactor * pPoint->m_position.Z + m_header.m_zOffset) - m_header.m_minZ;

		int classify = (int)pPoint->m_classification;
		osg::ref_ptr<PointCloudLayer> pcl;
		if (m_layers.find(classify) != m_layers.end())
		{
			pcl = m_layers[classify];
		}
		else{
			pcl = new PointCloudLayer(classify);
			m_layers[classify] = pcl;
		}

		pcl->PointsVertices->push_back(osg::Vec3d(x, y, z));

		double factor = (pPoint->m_intensity - m_minInten) / (m_maxInten - m_minInten);
		pcl->m_intentFactors.push_back(factor);

		factor = z / (m_header.m_maxZ - m_header.m_minZ);
		pcl->m_altitudeFactors.push_back(factor);

		pcl->pointNumber++;

		int newpercent = i * 100 / m_pHeader->m_pointRecordsCount;
		if (newpercent > percent){
			percent = newpercent;
			emit progress(percent);
			//qDebug("Time elapsed: %d ms", t.elapsed());
		}
	}

	emit processFinished();
}

void LasReader::splitFile(unsigned long ptNums, QString prefix)
{
	unsigned long totalnum = 0;
	int fileid = 0;
	while (totalnum < m_pHeader->m_pointRecordsCount)
	{
		unsigned long num = (totalnum + ptNums <= m_pHeader->m_pointRecordsCount) ? ptNums : m_pHeader->m_pointRecordsCount - totalnum;
		
		size_t filelen = m_pHeader->m_dataOffset + num * m_pHeader->m_pointDataRecordLength;
		
		QString fname = QString("%1_%2.las").arg(prefix).arg(fileid,3,10,QLatin1Char('0'));
		QFile file(fname);
		file.open(QIODevice::ReadWrite);
		file.resize(filelen);

		uchar *pMem = file.map(0, filelen);
		LasHeader *pHeader = (LasHeader *)pMem;
		uchar *pSrc = (uchar *)m_pHeader;

		memcpy(pMem, pSrc, m_pHeader->m_dataOffset);
		
		uchar *pData = pMem + m_pHeader->m_dataOffset;
		pSrc += (m_pHeader->m_dataOffset + totalnum * m_pHeader->m_pointDataRecordLength);

		memcpy(pData, pSrc, num*m_pHeader->m_pointDataRecordLength);

		PointFormat0 *pPoint = (PointFormat0 *)pData;
		long lMaxX = pPoint->m_position.X;
		long lMinX = pPoint->m_position.X;
		long lMaxY = pPoint->m_position.Y;
		long lMinY = pPoint->m_position.Y;
		long lMaxZ = pPoint->m_position.Z;
		long lMinZ = pPoint->m_position.Z;
		int returnNumber[5];
		memset(returnNumber, 0, sizeof returnNumber);
		for (unsigned long i = 0; i < num; i++){
			pPoint = (PointFormat0 *)(pData + i*m_pHeader->m_pointDataRecordLength);
			if (pPoint->m_position.X > lMaxX)
				lMaxX = pPoint->m_position.X;
			if (pPoint->m_position.X < lMinX)
				lMinX = pPoint->m_position.X;
			if (pPoint->m_position.Y > lMaxY)
				lMaxY = pPoint->m_position.Y;
			if (pPoint->m_position.Y < lMinY)
				lMinY = pPoint->m_position.Y;
			if (pPoint->m_position.Z > lMaxZ)
				lMaxZ = pPoint->m_position.Z;
			if (pPoint->m_position.Z < lMinZ)
				lMinZ = pPoint->m_position.Z;
			if (pPoint->m_returnNumber > 0 && pPoint->m_returnNumber < 6){
				returnNumber[pPoint->m_returnNumber - 1]++;
			}
		}
		pHeader->m_maxX = pHeader->m_xScaleFactor*lMaxX + pHeader->m_xOffset;
		pHeader->m_minX = pHeader->m_xScaleFactor*lMinX + pHeader->m_xOffset;
		pHeader->m_maxY = pHeader->m_yScaleFactor*lMaxY + pHeader->m_yOffset;
		pHeader->m_minY = pHeader->m_yScaleFactor*lMinY + pHeader->m_yOffset;
		pHeader->m_maxZ = pHeader->m_zScaleFactor*lMaxZ + pHeader->m_zOffset;
		pHeader->m_minZ = pHeader->m_zScaleFactor*lMinZ + pHeader->m_zOffset;
		pHeader->m_pointRecordsCount = num;
		for (int i = 0; i < 5; i++){
			pHeader->m_lasHeaderNumOfReturns[i] = returnNumber[i];
		}

		file.unmap(pMem);
		file.close();

		totalnum += num;
		fileid++;
	}
}

