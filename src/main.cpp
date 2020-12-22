#pragma execution_character_set("utf-8")

#include <QApplication>
#include <QTextCodec>
#include "MainWindow.h"

int main(int argc, char *argv[])
{
	// test byte order
	//int tmp = 1;
	//uchar *pTmp = (uchar *)&tmp;
	//bool bigEndian = ((*pTmp)==0);

	//qDebug()<<"bigEndian: "<<bigEndian;
	//qDebug()<<"Size of long: "<<sizeof(long);
	//qDebug()<<"Size of double: "<<sizeof(double);
	//qDebug() << "Size of uint32_t: " << sizeof(uint32_t);

	QTextCodec::setCodecForLocale(QTextCodec::codecForName("utf-8"));
	QApplication a(argc, argv);
	MainWindow w;
	w.showMaximized();

	return a.exec();
}

