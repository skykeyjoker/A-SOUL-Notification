#ifndef BILIBILIMESSAGE_H
#define BILIBILIMESSAGE_H

#include "Essential.h"
#include "json.hpp"

#include <QObject>
#include <QtCore>
#include <QEventLoop>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QTimer>
#include <QDebug>

typedef struct BilibiliMessageCard
{
	QString nickname;
	QString dynamic_id_str;
	int uid;
	int type;
	bool is_null;
}BilibiliMessageCard;

typedef struct BilibiliLiveCard
{
	QString nickname;
	QString title;
	int mid;
	int status;
	QString url;
	bool is_null;
}BilibiliLiveCard;

using Json = nlohmann::json;

class BiliBiliMessage: public QObject
{
	Q_OBJECT
public:
	BiliBiliMessage(QObject* parent = nullptr);
	void startQuery();

signals:
	void newBilibiliMessage(int user, int type, const QString dynamic_id_str);
	void newBilibiliLive(int user, const QString title, const QString url);
	void errorOccurred(const QString errorString);

private:
	BilibiliMessageCard messageQuery(const QString& url);
	BilibiliLiveCard liveQuery(const QString& url);
	Json getJson(const QString& url);

private:
	QMap<int, BilibiliMessageCard> oldMessageCardMap;
	int currentLive;
};




#endif /* BILIBILIMESSAGE_H */