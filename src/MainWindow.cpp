#include "MainWindow.h"
#include "ui_mainwindow.h"
#include <QHBoxLayout>
#include "lasFileStruct.h"
#include <QFileDialog>

#define Tr(s) QString::fromLocal8Bit(s)

typedef struct{
	QColor color;
	char name[32];
} ClassifyCfg;

const ClassifyCfg classifyColor[] = {
	{ QColor(255, 100, 100), "导线" },
	{ QColor(160, 160, 160), "下面的导线" },
	{ QColor(239, 250, 19), "其他导线" },
	{ QColor(170, 0, 127), "地线" },
	{ QColor(0, 180, 0), "低植被" },
	{ QColor(0, 255, 0), "高植被" },
	{ QColor(0, 255, 0), "植被" },
	{ QColor(139, 133, 30), "地面" },

	{ QColor(128, 64, 64), "地面关键点" },
	{ QColor(0, 0, 255), "建筑物" },
	{ QColor(80, 80, 255), "临时建筑物" },
	{ QColor(60, 60, 60), "道路" },
	{ QColor(170, 170, 170), "桥梁" },
	{ QColor(100, 60, 100), "铁路" },
	{ QColor(255, 128, 128), "上跨越" },
	{ QColor(255, 128, 128), "下跨越" },

	{ QColor(234, 236, 255), "杆塔" },
	{ QColor(190, 190, 190), "承力索/接触线" },
	{ QColor(151, 219, 242), "河流" },
	{ QColor(151, 219, 242), "通航河流" },
	{ QColor(151, 219, 242), "不通航河流" },
	{ QColor(255, 0, 0), "危险点" },
	{ QColor(255, 200, 50), "绝缘子" },
	{ QColor(19, 1, 0), "其他" },

	{ QColor(128, 128, 128), "噪声" },
	{ QColor(236, 236, 236), "变电站" },
	{ QColor(236, 236, 236), "汽车船舶" },
	{ QColor(151, 219, 242), "湖泊" },
	{ QColor(255, 123, 0), "引流线" },
	{ QColor(255, 0, 255), "平断面图地面线1" },
	{ QColor(128, 0, 255), "平断面图地面线2" },
	{ QColor(160, 160, 160), "无类别" }
};

const int classifyColorNum = sizeof(classifyColor) / sizeof(ClassifyCfg);

MainWindow::MainWindow()
	:QMainWindow(), m_colorMode(0), m_workPath(""), m_thinFactor(10), m_lasReader(NULL)
{
	ui = new Ui::MainWindow();
	ui->setupUi(this);

	readSetting();

	m_osgViewer = new QtOsgViewer(this);
	ui->horizontalLayout->addWidget(m_osgViewer);
	ui->cb_thinning->setCurrentText("10");
	ui->tableWidget->verticalHeader()->hide();
	ui->tableWidget->setColumnWidth(3, 35);
	ui->tableWidget->setColumnWidth(4, 35);
	ui->cb_thinning->setCurrentText(QString("%1").arg(m_thinFactor));
	m_progress = new QProgressBar(this);
	m_progress->setFormat("%p%");
	m_progress->setMinimum(0);
	m_progress->setMaximum(100);
	ui->statusbar->addPermanentWidget(m_progress);

	drawColorPalette();

	connect(this, SIGNAL(intentRangeChanged(int,int)), this, SLOT(on_intentRangeChanged(int,int)));
	connect(ui->gb_intentColor, SIGNAL(toggled(bool)), this, SLOT(on_intentColorChecked(bool)));
	connect(ui->gb_classify, SIGNAL(toggled(bool)), this, SLOT(on_classifyColorChecked(bool)));
	connect(ui->cb_colorMode, SIGNAL(currentIndexChanged(int)), this, SLOT(on_colorModeChanged(int)));
	connect(ui->cb_pointSize, SIGNAL(currentIndexChanged(int)), this, SLOT(on_pointSizeChanged(int)));
	connect(ui->cb_thinning, SIGNAL(currentIndexChanged(int)), this, SLOT(on_thinFactorChanged(int)));
	connect(ui->tableWidget, SIGNAL(cellChanged(int,int)), this, SLOT(on_tableCellChanged(int, int)));

	PointsGeode = new osg::Geode();
	PointAttr = new osg::Point();


	this->showMaximized();
}

MainWindow::~MainWindow()
{
	writeSetting();

	if (m_lasReader != NULL)
		delete m_lasReader;
}

void MainWindow::readSetting()
{
	QSettings settings("Yupont", "lasViewer");
	settings.beginGroup("config");
	if (settings.contains("workPath")){
		m_workPath = settings.value("workPath").toString();
	}
	if (settings.contains("thinFactor")){
		m_thinFactor = settings.value("thinFactor").toInt();
	}

	settings.endGroup();
}

void MainWindow::writeSetting()
{
	QSettings settings("Yupont", "lasViewer");
	settings.beginGroup("config");
	settings.setValue("workPath", m_workPath);
	settings.setValue("thisFactor", m_thinFactor);
	settings.endGroup();
}

