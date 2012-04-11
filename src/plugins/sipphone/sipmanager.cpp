#include "sipmanager.h"

#include "sipcall.h"
#include "frameconverter.h"
#include "pjsipdefines.h"
#include "pjsipcallbacks.h"

#include <definitions/namespaces.h>
#include <definitions/optionwidgetorders.h>
#include <definitions/notificationtypes.h>
#include <utils/log.h>
#include <utils/errorhandler.h>

#include <QProcess>

#if defined(Q_WS_WIN)
# include <windows.h>
#endif

#if defined(DEBUG_ENABLED)
# include <QDebug>
#endif

/* The PJSIP module instance. */
static pjsip_module mod_default_handler =
{
	NULL, NULL,				/* prev, next.		*/
	{ (char*)"mod-default-handler", 19 },	/* Name.		*/
	-1,					/* Id			*/
	PJSIP_MOD_PRIORITY_APPLICATION,		/* Priority	        */
	NULL,					/* load()		*/
	NULL,					/* start()		*/
	NULL,					/* stop()		*/
	NULL,					/* unload()		*/
	NULL,					/* on_rx_request()	*/
	NULL,					/* on_rx_response()	*/
	NULL,					/* on_tx_request.	*/
	NULL,					/* on_tx_response()	*/
	NULL,					/* on_tsx_state()	*/
};

/**************
 * SipManager *
 **************/

#define SHC_SIP_QUERY "/iq[@type='set']/query[@type='request'][@xmlns='" NS_RAMBLER_PHONE "']"

SipManager * SipManager::inst = NULL; // callback instance

SipManager::SipManager() :
	QObject(NULL)
{
	FDiscovery = NULL;
	FMetaContacts = NULL;
	FStanzaProcessor = NULL;

	FSHISipQuery = -1;

#ifdef DEBUG_ENABLED
	qDebug() << "SipManager::SipManager()";
#endif
	inst = this;
#if defined(HAS_VIDEO_SUPPORT) && (HAS_VIDEO_SUPPORT != 0)
	registerFrameCallbacks(myframe);
#endif

}

SipManager::~SipManager()
{
	pjsua_destroy();
}

QObject *SipManager::instance()
{
	return this;
}

QUuid SipManager::pluginUuid() const
{
	return SIPMANAGER_UUID;
}

void SipManager::pluginInfo(IPluginInfo *APluginInfo)
{
	APluginInfo->name = tr("SIP Manager");
	APluginInfo->description = tr("Allows to make voice and video calls over SIP protocol");
	APluginInfo->version = "1.0";
	APluginInfo->author = "Gorshkov V.A.";
	APluginInfo->homePage = "http://contacts.rambler.ru";
	APluginInfo->dependences.append(STANZAPROCESSOR_UUID);
}

bool SipManager::initConnections(IPluginManager *APluginManager, int &AInitOrder)
{
	Q_UNUSED(AInitOrder);

	IPlugin *plugin = APluginManager->pluginInterface("IStanzaProcessor").value(0,NULL);
	if (plugin)
		FStanzaProcessor = qobject_cast<IStanzaProcessor *>(plugin->instance());

	plugin = APluginManager->pluginInterface("IServiceDiscovery").value(0,NULL);
	if (plugin)
		FDiscovery = qobject_cast<IServiceDiscovery *>(plugin->instance());

	plugin = APluginManager->pluginInterface("IMetaContacts").value(0,NULL);
	if (plugin)
	{
		FMetaContacts = qobject_cast<IMetaContacts *>(plugin->instance());
		if(FMetaContacts)
		{
//			connect(FMetaContacts->instance(), SIGNAL(metaTabWindowCreated(IMetaTabWindow*)), SLOT(onMetaTabWindowCreated(IMetaTabWindow*)));
//			connect(FMetaContacts->instance(), SIGNAL(metaTabWindowDestroyed(IMetaTabWindow*)), SLOT(onMetaTabWindowDestroyed(IMetaTabWindow*)));
		}
	}

	plugin = APluginManager->pluginInterface("IRostersViewPlugin").value(0,NULL);
	if (plugin)
	{

		IRostersViewPlugin *rostersViewPlugin = qobject_cast<IRostersViewPlugin *>(plugin->instance());
		if (rostersViewPlugin)
		{
			FRostersView = rostersViewPlugin->rostersView();
//			connect(FRostersView->instance(),SIGNAL(indexContextMenu(IRosterIndex *, QList<IRosterIndex *>, Menu *)),
//				SLOT(onRosterIndexContextMenu(IRosterIndex *, QList<IRosterIndex *>, Menu *)));
//			connect(FRostersView->instance(),SIGNAL(labelToolTips(IRosterIndex *, int, QMultiMap<int,QString> &, ToolBarChanger *)),
//				SLOT(onRosterLabelToolTips(IRosterIndex *, int, QMultiMap<int,QString> &, ToolBarChanger *)));
		}
	}

	plugin = APluginManager->pluginInterface("IXmppStreams").value(0,NULL);
	if (plugin)
	{
//		connect(plugin->instance(), SIGNAL(opened(IXmppStream *)), SLOT(onXmppStreamOpened(IXmppStream *)));
//		connect(plugin->instance(), SIGNAL(aboutToClose(IXmppStream *)), SLOT(onXmppStreamAboutToClose(IXmppStream *)));
//		connect(plugin->instance(), SIGNAL(closed(IXmppStream *)), SLOT(onXmppStreamClosed(IXmppStream *)));
	}

//	connect(this, SIGNAL(streamCreated(const QString&)), this, SLOT(onStreamCreated(const QString&)));

	return FStanzaProcessor!=NULL;
}

