#pragma execution_character_set("utf-8")

#include <QFileDialog>
#include <QMessageBox>
#include <QDate>
#include <QtCharts/QLineSeries>
#include "DlgLasInfo.h"
#include "ui_lasinfo.h"
#include "LasReader.h"

#define Tr(s) QString::fromLocal8Bit(s)

QT_CHARTS_USE_NAMESPACE

DlgLasInfo::DlgLasInfo(QWidget *parent)
	:QDialog(parent), ui(new Ui::LasInfo), m_pLasReader(NULL)
{
	ui->setupUi(this);

	QPalette palette;
	palette.setColor(QPalette::Background, QColor(100,100,100));
	ui->lb_defaultColor->setAutoFillBackground(true);
	ui->lb_defaultColor->setPalette(palette);

	connect(ui->pb_export, SIGNAL(clicked(bool)), this, SLOT(on_export()));
	connect(ui->pb_statistic, SIGNAL(clicked(bool)), this, SLOT(on_statistic()));
}

DlgLasInfo::~DlgLasInfo()
{

}

void DlgLasInfo::hideEvent(QHideEvent *event)
{
	emit hideDlg();
	QDialog::hideEvent(event);
}

void DlgLasInfo::setLasReader(LasReader *lasReader)
{
	m_pLasReader = lasReader;
	setLasInfo(m_pLasReader->getHeader());
	connect(m_pLasReader, SIGNAL(statisticFinished()), this, SLOT(on_statisticFinished()));
}

void DlgLasInfo::setLasInfo(AsprsLasFile::LasHeader *pHeader)
{
	if (pHeader->m_recordsCount > 0){
		AsprsLasFile::VariLenRecord *pVarLenRecord = (AsprsLasFile::VariLenRecord *)(((uchar *)pHeader) + pHeader->m_headerSize);

		if (pVarLenRecord->recordId == 34735)  // GeoTiff
		{
			AsprsLasFile::sGeoKeys *pVarData = (AsprsLasFile::sGeoKeys *)(((uchar *)pVarLenRecord) + sizeof(AsprsLasFile::VariLenRecord));
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

	ui->le_minAltitude->setText(QString("%1").arg(pHeader->m_minZ, 10, 'f', 3));
	ui->le_maxAltitude->setText(QString("%1").arg(pHeader->m_maxZ, 10, 'f', 3));

	ui->gb_colorFilter->setEnabled((pHeader->m_pointDataFormatId == 3));

}

void DlgLasInfo::on_intentRangeChanged(int minIntent, int maxIntent)
{
	ui->le_minIntent->setText(QString("%1").arg(minIntent));
	ui->le_maxIntent->setText(QString("%1").arg(maxIntent));
}

void DlgLasInfo::on_export()
{
	if (m_pLasReader == NULL){
		QMessageBox::information(this, Tr("导出las文件"), Tr("没有打开的las文件"));
		return;
	}

	QString filename = QFileDialog::getSaveFileName(this, Tr("导出las文件"), "",
		Tr("las数据文件 (*.las);;All files (*.*)"));

	if (filename.isNull()) return;

	ExpParam param;
	if (ui->gb_intentFilter->isChecked())
	{
		param.intentMin = ui->le_minIntent->text().toInt();
		param.intentMax = ui->le_maxIntent->text().toInt();
	}
	if (ui->gb_altitudeFilter->isChecked())
	{
		param.altitudeMin = ui->le_minAltitude->text().toDouble();
		param.altitudeMax = ui->le_maxAltitude->text().toDouble();
	}
	if (ui->gb_splitFile->isChecked())
	{
		param.splitPointsNum = ui->le_pointsNum->text().toInt();
	}
	if (ui->gb_colorFilter->isChecked())
	{
		param.bFiltColor = true;
		param.filtColor = ui->lb_defaultColor->palette().window().color().rgb();
	}
	m_pLasReader->exportLas(filename, param);

	//int splitPointsNumber = 3000000;
	//if (ui->gb_splitFile->isChecked())
	//	splitPointsNumber = ui->le_pointsNum->text().toInt();

	//QFileInfo finfo(filename);
	//QString prefix = finfo.canonicalPath() + "/" + finfo.completeBaseName();

	//m_pLasReader->splitFile(splitPointsNumber, prefix);
}

void DlgLasInfo::on_statistic()
{
	if (m_pLasReader == NULL)
		return;

	m_pLasReader->statistic();
}

void DlgLasInfo::on_statisticFinished()
{
	QPointF *points = m_pLasReader->getIntentStatisticData();

	QLineSeries *series = new QLineSeries();
	for (int i = 0; i < 100; i++)
		series->append(points[i]);

	QChart *chart = new QChart();
	chart->legend()->hide();
	chart->addSeries(series);
	chart->createDefaultAxes();

	ui->chart_Intent->setRenderHint(QPainter::Antialiasing);
	ui->chart_Intent->setChart(chart);

	points = m_pLasReader->getAltitudeStatisticData();
	series = new QLineSeries();
	for (int i = 0; i < 100; i++)
		series->append(points[i]);
	chart = new QChart();
	chart->legend()->hide();
	chart->addSeries(series);
	chart->createDefaultAxes();

	ui->chart_Altitude->setRenderHint(QPainter::Antialiasing);
	ui->chart_Altitude->setChart(chart);

}