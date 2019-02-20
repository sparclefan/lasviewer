#ifndef __MainWindow_H_SPARCLE_2019_1_10
#define __MainWindow_H_SPARCLE_2019_1_10

#include <osg/Point>
#include <QMainWindow>
#include <QProgressBar>
#include <QLabel>
#include "QtOsgViewer.h"
#include "LasReader.h"
#include "DlgLasInfo.h"

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
	static QColor getBlendColor(double f1, double f2, int mode1, int mode2);

public slots:
	void on_actionLasInfo_triggered();
	void on_actionOpen_triggered();
	void on_actionSplit_triggered();
	void on_intentRangeChanged(int minIntent, int maxIntent);
	void on_altitudeRangeChanged(int minAlt, int maxAlt);
	void on_colorModeChanged(int id);
	void on_IntentColorSetChanged(int id);
	void on_AltitudeColorSetChanged(int id);
	void on_pointSizeChanged(int id);
	void on_tableCellChanged(int row, int coloumn);
	void on_thinFactorChanged(int id);

	void onProgress(int percent);
	void onReadFinished();

	void on_dlgLasInfoHide();

signals:
	void intentRangeChanged(int minIntent, int maxIntent);
	void altitudeRangeChanged(int minAlt, int maxAlt);

private:

	LasReader *m_lasReader;
	DlgLasInfo *m_dlgLasInfo;

	std::map<int, osg::ref_ptr<PointCloudLayer>> m_layers;

	osg::ref_ptr<osg::Geode> PointsGeode;
	osg::ref_ptr<osg::Point> PointAttr;

	Ui::MainWindow *ui;
	QtOsgViewer *m_osgViewer;
	QProgressBar *m_progress;

	void readSetting();
	void writeSetting();

	void drawColorPalette(QLabel *pal, int mode);
	void clearPointsData();
	void loadLasPoints();

	static void HSL2RGB(float H, float S, float L, float &R, float &G, float &B);
	static float Hue_2_RGB(float v1, float v2, float vH);
	static float caculateColorH(float factor, int mode);
	static float crCalculateBlend(float c1, float c2);

	int m_thinFactor;

	QString m_workPath;

};

#endif //__MainWindow_H_SPARCLE_2019_1_10