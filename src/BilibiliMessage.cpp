#include "BilibiliMessage.h"

BiliBiliMessage::BiliBiliMessage(std::shared_ptr<spdlog::logger>& logger, QStringList uidList, QObject* parent) :
	m_uidList(uidList),
	m_logger(logger),
	QObject(parent)
{
	qDebug() << "BilibiliMessage Starting...";
	m_logger->info("哔哩哔哩查询模块初始化。");
	currentLive = -1;
	qDebug() << "BilibiliMessage Started.";
	m_logger->info("哔哩哔哩查询模块初始化成功。");
}

void BiliBiliMessage::startQuery()
{
	/* 3/min查询 */
	qDebug() << "Start one query.";
	m_logger->info("哔哩哔哩查询模块开始查询。");

	while(1)
	{
		m_logger->info("哔哩哔哩查询模块开始一次查询。");
		m_logger->info("开始查询动态。");
		for (int i = 0; i < 6; ++i)
		{
			//QString url = QString(BMURLPREFIX) + ASOULUID[i];
			QString url = QString(BMURLPREFIX) + m_uidList[i];
			BilibiliMessageCard card = messageQuery(url);
			if (!card.is_null)
			{
				if (!oldMessageCardMap.contains(card.uid))
				{
					oldMessageCardMap.insert(card.uid, card);
				}
				else
				{
					if (oldMessageCardMap[card.uid].dynamic_id_str.compare(card.dynamic_id_str) != 0)
					{
						// 更新
						oldMessageCardMap.insert(card.uid, card);
						emit newBilibiliMessage(card.uid, card.type, card.dynamic_id_str);
					}
				}
			}
			else
			{
				QString errorString = "查询成员动态时发生错误，返回了空的消息卡片！";
				m_logger->error("查询成员动态时发生错误，返回了空的消息卡片！");
				emit errorOccurred(errorString);
			}
			// 延时
			QThread::sleep(5);

		}

		m_logger->info("开始查询直播。");
		for (int i = 0; i < 6; ++i)
		{
			//QString url = QString(BLURLPREFIX) + ASOULUID[i];
			QString url = QString(BLURLPREFIX) + m_uidList[i];
			BilibiliLiveCard card = liveQuery(url);
			if (!card.is_null)
			{
				if (card.status == 1)
				{
					if (currentLive != card.mid)
					{
						currentLive = card.mid;
						emit newBilibiliLive(card.mid, card.title, card.url);
					}
				}
			}
			else
			{
				QString errorString = "查询成员直播状态时发生错误，返回了空的直播状态卡片！";
				m_logger->error("查询成员直播状态时发生错误，返回了空的直播状态卡片！");
				emit errorOccurred(errorString);
			}
			// 延时
			QThread::sleep(5);
		}
		QThread::sleep(5);
		//emit errorOccurred("查询成员动态时发生错误，返回了空的消息卡片！");
		//emit errorOccurred("查询成员直播状态时发生错误，返回了空的直播状态卡片！");
		//emit errorOccurred("查询成员动态时发生错误，获取的JSON值为空！");
		//emit errorOccurred("查询成员直播状态时发生错误，获取的JSON值为空！");
		//emit errorOccurred("请求失败，错误类型：no network");
		//emit errorOccurred("请求失败，获取了空的返回数据！");
		//emit errorOccurred("请求失败，未能成功解析JSON！");
		//emit errorOccurred("请求失败，返回状态码：200");
		//emit newBilibiliMessage(672328094, 1, "585849885826328920");
		qDebug() << "One query end.";
		m_logger->info("哔哩哔哩查询模块结束一次查询。");
	}
}

BilibiliMessageCard BiliBiliMessage::messageQuery(const QString& url)
{
	BilibiliMessageCard ret;
	ret.is_null = true;

	Json doc = getJson(url);
	if (doc.is_null())
	{
		qDebug() << "Message Query Error. Reason: The json is null";
		QString errorString = "查询成员动态时发生错误，获取的JSON值为空！";
		m_logger->error("查询成员动态时发生错误，获取的JSON值为空！");
		emit errorOccurred(errorString);
		return ret;
	}

	/* []操作符理应有异常机制，对异常机制进行处理
	 * JSON_THROW(std::out_of_range("key not found"));
	 */
	int uid;
	int type;
	QString dynamic_id_str;
	QString nickname;
	try
	{
		uid = doc["data"]["cards"][0]["desc"]["uid"].get<int>();
		type = doc["data"]["cards"][0]["desc"]["type"].get<int>();
		dynamic_id_str = QString::fromStdString(doc["data"]["cards"][0]["desc"]["dynamic_id_str"].get<std::string>());
		nickname = QString::fromStdString(doc["data"]["cards"][0]["desc"]["user_profile"]["info"]["uname"].get<std::string>());
	}
	catch (...)
	{
		qDebug() << "Message Query Error. Reason: The json can not be parsed";
		QString errorString = "查询成员动态时发生错误，未能解析获取的JSON！";
		m_logger->error("查询成员动态时发生错误，未能解析获取的JSON！");
		emit errorOccurred(errorString);
		return ret;
	}

	ret.uid = uid;
	ret.type = type;
	ret.dynamic_id_str = dynamic_id_str;
	ret.nickname = nickname;
	ret.is_null = false;

	qDebug() << "Message Query: " << ret.uid << ret.nickname << ret.type  << ret.dynamic_id_str;
	m_logger->info("动态消息卡片：UID：{}，昵称：{}，类型：{}，动态ID：{}", ret.uid, ret.nickname.toStdString(), ret.type, ret.dynamic_id_str.toStdString());

	return ret;
}

