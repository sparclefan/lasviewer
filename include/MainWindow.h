#ifndef __MainWindow_H_SPARCLE_2019_1_10
#define __MainWindow_H_SPARCLE_2019_1_10

#include <osg/Point>
#include <QMainWindow>
#include <QProgressBar>
#include "QtOsgViewer.h"
#include "LasReader.h"

namespace Ui{
	class MainWindow;
}

class MainWindow : public QMainWindow
{
	Q_OBJECT
public:
	MainWindow();
	~MainWindow();

	static QColor getIntentColor(double factorr, int mode);

public slots:
	void on_actionOpen_triggered();
	void on_actionSplit_triggered();
	void on_intentRangeChanged(int minIntent, int maxIntent);
	void on_intentColorChecked(bool on);
	void on_classifyColorChecked(bool on);
	void on_colorModeChanged(int id);
	void on_pointSizeChanged(int id);
	void on_tableCellChanged(int row, int coloumn);
	void on_thinFactorChanged(int id);

	void onProgress(int percent);
	void onReadFinished();

signals:
	void intentRangeChanged(int minIntent, int maxIntent);

private:

	LasReader *m_lasReader;

	std::map<int, osg::ref_ptr<PointCloudLayer>> m_layers;

	osg::ref_ptr<osg::Geode> PointsGeode;
	osg::ref_ptr<osg::Point> PointAttr;

	Ui::MainWindow *ui;
	QtOsgViewer *m_osgViewer;
	QProgressBar *m_progress;

	void readSetting();
	void writeSetting();

	void drawColorPalette();
	void clearPointsData();
	void loadLasPoints();

	static void HSL2RGB(float H, float S, float L, float &R, float &G, float &B);
	static float Hue_2_RGB(float v1, float v2, float vH);
	static float caculateColorH(float factor, int mode);

	int m_colorMode;
	int m_thinFactor;

	QString m_workPath;

};

#endif //__MainWindow_H_SPARCLE_2019_1_10