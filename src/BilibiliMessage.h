#ifndef BILIBILIMESSAGE_H
#define BILIBILIMESSAGE_H

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
using BiliBiliMessageRes = QVector<BilibiliDynamicCard>;

class BiliBiliMessage : public QObject {
    Q_OBJECT
public:
    BiliBiliMessage(std::shared_ptr<spdlog::logger>& logger, QStringList& uidList, QObject* parent = nullptr);

public slots:
    void startQuery();

signals:
    void newBilibiliMessage(int user, int type, const QString dynamic_id_str);
    void newBilibiliLive(int user, const QString title, const QString url);
    void errorOccurred(const QString errorString);

private:
    BiliBiliMessageRes messageQuery(const QString& url);// ÿ�β�ѯ����һ�鶯̬
    BilibiliLiveCard liveQuery(const QString& url);
    Json getJson(const QString& url);

private:
    QNetworkAccessManager* m_networkManager;
    QNetworkRequest m_request;
    QMap<int, QSet<QString>> m_oldMessageCardMap;// ÿ���û���Ӧһ��ɶ�̬
    QMap<int, int> m_liveStatusMap;              // ÿ���û���Ӧһ��ֱ��״̬
    std::shared_ptr<spdlog::logger>& m_logger;
    const UidList& m_uidList;
};


#endif /* BILIBILIMESSAGE_H */