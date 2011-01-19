#ifndef CUSTOMBORDERCONTAINER_H
#define CUSTOMBORDERCONTAINER_H

#include <QWidget>
#include <QLayout>
#include <QDebug>
#include "utilsexport.h"
#include "menu.h"

class CustomBorderContainerPrivate;
struct HeaderButton;

// NOTE: QWidget::isMaximized() will return false even if widget is maximized. I will try to fix it later.
// and showMaximized() might be REALLY buggy!

class UTILS_EXPORT CustomBorderContainer : public QWidget
{
	Q_OBJECT
	friend class CustomBorderContainerPrivate;

public:
	explicit CustomBorderContainer(QWidget * widgetToContain = 0);
	CustomBorderContainer(const CustomBorderContainerPrivate &style);
	~CustomBorderContainer();
	QWidget * widget() const;
	// WARNING! the old widget will be deleted! use releaseWidget() to just unset widget
	void setWidget(QWidget * widget);
	QWidget * releaseWidget();
	void loadFile(const QString & fileName);
	bool isMovable() const;
	void setMovable(bool movable = true);
	bool isResizable() const;
	void setResizable(bool resizable = true);
signals:
	void minimizeClicked();
	void maximizeClicked();
	void closeClicked();
	void iconClicked();
	void minimized();
	void maximized();
	void closed();
protected:
	// event handlers
	void changeEvent(QEvent *e);
	void resizeEvent(QResizeEvent *);
	void mousePressEvent(QMouseEvent *);
	void mouseMoveEvent(QMouseEvent *);
	void mouseReleaseEvent(QMouseEvent *);
	void mouseDoubleClickEvent(QMouseEvent *);
	void moveEvent(QMoveEvent *);
	void paintEvent(QPaintEvent *);
	void enterEvent(QEvent *);
	void leaveEvent(QEvent *);
	void focusInEvent(QFocusEvent *);
	void focusOutEvent(QFocusEvent *);
	void contextMenuEvent(QContextMenuEvent *);
	// event filter
	bool event(QEvent *);
	bool eventFilter(QObject *, QEvent *);
protected:
	// common initialization
	void init();
	enum GeometryState
	{
		None,
		Resizing,
		Moving
	};
	GeometryState geometryState() const;
	void setGeometryState(GeometryState newGeometryState);
	void updateGeometry(const QPoint & p);
	enum HeaderButtonState
	{
		Normal,
		NormalHover,
		Pressed,
		Disabled,
		PressedDisabled
	};
	enum HeaderButtons
	{
		NoneButton,
		MinimizeButton,
		MaximizeButton,
		CloseButton
	};

public:
	enum HeaderButtonsFlag
	{
		MinimizeVisible = 0x01,
		MaximizeVisible = 0x02,
		CloseVisible = 0x04,
		MinimizeEnabled = 0x08,
		MaximizeEnabled = 0x10,
		CloseEnabled = 0x20
	};
	Q_DECLARE_FLAGS(HeaderButtonsFlags, HeaderButtonsFlag)
	HeaderButtonsFlags headerButtonsFlags() const;
	void setHeaderButtonFlags(HeaderButtonsFlag flags);
	// minimize button
	bool isMinimizeButtonVisible() const;
	void setMinimizeButtonVisible(bool visible = true);
	bool isMinimizeButtonEnabled() const;
	void setMinimizeButtonEnabled(bool enabled = true);
	// maximize button
	bool isMaximizeButtonVisible() const;
	void setMaximizeButtonVisible(bool visible = true);
	bool isMaximizeButtonEnabled() const;
	void setMaximizeButtonEnabled(bool enabled = true);
	// close button
	bool isCloseButtonVisible() const;
	void setCloseButtonVisible(bool visible = true);
	bool isCloseButtonEnabled() const;
	void setCloseButtonEnabled(bool enabled = true);
	// some public parameters
	// header move rect controls
	int headerMoveLeft() const;
	void setHeaderMoveLeft(int left);
	int headerMoveRight() const;
	void setHeaderMoveRight(int right);
	int headerMoveTop() const;
	void setHeaderMoveTop(int top);
	int headerMoveHeight() const;
	void setHeaderMoveHeight(int height);
	// border widths
	int leftBorderWidth() const;
	int rightBorderWidth() const;
	int topBorderWidth() const;
	int bottomBorderWidth() const;
protected:
	// header button flags manipulations
	void addHeaderButtonFlag(HeaderButtonsFlag flag);
	void removeHeaderButtonFlag(HeaderButtonsFlag flag);
	// header buttons under mouse
	int headerButtonsCount() const;
	QRect headerButtonRect(HeaderButtons button) const;
	bool minimizeButtonUnderMouse() const;
	bool maximizeButtonUnderMouse() const;
	bool closeButtonUnderMouse() const;
	HeaderButtons headerButtonUnderMouse() const;
	QRect headerButtonsRect() const;
	void repaintHeaderButtons();
	QRect windowIconRect() const;
	void showWindowMenu(const QPoint & p);
	// etc...
	enum BorderType
	{
		NoneBorder = 0,
		TopLeftCorner,
		TopRightCorner,
		BottomLeftCorner,
		BottomRightCorner,
		LeftBorder,
		RightBorder,
		TopBorder,
		BottomBorder
	};
	void mouseMove(const QPoint & p, QWidget * widget);
	bool mousePress(const QPoint & p, QWidget * widget);
	void mouseRelease(const QPoint & p, QWidget * widget, Qt::MouseButton button = Qt::LeftButton);
	void mouseDoubleClick(const QPoint & p, QWidget * widget);
	bool pointInBorder(BorderType border, const QPoint & p);
	bool pointInHeader(const QPoint & p);
	void checkResizeCondition(const QPoint & p);
	void checkMoveCondition(const QPoint & p);
	void updateCursor(QWidget * widget = 0);
	void updateShape();
	void updateIcons();
	void setLayoutMargins();
	QRect headerRect() const;
	QRect headerMoveRect() const;
	void drawHeader(QPainter * p);
	void drawButton(HeaderButton & button, QPainter * p, HeaderButtonState state = Normal);
	void drawButtons(QPainter * p);
	void drawIcon(QPainter * p);
	void drawTitle(QPainter * p);
	void drawBorders(QPainter * p);
	void drawCorners(QPainter * p);
	QPoint mapFromWidget(QWidget * widget, const QPoint &point);
	QImage loadImage(const QString & key);
	QIcon loadIcon(const QString & key);
	QPixmap loadPixmap(const QString & key);
protected slots:
	void minimizeWidget();
	void maximizeWidget();
	void closeWidget();
private:
	// widgets/layouts
	QWidget * containedWidget;
	QLayout * containerLayout;
	QWidget * headerWidget;
	GeometryState currentGeometryState;
	QRect oldGeometry;
	QPoint oldPressPoint;
	QPoint lastMousePosition;
	CustomBorderContainerPrivate * borderStyle;
	BorderType resizeBorder;
	bool canMove;
	bool movable;
	bool resizable;
	HeaderButtonsFlags buttonsFlags;
	HeaderButtons pressedHeaderButton;
	bool isMaximized;
	QRect normalGeometry;
	Menu * windowMenu;
	Action * minimizeAction;
	Action * maximizeAction;
	Action * closeAction;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(CustomBorderContainer::HeaderButtonsFlags)

#endif // CUSTOMBORDERCONTAINER_H