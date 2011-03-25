#include "edititemwidget.h"

#include <QVBoxLayout>

#define RESOLVE_WAIT_INTERVAL    1500

EditItemWidget::EditItemWidget(IGateways *AGateways, const Jid &AStreamJid, const IGateServiceDescriptor &ADescriptor, QWidget *AParent) : QWidget(AParent)
{
	ui.setupUi(this);
	setFocusProxy(ui.lneContact);

	FGateways = AGateways;

	FContactTextChanged = false;
	FStreamJid = AStreamJid;
	FDescriptor = ADescriptor;

	ui.wdtProfiles->setLayout(new QVBoxLayout);
	ui.wdtProfiles->layout()->setMargin(0);

	FResolveTimer.setSingleShot(true);
	connect(&FResolveTimer,SIGNAL(timeout()),SLOT(resolveContactJid()));

	connect(ui.lneContact,SIGNAL(editingFinished()),SLOT(onContactTextEditingFinished()));
	connect(ui.lneContact,SIGNAL(textEdited(const QString &)),SLOT(onContactTextEdited(const QString &)));

	IconStorage::staticStorage(RSR_STORAGE_MENUICONS)->insertAutoIcon(ui.lblIcon,ADescriptor.iconKey,0,0,"pixmap");
	ui.lneContact->setPlaceholderText(tr("Address in %1").arg(ADescriptor.name));
	connect(ui.cbtDelete,SIGNAL(clicked()),SIGNAL(deleteButtonClicked()));

	connect(FGateways->instance(),SIGNAL(loginReceived(const QString &, const QString &)),SLOT(onServiceLoginReceived(const QString &, const QString &)));
	connect(FGateways->instance(),SIGNAL(userJidReceived(const QString &, const Jid &)),SLOT(onLegacyContactJidReceived(const QString &, const Jid &)));
	connect(FGateways->instance(),SIGNAL(serviceEnableChanged(const Jid &, const Jid &, bool)),SLOT(onServiceEnableChanged(const Jid &, const Jid &, bool)));
	connect(FGateways->instance(),SIGNAL(errorReceived(const QString &, const QString &)),SLOT(onGatewayErrorReceived(const QString &, const QString &)));

	updateProfiles();
	setErrorMessage(QString::null);
}

EditItemWidget::~EditItemWidget()
{

}

Jid EditItemWidget::contactJid() const
{
	return FContactJid;
}

void EditItemWidget::setContactJid(const Jid &AContactJid)
{
	if (FContactJid != AContactJid.bare())
	{
		QString contact = AContactJid.bare();
		Jid serviceJid = AContactJid.domain();
		if (FGateways->streamServices(FStreamJid).contains(serviceJid))
		{
			contact = FGateways->legacyIdFromUserJid(AContactJid);
			if (FProfiles.contains(serviceJid))
				FProfiles[serviceJid]->setChecked(true);
		}
		setContactText(contact);
		startResolve(0);
	}
}

QString EditItemWidget::contactText() const
{
	return ui.lneContact->text();
}

void EditItemWidget::setContactText(const QString &AText)
{
	ui.lneContact->setText(AText);
	onContactTextEdited(AText);
}

Jid EditItemWidget::gatewayJid() const
{
	return selectedProfile();
}

void EditItemWidget::setGatewayJid(const Jid &AGatewayJid) 
{
	setSelectedProfile(AGatewayJid);
}

IGateServiceDescriptor EditItemWidget::gateDescriptor() const
{
	return FDescriptor;
}

void EditItemWidget::updateProfiles()
{
	IDiscoIdentity identity;
	identity.category = "gateway";
	identity.type = FDescriptor.type;
	
	QList<Jid> enabledGates;
	if (FDescriptor.needLogin)
	{
		foreach(Jid gateJid, FGateways->streamServices(FStreamJid,identity))
			if (FGateways->isServiceEnabled(FStreamJid,gateJid))
				enabledGates.append(gateJid);
	}
	else
	{
		enabledGates = FGateways->availServices(FStreamJid,identity);
	}

	QList<Jid> newProfiles = (enabledGates.toSet() - FProfiles.keys().toSet()).toList();
	QList<Jid> oldProfiles = (FProfiles.keys().toSet() - enabledGates.toSet()).toList();

	qSort(newProfiles);
	if (!FDescriptor.needGate && !FProfiles.contains(FStreamJid))
		newProfiles.prepend(FStreamJid);
	else if (FDescriptor.needGate && FProfiles.contains(FStreamJid))
		oldProfiles.prepend(FStreamJid);

	foreach(Jid serviceJid, newProfiles)
	{
		QRadioButton *button = new QRadioButton(ui.wdtProfiles);
		button->setText(serviceJid.pBare());
		button->setAutoExclusive(true);
		connect(button,SIGNAL(clicked(bool)),SLOT(onProfileButtonClicked(bool)));
		FProfiles.insert(serviceJid,button);
		ui.wdtProfiles->layout()->addWidget(button);
	}

	foreach(Jid serviceJid, newProfiles)
	{
		if (serviceJid != FStreamJid)
		{
			QString requestId = FGateways->sendLoginRequest(FStreamJid,serviceJid);
			if (!requestId.isEmpty())
				FLoginRequests.insert(requestId,serviceJid);
		}
	}

	foreach(Jid serviceJid, oldProfiles)
	{
		QRadioButton *button = FProfiles.take(serviceJid);
		ui.wdtProfiles->layout()->removeWidget(button);
		delete button;
	}

	if (selectedProfile().isEmpty() && !FProfiles.isEmpty())
	{
		if (!FDescriptor.needGate)
			setSelectedProfile(FStreamJid);
		else
			setSelectedProfile(FProfiles.keys().value(0));
	}

	ui.wdtSelectProfile->setVisible(FProfiles.count() > 1);

	emit adjustSizeRequired();
}

