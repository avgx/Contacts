#include "tabbaritem.h"

#include <QPaintEvent>
#include <QHBoxLayout>

#define BLINK_VISIBLE_TIME      750
#define BLINK_INVISIBLE_TIME    250

TabBarItem::TabBarItem(QWidget *AParent) : QFrame(AParent)
{
	FIconSize = QSize(15,15);

	setLayout(new QHBoxLayout);
	layout()->setMargin(2);
	layout()->setSpacing(2);

	layout()->addWidget(FIconLabel = new QLabel(this));
	layout()->addWidget(FTextLabel = new QLabel(this));
	layout()->addWidget(FCloseButton = new CloseButton(this));

	FIconLabel->installEventFilter(this);
	FIconLabel->setTextInteractionFlags(Qt::NoTextInteraction);
	FTextLabel->installEventFilter(this);
	FTextLabel->setTextInteractionFlags(Qt::NoTextInteraction);
	FCloseButton->installEventFilter(this);

	FIconLabel->setFixedSize(FIconSize);
	FTextLabel->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Preferred);
	connect(FCloseButton,SIGNAL(clicked()),SIGNAL(closeButtonClicked()));

	FIconHidden = false;
	FBlinkTimer.setSingleShot(true);
	connect(&FBlinkTimer,SIGNAL(timeout()),SLOT(onBlinkTimerTimeout()));

	setActive(false);
	setDraging(false);
}

TabBarItem::~TabBarItem()
{

}

bool TabBarItem::isActive() const
{
	return FActive;
}

void TabBarItem::setActive(bool AActive)
{
	FActive = AActive;
	setStyleSheet(styleSheet());
	update();
}

bool TabBarItem::isDraging() const
{
	return FDraging;
}

void TabBarItem::setDraging(bool ADragged)
{
	FDraging = ADragged;
	setStyleSheet(styleSheet());
	update();
}

bool TabBarItem::isCloseable() const
{
	return FCloseButton->isVisible();
}

void TabBarItem::setCloseable(bool ACloseable)
{
	FCloseButton->setVisible(ACloseable);
}

QSize TabBarItem::iconSize() const
{
	return FIconSize;
}

void TabBarItem::setIconSize(const QSize &ASize)
{
	FIconLabel->setFixedSize(ASize);
	FIconSize = ASize;
}

QIcon TabBarItem::icon() const
{
	return FIcon;
}

void TabBarItem::setIcon(const QIcon &AIcon)
{
	if (FNotify.priority < 0)
	{
		setIconKey(QString::null);
		showIcon(AIcon);
	}
	FIcon = AIcon;
}

QString TabBarItem::iconKey() const
{
	return FIconKey;
}

void TabBarItem::setIconKey(const QString &AIconKey)
{
	if (FNotify.priority < 0)
		showIconKey(AIconKey, RSR_STORAGE_MENUICONS);
	FIconKey = AIconKey;
}

QString TabBarItem::text() const
{
	return FText;
}

void TabBarItem::setText(const QString &AText)
{
	if (FNotify.priority < 0)
		showText(AText);
	FText = AText;
}

QString TabBarItem::toolTip() const
{
	return FToolTip;
}

void TabBarItem::setToolTip(const QString &AToolTip)
{
	if (FNotify.priority < 0)
		showToolTip(AToolTip);
	FToolTip = AToolTip;
}

ITabPageNotify TabBarItem::notify() const
{
	return FNotify;
}

void TabBarItem::setNotify(const ITabPageNotify &ANotify)
{
	FNotify = ANotify;
	FIconHidden = false;
	FBlinkTimer.stop();
	if (FNotify.priority > 0)
	{
		if (FNotify.blink)
			FBlinkTimer.start(BLINK_VISIBLE_TIME);
		if (!FNotify.iconKey.isEmpty() && !FNotify.iconStorage.isEmpty())
			showIconKey(FNotify.iconKey,FNotify.iconStorage);
		else
			showIcon(FNotify.icon);
		showToolTip(FNotify.toolTip);
		showStyleKey(FNotify.styleKey);
	}
	else
	{
		if (!FIconKey.isEmpty())
			showIconKey(FIconKey, RSR_STORAGE_MENUICONS);
		else
			showIcon(FIcon);
		showText(FText);
		showToolTip(FToolTip);
		showStyleKey(QString::null);
	}
	update();
}

void TabBarItem::showIcon(const QIcon &AIcon)
{
	if (!AIcon.isNull())
		FIconLabel->setPixmap(AIcon.pixmap(FIconSize));
	else
		FIconLabel->clear();
}

void TabBarItem::showIconKey(const QString &AIconKey, const QString &AIconStorage)
{
	if (!AIconKey.isEmpty())
	{
		IconStorage::staticStorage(AIconStorage)->insertAutoIcon(FIconLabel,AIconKey,0,0,"pixmap");
	}
	else if (!AIconStorage.isEmpty())
	{
		IconStorage::staticStorage(AIconStorage)->removeAutoIcon(FIconLabel);
	}
	else
	{
		FIconLabel->clear();
	}
}

void TabBarItem::showText(const QString &AText)
{
	FTextLabel->setText(Qt::escape(AText));
}

void TabBarItem::showToolTip(const QString &AToolTip)
{
	QString tip = !AToolTip.isEmpty() ? QString("<span>%1<br>%2</span>").arg(Qt::escape(FText)).arg(Qt::escape(AToolTip)) : QString("<span>%1</span>").arg(Qt::escape(FText));
	QFrame::setToolTip(tip);
}

void TabBarItem::showStyleKey(const QString &AStyleKey)
{
	if (!AStyleKey.isEmpty())
		StyleStorage::staticStorage(RSR_STORAGE_STYLESHEETS)->insertAutoStyle(this,AStyleKey);
	else
		StyleStorage::staticStorage(RSR_STORAGE_STYLESHEETS)->removeAutoStyle(this);
}

void TabBarItem::enterEvent(QEvent *AEvent)
{
	QFrame::enterEvent(AEvent);
	update();
}

void TabBarItem::leaveEvent(QEvent *AEvent)
{
	QFrame::leaveEvent(AEvent);
	update();
}

void TabBarItem::paintEvent(QPaintEvent *AEvent)
{
	if (!FDraging) 
		QFrame::paintEvent(AEvent);
}

bool TabBarItem::eventFilter(QObject *AObject, QEvent *AEvent)
{
	if (FDraging && AEvent->type()==QEvent::Paint)
		return true;
	if (FIconHidden && AObject==FIconLabel && AEvent->type()==QEvent::Paint)
		return true;
	return QFrame::eventFilter(AObject,AEvent);
}

void TabBarItem::onBlinkTimerTimeout()
{
	FIconHidden = !FIconHidden;
	if (FIconHidden)
		FBlinkTimer.start(BLINK_INVISIBLE_TIME);
	else
		FBlinkTimer.start(BLINK_VISIBLE_TIME);
	update(FIconLabel->geometry());
}