#ifndef ASOULNOTIFICATION_QUERYCENTER_H
#define ASOULNOTIFICATION_QUERYCENTER_H

#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/spdlog.h>

#include <QDebug>
#include <QDir>
#include <QObject>
#include <QThread>
#include <QTimer>

#include "BilibiliMessage.h"
#include "DouyinMessage.h"
#include "Essential.h"

class QueryCenter : public QObject {
    Q_OBJECT
public:
    explicit QueryCenter(std::shared_ptr<spdlog::logger>& logger,
                         QStringList& uidList, QStringList& secIdList,
                         const BiliBiliMemberMap& biliBiliMemberMap, const DouyinMemberMap& douyinMemberMap,
                         QObject* parent = nullptr);
    ~QueryCenter();

public:
    void startQuery();

signals:
    void newBilibiliDynamicMessage(QString message, QString avatar, QString url);
    void newBilibiliLiveMessage(QString message, QString avatar, QString title, QString url);

    void newDouyinDynamicMessage(QString message, QString avatar, QString title, QString url);

    // 风控消息提醒
    void newOverLoadMessage(QString message);

protected slots:
    void bilibiliDynamicMessageDiscontributor(int user, int type, const QString dynamic_id_str);
    void bilibiliLiveMessageDiscontributor(int user, const QString title, const QString url);

    void douyinDynamicMessageDiscontributor(QString uid, int type, const QString aweme_id, QString desc);

    void overLoadMessageDiscontributor(int type);

private:
    std::shared_ptr<spdlog::logger>& m_logger;

    QString m_dirPrefix;

    bool m_enableDouyin{false};

    const BiliBiliMemberMap& m_biliBiliMemberMap;
    const DouyinMemberMap& m_douyinMemberMap;

    QThread* m_bilibiliThread;
    BiliBiliMessage* m_biliBiliMessage;

    QThread* m_douyinThread;
    DouyinMessage* m_douyinMessage;
};


#endif//ASOULNOTIFICATION_QUERYCENTER_H
