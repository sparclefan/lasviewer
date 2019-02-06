#include <QApplication>
#include <QTextCodec>
#include "MainWindow.h"

int main(int argc, char *argv[])
{
	// test byte order
	int tmp = 1;
	uchar *pTmp = (uchar *)&tmp;
	bool bigEndian = ((*pTmp)==0);

	qDebug()<<"bigEndian: "<<bigEndian;
	qDebug()<<"Size of long: "<<sizeof(long);
	qDebug()<<"Size of double: "<<sizeof(double);

	QTextCodec::setCodecForLocale(QTextCodec::codecForName("gb2312"));
	QApplication a(argc, argv);
	MainWindow w;
	w.show();

	return a.exec();
}

