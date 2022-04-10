#include "BilibiliMessage.h"

BiliBiliMessage::BiliBiliMessage(std::shared_ptr<spdlog::logger>& logger, QStringList& uidList, QObject* parent) : m_uidList(uidList),
                                                                                                                   m_logger(logger),
                                                                                                                   QObject(parent) {
    qDebug() << "【哔哩哔哩】BilibiliMessage Starting...";
    m_logger->info("【哔哩哔哩】哔哩哔哩查询模块初始化。");

    // 初始化livestatusmap
    for (int i = 0; i < uidList.size(); ++i)
        m_liveStatusMap[uidList[i].toInt()] = 0;

    // QNetwork
    m_networkManager = new QNetworkAccessManager(this);

    qDebug() << "【哔哩哔哩】BilibiliMessage Started.";
    m_logger->info("【哔哩哔哩】哔哩哔哩查询模块初始化成功。");
}

[[noreturn]] void BiliBiliMessage::startQuery() {
    /* 30s/次查询 */
    qDebug() << "【哔哩哔哩】Start one query.";
    m_logger->info("【哔哩哔哩】哔哩哔哩查询模块开始查询。");

    while (1) {
        m_logger->info("【哔哩哔哩】哔哩哔哩查询模块开始一次查询。");
        m_logger->info("【哔哩哔哩】开始查询动态。");
        for (int userCnt = 0; userCnt < m_uidList.size(); ++userCnt) {
            QString url = QString(BDYNAMICQUERY) + m_uidList[userCnt];
            BiliBiliMessageRes res = messageQuery(url);// 查询一组动态
            if (!res.empty()) {
                // 判断当前是否为首次查询
                if (!m_oldMessageCardMap.contains(res[0].uid)) {
                    // 首次查询，填充oldMessageCardMap
                    QSet<QString> messageSet;
                    for (int i = 0; i < res.size(); ++i) {
                        messageSet.insert(res[i].dynamic_id_str);
                    }
                    m_oldMessageCardMap.insert(res[0].uid, messageSet);

                    qDebug() << messageSet.size() << messageSet;
                } else {
                    // 非首次查询
                    for (int i = 0; i < res.size(); ++i)// 遍历每条动态
                    {
                        const auto& currentMessageCard = res[i];
                        if (!m_oldMessageCardMap[currentMessageCard.uid].contains(currentMessageCard.dynamic_id_str)) {
                            // 新动态
                            m_oldMessageCardMap[currentMessageCard.uid].insert(currentMessageCard.dynamic_id_str);                      // 更新oldMessageMap
                            emit newBilibiliMessage(currentMessageCard.uid, currentMessageCard.type, currentMessageCard.dynamic_id_str);// 发送新动态消息

                            qDebug() << m_oldMessageCardMap[currentMessageCard.uid].size() << m_oldMessageCardMap[currentMessageCard.uid].size();
                        }
                    }
                }
            }
            // 延时
            QThread::sleep(1);
        }

        m_logger->info("开始查询直播。");
        for (int i = 0; i < m_uidList.size(); ++i) {
            QString url = QString(BLIVEQUERY) + m_uidList[i];
            BilibiliLiveCard card = liveQuery(url);
            if (!card.is_null) {
                if (card.status != m_liveStatusMap[card.mid]) {
                    // 与历史状态不同
                    if (card.status == 1)// 正在直播
                        emit newBilibiliLive(card.mid, card.title, card.url);

                    m_liveStatusMap[card.mid] = card.status;// 更新liveStatusMap
                }
            }
            // 延时
            QThread::sleep(1);
        }
        qDebug() << "【哔哩哔哩】One query end.";
        m_logger->info("【哔哩哔哩】哔哩哔哩查询模块结束一次查询。");
        QThread::sleep(CRON);
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
        //emit newBilibiliLive(672346917, "【3D】是谁在觊觎枝江GAMER!", "https://live.bilibili.com/22625025");
    }
}

