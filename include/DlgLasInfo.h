#ifndef __DlgLasInfo_h_SPARCLE_2019_02_18
#define __DlgLasInfo_h_SPARCLE_2019_02_18

#include <QDialog>

namespace Ui{
	class LasInfo;
}

namespace AsprsLasFile{
	struct LasHeader;
}

class LasReader;

class DlgLasInfo : public QDialog
{
	Q_OBJECT
public:
	DlgLasInfo(QWidget *parent = Q_NULLPTR);
	~DlgLasInfo();

	void setLasReader(LasReader *lasReader);

public slots:
	void on_intentRangeChanged(int minIntent, int maxIntent);
	void on_export();
	void on_statistic();
	void on_statisticFinished();

signals:
	void hideDlg();

protected:
	virtual void hideEvent(QHideEvent *event);

private:
	Ui::LasInfo *ui;
	LasReader *m_pLasReader;
	void setLasInfo(AsprsLasFile::LasHeader *pHeader);
};

#endif //__DlgLasInfo_h_SPARCLE_2019_02_18