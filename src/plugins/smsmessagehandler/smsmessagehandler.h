#ifndef SMSMESSAGEHANDLER_H
#define SMSMESSAGEHANDLER_H

#include <QTimer>
#include <definitions/namespaces.h>
#include <definitions/resources.h>
#include <definitions/menuicons.h>
#include <definitions/stylesheets.h>
#include <definitions/soundfiles.h>
#include <definitions/stanzahandlerorders.h>
#include <definitions/chatwindowwidgetorders.h>
#include <definitions/messagedataroles.h>
#include <definitions/messagehandlerorders.h>
#include <definitions/notificators.h>
#include <definitions/notificationdataroles.h>
#include <definitions/rosternotifyorders.h>
#include <definitions/tabpagenotifypriorities.h>
#include <interfaces/ipluginmanager.h>
#include <interfaces/ismsmessagehandler.h>
#include <interfaces/imessageprocessor.h>
#include <interfaces/imessagewidgets.h>
#include <interfaces/imessagestyles.h>
#include <interfaces/iramblerhistory.h>
#include <interfaces/iservicediscovery.h>
#include <interfaces/istatusicons.h>
#include <interfaces/istatuschanger.h>
#include <interfaces/irostersview.h>
#include <interfaces/ixmppstreams.h>
#include <interfaces/istanzaprocessor.h>
#include <utils/stanza.h>
#include "smsinfowidget.h"

struct WindowStatus
{
	QDateTime createTime;
	QString historyId;
	QDateTime historyTime;
	QUuid historyRequestId;
	QString lastStatusShow;
	QList<QDate> separators;
	QList<int> notified;
	QList<Message> unread;
	QList<Message> offline;
};

struct StyleExtension
{
	StyleExtension() {
		action = IMessageContentOptions::InsertAfter;
		extensions = 0;
	}
	int action;
	int extensions;
	QString contentId;
};

enum HisloryLoadState {
	HLS_READY,
	HLS_WAITING,
	HLS_FINISHED
};

class SmsMessageHandler :
	public QObject,
	public IPlugin,
	public ISmsMessageHandler,
	public IMessageHandler,
	public IStanzaHandler,
	public IStanzaRequestOwner
{
	Q_OBJECT;
	Q_INTERFACES(IPlugin ISmsMessageHandler IMessageHandler IStanzaHandler IStanzaRequestOwner);
public:
	SmsMessageHandler();
	~SmsMessageHandler();
	//IPlugin
	virtual QObject *instance() { return this; }
	virtual QUuid pluginUuid() const { return SMSMESSAGEHANDLER_UUID; }
	virtual void pluginInfo(IPluginInfo *APluginInfo);
	virtual bool initConnections(IPluginManager *APluginManager, int &AInitOrder);
	virtual bool initObjects();
	virtual bool initSettings() { return true; }
	virtual bool startPlugin() { return true; }
	//IStanzaHandler
	virtual bool stanzaReadWrite(int AHandleId, const Jid &AStreamJid, Stanza &AStanza, bool &AAccept);
	//IStanzaRequestOwner
	virtual void stanzaRequestResult(const Jid &AStreamJid, const Stanza &AStanza);
	virtual void stanzaRequestTimeout(const Jid &AStreamJid, const QString &AStanzaId);
	//IMessageHandler
	virtual bool checkMessage(int AOrder, const Message &AMessage);
	virtual bool showMessage(int AMessageId);
	virtual bool receiveMessage(int AMessageId);
	virtual INotification notification(INotifications *ANotifications, const Message &AMessage);
	virtual bool createWindow(int AOrder, const Jid &AStreamJid, const Jid &AContactJid, Message::MessageType AType, int AShowMode);
	//SmsMessageHandler
	virtual bool isSmsContact(const Jid &AStreamJid, const Jid &AContactJid) const;
	virtual int smsBalance(const Jid &AStreamJid, const Jid &AServiceJid) const;
	virtual bool requestSmsBalance(const Jid &AStreamJid, const Jid &AServiceJid);
signals:
	void smsBalanceChanged(const Jid &AStreamJid, const Jid &AServiceJid, int ABalance);
protected:
	int smsBalanceFromStanza(const Stanza &AStanza) const;
	void setSmsBalance(const Jid &AStreamJid, const Jid &AServiceJid, int ABalance);
protected:
	IChatWindow *getWindow(const Jid &AStreamJid, const Jid &AContactJid);
	IChatWindow *findWindow(const Jid &AStreamJid, const Jid &AContactJid);
	void updateWindow(IChatWindow *AWindow);
	void removeMessageNotifications(IChatWindow *AWindow);
	void replaceUnreadMessages(IChatWindow *AWindow);
protected:
	void requestHistoryMessages(IChatWindow *AWindow, int ACount);
	void showHistoryLinks(IChatWindow *AWindow, HisloryLoadState AState, bool AInit = false);
protected:
	void setMessageStyle(IChatWindow *AWindow);
	void fillContentOptions(IChatWindow *AWindow, IMessageContentOptions &AOptions) const;
	QUuid showDateSeparator(IChatWindow *AWindow, const QDate &ADate);
	QUuid showStyledStatus(IChatWindow *AWindow, const QString &AMessage);
	QUuid showStyledMessage(IChatWindow *AWindow, const Message &AMessage, const StyleExtension &AExtension = StyleExtension());
protected:
	virtual bool eventFilter(QObject *AObject, QEvent *AEvent);
protected slots:
	void onMessageReady();
	void onUrlClicked(const QUrl &AUrl);
	void onWindowActivated();
	void onWindowClosed();
	void onWindowDestroyed();
	void onStatusIconsChanged();
	void onRamblerHistoryMessagesLoaded(const QString &AId, const IRamblerHistoryMessages &AMessages);
	void onRamblerHistoryRequestFailed(const QString &AId, const QString &AError);
	void onStyleOptionsChanged(const IMessageStyleOptions &AOptions, int AMessageType, const QString &AContext);
	void onXmppStreamOpened(IXmppStream *AXmppStream);
	void onXmppStreamClosed(IXmppStream *AXmppStream);
private:
	IMessageStyles *FMessageStyles;
	IMessageWidgets *FMessageWidgets;
	IMessageProcessor *FMessageProcessor;
	IRamblerHistory *FRamblerHistory;
	IXmppStreams *FXmppStreams;
	IServiceDiscovery *FDiscovery;
	IStatusIcons *FStatusIcons;
	IStatusChanger *FStatusChanger;
	IStanzaProcessor *FStanzaProcessor;
private:
	QList<IChatWindow *> FWindows;
	QMap<IChatWindow *, QTimer *> FWindowTimers;
	QMap<IChatWindow *, WindowStatus> FWindowStatus;
private:
	QMap<QString, IChatWindow *> FHistoryRequests;
private:
	QMap<Jid, int> FSHISmsBalance;
	QMap<QString, Jid> FSmsBalanceRequests;
	QMap<Jid, QMap<Jid, int> > FSmsBalance;
};

#endif // SMSMESSAGEHANDLER_H
