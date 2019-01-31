#include "QtOsgViewer.h"

bool QtOSGAdapterWidget::_ismax = false;

QtOSGAdapterWidget::QtOSGAdapterWidget(QWidget * parent, const char * name,
	const QGLWidget * shareWidget, WindowFlags f)
	: QGLWidget(parent, shareWidget, f)
{
	_gw = new osgViewer::GraphicsWindowEmbedded(0, 0, width(), height());
	setFocusPolicy(Qt::ClickFocus);
	setMouseTracking(true);
	_ismax = false;
}

void QtOSGAdapterWidget::resizeGL(int width, int height)
{
	_gw->getEventQueue()->windowResize(0, 0, width, height);
	_gw->resized(0, 0, width, height);
}

void QtOSGAdapterWidget::keyPressEvent(QKeyEvent* event)
{
	_gw->getEventQueue()->keyPress((osgGA::GUIEventAdapter::KeySymbol) *(event->text().toLatin1().data()));
}

void QtOSGAdapterWidget::keyReleaseEvent(QKeyEvent* event)
{
	_gw->getEventQueue()->keyRelease((osgGA::GUIEventAdapter::KeySymbol) *(event->text().toLatin1().data()));
}

void QtOSGAdapterWidget::mousePressEvent(QMouseEvent* event)
{
	int button = 0;
	switch (event->button())
	{
	case(Qt::LeftButton) : button = 2; break;
	case(Qt::MidButton) : button = 3; break;
	case(Qt::RightButton) : button = 1; break;
	case(Qt::NoButton) : button = 0; break;
	default: button = 0; break;
	}
	_gw->getEventQueue()->mouseButtonPress(event->x(), event->y(), button);

	emit signalMousePressEvent(event);
}

void QtOSGAdapterWidget::mouseReleaseEvent(QMouseEvent* event)
{
	int button = 0;
	switch (event->button())
	{
	case(Qt::LeftButton) : button = 2; break;
	case(Qt::MidButton) : button = 3; break;
	case(Qt::RightButton) : button = 1; break;
	case(Qt::NoButton) : button = 0; break;
	default: button = 0; break;
	}
	_gw->getEventQueue()->mouseButtonRelease(event->x(), event->y(), button);

	emit signalMouseReleaseEvent(event);
}

void QtOSGAdapterWidget::mouseMoveEvent(QMouseEvent* event)
{
	_gw->getEventQueue()->mouseMotion(event->x(), event->y());

	emit signalMouseMoveEvent(event);
}

void QtOSGAdapterWidget::wheelEvent(QWheelEvent * event)
{
	_gw->getEventQueue()->mouseScroll(
		event->delta()<0 ? osgGA::GUIEventAdapter::SCROLL_UP :
		osgGA::GUIEventAdapter::SCROLL_DOWN);
}

void QtOSGAdapterWidget::mouseDoubleClickEvent(QMouseEvent * event)
{
	int button = 0;
	switch (event->button())
	{
	case(Qt::LeftButton) : button = 2; break;
	case(Qt::MidButton) : button = 3; break;
	case(Qt::RightButton) : button = 1; break;
	case(Qt::NoButton) : button = 0; break;
	default: button = 0; break;
	}
	_gw->getEventQueue()->mouseDoubleButtonPress(event->x(), event->y(), button);

	if (event->button() == Qt::RightButton)
	{
		if (_ismax)
		{
			emit normalW();
		}

		else
		{
			emit maxW();
		}

		_ismax = !_ismax;
	}
}

///////////////////////////////////////////////////////////////////////

QtOsgViewer::QtOsgViewer(QWidget * parent, const char * name, const QGLWidget * shareWidget, WindowFlags f)
	: QtOSGAdapterWidget(parent, name, shareWidget, f)
{
	getCamera()->setViewport(new osg::Viewport(0, 0, 640, 480));//width()/**10*/, height()/**10*/));
	getCamera()->setProjectionMatrixAsPerspective(30.0f,
		static_cast<double>(width()) / static_cast<double>(height()),
		1.0f, 10000.0f);
	getCamera()->setGraphicsContext(getGraphicsWindow());
	getCamera()->setClearColor(osg::Vec4(0.0, 0.0, 0.0, 1.0));

	setCameraManipulator(new osgGA::TrackballManipulator);
	setThreadingModel(osgViewer::Viewer::SingleThreaded);

	QObject::connect(&_timer, SIGNAL(timeout()), this, SLOT(updateGL()));
	setMinimumSize(10, 10);
	_timer.start(10);

}

void QtOsgViewer::paintGL()
{
	try{
		frame();
	}
	catch (...)
	{
		return;
	}
}

void QtOsgViewer::closeEvent(QCloseEvent *event)
{
	//osg::Group *root = dynamic_cast<osg::Group*>(getSceneData());
	//root->removeChildren(0, 1);
	osg::ref_ptr<osg::Node> tmpnode = new osg::Node;
	this->setSceneData(tmpnode);

	event->accept();
}

void PointCloudCallback::operator()(osg::Node* node, osg::NodeVisitor* nv)
{
	if (osg::ref_ptr<osg::Group> pgroup = dynamic_cast<osg::Group*>(node)){
		
		osg::ref_ptr<osg::Geode> PointsGeode = dynamic_cast<osg::Geode*>(pgroup.get()->getChild(0));

		osg::ref_ptr<osg::Geometry> PointsGeometry = dynamic_cast<osg::Geometry*>(PointsGeode.get()->getDrawable(0));

		osg::Array * array = PointsGeometry->getVertexArray();

	}

	traverse(node, nv);
}