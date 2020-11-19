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
	:QMainWindow(), m_workPath(""), m_thinFactor(10)
{
	ui = new Ui::MainWindow();
	ui->setupUi(this);

	m_dlgLasInfo = new DlgLasInfo(this);

	// tabifyDockWidget(ui->dock_classify, ui->dock_options);

	readSetting();

	m_osgViewer = new QtOsgViewer(this);
	ui->horizontalLayout->addWidget(m_osgViewer);
	ui->cb_thinning->setCurrentText("10");
	ui->tableWidget->verticalHeader()->hide();
	ui->tableWidget->setColumnWidth(0, 35);
	ui->tableWidget->setColumnWidth(1, 35);
	ui->cb_thinning->setCurrentText(QString("%1").arg(m_thinFactor));
	m_progress = new QProgressBar(this);
	m_progress->setFormat("%p%");
	m_progress->setMinimum(0);
	m_progress->setMaximum(100);
	ui->statusbar->addPermanentWidget(m_progress);

	ui->lw_fileList->setContextMenuPolicy (Qt::CustomContextMenu);

	// ui->menu_2->addAction(ui->dock_classify->toggleViewAction());
	ui->menu_2->addAction(ui->dock_options->toggleViewAction());

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
		std::map<int, osg::ref_ptr<PointCloudLayer>> layers = reader->readPointsLayers();

		for (auto itl : layers)
		{
			osg::ref_ptr<PointCloudLayer> layer = itl.second;
			int classify = itl.first;
			int classid = classify > 31 ? 31 : classify;
			switch (id)
			{
				case 0: // 强度
					layer->setIntentColor((IntentColorCallBack)(&ColorUtils::getIntentColor), ui->cb_IndentColorSet->currentIndex());
					break;
				case 1: // 高程
					layer->setAltitudeColor((IntentColorCallBack)(&ColorUtils::getIntentColor), ui->cb_AltitudeColorSet->currentIndex());
					break;
				case 2: // 高程+强度
					layer->setBlendColor((BlendColorCallBack)(&ColorUtils::getBlendColor),
						ui->cb_IndentColorSet->currentIndex(), ui->cb_AltitudeColorSet->currentIndex());
					break;
				case 3: // 分类
					layer->setOverallColor(classifyColor[classid].color);
					break;
				case 4: // RGB
					layer->setRGBColor();
					break;
				default:
					layer->setOverallColor(QColor(255, 255, 255));
					break;
				}
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

	for(auto it: m_lasReaders){
		LasReader *reader = it.second;
		std::map<int, osg::ref_ptr<PointCloudLayer>> layers = reader->readPointsLayers();

		for (auto itl : layers)
		{
			osg::ref_ptr<PointCloudLayer> layer = itl.second;
			layer->setAltitudeRange(maxAlt, minAlt, (IntentColorCallBack)(&ColorUtils::getIntentColor), ui->cb_AltitudeColorSet->currentIndex());
		}
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

void MainWindow::on_thinFactorChanged(int id)
{
	m_thinFactor = ui->cb_thinning->currentText().toInt();
	
	for( auto it : m_fileGroups )
	{
		osg::ref_ptr<osg::Group> group = it.second;
		root->removeChild(group);
	}
	std::map<QString, osg::ref_ptr<osg::Switch>> empty;
	m_fileGroups.swap(empty);
	ui->lw_fileList->clear();

	for( auto it : m_lasReaders )
	{
		LasReader *reader = it.second;
		reader->setThinFactor(m_thinFactor);

		reader->start();
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

		LasReader *reader = new LasReader(filename);
		reader->setThinFactor(m_thinFactor);

		connect(reader, SIGNAL(progress(int)), this, SLOT(onProgress(int)));
		connect(reader, SIGNAL(processFinished(LasReader *)), this, SLOT(onReadFinished(LasReader *)));

		m_lasReaders[filename] = reader;

		reader->start();
	}

}

void MainWindow::onReadFinished(LasReader *reader)
{
	ui->statusbar->showMessage("Ready");
	m_progress->setValue(100);

	QString filename = reader->filePath();
	osg::ref_ptr<Switch> filegroup = new osg::Switch();
	osg::ref_ptr<Group> group = new osg::Group();
	filegroup->addChild(group);
	root->addChild(filegroup);

	QFileInfo finfo(filename);
	QListWidgetItem *item = new QListWidgetItem(finfo.baseName());
	item->setToolTip(filename);
	ui->lw_fileList->addItem(item);

	std::map<int, osg::ref_ptr<PointCloudLayer>> layers = reader->readPointsLayers();

	int row;
	for (auto it : layers)
	{
		osg::ref_ptr<PointCloudLayer> layer = it.second;
		
		osg::ref_ptr<Switch> layerswitch = new osg::Switch();
		group->addChild(layerswitch);

		osg::ref_ptr<osg::Geode> pointsGeode = new osg::Geode();
		pointsGeode->setStateSet(set.get());
		layerswitch->addChild(pointsGeode.get());
		pointsGeode->addDrawable(layer->getGeometry());

		int classify = it.first;
		osg::ref_ptr<ClassifyLayers> classifyLayer;
		if( m_classifyLayers.find(classify) != m_classifyLayers.end()){
			classifyLayer = m_classifyLayers[classify];
			QList<QTableWidgetItem *> list = ui->tableWidget->findItems(QString("%1").arg(classify), Qt::MatchExactly);
			row = ui->tableWidget->row(list[0]);
		}else{
			classifyLayer = new ClassifyLayers(classify);
			m_classifyLayers[classify] = classifyLayer;

			row = ui->tableWidget->rowCount();
			ui->tableWidget->setRowCount(row + 1);

			int id = classify>31 ? 31 : classify;
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
		classifyLayer->addLayer(filename, layerswitch);

		// item = new QTableWidgetItem(QString("%1").arg(layer->getPointNumber()));
		// ui->tableWidget->setItem(row, 2, item);
	}

	AsprsLasFile::LasHeader *header = reader->getHeader();

	if( m_fileGroups.empty()){
		m_osgViewer->getCameraManipulator()->home(0.0);	
		emit altitudeRangeChanged(header->m_minZ, header->m_maxZ);
	}else{
		double minZ = ui->lb_minAltitude->text().toDouble();
		double maxZ = ui->lb_maxAltitude->text().toDouble();

		if( header->m_minZ < minZ || header->m_maxZ > maxZ )
		{
			if( header->m_minZ < minZ )
				minZ = header->m_minZ;
			if( header->m_maxZ > maxZ )
				maxZ = header->m_maxZ;
			emit altitudeRangeChanged(minZ, maxZ);
		}else{
			for (auto itl : layers)
			{
				osg::ref_ptr<PointCloudLayer> layer = itl.second;
				layer->setAltitudeRange(maxZ, minZ, (IntentColorCallBack)(&ColorUtils::getIntentColor), ui->cb_AltitudeColorSet->currentIndex());
			}			
		}
	}

	m_fileGroups[filename] = filegroup;


	on_colorModeChanged(ui->cb_colorMode->currentIndex());

	emit intentRangeChanged(reader->getMinIntent(), reader->getMaxIntent());
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

	for( auto it : m_classifyLayers)
	{
		osg::ref_ptr<ClassifyLayers> layer = it.second;
		if( layer->removeLayer(filename)){

			int classify = layer->classify();
			QList<QTableWidgetItem *> list = ui->tableWidget->findItems(QString("%1").arg(classify), Qt::MatchExactly);
			int row = ui->tableWidget->row(list[0]);
			ui->tableWidget->removeRow(row);

			m_classifyLayers.erase(classify);

		}
	}

	ui->lw_fileList->takeItem(ui->lw_fileList->row(item));

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