bool SipManager::initObjects()
{
	if (FDiscovery)
	{
		IDiscoFeature sipPhone;
		sipPhone.active = true;
		sipPhone.var = NS_RAMBLER_PHONE;
		sipPhone.name = tr("SIP Phone 2");
		sipPhone.description = tr("SIP voice and video calls");
		FDiscovery->insertDiscoFeature(sipPhone);
	}
	if (FStanzaProcessor)
	{
		IStanzaHandle shandle;
		shandle.handler = this;
		shandle.order = SHO_DEFAULT;
		shandle.direction = IStanzaHandle::DirectionIn;
		shandle.conditions.append(SHC_SIP_QUERY);
		FSHISipQuery = FStanzaProcessor->insertStanzaHandle(shandle);
	}
	return true;
}

bool SipManager::initSettings()
{
	return true;
}

bool SipManager::startPlugin()
{
	return true;
}

bool SipManager::isCallSupported(const Jid &AStreamJid, const Jid &AContactJid) const
{
	return FDiscovery && FDiscovery->discoInfo(AStreamJid, AContactJid).features.contains(NS_RAMBLER_PHONE);
}

ISipCall *SipManager::newCall(const Jid &AStreamJid, const QList<Jid> &AContacts)
{
	SipCall *call = new SipCall(this,FStanzaProcessor,AStreamJid,AContacts,QUuid::createUuid().toString());
	connect(call, SIGNAL(destroyed(QObject*)), SLOT(onCallDestroyed(QObject*)));
	calls << call;
	emit sipCallCreated(call);
	return call;
}

QList<ISipCall*> SipManager::findCalls(const Jid &AStreamJid, const Jid &AContactJid, const QString &ASessionId) const
{
	QList<ISipCall*> found;
	foreach (ISipCall *call, calls)
	{
		if (AStreamJid.isEmpty() || call->streamJid()==AStreamJid)
		{
			if (AContactJid.isEmpty() || call->contactJid()==AContactJid)
			{
				if (ASessionId.isEmpty() || call->sessionId()==ASessionId)
				{
					found << call;
				}
			}
		}
	}
	return found;
}

bool SipManager::isRegisteredAtServer(const Jid &AStreamJid) const
{
	return accountIds.value(AStreamJid, -1) != -1;
}

bool SipManager::registerAtServer(const Jid &AStreamJid, const QString &APassword)
{
	// TODO: check implementation
	return initStack(SIP_DOMAIN, SIP_PORT, AStreamJid, APassword);
}

bool SipManager::unregisterAtServer(const Jid &AStreamJid)
{
	// TODO: check implementation
	setRegistration(AStreamJid, false);
	return false;
}

QList<ISipDevice> SipManager::availDevices(ISipDevice::Type AType) const
{
	Q_UNUSED(AType)
	// TODO: implementation
	return QList<ISipDevice>();
}

