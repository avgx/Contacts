#ifndef IRAMBLERHISTORY_H
#define IRAMBLERHISTORY_H

#include <QDateTime>
#include <utils/jid.h>
#include <utils/message.h>

#define RAMBLERHISTORY_UUID "{4460E68D-CC30-41df-A63D-F4F11A894CC7}"

struct IRamblerHistoryRetrieve 
{
	Jid with;
	int count;
	QDateTime before;
};

struct IRamblerHistoryMessages 
{
	Jid with;
	QList<Message> messages;
};

class IRamblerHistory
{
public:
	virtual QObject *instance() = 0;
	virtual QString loadServerMessages(const Jid &AStreamJid, const IRamblerHistoryRetrieve &ARetrieve) =0;
protected:
	virtual void serverMessagesLoaded(const QString &AId, const IRamblerHistoryMessages &AMessages) =0;
	virtual void requestFailed(const QString &AId, const QString &AError) =0;
};

Q_DECLARE_INTERFACE(IRamblerHistory,"Virtus.Plugin.IRamblerHistory/1.0")

#endif // IRAMBLERHISTORY_H