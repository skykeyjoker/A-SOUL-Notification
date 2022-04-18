#include "DouyinMessage.h"

DouyinMessage::DouyinMessage(std::shared_ptr<spdlog::logger> &logger, QStringList &secIdList, QObject *parent)
    : m_logger(logger),
      m_secIdList(secIdList),
      QObject(parent) {
    qDebug() << "【抖音】DouyinMessage Starting...";
    m_logger->info("【抖音】抖音查询模块初始化。");

    // QNetwork
    m_networkManager = new QNetworkAccessManager(this);

    qDebug() << "【抖音】DouyinMessage Started.";
    m_logger->info("【抖音】抖音查询模块初始化成功。");
}

DouyinMessage::~DouyinMessage() {
}

[[noreturn]] void DouyinMessage::startQuery() {
    /* 30s/次查询 */
    qDebug() << "【抖音】Start one query.";
    m_logger->info("【抖音】抖音查询模块开始查询。");

    int errCnt = 0;

    while (1) {
        m_logger->info("【抖音】抖音查询模块开始一次查询。");
        m_logger->info("【抖音】开始查询动态。");
        for (int userCnt = 0; userCnt < m_secIdList.size(); ++userCnt) {
            QString url = QString(DOUYINDYNAMICQUERY) + m_secIdList[userCnt];
            DouyinDynamicRes res = dynamicQuery(url);// 查询一组动态
            if (!res.empty()) {
                // 判断当前是否为首次查询
                if (!m_oldMessageCardMap.contains(res[0].uid)) {
                    // 首次查询，填充oldMessageCardMap
                    QSet<QString> messageSet;
                    for (int i = 0; i < res.size(); ++i) {
                        messageSet.insert(res[i].aweme_id);
                    }
                    m_oldMessageCardMap.insert(res[0].uid, messageSet);

                    qDebug() << messageSet.size() << messageSet;
                } else {
                    // 非首次查询
                    for (int i = 0; i < res.size(); ++i)// 遍历每条动态
                    {
                        const auto &currentMessageCard = res[i];
                        if (!m_oldMessageCardMap[currentMessageCard.uid].contains(currentMessageCard.aweme_id)) {
                            // 新动态
                            m_oldMessageCardMap[currentMessageCard.uid].insert(currentMessageCard.aweme_id);                                             // 更新oldMessageMap
                            emit newDouyinDynamic(currentMessageCard.uid, currentMessageCard.type, currentMessageCard.aweme_id, currentMessageCard.desc);// 发送新动态消息

                            qDebug() << m_oldMessageCardMap[currentMessageCard.uid].size() << m_oldMessageCardMap[currentMessageCard.uid].size();
                        }
                    }
                }
            }
            // 延时
            QThread::sleep(1);
        }
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
        //emit newDouyinDynamic("ASOULofficial", 4, "7078993288186907940", "身为女团一定要懂得察言观色、坚持不懈、先发制人 ，所以我们今天练习…砍价！#轻漫计划 #充能计划 ");
        qDebug() << "【抖音】One query end.";
        m_logger->info("【抖音】哔哩哔哩查询模块结束一次查询。");
        QThread::sleep(CRON);
    }
}

