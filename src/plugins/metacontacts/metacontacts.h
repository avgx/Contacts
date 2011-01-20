#ifndef METACONTACTS_H
#define METACONTACTS_H

#include <QMultiMap>
#include <QObjectCleanupHandler>
#include <definitions/resources.h>
#include <definitions/menuicons.h>
#include <definitions/actiongroups.h>
#include <definitions/rosterproxyorders.h>
#include <definitions/rosterindextyperole.h>
#include <definitions/rosterclickhookerorders.h>
#include <definitions/rosterdragdropmimetypes.h>
#include <interfaces/ipluginmanager.h>
#include <interfaces/imetacontacts.h>
#include <interfaces/istanzaprocessor.h>
#include <interfaces/irostersview.h>
#include <interfaces/imessagewidgets.h>
#include <interfaces/imessageprocessor.h>
#include <utils/widgetmanager.h>
#include "metaroster.h"
#include "metaproxymodel.h"
#include "metatabwindow.h"
#include "mergecontactsdialog.h"

class GroupMenu : 
	public Menu
{
	Q_OBJECT;
public:
	GroupMenu(QWidget* AParent = NULL) : Menu(AParent) { }
	virtual ~GroupMenu() {}
protected:
	void mouseReleaseEvent(QMouseEvent *AEvent);
};

class MetaContacts : 
	public QObject,
	public IPlugin,
	public IMetaContacts,
	public IRostersClickHooker,
	public IRostersDragDropHandler
{
	Q_OBJECT;
	Q_INTERFACES(IPlugin IMetaContacts IRostersClickHooker IRostersDragDropHandler);
public:
	MetaContacts();
	~MetaContacts();
	virtual QObject* instance() { return this; }
	//IPlugin
	virtual QUuid pluginUuid() const { return METACONTACTS_UUID; }
	virtual void pluginInfo(IPluginInfo *APluginInfo);
	virtual bool initConnections(IPluginManager *APluginManager, int &AInitOrder);
	virtual bool initObjects();
	virtual bool initSettings() { return true; }
	virtual bool startPlugin() { return true; }
	//IRostersClickHooker
	virtual bool rosterIndexClicked(IRosterIndex *AIndex, int AOrder);
	//IRostersDragDropHandler
	virtual Qt::DropActions rosterDragStart(const QMouseEvent *AEvent, const QModelIndex &AIndex, QDrag *ADrag);
	virtual bool rosterDragEnter(const QDragEnterEvent *AEvent);
	virtual bool rosterDragMove(const QDragMoveEvent *AEvent, const QModelIndex &AHover);
	virtual void rosterDragLeave(const QDragLeaveEvent *AEvent);
	virtual bool rosterDropAction(const QDropEvent *AEvent, const QModelIndex &AIndex, Menu *AMenu);
	//IMetaContacts
	virtual IMetaRoster *newMetaRoster(IRoster *ARoster);
	virtual IMetaRoster *findMetaRoster(const Jid &AStreamJid) const;
	virtual void removeMetaRoster(IRoster *ARoster);
	virtual QString metaRosterFileName(const Jid &AStreamJid) const;
	virtual QList<IMetaTabWindow *> metaTabWindows() const;
	virtual IMetaTabWindow *newMetaTabWindow(const Jid &AStreamJid, const Jid &AMetaId);
	virtual IMetaTabWindow *findMetaTabWindow(const Jid &AStreamJid, const Jid &AMetaId) const;
signals:
	void metaRosterAdded(IMetaRoster *AMetaRoster);
	void metaRosterOpened(IMetaRoster *AMetaRoster);
	void metaContactReceived(IMetaRoster *AMetaRoster, const IMetaContact &AContact, const IMetaContact &ABefore);
	void metaActionResult(IMetaRoster *AMetaRoster, const QString &AActionId, const QString &AErrCond, const QString &AErrMessage);
	void metaRosterClosed(IMetaRoster *AMetaRoster);
	void metaRosterEnabled(IMetaRoster *AMetaRoster, bool AEnabled);
	void metaRosterStreamJidAboutToBeChanged(IMetaRoster *AMetaRoster, const Jid &AAfter);
	void metaRosterStreamJidChanged(IMetaRoster *AMetaRoster, const Jid &ABefore);
	void metaRosterRemoved(IMetaRoster *AMetaRoster);
	void metaTabWindowCreated(IMetaTabWindow *AWindow);
	void metaTabWindowDestroyed(IMetaTabWindow *AWindow);
protected:
	void deleteMetaRosterWindows(IMetaRoster *AMetaRoster);
protected slots:
	void onMetaRosterOpened();
	void onMetaContactReceived(const IMetaContact &AContact, const IMetaContact &ABefore);
	void onMetaActionResult(const QString &AActionId, const QString &AErrCond, const QString &AErrMessage);
	void onMetaRosterClosed();
	void onMetaRosterEnabled(bool AEnabled);
	void onMetaRosterStreamJidAboutToBeChanged(const Jid &AAfter);
	void onMetaRosterStreamJidChanged(const Jid &ABefour);
	void onMetaRosterDestroyed(QObject *AObject);
	void onRosterAdded(IRoster *ARoster);
	void onRosterRemoved(IRoster *ARoster);
protected slots:
	void onMetaTabWindowItemPageRequested(const Jid &AItemJid);
	void onMetaTabWindowDestroyed();
protected slots:
	void onLoadMetaRosters();
	void onChatWindowCreated(IChatWindow *AWindow);
protected slots:
	void onRenameContact(bool);
	void onDeleteContact(bool);
	void onMergeContacts(bool);
	void onReleaseContactItems(bool);
	void onChangeContactGroups(bool AChecked);
protected slots:
	void onRosterIndexContextMenu(IRosterIndex *AIndex, Menu *AMenu);
private:
	IPluginManager *FPluginManager;
	IRosterPlugin *FRosterPlugin;
	IRostersViewPlugin *FRostersViewPlugin;
	IStanzaProcessor *FStanzaProcessor;
	IMessageWidgets *FMessageWidgets;
	IMessageProcessor *FMessageProcessor;
private:
	QList<IMetaRoster *> FLoadQueue;
	QList<IMetaRoster *> FMetaRosters;
	QObjectCleanupHandler FCleanupHandler;
private:
	QList<IMetaTabWindow *> FMetaTabWindows;
};

#endif // METACONTACTS_H