void MainWindow::drawColorPalette()
{
	QSize size = ui->lb_intentColor->size();
	QPixmap image(size);
	QPainter painter(&image);
	float L = 1.0;
	float S = 0.5;
	float r, g, b;
	for (int i = 0; i < size.width(); i++)
	{
		float H = caculateColorH((float)i / size.width(), m_colorMode);
		HSL2RGB(H, L, S, r, g, b);
		painter.setPen(QColor(r*255.0, g*255.0, b*255.0));
		painter.drawLine(i, 0, i, size.height());
	}
	ui->lb_intentColor->setPixmap(image);
}

void MainWindow::clearPointsData()
{
	PointsGeode = new osg::Geode();
	ui->tableWidget->clearContents();
}

void MainWindow::on_colorModeChanged(int id)
{
	m_colorMode = id;
	drawColorPalette();

	for (auto it : m_layers)
	{
		osg::ref_ptr<PointCloudLayer> layer = it.second;
		layer->setIntentColor((IntentColorCallBack)(&MainWindow::getIntentColor), m_colorMode);
	}

	if (m_osgViewer)
		m_osgViewer->requestRedraw();

}

void MainWindow::on_classifyColorChecked(bool on)
{
	for (auto it : m_layers)
	{
		int classify = it.first;
		osg::ref_ptr<PointCloudLayer> layer = it.second;
		if (on)
		{
			int id = classify>31 ? 31 : classify;
			layer->setOverallColor(classifyColor[id].color);
		}
		else{
			if (ui->gb_intentColor->isChecked()){
				layer->setIntentColor((IntentColorCallBack)(&MainWindow::getIntentColor), m_colorMode);
			}
			else{
				layer->setOverallColor(QColor(255,255,255));
			}
		}
	}

	ui->gb_intentColor->setEnabled(!on);

	if (m_osgViewer)
		m_osgViewer->requestRedraw();
}

void MainWindow::on_intentRangeChanged(int minIntent, int maxIntent)
{
	ui->lb_minIntent->setText(QString("%1").arg(minIntent));
	ui->lb_maxIntent->setText(QString("%1").arg(maxIntent));
}

void MainWindow::on_intentColorChecked(bool on)
{
	if (!on)
	{
		for (auto it : m_layers)
		{
			osg::ref_ptr<PointCloudLayer> layer = it.second;
			layer->setOverallColor(QColor(255,255,255));
		}
	}
	else{
		for (auto it : m_layers)
		{
			osg::ref_ptr<PointCloudLayer> layer = it.second;
			layer->setIntentColor((IntentColorCallBack)(&MainWindow::getIntentColor), m_colorMode);
		}
	}
	if (m_osgViewer)
		m_osgViewer->requestRedraw();
}

void MainWindow::on_tableCellChanged(int row, int coloumn)
{
	if( row<0 || (coloumn != 4)) 
		return;

	int classify = ui->tableWidget->item(row, 0)->text().toInt();
	osg::ref_ptr<PointCloudLayer> layer = m_layers[classify];

	if (ui->tableWidget->item(row, coloumn)->checkState() == Qt::Checked)
	{
		PointsGeode->addDrawable(layer->getGeometry());
	}
	else{
		PointsGeode->removeDrawable(layer->getGeometry());
	}

	if (m_osgViewer)
		m_osgViewer->requestRedraw();
}

void MainWindow::on_pointSizeChanged(int id)
{
	int ptSize = ui->cb_pointSize->currentText().toInt();
	PointAttr->setSize(ptSize);
	if (m_osgViewer)
		m_osgViewer->requestRedraw();
}

void MainWindow::onProgress(int percent)
{
	m_progress->setValue(percent);
}

void MainWindow::loadLasPoints()
{
	if (m_lasReader == NULL)
		return;

	if (m_lasReader->getThinFactor() == m_thinFactor)
	{
		onReadFinished();
		return;
	}

	ui->statusbar->showMessage(Tr("读取las文件..."));
	m_progress->setValue(0);

	connect(m_lasReader, SIGNAL(progress(int)), this, SLOT(onProgress(int)));
	connect(m_lasReader, SIGNAL(processFinished()), this, SLOT(onReadFinished()));

	m_lasReader->setThinFactor(m_thinFactor);
	m_lasReader->start();
}

