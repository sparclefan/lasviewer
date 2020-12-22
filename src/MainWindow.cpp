#pragma execution_character_set("utf-8")

#include "MainWindow.h"
#include "ui_mainwindow.h"
#include <QHBoxLayout>
#include "lasFileStruct.h"
#include <QFileDialog>
#include "qchartview.h"
#include "ColorUtils.h"

#define Tr(s) QString::fromUtf8(s)


MainWindow::MainWindow()
	:QMainWindow(), m_workPath(""), m_bInitView(false)
{
	ui = new Ui::MainWindow();
	ui->setupUi(this);

	m_dlgLasInfo = new DlgLasInfo(this);

	readSetting();

	m_osgViewer = new QtOsgViewer(this);
	ui->horizontalLayout->addWidget(m_osgViewer);
	ui->tableWidget->verticalHeader()->hide();
	ui->tableWidget->setColumnWidth(0, 35);
	ui->tableWidget->setColumnWidth(1, 35);
	m_progress = new QProgressBar(this);
	m_progress->setFormat("%p%");
	m_progress->setMinimum(0);
	m_progress->setMaximum(100);
	ui->statusbar->addPermanentWidget(m_progress);
	ui->lw_fileList->setContextMenuPolicy (Qt::CustomContextMenu);
	ui->menu_2->addAction(ui->dock_options->toggleViewAction());

	drawColorPalette(ui->lb_intentColor, 0);
	drawColorPalette(ui->lb_altitudeColor, 0);

	connect(this, SIGNAL(intentRangeChanged(int,int)), this, SLOT(on_intentRangeChanged(int,int)));
	connect(this, SIGNAL(altitudeRangeChanged(int, int)), this, SLOT(on_altitudeRangeChanged(int, int)));
	connect(ui->cb_colorMode, SIGNAL(currentIndexChanged(int)), this, SLOT(on_colorModeChanged(int)));
	connect(ui->cb_IndentColorSet, SIGNAL(currentIndexChanged(int)), this, SLOT(on_IntentColorSetChanged(int)));
	connect(ui->cb_AltitudeColorSet, SIGNAL(currentIndexChanged(int)), this, SLOT(on_AltitudeColorSetChanged(int)));
	connect(ui->cb_pointSize, SIGNAL(currentIndexChanged(int)), this, SLOT(on_pointSizeChanged(int)));
	connect(ui->tableWidget, SIGNAL(cellChanged(int,int)), this, SLOT(on_tableCellChanged(int, int)));
	connect(m_dlgLasInfo, SIGNAL(hideDlg()), this, SLOT(on_dlgLasInfoHide()));
	connect(this, SIGNAL(intentRangeChanged(int, int)), m_dlgLasInfo, SLOT(on_intentRangeChanged(int, int)));

	connect(ui->lw_fileList,SIGNAL(customContextMenuRequested(QPoint)),this,SLOT(filelist_showmenu(QPoint)));

	PointAttr = new osg::Point();

	root = new osg::Group;
	set = new osg::StateSet();
	int pointSize = 1; //ui->cb_pointSize->currentText().toInt();
	PointAttr->setSize(pointSize);
	/// Disable depth test to avoid sort problems and Lighting
	set->setMode(GL_DEPTH_TEST, osg::StateAttribute::ON);
	set->setMode(GL_LIGHTING, osg::StateAttribute::OFF);
	set->setAttribute(PointAttr.get());

	m_osgViewer->setSceneData(root);
}

MainWindow::~MainWindow()
{
	writeSetting();

}

void MainWindow::initIntentAltitudeRange()
{
	ui->lb_minIntent->setText("100");
	ui->lb_maxIntent->setText("0");
	ui->lb_maxAltitude->setText("0");
	ui->lb_minAltitude->setText("100");
}

void MainWindow::readSetting()
{
	QSettings settings("Yupont", "lasViewer");
	settings.beginGroup("config");
	if (settings.contains("workPath")){
		m_workPath = settings.value("workPath").toString();
	}
	settings.endGroup();
}

