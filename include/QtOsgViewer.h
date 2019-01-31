#ifndef __QT_OSG_VIEWER_H_SPARCLE_20190108
#define __QT_OSG_VIEWER_H_SPARCLE_20190108
	
#include <QtCore/QTimer>
#include <QtCore/QString>
#include <QtOpenGL/QGLWidget>
#include <QtWidgets/QApplication>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMdiSubWindow>
#include <QtWidgets/QMdiArea>
#include <QtWidgets/QTextBrowser>

#include <QtGui>
#include <QKeyEvent>

#include <osg/Array>
#include <osgViewer/Viewer>
#include <osgViewer/CompositeViewer>
#include <osgViewer/GraphicsWindow>
#include <osgViewer/ViewerEventHandlers>
#include <osgGA/TrackballManipulator>
#include <osgDB/ReadFile>
#include <osgText/Font>
#include <osgText/Text>

using Qt::WindowFlags;
using namespace osg;

class QtOSGAdapterWidget :public QGLWidget
{
	Q_OBJECT
public:
	QtOSGAdapterWidget(QWidget * parent = 0, const char * name = 0,
		const QGLWidget * shareWidget = 0, WindowFlags f = 0);

	virtual ~QtOSGAdapterWidget() { }

	osgViewer::GraphicsWindow* getGraphicsWindow() { return _gw.get(); }
	const osgViewer::GraphicsWindow* getGraphicsWindow() const { return _gw.get(); }

signals:
	void signalMouseMoveEvent(QMouseEvent* event);
	void signalMousePressEvent(QMouseEvent* event);
	void signalMouseReleaseEvent(QMouseEvent* event);
	void maxW();
	void normalW();
public:

	void init();
	virtual void resizeGL(int width, int height);
	virtual void keyPressEvent(QKeyEvent* event);
	virtual void keyReleaseEvent(QKeyEvent* event);
	virtual void mousePressEvent(QMouseEvent* event);
	virtual void mouseReleaseEvent(QMouseEvent* event);
	virtual void mouseMoveEvent(QMouseEvent* event);
	virtual void wheelEvent(QWheelEvent * event);
	virtual void mouseDoubleClickEvent(QMouseEvent * event);

	osg::ref_ptr<osgViewer::GraphicsWindowEmbedded> _gw;
	static bool _ismax;
};

class  QtOsgViewer :public QtOSGAdapterWidget, public osgViewer::Viewer
{
	Q_OBJECT
public:
	QtOsgViewer(QWidget * parent = 0, const char * name = 0,
		const QGLWidget * shareWidget = 0, WindowFlags f = 0);

	virtual void paintGL();

protected:

	virtual void closeEvent(QCloseEvent *event);
	QTimer _timer;

signals:
	void OutSetChecked(bool);
};

class PointCloudCallback : public osg::NodeCallback
{
public:
	PointCloudCallback(){};

	virtual void operator()(osg::Node* node, osg::NodeVisitor* nv);
};

#endif //__QT_OSG_VIEWER_H_SPARCLE_20190108