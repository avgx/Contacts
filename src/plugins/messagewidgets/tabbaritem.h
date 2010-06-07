#ifndef TABBARITEM_H
#define TABBARITEM_H

#include <QSize>
#include <QLabel>
#include <QFrame>
#include <definations/resources.h>
#include <utils/closebutton.h>
#include <utils/iconstorage.h>

enum TabPageStates
{
	TPS_NORMAL   =0x00
};

class TabBarItem : 
			public QFrame
{
	Q_OBJECT;
public:
	TabBarItem(QWidget *AParent);
	~TabBarItem();
	int state() const;
	void setState(int AState);
	bool active() const;
	void setActive(bool AActive);
	QSize iconSize() const;
	void setIconSize(const QSize &ASize);
	QIcon icon() const;
	void setIcon(const QIcon &AIcon);
	QString iconKey() const;
	void setIconKey(const QString &AIconKey);
	QString text() const;
	void setText(const QString &AText);
	bool isCloseable() const;
	void setCloseable(bool ACloseable);
	void setDragged(bool ADragged);
signals:
	void closeButtonClicked();
protected:
	virtual void enterEvent(QEvent *AEvent);
	virtual void leaveEvent(QEvent *AEvent);
	virtual void paintEvent(QPaintEvent *AEvent);
	virtual bool eventFilter(QObject *AObject, QEvent *AEvent);
private:
	QLabel *FIcon;
	QLabel *FLabel;
	CloseButton *FClose;
private:
	int FState;
	bool FActive;
	bool FDragged;
	QSize FIconSize;
	QString FIconKey;
};

#endif // TABBARITEM_H
