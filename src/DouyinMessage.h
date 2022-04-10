#ifndef ASOULNOTIFICATION_DOUYINMESSAGE_H
#define ASOULNOTIFICATION_DOUYINMESSAGE_H

#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/spdlog.h>

#include <QDebug>
#include <QEventLoop>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QObject>
#include <QThread>

#include "Essential.h"
#include "json.hpp"

using Json = nlohmann::json;
using DouyinDynamicRes = QVector<DouyinDynamicCard>;

class DouyinMessage : public QObject {
    Q_OBJECT
public:
    explicit DouyinMessage(std::shared_ptr<spdlog::logger>& logger, QStringList& secIdList, QObject* parent = nullptr);
    ~DouyinMessage();

signals:
    void newDouyinDynamic(QString uid, int type, const QString aweme_id, QString desc);
    void errorOccurred(const QString errorString);
    // overLoad()风控消息
    void overLoadMessage();

public slots:
    [[noreturn]] void startQuery();

private:
    DouyinDynamicRes dynamicQuery(const QString& url);// 每次查询返回一组动态
    Json getJson(const QString& url);

private:
    std::shared_ptr<spdlog::logger>& m_logger;
    const SecUidList& m_secIdList;

    QNetworkAccessManager* m_networkManager;
    QNetworkRequest m_request;

    QMap<QString, QSet<QString>> m_oldMessageCardMap;// 每名用户对应一组旧动态
};


#endif//ASOULNOTIFICATION_DOUYINMESSAGE_H