ISipDevice SipManager::getDevice(ISipDevice::Type AType, int ADeviceId) const
{
	Q_UNUSED(AType)
	Q_UNUSED(ADeviceId)
	// TODO: implementation
	return ISipDevice();
}

void SipManager::showSystemSoundPreferences() const
{
#if defined(Q_WS_WIN)
	OSVERSIONINFO m_osinfo;
	ZeroMemory(&m_osinfo, sizeof(m_osinfo));
	m_osinfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
	if (GetVersionEx((LPOSVERSIONINFO) &m_osinfo))
	{
		if(m_osinfo.dwMajorVersion < 6)
		{
			QProcess::startDetached("sndvol32.exe");
		}
		else
		{
			QProcess::startDetached("sndvol.exe");
		}
	}
#elif defined (Q_WS_MAC)
	QProcess::startDetached("open -W \"/System/Library/PreferencePanes/Sound.prefPane\"");
#endif
}

void SipManager::insertSipCallHandler(int AOrder, ISipCallHandler *AHandler)
{
	handlers.insert(AOrder, AHandler);
}

void SipManager::removeSipCallHandler(int AOrder, ISipCallHandler *AHandler)
{
	if (handlers.value(AOrder, NULL) == AHandler)
		handlers.remove(AOrder);
}

bool SipManager::stanzaReadWrite(int AHandleId, const Jid &AStreamJid, Stanza &AStanza, bool &AAccept)
{
	if (FSHISipQuery == AHandleId)
	{
		AAccept = true;
		QDomElement queryElem = AStanza.firstElement("query",NS_RAMBLER_PHONE);
		QString type = queryElem.attribute("type");
		if (type == "request")
		{
			QString sessionId = queryElem.attribute("sid");
			if (sessionId.isEmpty())
			{
				LogError(QString("[SipManager]: Invalid incoming call request from %1 to %2").arg(AStanza.from(), AStreamJid.full()));
				ErrorHandler err(ErrorHandler::BAD_REQUEST);
				Stanza error = FStanzaProcessor->makeReplyError(AStanza,err);
				FStanzaProcessor->sendStanzaOut(AStreamJid,error);
			}
			else if (!findCalls(Jid::null,Jid::null,sessionId).isEmpty())
			{
				LogError(QString("[SipManager]: Duplicated sessionID in incoming call request from %1 to %2").arg(AStanza.from(), AStreamJid.full()));
				ErrorHandler err(ErrorHandler::CONFLICT);
				Stanza error = FStanzaProcessor->makeReplyError(AStanza,err);
				FStanzaProcessor->sendStanzaOut(AStreamJid,error);
			}
			else if (!findCalls(Jid::null,AStanza.from()).isEmpty())
			{
				LogError(QString("[SipManager]: Duplicated incoming call request from %1 to %2").arg(AStanza.from(), AStreamJid.full()));
				ErrorHandler err(ErrorHandler::FORBIDDEN);
				Stanza error = FStanzaProcessor->makeReplyError(AStanza,err);
				FStanzaProcessor->sendStanzaOut(AStreamJid,error);
			}
			else if (!handleIncomingCall(AStreamJid, AStanza.from(), sessionId))
			{
				LogError(QString("[SipManager]: Failed to handle incoming call request from %1 to %2").arg(AStanza.from(), AStreamJid.full()));
				ErrorHandler err(ErrorHandler::UNEXPECTED_REQUEST);
				Stanza error = FStanzaProcessor->makeReplyError(AStanza,err);
				FStanzaProcessor->sendStanzaOut(AStreamJid,error);
			}
			else
			{
				LogDetail(QString("[SipManager]: Incoming call from %1 to %2").arg(AStanza.from(), AStreamJid.full()));
				Stanza result = FStanzaProcessor->makeReplyResult(AStanza);
				FStanzaProcessor->sendStanzaOut(AStreamJid,result);
			}
		}
	}
	return false;
}