BilibiliLiveCard BiliBiliMessage::liveQuery(const QString& url)
{
	BilibiliLiveCard ret;
	ret.is_null = true;

	Json doc = getJson(url);
	if(doc.is_null())
	{
		qDebug() << "Live Query Error. Reason: The json is null";
		QString errorString = "查询成员直播状态时发生错误，获取的JSON值为空！";
		m_logger->error("查询成员直播状态时发生错误，获取的JSON值为空！");
		emit errorOccurred(errorString);
		return ret;
	}

	/* []操作符理应有异常机制，对异常机制进行处理
	 * JSON_THROW(std::out_of_range("key not found"));
	 */
	int mid;
	int status;
	QString title;
	QString nickname;
	QString liveurl;
	try
	{
		mid = doc["data"]["mid"].get<int>();
		status = doc["data"]["live_room"]["liveStatus"].get<int>();
		title = QString::fromStdString(doc["data"]["live_room"]["title"].get<std::string>());
		nickname = QString::fromStdString(doc["data"]["name"].get<std::string>());
		liveurl = QString::fromStdString(doc["data"]["live_room"]["url"].get<std::string>());
	}
	catch (...)
	{
		qDebug() << "Live Query Error. Reason: The json can not be parsed";
		QString errorString = "查询成员直播状态时发生错误，未能解析获取的JSON！";
		m_logger->error("查询成员直播状态时发生错误，未能解析获取的JSON！");
		emit errorOccurred(errorString);
		return ret;
	}

	ret.mid = mid;
	ret.status = status;
	ret.title = title;
	ret.nickname = nickname;
	ret.url = liveurl;
	ret.is_null = false;

	qDebug() << "Live Query: " << ret.mid << ret.nickname << ret.status << ret.title << ret.url;
	m_logger->info("直播消息卡片：MID：{}，昵称：{}，状态：{}，标题：{}，网址：{}", ret.mid, ret.nickname.toStdString(), ret.status, ret.title.toStdString(), ret.url.toStdString());

	return ret;
}

Json BiliBiliMessage::getJson(const QString& url)
{
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
		qDebug() << "Query Error! Reason: request error" << reply->errorString() << url;
		m_logger->error("请求失败，错误类型：{}", reply->errorString().toStdString());
		QString errorString = "请求失败，错误类型：" + reply->errorString();
		emit errorOccurred(errorString);
		return j;
	}

	QByteArray buf = reply->readAll();
	if (buf.isNull())
	{
		qDebug() << "No Data return";
		m_logger->error("请求失败，获取了空的返回数据！");
		QString errorString = "请求失败，获取了空的返回数据！";
		emit errorOccurred(errorString);
		return j;
	}

	j = Json::parse(buf.data(), nullptr, false);  // 不抛出异常
	if(j.is_null())
	{
		qDebug() << "Parse Json Error!"<< url;
		m_logger->error("请求失败，未能成功解析JSON！");
		QString errorString = "请求失败，未能成功解析JSON！";
		emit errorOccurred(errorString);
		return j;
	}

	// 判断返回code状态码
	int retCode;
	try
	{
		retCode = j["code"].get<int>();
	}
	catch (...)
	{
		qDebug() << "Parse Json Error! Can not get the [code]";
		m_logger->error("请求失败，解析JSON[code]时失败！");
		QString errorString = "请求失败，解析JSON[code]时失败！";
		emit errorOccurred(errorString);
		return Json();  // 返回空的JSON
	}
	
	if (retCode != 0)
	{
		qDebug() << "Return Code is not null";
		m_logger->error("请求失败，返回状态码：{}", retCode);
		QString errorString = "请求失败，返回状态码：" + QString::number(retCode);
		emit errorOccurred(errorString);
		return Json();  // 返回空的JSON
	}

	return j;
}