BiliBiliMessageRes BiliBiliMessage::messageQuery(const QString& url) {
    BiliBiliMessageRes res;

    Json doc = getJson(url);
    if (doc.is_null()) {
        qDebug() << "【哔哩哔哩】Message Query Error. Reason: The json is null";
        m_logger->error("【哔哩哔哩】查询成员动态时发生错误，获取的JSON值为空！");
        return res;
    }

    /* []操作符理应有异常机制，对异常机制进行处理
	 * JSON_THROW(std::out_of_range("key not found"));
	 */

    if (doc.contains("data") && doc["data"].contains("cards")) {
        Json cardArr = doc["data"]["cards"];
        int uid = cardArr[0]["desc"]["uid"].get<int>();
        QString nickname = QString::fromStdString(cardArr[0]["desc"]["user_profile"]["info"]["uname"].get<std::string>());
        int type;
        QString dynamic_id_str;

        qDebug() << tr("【哔哩哔哩】Start query %1 %2's message card.").arg(QString::number(uid)).arg(nickname);
        m_logger->info("【哔哩哔哩】开始查询用户UID：{}，昵称：{}的一组动态：", uid, nickname.toStdString());
        for (size_t i = 0; i < cardArr.size(); ++i) {
            Json currentCardJson = cardArr[i];
            BilibiliDynamicCard currentMessageCard;

            try {
                type = currentCardJson["desc"]["type"].get<int>();
                dynamic_id_str = QString::fromStdString(currentCardJson["desc"]["dynamic_id_str"].get<std::string>());

                currentMessageCard.uid = uid;
                currentMessageCard.type = type;
                currentMessageCard.dynamic_id_str = dynamic_id_str;
                currentMessageCard.nickname = nickname;

                qDebug() << "【哔哩哔哩】Message Query: " << currentMessageCard.uid << currentMessageCard.nickname << currentMessageCard.type << currentMessageCard.dynamic_id_str;
                m_logger->info("【哔哩哔哩】动态消息卡片：UID：{}，昵称：{}，类型：{}，动态ID：{}", currentMessageCard.uid, currentMessageCard.nickname.toStdString(), currentMessageCard.type, currentMessageCard.dynamic_id_str.toStdString());

                res.push_back(currentMessageCard);
            } catch (Json::exception& ex) {
                qDebug() << "【哔哩哔哩】Message Query Error. Exception occurred when parsing the json. Exception:" << ex.what();
                m_logger->error("【哔哩哔哩】查询成员动态时发生错误，解析获取的JSON时发生异常：{}", ex.what());
                return res;
            } catch (...) {
                qDebug() << "【哔哩哔哩】Message Query Error. Unknown Exception occurred when parsing the json.";
                m_logger->error("【哔哩哔哩】查询成员动态时发生错误，解析获取的JSON时发生未知异常");
                return res;
            }
        }
    } else {
        qDebug() << "【哔哩哔哩】Message Query Error. Reason: The json can not be parsed. [data]or[cards] not exists.";
        m_logger->error("【哔哩哔哩】查询成员动态时发生错误，未能解析获取的JSON，[data]或[cards]字段不存在。");
        return res;
    }

    return res;
}

