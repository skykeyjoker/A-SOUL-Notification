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

    void newErrorMessage(QString message);

    void sayGoodbye();

protected slots:
    void bilibiliDynamicMessageDiscontributor(int user, int type, const QString dynamic_id_str);
    void bilibiliLiveMessageDiscontributor(int user, const QString title, const QString url);

    void douyinDynamicMessageDiscontributor(QString uid, int type, const QString aweme_id, QString desc);

    void errorMessageDiscontributor(int type, const QString &message);

private:
    std::shared_ptr<spdlog::logger>& m_logger;

    QString m_dirPrefix;

    const BiliBiliMemberMap& m_biliBiliMemberMap;
    const DouyinMemberMap& m_douyinMemberMap;

    QThread* m_bilibiliThread;
    BiliBiliMessage* m_biliBiliMessage;

    QThread* m_douyinThread;
    DouyinMessage* m_douyinMessage;

    // 错误计数
    int m_errCountArr[2]; // 0:Bilibili 1:Douyin
};


#endif//ASOULNOTIFICATION_QUERYCENTER_H
