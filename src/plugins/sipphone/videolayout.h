#ifndef VIDEOLAYOUT_H
#define VIDEOLAYOUT_H

#include <QRectF>
#include <QLayout>
#include <QWidget>
#include "videolabel.h"

class VideoLayout : 
	public QLayout
{
	Q_OBJECT;
public:
	VideoLayout(VideoLabel *ARemoteVideo, VideoLabel *ALocalVideo, QWidget *AParent);
	~VideoLayout();
	// QLayout
	int count() const;
	void addItem(QLayoutItem *AItem);
	QLayoutItem *itemAt(int AIndex) const;
	QLayoutItem *takeAt(int AIndex);
	// QLayoutItem
	QSize sizeHint() const;
	void setGeometry(const QRect &ARect);
	// VideoLayout
	int locaVideoMargin() const;
	void setLocalVideoMargin(int AMargin);
	void saveLocalVideoGeometry();
	void restoreLocalVideoGeometry();
protected:
	void saveLocalVideoGeometryScale();
	Qt::Alignment remoteVideoAlignment() const;
	Qt::Alignment geometryAlignment(const QRect &AGeometry) const;
	QRect adjustLocalVideoSize(const QRect &AGeometry) const;
	QRect adjustLocalVideoPosition(const QRect &AGeometry) const;
	QRect correctLocalVideoPosition(const QRect &AGeometry) const;
	QRect correctLocalVideoSize(Qt::Corner ACorner, const QRect &AGeometry) const;
protected slots:
	void onMoveLocalVideo(const QPoint &APos);
	void onResizeLocalVideo(Qt::Corner ACorner, const QPoint &APos);
private:
	QRectF FLocalScale;
	int FLocalMargin;
	int FLocalStickDelta;
	QWidget *FControlls;
	VideoLabel *FLocalVideo;
	VideoLabel *FRemoteVideo;
};

#endif // VIDEOLAYOUT_H