void MainWindow::writeSetting()
{
	QSettings settings("Yupont", "lasViewer");
	settings.beginGroup("config");
	settings.setValue("workPath", m_workPath);
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
		float H = ColorUtils::caculateColorH((float)i / size.width(), mode);
		ColorUtils::HSL2RGB(H, L, S, r, g, b);
		painter.setPen(QColor(r*255.0, g*255.0, b*255.0));
		painter.drawLine(i, 0, i, size.height());
	}
	pal->setPixmap(image);
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
	for(auto it: m_lasReaders){
		LasReader *reader = it.second;

		switch (id)
		{
			case 0: // 强度
				reader->setIntentColor(ui->cb_IndentColorSet->currentIndex());
				break;
			case 1: // 高程
				reader->setAltitudeColor(ui->cb_AltitudeColorSet->currentIndex());
				break;
			case 2: // 高程+强度
				reader->setBlendColor(ui->cb_IndentColorSet->currentIndex(), ui->cb_AltitudeColorSet->currentIndex());
				break;
			case 3: // 分类
				reader->setClassifyColor();
				break;
			case 4: // RGB
				reader->setRGBColor();
				break;
			default:
				reader->setOverallColor(QColor(255, 255, 255));
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

	for(auto it: m_lasReaders){
		LasReader *reader = it.second;
		reader->setIntentRange(maxIntent, minIntent, ui->cb_IndentColorSet->currentIndex());
	}

}

void MainWindow::on_altitudeRangeChanged(int minAlt, int maxAlt)
{
	ui->lb_minAltitude->setText(QString("%1").arg(minAlt));
	ui->lb_maxAltitude->setText(QString("%1").arg(maxAlt));

	for(auto it: m_lasReaders){
		LasReader *reader = it.second;
		reader->setAltitudeRange(maxAlt, minAlt, ui->cb_AltitudeColorSet->currentIndex());
	}
}

void MainWindow::on_tableCellChanged(int row, int coloumn)
{
	if( row<0 || (coloumn != 0)) 
		return;

	int classify = ui->tableWidget->item(row, 3)->text().toInt();
	osg::ref_ptr<ClassifyLayers> layer = m_classifyLayers[classify];
	layer->showLayers( (ui->tableWidget->item(row, coloumn)->checkState() == Qt::Checked) );

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

void MainWindow::on_actionResetmap_triggered()
{
	if (m_osgViewer){
		// printf("reset map....\n");
		m_osgViewer->getCameraManipulator()->home(0.0);
	}
}

void MainWindow::on_actionOpen_triggered()
{
	QStringList filelist = QFileDialog::getOpenFileNames(this, Tr("打开点云数据文件"), m_workPath,
		Tr("las数据文件 (*.las);;All files (*.*)"));

	if( filelist.isEmpty() ) return;

	for(int i=0; i<filelist.size(); i++)
	{
		QString filename = filelist.at(i);
		QFileInfo finfo(filename);
		m_workPath = finfo.canonicalPath();

		if( m_fileGroups.find(filename) != m_fileGroups.end() )
		{
			continue;
		}

		osg::ref_ptr<Switch> filegroup = new osg::Switch();
		m_fileGroups[filename] = filegroup;

		osg::ref_ptr<Group> group = new osg::Group();
		filegroup->addChild(group);
		root->addChild(filegroup);

		QListWidgetItem *item = new QListWidgetItem(finfo.baseName());
		item->setToolTip(filename);
		ui->lw_fileList->addItem(item);

		LasReader * reader = new LasReader(filename, group, set);
		
		connect(reader, SIGNAL(progress(int)), this, SLOT(onProgress(int)));
		connect(reader, SIGNAL(processFinished(int, int, double, double)), this, SLOT(onReadFinished(int, int, double, double)));
		connect(reader, SIGNAL(newClassifyLayerAdded(QString, osg::Switch *, int)), this, SLOT(onNewLayerAdded(QString, osg::Switch *, int)));

		m_lasReaders[filename] = reader;

		reader->start();
	}

}

void MainWindow::onNewLayerAdded(QString filename, osg::Switch *layerSwitch, int classify)
{
	int row;
	osg::ref_ptr<ClassifyLayers> classifyLayer;
	if (m_classifyLayers.find(classify) != m_classifyLayers.end()) {
		classifyLayer = m_classifyLayers[classify];
		QList<QTableWidgetItem *> list = ui->tableWidget->findItems(QString("%1").arg(classify), Qt::MatchExactly);
		row = ui->tableWidget->row(list[0]);
	}
	else {
		classifyLayer = new ClassifyLayers(classify);
		m_classifyLayers[classify] = classifyLayer;

		row = ui->tableWidget->rowCount();
		ui->tableWidget->setRowCount(row + 1);

		int id = classify > 31 ? 31 : classify;
		QTableWidgetItem *item = new QTableWidgetItem();
		item->setBackground(classifyColor[id].color);
		ui->tableWidget->setItem(row, 1, item);

		if (classify < 32)
			item = new QTableWidgetItem(Tr(classifyColor[classify].name));
		else
			item = new QTableWidgetItem(Tr("未定义类别"));
		ui->tableWidget->setItem(row, 2, item);

		item = new QTableWidgetItem(QString("%1").arg(classify));
		ui->tableWidget->setItem(row, 3, item);

		item = new QTableWidgetItem();
		item->setCheckState(Qt::Checked);
		ui->tableWidget->setItem(row, 0, item);
	}
	classifyLayer->addLayer(filename, layerSwitch);

}

void MainWindow::onReadFinished(int minIntent, int maxIntent, double minAltitude, double maxAltitude)
{
	if (!m_bInitView ) {
		m_osgViewer->getCameraManipulator()->home(0.0);
		emit altitudeRangeChanged((int)minAltitude, (int)maxAltitude);
		emit intentRangeChanged(minIntent, maxIntent);
		m_bInitView = true;
	}
	else {

		double minZ = ui->lb_minAltitude->text().toDouble();
		double maxZ = ui->lb_maxAltitude->text().toDouble();

		if (minAltitude < minZ || maxAltitude > maxZ)
		{
			if (minAltitude < minZ)
				minZ = minAltitude;
			if (maxAltitude > maxZ)
				maxZ = maxAltitude;
			emit altitudeRangeChanged(minZ, maxZ);
		}

		int maxIn = ui->lb_maxIntent->text().toInt();
		int minIn = ui->lb_minIntent->text().toInt();
		if (maxIntent > maxIn || minIntent < minIn) {
			if (minIntent < minIn)
				minIn = minIntent;
			if (maxIntent > maxIn)
				maxIn = maxIntent;
			emit intentRangeChanged(minIn, maxIn);
		}
	}
	on_colorModeChanged(ui->cb_colorMode->currentIndex());
}

void MainWindow::filelist_showmenu(QPoint pt)
{
	QListWidgetItem *item = ui->lw_fileList->itemAt(pt); 
	if( item== NULL ) return;

	ActionMapper *actionMapper = new ActionMapper(item);

	QMenu *menu = new QMenu(ui->tableWidget); 
	QAction *action0 = new QAction("统计信息",ui->lw_fileList); 
	QAction *action1 = new QAction("显示",ui->lw_fileList); 
	QAction *action2 = new QAction("关闭",ui->lw_fileList); 

	QString filename = item->toolTip();
	osg::ref_ptr<osg::Switch> group = m_fileGroups[filename];
	action1->setCheckable(TRUE);
	action1->setChecked(group->getValue(0));

	connect (action0,SIGNAL(triggered()),actionMapper,SLOT(statisticFile())); 
	connect (action1,SIGNAL(toggled(bool)),actionMapper,SLOT(showFile(bool))); 
	connect (action2,SIGNAL(triggered()),actionMapper,SLOT(closeFile())); 

	connect(actionMapper, SIGNAL(sigStatisticFile(QListWidgetItem *)), this, SLOT(statisticFile(QListWidgetItem *)));
	connect(actionMapper, SIGNAL(sigShowFile(QListWidgetItem *, bool)), this, SLOT(showFile(QListWidgetItem *, bool)));
	connect(actionMapper, SIGNAL(sigCloseFile(QListWidgetItem *)), this, SLOT(closeFile(QListWidgetItem *)));

	menu->addAction(action0); 
	menu->addAction(action1); 
	menu->addAction(action2); 
	menu->move (cursor ().pos ()); 
	menu->show (); 

}

void MainWindow::showFile(QListWidgetItem *item, bool bShow)
{
	QString filename = item->toolTip();
	osg::ref_ptr<osg::Switch> group = m_fileGroups[filename];

	group->setValue(0, bShow);
}

void MainWindow::closeFile(QListWidgetItem *item)
{
	QString filename = item->toolTip();
	osg::ref_ptr<osg::Switch> group = m_fileGroups[filename];
	root->removeChild(group);
	m_fileGroups.erase(filename);

	LasReader *reader = m_lasReaders[filename];
	delete reader;
	m_lasReaders.erase(filename);

	QList<int> delList;
	for( auto it : m_classifyLayers)
	{
		osg::ref_ptr<ClassifyLayers> layer = it.second;
		if( layer->removeLayer(filename)){

			int classify = layer->classify();
			QList<QTableWidgetItem *> list = ui->tableWidget->findItems(QString("%1").arg(classify), Qt::MatchExactly);
			int row = ui->tableWidget->row(list[0]);
			ui->tableWidget->removeRow(row);
			delList.push_back(it.first);
		}
	}

	for (auto c : delList)
	{
		m_classifyLayers.erase(c);
	}

	ui->lw_fileList->takeItem(ui->lw_fileList->row(item));

	if(m_fileGroups.empty()){
		LasReader::resetAdjCoord();
		initIntentAltitudeRange();
		m_bInitView = false;
	}

	m_osgViewer->requestRedraw();

}

void MainWindow::statisticFile(QListWidgetItem *item)
{
	QString filename = item->toolTip();
	LasReader *reader = m_lasReaders[filename];

	m_dlgLasInfo->setLasReader(reader);
	m_dlgLasInfo->setWindowTitle("Las文件信息 -"+item->text());
	m_dlgLasInfo->setVisible(TRUE);
	ui->actionLasInfo->setChecked(true);
}