bool SipManager::handleIncomingCall(const Jid &AStreamJid, const Jid &AContactJid, const QString &ASessionId)
{
	// TODO: check availability of answering the call (busy) <- handler should do this
	SipCall *call = new SipCall(this,FStanzaProcessor,AStreamJid,AContactJid,ASessionId);
	emit sipCallCreated(call);

	bool handled = false;
	foreach (ISipCallHandler * handler, handlers.values())
	{
		if ((handled = handler->checkCall(call)))
			break;
	}
	
	if (!handled)
	{
		call->rejectCall(ISipCall::RC_NOHANDLER);
		call->deleteLater();
	}

	return handled;
}

bool SipManager::initStack(const QString &ASipServer, int ASipPort, const Jid &ASipUser, const QString &ASipPassword)
{
	// TODO: new implementation
	pj_status_t status;

	status = pjsua_create();
	if (status != PJ_SUCCESS)
	{
		LogError(QString("[SipManager::initStack]: Failed to create pjsua! pjsua_crete() returned %1").arg(status));
		return false;
	}

	pjsua_config ua_cfg;
	pjsua_config_default(&ua_cfg);
	pjsua_callback ua_cb;
	pj_bzero(&ua_cb, sizeof(ua_cb));

	registerCallbacks(ua_cfg.cb);

	ua_cfg.outbound_proxy_cnt = 1;

	char proxyTmp[512];
	pj_ansi_snprintf(proxyTmp, sizeof(proxyTmp), "sip:%s", ASipServer.toAscii().constData());
	ua_cfg.outbound_proxy[0] = pj_str((char*)proxyTmp);

	pjsua_logging_config log_cfg;
	pjsua_logging_config_default(&log_cfg);
	log_cfg.log_filename = pj_str((char*)"pjsip.log");

	pjsua_media_config med_cfg;
	pjsua_media_config_default(&med_cfg);
	med_cfg.thread_cnt = 1;

	status = pjsua_init(&ua_cfg, &log_cfg, &med_cfg);
	if (status == PJ_SUCCESS)
	{
		pjsua_transport_config udp_cfg;
		pjsua_transport_id udp_id;
		pjsua_transport_config_default(&udp_cfg);
		udp_cfg.port = ASipPort;

		status = pjsua_transport_create(PJSIP_TRANSPORT_UDP, &udp_cfg, &udp_id);
		if (status == PJ_SUCCESS)
		{
			pjsua_transport_info udp_info;
			status = pjsua_transport_get_info(udp_id, &udp_info);
			if (status == PJ_SUCCESS)
			{
				// Create account
				pjsua_acc_config acc_cfg;
				pjsua_acc_config_default(&acc_cfg);


				char idtmp[1024];
				QString idString = ASipUser.pBare();
				pj_ansi_snprintf(idtmp, sizeof(idtmp), "sip:%s", idString.toAscii().constData());

				acc_cfg.id = pj_str((char*)idtmp);

				char reg_uritmp[1024];
				pj_ansi_snprintf(reg_uritmp, sizeof(reg_uritmp), "sip:%s", ASipUser.pDomain().toAscii().constData());
				acc_cfg.reg_uri = pj_str((char*)reg_uritmp);

				acc_cfg.cred_count = 1;
				acc_cfg.cred_info[0].realm = pj_str((char*)"*");
				acc_cfg.cred_info[0].scheme = pj_str((char*)"digest");

				char usernametmp[512];
				pj_ansi_snprintf(usernametmp, sizeof(usernametmp), "%s", idString.toAscii().constData());
				acc_cfg.cred_info[0].username = pj_str((char*)usernametmp);

				char passwordtmp[512];
				pj_ansi_strncpy(passwordtmp, ASipPassword.toAscii().constData(), sizeof(passwordtmp));
				acc_cfg.cred_info[0].data = pj_str((char*)passwordtmp);


				acc_cfg.vid_cap_dev = DEFAULT_CAP_DEV;
				acc_cfg.vid_rend_dev = DEFAULT_REND_DEV;
				acc_cfg.vid_in_auto_show = PJ_TRUE;
				acc_cfg.vid_out_auto_transmit = PJ_TRUE;
				acc_cfg.register_on_acc_add = PJ_FALSE;

				pjsua_acc_id acc;

				status = pjsua_acc_add(&acc_cfg, PJ_TRUE, &acc);
				if (status == PJ_SUCCESS)
				{
					accountIds.insert(ASipUser, acc);
					status = pjsua_start();
					if (status != PJ_SUCCESS)
					{
						if (pjsua_get_pjsip_endpt())
						{
							registerModuleCallbacks(mod_default_handler);
							pjsip_endpt_register_module(pjsua_get_pjsip_endpt(), &mod_default_handler);
							return true;
						} // pjsua_get_pjsip_endpt
						else
						{
							LogError("[SipManager::initStack]: pjsua_get_pjsip_endpt() invalid!");
						}
					} // pjsua_start
					else
					{
						LogError(QString("[SipManager::initStack]: pjsua_start() failed with status %1!").arg(status));
					}
				} // pjsua_acc_add
				else
				{
					LogError(QString("[SipManager::initStack]: pjsua_acc_add() failed with status %1!").arg(status));
				}
			} // pjsua_transport_get_info
			else
			{
				LogError(QString("[SipManager::initStack]: pjsua_transport_get_info() failed with status %1!").arg(status));
			}
		} // pjsua_transport_create
		else
		{
			LogError(QString("[SipManager::initStack]: pjsua_transport_create() failed with status %1!").arg(status));
		}
	} // pjsua_init
	else
	{
		LogError(QString("[SipManager::initStack]: pjsua_init() failed with status %1!").arg(status));
	}

	LogError("[SipManager::initStack]: calling pjsua_destroy()...");

	pjsua_destroy();
	return false;
}

