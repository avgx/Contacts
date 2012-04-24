#ifndef SIPMANAGER_H
#define SIPMANAGER_H

#include <interfaces/ipluginmanager.h>
#include <interfaces/isipphone.h>
#include <interfaces/istanzaprocessor.h>
#include <interfaces/imessagewidgets.h>
#include <interfaces/imessageprocessor.h>
#include <interfaces/iservicediscovery.h>
#include <interfaces/irostersview.h>
#include <interfaces/inotifications.h>
#include <interfaces/ixmppstreams.h>
#include <interfaces/igateways.h>
#include <interfaces/iconnectionmanager.h>
#include <interfaces/idefaultconnection.h>
#include <interfaces/imetacontacts.h>
#include <interfaces/irosterchanger.h>
#include <interfaces/imessagestyles.h>

class TestCallWidget;

class SipManager :
		public QObject,
		public IPlugin,
		public ISipManager,
		public ISipCallHandler,
		public IStanzaHandler
{
	Q_OBJECT
	Q_INTERFACES(IPlugin ISipManager ISipCallHandler IStanzaHandler)
public:
	SipManager();
	~SipManager();
	// IPlugin
	virtual QObject *instance();
	virtual QUuid pluginUuid() const;
	virtual void pluginInfo(IPluginInfo *APluginInfo);
	virtual bool initConnections(IPluginManager *APluginManager, int &AInitOrder);
	virtual bool initObjects();
	virtual bool initSettings();
	virtual bool startPlugin();
	// ISipManager
	virtual bool isCallSupported(const Jid &AStreamJid, const Jid &AContactJid) const;
	// calls
	virtual ISipCall *newCall(const Jid &AStreamJid, const QList<Jid> &ADestinations);
	virtual QList<ISipCall*> findCalls(const Jid &AStreamJid=Jid::null, const Jid &AContactJid=Jid::null, const QString &ASessionId=QString::null) const;
	// SIP registration
	virtual int registeredAccountId(const Jid &AStreamJid) const;
	virtual bool isRegisteredAtServer(const Jid &AStreamJid) const;
	virtual bool registerAtServer(const Jid &AStreamJid);
	virtual bool unregisterAtServer(const Jid &AStreamJid);
	// devices
	virtual void showSystemSoundPreferences() const;
	virtual QList<ISipDevice> availDevices(ISipDevice::Type AType) const;
	virtual ISipDevice getDevice(ISipDevice::Type AType, int ADeviceId) const;
	// handlers
	virtual void insertSipCallHandler(int AOrder, ISipCallHandler * AHandler);
	virtual void removeSipCallHandler(int AOrder, ISipCallHandler * AHandler);
signals:
	void availDevicesChanged(int AType);
	void sipCallCreated(ISipCall *ACall);
	void sipCallDestroyed(ISipCall *ACall);
	void registeredAtServer(const QString &AAccount);
	void unregisteredAtServer(const QString &AAccount);
	void registrationAtServerFailed(const QString &AAccount);
	void sipCallHandlerInserted(int AOrder, ISipCallHandler * AHandler);
	void sipCallHandlerRemoved(int AOrder, ISipCallHandler * AHandler);
public:
	// ISipCallHandler
	virtual bool handleSipCall(int AOrder, ISipCall * ACall);
	// IStanzaHandler
	virtual bool stanzaReadWrite(int AHandleId, const Jid &AStreamJid, Stanza &AStanza, bool &AAccept);
public:
	// SipManager internals
	static SipManager *callbackInstance();
public:
	// pjsip callbacks
	void onRegState(int acc_id);
	void onRegState2(int acc_id, void * /* pjsua_reg_info * */info);
	void onIncomingCall(int acc_id, int call_id, void * /* pjsip_rx_data * */rdata);
protected:
	bool createSipStack();
	void destroySipStack();
	bool handleIncomingCall(const Jid &AStreamJid, const Jid &AContactJid, const QString &ASessionId);
protected slots:
	void onXmppStreamOpened(IXmppStream * stream);
	void onXmppStreamAboutToClose(IXmppStream * stream);
	void onXmppStreamClosed(IXmppStream * stream);
protected slots:
	// for ISipCallHandler
	void onCallStateChanged(int AState);
	void onCallActiveDeviceChanged(int ADeviceType);
	void onCallDeviceStateChanged(ISipDevice::Type AType, ISipDevice::State AState);
	void onCallDevicePropertyChanged(ISipDevice::Type AType, int AProperty, const QVariant & AValue);
protected slots:
	void onStartVideoCall();
	void onShowAddContactDialog();
	void onCallMenuAboutToShow();
	void onCallMenuAboutToHide();
	void onMetaTabWindowCreated(IMetaTabWindow *AWindow);
	void onMetaTabWindowDestroyed(IMetaTabWindow *AWindow);
private:
	IPluginManager *FPluginManager;
	IServiceDiscovery *FDiscovery;
	IStanzaProcessor *FStanzaProcessor;
	IMetaContacts *FMetaContacts;
	IXmppStreams *FXmppStreams;
	IRosterChanger *FRosterChanger;
private:
	int FSHISipQuery;
private:
	bool FSipStackCreated;
	QMap<Jid, int> FAccounts;
	QMap<IMetaTabWindow *, Menu *> FCallMenus;
	QMap<Jid, TestCallWidget*> testCallWidgets;
	QMultiMap<int, ISipCallHandler*> FCallHandlers;
private:
	// for ISipCallHandler
	QList<ISipCall *> handledCalls;
private:
	static SipManager * inst;
};

#endif // SIPMANAGER_H
