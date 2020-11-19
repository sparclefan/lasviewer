#ifndef __MainWindow_H_SPARCLE_2019_1_10
#define __MainWindow_H_SPARCLE_2019_1_10

#include <osg/Point>
#include <QMainWindow>
#include <QProgressBar>
#include <QLabel>
#include <QListWidgetItem>
#include "QtOsgViewer.h"
#include "LasReader.h"
#include "DlgLasInfo.h"
#include "ClassifyLayers.h"

namespace Ui{
	class MainWindow;
}

// 为了向列表中右键菜单 action 传参数(listitem)，定义一个mapper
class ActionMapper : public QObject
{
	Q_OBJECT
public:
	ActionMapper(QListWidgetItem *item):m_item(item){};
	~ActionMapper(){};

public slots:
	void statisticFile(){emit sigStatisticFile(m_item);};
	void showFile(bool checked){emit sigShowFile(m_item, checked);};
	void closeFile(){emit sigCloseFile(m_item);};

signals:
	void sigStatisticFile(QListWidgetItem *);
	void sigShowFile(QListWidgetItem *, bool);
	void sigCloseFile(QListWidgetItem *);

private:
	QListWidgetItem *m_item;
};

class MainWindow : public QMainWindow
{
	Q_OBJECT
public:
	MainWindow();
	~MainWindow();

public slots:
	void on_actionLasInfo_triggered();
	void on_actionOpen_triggered();
	void on_intentRangeChanged(int minIntent, int maxIntent);
	void on_altitudeRangeChanged(int minAlt, int maxAlt);
	void on_colorModeChanged(int id);
	void on_IntentColorSetChanged(int id);
	void on_AltitudeColorSetChanged(int id);
	void on_pointSizeChanged(int id);
	void on_tableCellChanged(int row, int coloumn);
	void on_thinFactorChanged(int id);

	void onProgress(int percent);
	void onReadFinished(LasReader *reader);

	void on_dlgLasInfoHide();

	void filelist_showmenu(QPoint pt);
	void showFile(QListWidgetItem *item, bool bShow);
	void closeFile(QListWidgetItem *item);
	void statisticFile(QListWidgetItem *item);

signals:
	void intentRangeChanged(int minIntent, int maxIntent);
	void altitudeRangeChanged(int minAlt, int maxAlt);

private:

	DlgLasInfo *m_dlgLasInfo;

	std::map<QString, osg::ref_ptr<osg::Switch>> m_fileGroups;
	std::map<QString, LasReader *> m_lasReaders;
	std::map<int, osg::ref_ptr<ClassifyLayers>> m_classifyLayers;

	osg::ref_ptr<osg::Group> root;
	osg::ref_ptr<osg::StateSet> set;
	osg::ref_ptr<osg::Point> PointAttr;

	Ui::MainWindow *ui;
	QtOsgViewer *m_osgViewer;
	QProgressBar *m_progress;

	void readSetting();
	void writeSetting();

	void drawColorPalette(QLabel *pal, int mode);

	int m_thinFactor;

	QString m_workPath;

};

#endif //__MainWindow_H_SPARCLE_2019_1_10