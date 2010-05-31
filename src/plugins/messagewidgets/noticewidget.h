#ifndef NOTICEWIDGET_H
#define NOTICEWIDGET_H

#include <QTimer>
#include <QWidget>
#include <definations/resources.h>
#include <interfaces/imessagewidgets.h>
#include <utils/actionbutton.h>
#include <utils/iconstorage.h>
#include "ui_noticewidget.h"

class NoticeWidget : 
			public QWidget,
			public INoticeWidget
{
	Q_OBJECT;
	Q_INTERFACES(INoticeWidget);
public:
	NoticeWidget(IMessageWidgets *AMessageWidgets, const Jid &AStreamJid, const Jid &AContactJid);
	~NoticeWidget();
	virtual QWidget *instance() { return this; }
	virtual const Jid &streamJid() const;
	virtual void setStreamJid(const Jid &AStreamJid);
	virtual const Jid &contactJid() const;
	virtual void setContactJid(const Jid &AContactJid);
	virtual int activeNotice() const;
	virtual QList<int> noticeQueue() const;
	virtual INotice noticeById(int ANoticeId) const;
	virtual int insertNotice(const INotice &ANotice);
	virtual void removeNotice(int ANoticeId);
signals:
	void streamJidChanged(const Jid &ABefour);
	void contactJidChanged(const Jid &ABefour);
	void noticeInserted(int ANoticeId);
	void noticeActivated(int ANoticeId);
	void noticeRemoved(int ANoticeId);
protected:
	void updateNotice();
	void updateWidgets(int ANoticeId);
protected slots:
	void onUpdateTimerTimeout();
	void onCloseTimerTimeout();
	void onCloseButtonClicked(bool);
	void onMessageLinkActivated(const QString &ALink);
private:
	Ui::NoticeWidgetClass ui;
private:
	IMessageWidgets *FMessageWidgets;
private:
	Jid FStreamJid;
	Jid FContactJid;
	int FActiveNotice;
	QTimer FUpdateTimer;
	QTimer FCloseTimer;
	QMap<int, INotice> FNotices;
	QMultiMap<int, int> FNoticeQueue;
};

#endif // NOTICEWIDGET_H