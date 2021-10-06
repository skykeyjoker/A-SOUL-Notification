#include "BilibiliMessage.h"
#include <QFile>
#include <iostream>



BiliBiliMessage::BiliBiliMessage(QObject* parent):
	QObject(parent)
{
	qDebug() << "BilibiliMessage Starting...";
	currentLive = -1;

	/* 首先进行一轮查询，填充oldMessageCard */
	for(int i=0; i<6; ++i)
	{
		QString url = QString(BMURLPREFIX) + ASOULUID[i];
		BilibiliMessageCard card = messageQuery(url);
		oldMessageCardMap.insert(card.uid, card);
		// 延时
		QThread::sleep(20);
	}
	QThread::sleep(15);
	/* 填充currentLive */
	for(int i=0;i<6;++i)
	{
		QString url = QString(BLURLPREFIX) + ASOULUID[i];
		BilibiliLiveCard card = liveQuery(url);
		if (card.status == 1)
			currentLive = card.mid;
		/* *TODO 发送直播消息 */
		emit newBilibiliLive(card.mid, card.title, card.url);
		// 延时
		QThread::sleep(20);
	}
	qDebug() << "BilibiliMessage Started.";
	QThread::sleep(10);

	// test
	//emit newBilibiliMessage(672346917, 4, "577974217179897424");
	//emit newBilibiliMessage(672346917, 1, "576419159846716883");
	//emit newBilibiliMessage(672346917, 8, "576109793352460431");
	//emit newBilibiliMessage(672353429, 8, "577750719968869443");
	//emit newBilibiliMessage(351609538, 4, "577285630847094565");
	//emit newBilibiliMessage(672328094, 4, "577931684122663291");
	//emit newBilibiliMessage(672342685, 4, "577908499879659285");
	//emit newBilibiliMessage(703007996, 1, "577686188082574470");

	//emit newBilibiliLive(672346917, "向晚直播提醒测试","https://live.bilibili.com/22625025");
	//emit newBilibiliLive(672353429, "贝拉直播提醒测试","https://live.bilibili.com/22632424");
	//emit newBilibiliLive(351609538, "珈乐直播提醒测试","https://live.bilibili.com/22634198");
	//emit newBilibiliLive(672328094, "嘉然直播提醒测试","https://live.bilibili.com/22637261");
	//emit newBilibiliLive(672342685, "乃琳直播提醒测试","https://live.bilibili.com/22625027");
	//emit newBilibiliLive(703007996, "A-Soul Official直播提醒测试","https://live.bilibili.com/22632157");
}

void BiliBiliMessage::startQuery()
{
	/* 3/min查询 */
	qDebug() << "Start one query.";
	QTimer* timer = new QTimer(this);
	connect(timer, &QTimer::timeout, this, [&]()
	{
		qDebug() << "time out start";
		for (int i = 0; i < 6; ++i)
		{
			QString url = QString(BMURLPREFIX) + ASOULUID[i];
			BilibiliMessageCard card = messageQuery(url);
			if(oldMessageCardMap[card.uid].dynamic_id_str.compare(card.dynamic_id_str)!=0)
			{
				// 更新
				oldMessageCardMap.insert(card.uid,card);
				emit newBilibiliMessage(card.uid, card.type, card.dynamic_id_str);
			}
			// 延时
			QThread::sleep(20);
		}
		QThread::sleep(15);
		for (int i = 0; i < 6; ++i)
		{
			QString url = QString(BLURLPREFIX) + ASOULUID[i];
			BilibiliLiveCard card = liveQuery(url);
			if (card.status == 1)
			{
				if(currentLive!=card.mid)
				{
					currentLive = card.mid;
					emit newBilibiliLive(card.mid, card.title, card.url);
				}
			}
			// 延时
			QThread::sleep(20);
		}
		QThread::sleep(20);
		qDebug() << "One query end.";
	});
	timer->start(25000);

	// test
	//emit newBilibiliMessage(672346917, 4, "577974217179897424");
	//emit newBilibiliMessage(672346917, 1, "576419159846716883");
	//emit newBilibiliMessage(672346917, 8, "576109793352460431");
	//emit newBilibiliMessage(672353429, 8, "577750719968869443");
	//emit newBilibiliMessage(351609538, 4, "577285630847094565");
	//emit newBilibiliMessage(672328094, 4, "577931684122663291");
	//emit newBilibiliMessage(672342685, 4, "577908499879659285");
	//emit newBilibiliMessage(703007996, 1, "577686188082574470");

	//emit newBilibiliLive(672346917, "向晚直播提醒测试", "https://live.bilibili.com/22625025");
	//emit newBilibiliLive(672353429, "贝拉直播提醒测试", "https://live.bilibili.com/22632424");
	//emit newBilibiliLive(351609538, "珈乐直播提醒测试", "https://live.bilibili.com/22634198");
	//emit newBilibiliLive(672328094, "嘉然直播提醒测试", "https://live.bilibili.com/22637261");
	//emit newBilibiliLive(672342685, "乃琳直播提醒测试", "https://live.bilibili.com/22625027");
	//emit newBilibiliLive(703007996, "A-Soul Official直播提醒测试", "https://live.bilibili.com/22632157");
}

BilibiliMessageCard BiliBiliMessage::messageQuery(const QString& url)
{
	BilibiliMessageCard ret;

	Json doc = getJson(url);

	int uid = doc["data"]["cards"][0]["desc"]["uid"].get<int>();
	int type = doc["data"]["cards"][0]["desc"]["uid"].get<int>();
	QString dynamic_id_str = QString::fromStdString(doc["data"]["cards"][0]["desc"]["dynamic_id_str"].get<std::string>());
	QString nickname = QString::fromStdString(doc["data"]["cards"][0]["desc"]["user_profile"]["info"]["uname"].get<std::string>());

	ret.uid = uid;
	ret.type = type;
	ret.dynamic_id_str = dynamic_id_str;
	ret.nickname = nickname;

	qDebug() << "Message Query: " << ret.uid << ret.nickname << ret.type  << ret.dynamic_id_str;

	return ret;
}

BilibiliLiveCard BiliBiliMessage::liveQuery(const QString& url)
{
	BilibiliLiveCard ret;

	Json doc = getJson(url);

	int mid = doc["data"]["mid"].get<int>();
	int status = doc["data"]["live_room"]["liveStatus"].get<int>();
	QString title = QString::fromStdString(doc["data"]["live_room"]["title"].get<std::string>());
	QString nickname = QString::fromStdString(doc["data"]["name"].get<std::string>());
	QString liveurl = QString::fromStdString(doc["data"]["live_room"]["url"].get<std::string>());

	ret.mid = mid;
	ret.status = status;
	ret.title = title;
	ret.nickname = nickname;
	ret.url = liveurl;

	qDebug() << "Live Query: " << ret.mid << ret.nickname << ret.status << ret.title << ret.url;

	return ret;
}

Json BiliBiliMessage::getJson(const QString& url)
{
	//QJsonDocument doc;
	Json j;
	QNetworkAccessManager manager(this);
	QNetworkRequest request;
	request.setUrl(QUrl(url));

	QEventLoop eventLoop;
	connect(&manager, SIGNAL(finished(QNetworkReply*)), &eventLoop, SLOT(quit()));
	QNetworkReply* reply = manager.get(request);
	eventLoop.exec();

	if (reply->error() != QNetworkReply::NoError)
	{
		qDebug() << "Query Error!" << url;
		return j;
	}

	QByteArray buf = reply->readAll();
	j = Json::parse(buf.data());
	if(j.is_null())
	{
		qDebug() << "Parse Json Error!"<< url;
	}

	return j;
}
