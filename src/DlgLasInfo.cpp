#include <QDate>
#include "DlgLasInfo.h"
#include "ui_lasinfo.h"
#include "lasFileStruct.h"

DlgLasInfo::DlgLasInfo(QWidget *parent)
	:QDialog(parent), ui(new Ui::LasInfo)
{
	ui->setupUi(this);
}

DlgLasInfo::~DlgLasInfo()
{

}

void DlgLasInfo::hideEvent(QHideEvent *event)
{
	emit hideDlg();
	QDialog::hideEvent(event);
}

void DlgLasInfo::setLasInfo(YupontLasFile::LasHeader *pHeader)
{
	if (pHeader->m_recordsCount > 0){
		YupontLasFile::VariLenRecord *pVarLenRecord = (YupontLasFile::VariLenRecord *)(((uchar *)pHeader) + pHeader->m_headerSize);

		if (pVarLenRecord->recordId == 34735)  // GeoTiff
		{
			YupontLasFile::sGeoKeys *pVarData = (YupontLasFile::sGeoKeys *)(((uchar *)pVarLenRecord) + sizeof(YupontLasFile::VariLenRecord));
		}
	}

	QString version = QString("%1.%2").arg(pHeader->m_versionMajor).arg(pHeader->m_versionMinor);
	ui->lb_version->setText(version);
	char tmpbuf[33];
	memset(tmpbuf, 0, sizeof tmpbuf);
	memcpy(tmpbuf, pHeader->m_systemId, 32);
	ui->lb_systemId->setText(tmpbuf);
	memcpy(tmpbuf, pHeader->m_softwareId, 32);
	ui->lb_softwareId->setText(tmpbuf);
	QDate dt(pHeader->m_createYear, 1, 1);
	qint64 julianDay = dt.toJulianDay() + pHeader->m_createDOY;
	QDate dtCreate = QDate::fromJulianDay(julianDay);
	ui->lb_createDate->setText(dtCreate.toString("yyyy-MM-dd"));
	ui->lb_formatId->setText(QString("%1").arg((int)pHeader->m_pointDataFormatId));
	ui->lb_pointCount->setText(QString("%1").arg(pHeader->m_pointRecordsCount));
	ui->lb_return1->setText(QString("%1").arg(pHeader->m_lasHeaderNumOfReturns[0]));
	ui->lb_return2->setText(QString("%1").arg(pHeader->m_lasHeaderNumOfReturns[1]));
	ui->lb_return3->setText(QString("%1").arg(pHeader->m_lasHeaderNumOfReturns[2]));
	ui->lb_maxX->setText(QString("%1").arg(pHeader->m_maxX, 10, 'f', 3));
	ui->lb_minX->setText(QString("%1").arg(pHeader->m_minX, 10, 'f', 3));
	ui->lb_maxY->setText(QString("%1").arg(pHeader->m_maxY, 10, 'f', 3));
	ui->lb_minY->setText(QString("%1").arg(pHeader->m_minY, 10, 'f', 3));
	ui->lb_maxZ->setText(QString("%1").arg(pHeader->m_maxZ, 10, 'f', 3));
	ui->lb_minZ->setText(QString("%1").arg(pHeader->m_minZ, 10, 'f', 3));
}