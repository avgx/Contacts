#include "viewhistorywindow.h"

#include <QWebFrame>
#include <QDesktopServices>

ViewHistoryWindow::ViewHistoryWindow(IRoster *ARoster, const Jid &AContactJid, QWidget *AParent) : QMainWindow(AParent)
{
	ui.setupUi(this);
	setAttribute(Qt::WA_DeleteOnClose,true);
	StyleStorage::staticStorage(RSR_STORAGE_STYLESHEETS)->insertAutoStyle(this,STS_RAMBLERHISTORY_VIEWHISTORYWINDOW);

	FRoster = ARoster;
	FContactJid = AContactJid;

	resize(650,500);

	connect(FRoster->instance(),SIGNAL(received(const IRosterItem &, const IRosterItem &)),
		SLOT(onRosterItemReceived(const IRosterItem &, const IRosterItem &)));
	connect(FRoster->instance(),SIGNAL(destroyed(QObject *)),SLOT(deleteLater()));

	IRosterItem ritem = FRoster->rosterItem(AContactJid);
	ritem.itemJid = FContactJid;
	onRosterItemReceived(ritem,ritem);
	
	ui.wbvHistoryView->page()->setLinkDelegationPolicy(QWebPage::DelegateAllLinks);
	connect(ui.wbvHistoryView->page(),SIGNAL(linkClicked(const QUrl &)),SLOT(onWebPageLinkClicked(const QUrl &)));

	initViewHtml();
}

ViewHistoryWindow::~ViewHistoryWindow()
{
	emit windowDestroyed();
}

Jid ViewHistoryWindow::streamJid() const
{
	return FRoster->streamJid();
}

Jid ViewHistoryWindow::contactJid() const
{
	return FContactJid;
}

void ViewHistoryWindow::initViewHtml()
{
	static const QString HtmlTemplate = 
		"<div style=\"display:none\"> \
		  <form method=\"post\" action=\"http://id.rambler.ru/script/auth.cgi?mode=login\" name=\"auth_form\"> \
			  <input type=\"hidden\" name=\"back\" value=\"http://m2.mail-test.rambler.ru/mail/messenger_history.cgi?user=%1\"> \
			  <input type=\"text\" name=\"login\" value=\"%2\"> \
			  <input type=\"text\" name=\"domain\" value=\"%3\"> \
			  <input type=\"password\" name=\"passw\" value=\"%4\"> \
			  <input type=\"text\" name=\"long_session\" value=\"0\"> \
			  <input type=\"submit\" name=\"user.password\" value=\"%5\"> \
		  </form> \
		</div>";

	QString html = HtmlTemplate.arg(contactJid().bare()).arg(streamJid().bare()).arg(streamJid().domain()).arg(FRoster->xmppStream()->password()).arg(tr("Enter"));
	ui.wbvHistoryView->setHtml(html);
	ui.wbvHistoryView->page()->mainFrame()->evaluateJavaScript("document.forms.auth_form.submit()");
}

void ViewHistoryWindow::onWebPageLinkClicked(const QUrl &AUrl)
{
	QDesktopServices::openUrl(AUrl);
}

void ViewHistoryWindow::onRosterItemReceived(const IRosterItem &AItem, const IRosterItem &ABefore)
{
	Q_UNUSED(ABefore);
	if (AItem.itemJid && FContactJid)
	{
		setWindowTitle(tr("Chat history - %1").arg(!AItem.name.isEmpty() ? AItem.name : contactJid().bare()));
		ui.lblCaption->setText(windowTitle());
	}
}