Jid EditItemWidget::selectedProfile() const
{
	for (QMap<Jid, QRadioButton *>::const_iterator it = FProfiles.constBegin(); it!=FProfiles.constEnd(); it++)
		if (it.value()->isChecked())
			return it.key();
	return Jid::null;
}

void EditItemWidget::setSelectedProfile(const Jid &AServiceJid)
{
	if (FProfiles.contains(AServiceJid))
	{
		FProfiles.value(AServiceJid)->setChecked(true);
		startResolve(0);
	}
}

QString EditItemWidget::normalContactText(const QString &AText) const
{
	return AText.trimmed().toLower();
}

void EditItemWidget::startResolve(int ATimeout)
{
	setRealContactJid(Jid::null);
	setErrorMessage(QString::null);
	FResolveTimer.start(ATimeout);
}

void EditItemWidget::setRealContactJid(const Jid &AContactJid)
{
	if (FContactJid != AContactJid.bare())
	{
		FContactJid = AContactJid.bare();
		emit contactJidChanged(AContactJid);
	}
}

void EditItemWidget::setErrorMessage(const QString &AMessage)
{
	if (AMessage.isEmpty() && ui.lblError->isVisible())
		QTimer::singleShot(1,this,SIGNAL(adjustSizeRequired()));
	else if (!AMessage.isEmpty() && !ui.lblError->isVisible())
		QTimer::singleShot(1,this,SIGNAL(adjustSizeRequired()));

	ui.lblError->setText(AMessage);
	ui.lblError->setVisible(!AMessage.isEmpty());
}

void EditItemWidget::resolveContactJid()
{
	QString errMessage;

	QString contact = normalContactText(contactText());
	if (ui.lneContact->text() != contact)
		ui.lneContact->setText(contact);
	FContactTextChanged = false;

	if (!contact.isEmpty())
	{
		Jid userJid = contact;
		Jid serviceJid = selectedProfile();

		QRegExp availRegExp(FDescriptor.homeContactPattern);
		availRegExp.setCaseSensitivity(Qt::CaseInsensitive);

		if (!contact.contains(availRegExp))
		{
			errMessage = tr("Invalid address. Please check the address and try again.");
		}
		else if (serviceJid != FStreamJid)
		{
			FContactJidRequest = FGateways->sendUserJidRequest(FStreamJid,serviceJid,contact);
			if (FContactJidRequest.isEmpty())
				errMessage = tr("Unable to determine the contact ID");
		}
		else if (!userJid.isValid() || userJid.node().isEmpty())
		{
			errMessage = tr("Invalid address. Please check the address and try again.");
		}
		else
		{
				setRealContactJid(userJid);
		}
	}

	setErrorMessage(errMessage);
}

void EditItemWidget::onContactTextEditingFinished()
{
	if (FContactTextChanged)
		startResolve(0);
}

void EditItemWidget::onContactTextEdited(const QString &AText)
{
	Q_UNUSED(AText);
	FContactTextChanged = true;
	startResolve(RESOLVE_WAIT_INTERVAL);
}

void EditItemWidget::onProfileButtonClicked(bool)
{
	QRadioButton *button = qobject_cast<QRadioButton *>(sender());
	if (button)
	{
		setSelectedProfile(FProfiles.key(button));
	}
}

void EditItemWidget::onServiceLoginReceived(const QString &AId, const QString &ALogin)
{
	if (FLoginRequests.contains(AId))
	{
		Jid serviceJid = FLoginRequests.take(AId);
		if (FProfiles.contains(serviceJid))
		{
			FProfiles[serviceJid]->setText(ALogin);
		}
	}
}

void EditItemWidget::onLegacyContactJidReceived(const QString &AId, const Jid &AUserJid)
{
	if (FContactJidRequest == AId)
	{
		setRealContactJid(AUserJid);
	}
}

void EditItemWidget::onGatewayErrorReceived(const QString &AId, const QString &AError)
{
	if (FLoginRequests.contains(AId))
	{
		FLoginRequests.remove(AId);
	}
	else if (FContactJidRequest == AId)
	{
		setRealContactJid(Jid::null);
		setErrorMessage(tr("Unable to determine the contact ID: %1").arg(AError));
	}
}

void EditItemWidget::onServiceEnableChanged(const Jid &AStreamJid, const Jid &AServiceJid, bool AEnabled)
{
	Q_UNUSED(AServiceJid); Q_UNUSED(AEnabled);
	if (AStreamJid == FStreamJid)
	{
		updateProfiles();
	}
}
