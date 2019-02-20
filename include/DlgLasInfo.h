#ifndef __DlgLasInfo_h_SPARCLE_2019_02_18
#define __DlgLasInfo_h_SPARCLE_2019_02_18

#include <QDialog>

namespace Ui{
	class LasInfo;
}

namespace YupontLasFile{
	struct LasHeader;
}

class DlgLasInfo : public QDialog
{
	Q_OBJECT
public:
	DlgLasInfo(QWidget *parent = Q_NULLPTR);
	~DlgLasInfo();

	void setLasInfo(YupontLasFile::LasHeader *pHeader);

signals:
	void hideDlg();

protected:
	virtual void hideEvent(QHideEvent *event);

private:
	Ui::LasInfo *ui;
};

#endif //__DlgLasInfo_h_SPARCLE_2019_02_18