BilibiliLiveCard BiliBiliMessage::liveQuery(const QString& url) {
    BilibiliLiveCard ret;
    ret.is_null = true;

    Json doc = getJson(url);
    if (doc.is_null()) {
        qDebug() << "【哔哩哔哩】Live Query Error. Reason: The json is null";
        m_logger->error("【哔哩哔哩】查询成员直播状态时发生错误，获取的JSON值为空！");
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
    try {
        mid = doc["data"]["mid"].get<int>();
        status = doc["data"]["live_room"]["liveStatus"].get<int>();
        title = QString::fromStdString(doc["data"]["live_room"]["title"].get<std::string>());
        nickname = QString::fromStdString(doc["data"]["name"].get<std::string>());
        liveurl = QString::fromStdString(doc["data"]["live_room"]["url"].get<std::string>());
        // Fuck You Bilibili, live room url TRAITS https://live.bilibili.com/22625027?broadcast_type=0\u0026is_room_feed=1
        liveurl = liveurl.mid(0, liveurl.indexOf('?'));
    } catch (Json::exception& ex) {
        qDebug() << "【哔哩哔哩】Live Query Error, Exception occurred when parsing the json. Exception:" << ex.what();
        m_logger->error("【哔哩哔哩】查询成员直播状态时发生错误，解析获取的JSON时发生异常：{}", ex.what());
        return ret;
    } catch (...) {
        qDebug() << "【哔哩哔哩】Live Query Error, Unknown Exception occurred when parsing the json.";
        m_logger->error("【哔哩哔哩】查询成员直播状态时发生错误，解析获取的JSON时发生未知异常");
        return ret;
    }

    ret.mid = mid;
    ret.status = status;
    ret.title = title;
    ret.nickname = nickname;
    ret.url = liveurl;
    ret.is_null = false;

    qDebug() << "【哔哩哔哩】Live Query: " << ret.mid << ret.nickname << ret.status << ret.title << ret.url;
    m_logger->info("【哔哩哔哩】直播消息卡片：MID：{}，昵称：{}，状态：{}，标题：{}，网址：{}", ret.mid, ret.nickname.toStdString(), ret.status, ret.title.toStdString(), ret.url.toStdString());

    return ret;
}

Json BiliBiliMessage::getJson(const QString& url) {
    Json j;
    m_request.setUrl(QUrl(url));

    QEventLoop eventLoop;
    connect(m_networkManager, SIGNAL(finished(QNetworkReply*)), &eventLoop, SLOT(quit()));
    QNetworkReply* reply = m_networkManager->get(m_request);
    eventLoop.exec();

    if (reply->error() != QNetworkReply::NoError) {
        qDebug() << "【哔哩哔哩】Query Error! Reason: request error" << reply->error() << reply->errorString() << url;
        m_logger->error("【哔哩哔哩】请求失败，错误类型：{}，{}", (int) reply->error(), reply->errorString().toStdString());

        switch (reply->error()) {
            case QNetworkReply::RemoteHostClosedError: {
                // Connection Closed
                qDebug() << "【哔哩哔哩】请求时服务器关闭了链接，线程睡眠3s。";
                m_logger->error("【哔哩哔哩】请求时服务器关闭了链接，线程睡眠3s。");
                QThread::sleep(3);
                qDebug() << "【哔哩哔哩】从请求时发生的链接关闭错误恢复";
                m_logger->info("【哔哩哔哩】从请求时发生的链接关闭错误恢复");
                break;
            }

            case QNetworkReply::ProxyConnectionRefusedError:
            case QNetworkReply::ProxyConnectionClosedError:
            case QNetworkReply::ProxyNotFoundError:
            case QNetworkReply::ProxyTimeoutError:
            case QNetworkReply::ProxyAuthenticationRequiredError:
            case QNetworkReply::UnknownProxyError: {
                // Proxy Error 代理错误
                qDebug() << "【哔哩哔哩】请求时发生代理错误，线程睡眠3s。";
                m_logger->error("【哔哩哔哩】请求时发生代理错误，线程睡眠3s。");
                QThread::sleep(3);
                qDebug() << "【哔哩哔哩】从请求时发生的代理错误恢复。";
                m_logger->info("【哔哩哔哩】从请求时发生的代理错误恢复。");
                break;
            }

            case QNetworkReply::UnknownContentError: {
                // Precondition Failed 风控
                if (reply->errorString().contains("Error transferring")) {
                    qDebug() << "【哔哩哔哩】请求时检测到风控，线程睡眠30min。";
                    m_logger->error("【哔哩哔哩】请求时检测到风控，线程睡眠30min。");
                    emit overLoadMessage();// 发送风控消息
                    QThread::sleep(60 * 10 * 3);
                    qDebug() << "【哔哩哔哩】从风控状态恢复。";
                    m_logger->info("【哔哩哔哩】从风控状态恢复。");
                }
                break;
            }

            default:
                break;
        }

        reply->deleteLater();
        return j;
    }

    QByteArray buf = reply->readAll();
    reply->deleteLater();
    if (buf.isNull()) {
        qDebug() << "【哔哩哔哩】No Data return";
        m_logger->error("【哔哩哔哩】请求失败，获取了空的返回数据！");
        return j;
    }

    j = Json::parse(buf.data(), nullptr, false);// 不抛出异常
    if (j.is_null()) {
        qDebug() << "【哔哩哔哩】Parse Json Error!" << url;
        m_logger->error("【哔哩哔哩】请求失败，未能成功解析JSON！");
        return j;
    }

    // 判断返回code状态码
    int retCode;
    try {
        retCode = j["code"].get<int>();
    } catch (Json::exception& ex) {
        qDebug() << "【哔哩哔哩】Parse Json Error! Exception occurred whe getting the [code]. Exception:" << ex.what();
        m_logger->error("【哔哩哔哩】请求失败，解析JSON[code]时发生异常：{}！", ex.what());
        return Json();// 返回空的JSON
    } catch (...) {
        qDebug() << "【哔哩哔哩】Parse Json Error! Exception occurred whe getting the [code]. Unknown Exception:";
        m_logger->error("【哔哩哔哩】请求失败，解析JSON[code]时发生未知异常。");
        return Json();// 返回空的JSON
    }

    if (retCode != 0) {
        qDebug() << "【哔哩哔哩】Return Code Error" << retCode;
        m_logger->error("【哔哩哔哩】请求失败，返回状态码：{}", retCode);
        return Json();// 返回空的JSON
    }

    return j;
}
