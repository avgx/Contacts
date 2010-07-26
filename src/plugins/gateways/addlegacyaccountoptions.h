#ifndef ADDLEGACYACCOUNTOPTIONS_H
#define ADDLEGACYACCOUNTOPTIONS_H

#include <QWidget>
#include <interfaces/igateways.h>
#include <interfaces/ioptionsmanager.h>
#include <utils/action.h>
#include "ui_addlegacyaccountoptions.h"

class AddLegacyAccountOptions : 
			public QWidget,
			public IOptionsWidget
{
	Q_OBJECT;
	Q_INTERFACES(IOptionsWidget);
public:
	AddLegacyAccountOptions(IGateways *AGateways, IServiceDiscovery *ADiscovery, const Jid &AStreamJid, QWidget *AParent=NULL);
	~AddLegacyAccountOptions();
	virtual QWidget* instance() { return this; }
public slots:
	virtual void apply();
	virtual void reset();
signals:
	void modified();
	void childApply();
	void childReset();
protected:
	void createButtons();
	void updateButtons();
protected slots:
	void onGateActionTriggeted(bool);
private:
	Ui::AddLegacyAccountOptionsClass ui;
private:
	IGateways *FGateways;
	IServiceDiscovery *FDiscovery;
private:
	Jid FStreamJid;
	QList<Action *> FGateActions;
};

#endif // ADDLEGACYACCOUNTOPTIONS_H