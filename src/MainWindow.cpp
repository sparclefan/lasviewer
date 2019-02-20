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
	:QMainWindow(), m_workPath(""), m_thinFactor(10), m_lasReader(NULL)
{
	ui = new Ui::MainWindow();
	ui->setupUi(this);

	m_dlgLasInfo = new DlgLasInfo(this);

	tabifyDockWidget(ui->dock_classify, ui->dock_options);

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

	drawColorPalette(ui->lb_intentColor, 0);
	drawColorPalette(ui->lb_altitudeColor, 0);

	connect(this, SIGNAL(intentRangeChanged(int,int)), this, SLOT(on_intentRangeChanged(int,int)));
	connect(this, SIGNAL(altitudeRangeChanged(int, int)), this, SLOT(on_altitudeRangeChanged(int, int)));
	connect(ui->cb_colorMode, SIGNAL(currentIndexChanged(int)), this, SLOT(on_colorModeChanged(int)));
	connect(ui->cb_IndentColorSet, SIGNAL(currentIndexChanged(int)), this, SLOT(on_IntentColorSetChanged(int)));
	connect(ui->cb_AltitudeColorSet, SIGNAL(currentIndexChanged(int)), this, SLOT(on_AltitudeColorSetChanged(int)));
	connect(ui->cb_pointSize, SIGNAL(currentIndexChanged(int)), this, SLOT(on_pointSizeChanged(int)));
	connect(ui->cb_thinning, SIGNAL(currentIndexChanged(int)), this, SLOT(on_thinFactorChanged(int)));
	connect(ui->tableWidget, SIGNAL(cellChanged(int,int)), this, SLOT(on_tableCellChanged(int, int)));
	connect(m_dlgLasInfo, SIGNAL(hideDlg()), this, SLOT(on_dlgLasInfoHide()));

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

void MainWindow::drawColorPalette(QLabel *pal, int mode)
{
	QSize size = pal->size();
	QPixmap image(size);
	QPainter painter(&image);
	float L = 1.0;
	float S = 0.5;
	float r, g, b;
	for (int i = 0; i < size.width(); i++)
	{
		float H = caculateColorH((float)i / size.width(), mode);
		HSL2RGB(H, L, S, r, g, b);
		painter.setPen(QColor(r*255.0, g*255.0, b*255.0));
		painter.drawLine(i, 0, i, size.height());
	}
	pal->setPixmap(image);
}

void MainWindow::clearPointsData()
{
	PointsGeode = new osg::Geode();
	ui->tableWidget->clearContents();
}

void MainWindow::on_dlgLasInfoHide()
{
	ui->actionLasInfo->setChecked(false);
}

void MainWindow::on_actionLasInfo_triggered()
{
	m_dlgLasInfo->setVisible(ui->actionLasInfo->isChecked());
}

void MainWindow::on_colorModeChanged(int id)
{
	for (auto it : m_layers)
	{
		osg::ref_ptr<PointCloudLayer> layer = it.second;
		int classify = it.first;
		int classid = classify > 31 ? 31 : classify;
		switch (id)
		{
			case 0: // 强度
				layer->setIntentColor((IntentColorCallBack)(&MainWindow::getIntentColor), ui->cb_IndentColorSet->currentIndex());
				break;
			case 1: // 高程
				layer->setAltitudeColor((IntentColorCallBack)(&MainWindow::getIntentColor), ui->cb_AltitudeColorSet->currentIndex());
				break;
			case 2: // 高程+强度
				layer->setBlendColor((BlendColorCallBack)(&MainWindow::getBlendColor),
					ui->cb_IndentColorSet->currentIndex(), ui->cb_AltitudeColorSet->currentIndex());
				break;
			case 3: // 分类
				layer->setOverallColor(classifyColor[classid].color);
				break;
			default:
				layer->setOverallColor(QColor(255, 255, 255));
				break;
			}
	}
	if (m_osgViewer)
		m_osgViewer->requestRedraw();
}

void MainWindow::on_IntentColorSetChanged(int id)
{
	drawColorPalette(ui->lb_intentColor, id);
	on_colorModeChanged(ui->cb_colorMode->currentIndex());
}

void MainWindow::on_AltitudeColorSetChanged(int id)
{
	drawColorPalette(ui->lb_altitudeColor, id);
	on_colorModeChanged(ui->cb_colorMode->currentIndex());
}

void MainWindow::on_intentRangeChanged(int minIntent, int maxIntent)
{
	ui->lb_minIntent->setText(QString("%1").arg(minIntent));
	ui->lb_maxIntent->setText(QString("%1").arg(maxIntent));
}

void MainWindow::on_altitudeRangeChanged(int minAlt, int maxAlt)
{
	ui->lb_minAltitude->setText(QString("%1").arg(minAlt));
	ui->lb_maxAltitude->setText(QString("%1").arg(maxAlt));
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

	on_colorModeChanged(ui->cb_colorMode->currentIndex());

	emit intentRangeChanged(m_lasReader->getMinIntent(), m_lasReader->getMaxIntent());
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
	emit altitudeRangeChanged(pHeader->m_minZ, pHeader->m_maxZ);

	m_dlgLasInfo->setLasInfo(pHeader);

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

QColor MainWindow::getBlendColor(double f1, double f2, int mode1, int mode2)
{
	float H1 = caculateColorH(f1, mode1); 
	float H2 = caculateColorH(f2, mode2);
	float L = 1.0;
	float S = 0.5;
	float r1, g1, b1, r2, g2, b2;
	HSL2RGB(H1, L, S, r1, g1, b1);
	HSL2RGB(H2, L, S, r2, g2, b2);
	float r = crCalculateBlend(r1, r2);
	float g = crCalculateBlend(g2, g2);
	float b = crCalculateBlend(b1, b2);
	return QColor((int)(r * 255), (int)(g * 255), (int)(b * 255));
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

float MainWindow::crCalculateBlend( float c1, float c2)
{
	//(c1 * a1 * (1.0 - a2) + c2 ) / (a1 + a2 - a1 * a2);
	return (c1*0.7 + c2*0.3);
}