void MainWindow::onReadFinished()
{
	ui->statusbar->showMessage("Ready");
	m_progress->setValue(100);

	clearPointsData();
	m_layers = m_lasReader->readPointsLayers();

	int row = 0;
	for (auto it : m_layers)
	{
		osg::ref_ptr<PointCloudLayer> layer = it.second;
		layer->getGeometry();

		ui->tableWidget->setRowCount(row + 1);
		int classify = it.first;
		QTableWidgetItem *item = new QTableWidgetItem(QString("%1").arg(classify));
		ui->tableWidget->setItem(row, 0, item);

		if (classify < 32)
			item = new QTableWidgetItem(Tr(classifyColor[classify].name));
		else
			item = new QTableWidgetItem(Tr("未定义类别"));
		ui->tableWidget->setItem(row, 1, item);

		item = new QTableWidgetItem(QString("%1").arg(layer->getPointNumber()));
		ui->tableWidget->setItem(row, 2, item);

		int id = classify>31 ? 31 : classify;
		item = new QTableWidgetItem();
		item->setBackground(classifyColor[id].color);
		ui->tableWidget->setItem(row, 3, item);

		if (ui->gb_classify->isChecked()){
			layer->setOverallColor(classifyColor[id].color);
		}
		else if (ui->gb_intentColor->isChecked()){
			layer->setIntentColor((IntentColorCallBack)(&MainWindow::getIntentColor), m_colorMode);
		}
		else{
			layer->setOverallColor(QColor(255, 255, 255));
		}

		item = new QTableWidgetItem();
		item->setCheckState(Qt::Checked);
		ui->tableWidget->setItem(row, 4, item);

		row++;

	}

	osg::ref_ptr<osg::Group> root = new osg::Group;
	osg::ref_ptr<osg::StateSet> set = new osg::StateSet();
	int pointSize = ui->cb_pointSize->currentText().toInt();
	PointAttr->setSize(pointSize);
	/// Disable depth test to avoid sort problems and Lighting
	set->setMode(GL_DEPTH_TEST, osg::StateAttribute::ON);
	set->setMode(GL_LIGHTING, osg::StateAttribute::OFF);
	set->setAttribute(PointAttr.get());
	PointsGeode->setStateSet(set.get());

	root->addChild(PointsGeode.get());

	m_osgViewer->setSceneData(root.get());
}

void MainWindow::on_thinFactorChanged(int id)
{
	m_thinFactor = ui->cb_thinning->currentText().toInt();
	loadLasPoints();
}

void MainWindow::on_actionSplit_triggered()
{
	QString filename = QFileDialog::getOpenFileName(this, Tr("打开激光数据文件"), m_workPath,
		Tr("las数据文件 (*.las);;All files (*.*)"));

	if (filename.isNull()) return;

	QFileInfo finfo(filename);
	QString prefix = finfo.canonicalPath() + "/"+finfo.completeBaseName();

	LasReader reader(filename);
	reader.splitFile(3000000, prefix);
}

void MainWindow::on_actionOpen_triggered()
{
	QString filename = QFileDialog::getOpenFileName(this, Tr("打开激光数据文件"), m_workPath,
		Tr("las数据文件 (*.las);;All files (*.*)"));

	if (filename.isNull()) return;

	this->setWindowTitle(Tr("las文件查看器 --")+filename);

	QFileInfo finfo(filename);
	m_workPath = finfo.canonicalPath();

	if (m_lasReader != NULL)
		delete m_lasReader;

	m_lasReader = new LasReader(filename);
	YupontLasFile::LasHeader *pHeader = m_lasReader->getHeader();

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
	qint64 julianDay = dt.toJulianDay()+pHeader->m_createDOY;
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

	loadLasPoints();

}

QColor MainWindow::getIntentColor(double factor, int mode)
{
	float H = caculateColorH(factor, mode); //1.0 / 3 + (factor)*2.0 / 3; //(2.0 / 3)*(1.0-factor);
	float L = 1.0;
	float S = 0.5;
	float r, g, b;
	HSL2RGB(H, L, S, r, g, b);
	return QColor((int)(r*255),(int)(g*255),(int)(b*255));
}

float MainWindow::caculateColorH(float factor, int mode)
{
	switch (mode){
	case 0:
		return ((1.0 - factor)*2.0 / 3) - (1.0 / 3);    // 绿-红-蓝
	case 1:
		return (2.0 / 3)*(1.0 - factor);   // 蓝-绿-红
	case 2:
		return (1.0 / 3 + (factor)*2.0 / 3);  // 绿-蓝-红
	case 3:
		return 1.0/3 + factor/3;  // 绿-蓝
	case 4:
		return 1.0 / 3 - factor / 3;  // 绿-红
	default:
		return ((1.0 - factor)*2.0 / 3) - (1.0 / 3);    // 绿-红-蓝
	}
}

float MainWindow::Hue_2_RGB(float v1, float v2, float vH)             //Function Hue_2_RGB
{
	if (vH < 0) vH += 1;
	if (vH > 1) vH -= 1;
	if ((6 * vH) < 1)
		return (v1 + (v2 - v1) * 6 * vH);
	if ((2 * vH) < 1)
		return (v2);

	if ((3 * vH) < 2)
		return (v1 + (v2 - v1) * ((2.0 / 3) - vH) * 6);

	return (v1);
}

void MainWindow::HSL2RGB(float H, float S, float L, float &R, float &G, float &B)
{
	if (S == 0)                       //HSL from 0 to 1
	{
		R = L;                      //RGB results from 0 to 1
		G = L;
		B = L;
	}
	else
	{
		float var_2;
		if (L < 0.5)
			var_2 = L * (1 + S);
		else
			var_2 = (L + S) - (S * L);

		float var_1 = 2 * L - var_2;

		R = Hue_2_RGB(var_1, var_2, H + (1.0 / 3));
		G = Hue_2_RGB(var_1, var_2, H);
		B = Hue_2_RGB(var_1, var_2, H - (1.0 / 3));
	}
}