DouyinDynamicRes DouyinMessage::dynamicQuery(const QString &url) {
    DouyinDynamicRes res;

    Json doc = getJson(url);
    if (doc.is_null()) {
        qDebug() << "【抖音】Message Query Error. Reason: The json is null";
        m_logger->error("【抖音】查询成员动态时发生错误，获取的JSON值为空！");
        return res;
    }

    /* []操作符理应有异常机制，对异常机制进行处理
	 * JSON_THROW(std::out_of_range("key not found"));
	 */
    try {
        Json aweme_list = doc["aweme_list"];

        QString uid = QString::fromStdString(aweme_list[0]["author"]["unique_id"].get<std::string>());
        QString nickname = QString::fromStdString(aweme_list[0]["author"]["nickname"].get<std::string>());

        int type;
        QString desc;
        QString aweme_id;

        qDebug() << tr("【抖音】Start query %1 %2's message card.").arg(uid).arg(nickname);
        m_logger->info("【抖音】开始查询用户UID：{}，昵称：{}的一组动态：", uid.toStdString(), nickname.toStdString());

        for (size_t i = 0; i < aweme_list.size(); ++i) {
            Json currentDynamicCardJson = aweme_list[i];
            DouyinDynamicCard currentDynamicCard;

            type = currentDynamicCardJson["aweme_type"].get<int>();
            aweme_id = QString::fromStdString(currentDynamicCardJson["aweme_id"].get<std::string>());
            desc = QString::fromStdString(currentDynamicCardJson["desc"].get<std::string>());

            currentDynamicCard.uid = uid;
            currentDynamicCard.type = type;
            currentDynamicCard.aweme_id = aweme_id;
            currentDynamicCard.nickname = nickname;
            currentDynamicCard.desc = desc;

            qDebug() << "【抖音】Message Query: " << currentDynamicCard.uid << currentDynamicCard.nickname
                     << currentDynamicCard.type << currentDynamicCard.aweme_id << currentDynamicCard.desc;
            m_logger->info("【抖音】动态消息卡片：UID：{}，昵称：{}，类型：{}，动态ID：{}，标题：{}",
                           currentDynamicCard.uid.toStdString(),
                           currentDynamicCard.nickname.toStdString(),
                           currentDynamicCard.type,
                           currentDynamicCard.aweme_id.toStdString(),
                           currentDynamicCard.desc.toStdString());

            res.push_back(currentDynamicCard);
        }
    } catch (Json::exception &ex) {
        qDebug() << "【抖音】Message Query Error. Exception occurred when parsing the json. Exception:{}" << ex.what();
        m_logger->error("【抖音】查询成员动态时发生错误，解析获取的JSON时发生异常：{}", ex.what());
        return res;
    } catch (...) {
        qDebug() << "【抖音】Message Query Error. Unknown exception occurred when parsing the json.";
        m_logger->error("【抖音】查询成员动态时发生错误，解析获取的JSON时发生未知异常");
        return res;
    }

    return res;
}

Json DouyinMessage::getJson(const QString &url) {
    Json j;
    m_request.setUrl(QUrl(url));

    QEventLoop eventLoop;
    connect(m_networkManager, SIGNAL(finished(QNetworkReply *)), &eventLoop, SLOT(quit()));
    QNetworkReply *reply = m_networkManager->get(m_request);
    eventLoop.exec();

    if (reply->error() != QNetworkReply::NoError) {
        qDebug() << "【抖音】Query Error! Reason: request error" << reply->error() << reply->errorString() << url;
        m_logger->error("【抖音】请求失败，错误类型：{}，{}", (int) reply->error(), reply->errorString().toStdString());

        switch (reply->error()) {
            default:
                break;
        }

        reply->deleteLater();
        return j;
    }

    QByteArray buf = reply->readAll();
    reply->deleteLater();
    if (buf.isNull()) {
        qDebug() << "【抖音】No Data return";
        m_logger->error("【抖音】请求失败，获取了空的返回数据！");
        return j;
    }

    j = Json::parse(buf.data(), nullptr, false);// 不抛出异常
    if (j.is_null()) {
        qDebug() << "【抖音】Parse Json Error!" << url;
        m_logger->error("【抖音】请求失败，未能成功解析JSON！");
        return j;
    }

    // 判断返回code状态码
    int retCode;
    try {
        retCode = j["status_code"].get<int>();
    } catch (Json::exception &ex) {
        qDebug() << "【抖音】Parse Json Error! Can not get the [status_code]. Exception:" << ex.what();
        m_logger->error("【抖音】请求失败，解析JSON[status_code]时出现异常：{}", ex.what());
        return Json();// 返回空的JSON
    } catch (...) {
        qDebug() << "【抖音】Parse Json Error! Can not get the [status_code]. Unknown Exception.";
        m_logger->error("【抖音】请求失败，解析JSON[status_code]时出现未知异常。");
        return Json();// 返回空的JSON
    }

    if (retCode != 0) {
        qDebug() << "【抖音】Return Code Error" << retCode;
        m_logger->error("【抖音】请求失败，返回状态码：{}", retCode);
        return Json();// 返回空的JSON
    }

    return j;
}
