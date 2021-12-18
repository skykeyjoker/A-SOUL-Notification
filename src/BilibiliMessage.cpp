#include "BilibiliMessage.h"

BiliBiliMessage::BiliBiliMessage(std::shared_ptr<spdlog::logger>& logger, QStringList &uidList, QObject* parent) :
	m_uidList(uidList),
	m_logger(logger),
	QObject(parent)
{
	qDebug() << "BilibiliMessage Starting...";
	m_logger->info("哔哩哔哩查询模块初始化。");

	// 初始化Time Out
	if(uidList.size()!=6)  // 仅当用户自定义查询用户后（数量改变）更改time out
		m_timeOut = 60 / uidList.size() >= 5 ? (60 / uidList.size() >= 20 ? 20 : 60 / uidList.size()) : 5;  // 最短查询间隔为5s，最长为20s
	qDebug() << "Time out:" << m_timeOut;

	// 初始化livestatusmap
	for (size_t i = 0; i < uidList.size(); ++i)
		m_liveStatusMap[uidList[i].toInt()] = 0;

	qDebug() << "BilibiliMessage Started.";
	m_logger->info("哔哩哔哩查询模块初始化成功。");
}

void BiliBiliMessage::startQuery()
{
	/* 5s/次查询 */
	qDebug() << "Start one query.";
	m_logger->info("哔哩哔哩查询模块开始查询。");

	while(1)
	{
		m_logger->info("哔哩哔哩查询模块开始一次查询。");
		m_logger->info("开始查询动态。");
		for (int i = 0; i < m_uidList.size(); ++i)
		{
			QString url = QString(BMURLPREFIX) + m_uidList[i];
			BiliBiliMessageRes res = messageQuery(url);  // 查询一组动态
			if (!res.empty())
			{
				// 判断当前是否为首次查询
				if(!m_oldMessageCardMap.contains(res[0].uid))
				{
					// 首次查询，填充oldMessageCardMap
					QSet<QString> messageSet;
					for(size_t i=0;i<res.size();++i)
					{
						messageSet.insert(res[i].dynamic_id_str);
					}
					m_oldMessageCardMap.insert(res[0].uid,messageSet);

					qDebug() << messageSet.size() << messageSet;
				}
				else
				{
					// 非首次查询
					for (size_t i = 0; i < res.size(); ++i) // 遍历每条动态
					{
						const auto& currentMessageCard = res[i];
						if (!m_oldMessageCardMap[currentMessageCard.uid].contains(currentMessageCard.dynamic_id_str))
						{
							// 新动态
							m_oldMessageCardMap[currentMessageCard.uid].insert(currentMessageCard.dynamic_id_str);  // 更新oldMessageMap
							emit newBilibiliMessage(currentMessageCard.uid, currentMessageCard.type, currentMessageCard.dynamic_id_str); // 发送新动态消息

							qDebug() << m_oldMessageCardMap[currentMessageCard.uid].size()<< m_oldMessageCardMap[currentMessageCard.uid].size();
						}
					}
				}
			}
			else
			{
				QString errorString = "查询成员动态时发生错误，返回了空的消息卡片组！";
				m_logger->error("查询成员动态时发生错误，返回了空的消息卡片组！");
				emit errorOccurred(errorString);
			}
			// 延时
			QThread::sleep(m_timeOut);

		}

		m_logger->info("开始查询直播。");
		for (int i = 0; i < m_uidList.size(); ++i)
		{
			QString url = QString(BLURLPREFIX) + m_uidList[i];
			BilibiliLiveCard card = liveQuery(url);
			if (!card.is_null)
			{
				if(card.status!=m_liveStatusMap[card.mid])
				{
					// 与历史状态不同
					if(card.status==1) // 正在直播
						emit newBilibiliLive(card.mid, card.title, card.url);

					m_liveStatusMap[card.mid] = card.status; // 更新liveStatusMap
				}
			}
			else
			{
				QString errorString = "查询成员直播状态时发生错误，返回了空的直播状态卡片！";
				m_logger->error("查询成员直播状态时发生错误，返回了空的直播状态卡片！");
				emit errorOccurred(errorString);
			}
			// 延时
			QThread::sleep(m_timeOut);
		}
		QThread::sleep(m_timeOut);
		//errCnt += 2;
		//if (errCnt < 6)
		//{
		//	emit errorOccurred("查询成员动态时发生错误，返回了空的消息卡片！");
		//	emit errorOccurred("查询成员直播状态时发生错误，返回了空的直播状态卡片！");
		//}
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

BiliBiliMessageRes BiliBiliMessage::messageQuery(const QString& url)
{
	BiliBiliMessageRes res;
	//ret.is_null = true;

	Json doc = getJson(url);
	if (doc.is_null())
	{
		qDebug() << "Message Query Error. Reason: The json is null";
		QString errorString = "查询成员动态时发生错误，获取的JSON值为空！";
		m_logger->error("查询成员动态时发生错误，获取的JSON值为空！");
		//emit errorOccurred(errorString);
		return res;
	}

	/* []操作符理应有异常机制，对异常机制进行处理
	 * JSON_THROW(std::out_of_range("key not found"));
	 */

	if(doc.contains("data")&&doc["data"].contains("cards"))
	{
		Json cardArr = doc["data"]["cards"];
		int uid = cardArr[0]["desc"]["uid"].get<int>();
		QString nickname = QString::fromStdString(cardArr[0]["desc"]["user_profile"]["info"]["uname"].get<std::string>());
		int type;
		QString dynamic_id_str;

		qDebug() << tr("Start query %1 %2's message card.").arg(QString::number(uid)).arg(nickname);
		m_logger->info("开始查询用户UID：{}，昵称：{}的一组动态：",uid,nickname.toStdString());
		for(size_t i=0;i<cardArr.size();++i)
		{
			Json currentCardJson = cardArr[i];
			BilibiliMessageCard currentMessageCard;

			try
			{
				type = currentCardJson["desc"]["type"].get<int>();
				dynamic_id_str = QString::fromStdString(currentCardJson["desc"]["dynamic_id_str"].get<std::string>());

				currentMessageCard.uid = uid;
				currentMessageCard.type = type;
				currentMessageCard.dynamic_id_str = dynamic_id_str;
				currentMessageCard.nickname = nickname;

				qDebug() << "Message Query: " << currentMessageCard.uid << currentMessageCard.nickname << currentMessageCard.type << currentMessageCard.dynamic_id_str;
				m_logger->info("动态消息卡片：UID：{}，昵称：{}，类型：{}，动态ID：{}", currentMessageCard.uid, currentMessageCard.nickname.toStdString(), currentMessageCard.type, currentMessageCard.dynamic_id_str.toStdString());

				res.push_back(currentMessageCard);
			}
			catch (...)
			{
				qDebug() << "Message Query Error. Reason: The json can not be parsed";
				QString errorString = "查询成员动态时发生错误，未能解析获取的JSON！";
				m_logger->error("查询成员动态时发生错误，未能解析获取的JSON！");
				//emit errorOccurred(errorString);
				return res;
			}
		}
	}
	else
	{
		qDebug() << "Message Query Error. Reason: The json can not be parsed";
		QString errorString = "查询成员动态时发生错误，未能解析获取的JSON！";
		m_logger->error("查询成员动态时发生错误，未能解析获取的JSON！");
		//emit errorOccurred(errorString);
		return res;
	}

	return res;
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
		//emit errorOccurred(errorString);
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
		//emit errorOccurred(errorString);
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
		//emit errorOccurred(errorString);
		return j;
	}

	QByteArray buf = reply->readAll();
	if (buf.isNull())
	{
		qDebug() << "No Data return";
		m_logger->error("请求失败，获取了空的返回数据！");
		QString errorString = "请求失败，获取了空的返回数据！";
		//emit errorOccurred(errorString);
		return j;
	}

	j = Json::parse(buf.data(), nullptr, false);  // 不抛出异常
	if(j.is_null())
	{
		qDebug() << "Parse Json Error!"<< url;
		m_logger->error("请求失败，未能成功解析JSON！");
		QString errorString = "请求失败，未能成功解析JSON！";
		//emit errorOccurred(errorString);
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
		//emit errorOccurred(errorString);
		return Json();  // 返回空的JSON
	}
	
	if (retCode != 0)
	{
		qDebug() << "Return Code is not null";
		m_logger->error("请求失败，返回状态码：{}", retCode);
		QString errorString = "请求失败，返回状态码：" + QString::number(retCode);
		//emit errorOccurred(errorString);
		return Json();  // 返回空的JSON
	}

	return j;
}