void SipManager::setRegistration(const Jid & AStreamJid, bool ARenew)
{
	int acc = accountIds.value(AStreamJid, -1);
	if (acc != -1)
		pjsua_acc_set_registration(acc, ARenew);
	else
		LogError(QString("[SipManager::setRegistration]: Invalid stream jid - %1").arg(AStreamJid.full()));
}

void SipManager::onCallDestroyed(QObject * object)
{
	ISipCall * call = qobject_cast<ISipCall*>(object);
	if (call)
	{
		calls.removeAll(call);
		emit sipCallDestroyed(call);
	}
}

SipManager *SipManager::callbackInstance()
{
	return inst;
}

void SipManager::onRegState(int acc_id)
{
	Q_UNUSED(acc_id)
	// TODO: new implementation
	pjsua_acc_info info;

	pjsua_acc_get_info(acc_id, &info);

	accRegistered = (info.status == PJSIP_SC_OK);
	QString accountId = QString("%1").arg(info.acc_uri.ptr);
	if (accRegistered)
		emit registeredAtServer(accountId);
	else
		emit failedToRegisterAtServer(accountId);
}

void SipManager::onRegState2(int acc_id, void *info)
{
	Q_UNUSED(acc_id)
	// TODO: check this MAGIC implementation
	int i;
	i = ((pjsua_reg_info*)info)->cbparam->code;
	i++;
}

void SipManager::onIncomingCall(int acc_id, int call_id, void *rdata)
{
	Q_UNUSED(acc_id)
	Q_UNUSED(rdata)
	// TODO: chaeck implementation
	pjsua_call_info ci;
	pjsua_call_get_info(call_id, &ci);
	QString callerId = QString("%s").arg(ci.remote_info.ptr);
	QString receiverId = QString("%s").arg(ci.local_info.ptr);
	ISipCall *call = findCalls(receiverId,callerId).value(0); // TODO: ensure receiverId and callerId is full Jid
	if (call && call->role()==ISipCall::CR_RESPONDER && call->state()==ISipCall::CS_CONNECTING)
	{
		// TODO: accept call here and send call_id to call
	}
	else
	{
		// TODO: decline call here while we do not support direct incoming calls
	}

	//if (SipCall::activeCallForId(call_id))
	//{
	//	// busy
	//	pjsua_call_answer(call_id, PJSIP_SC_BUSY_HERE, NULL, NULL);
	//	return;
	//}
	//pjsua_call_info ci;
	//pjsua_call_get_info(call_id, &ci);
	//QString callerId = QString("%s").arg(ci.remote_info.ptr);
	//QString receiverId = QString("%s").arg(ci.local_info.ptr);
	//handleIncomingCall(receiverId, callerId, QUuid::createUuid().toString());
}

Q_EXPORT_PLUGIN2(plg_sipmanager, SipManager)
