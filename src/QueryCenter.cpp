#include "QueryCenter.h"

QueryCenter::QueryCenter(std::shared_ptr<spdlog::logger>& logger,
                         QStringList& uidList, QStringList& secIdList,
                         const BiliBiliMemberMap& biliBiliMemberMap, const DouyinMemberMap& douyinMemberMap,
                         QObject* parent)
    : m_logger(logger),
      m_biliBiliMemberMap(biliBiliMemberMap),
      m_douyinMemberMap(douyinMemberMap),
      QObject(parent) {
    // 初始化成员数据
    m_dirPrefix = QDir::currentPath() + "/";// 目录前缀

    if (!douyinMemberMap.isEmpty())
        m_enableDouyin = true;

    // 初始化查询进程
    m_bilibiliThread = new QThread;
    m_biliBiliMessage = new BiliBiliMessage(m_logger, uidList);
    m_biliBiliMessage->moveToThread(m_bilibiliThread);
    connect(m_bilibiliThread, &QThread::started, m_biliBiliMessage, &BiliBiliMessage::startQuery);
    connect(m_bilibiliThread, &QThread::finished, m_biliBiliMessage, &BiliBiliMessage::deleteLater);

    connect(m_biliBiliMessage, &BiliBiliMessage::newBilibiliMessage, this, &QueryCenter::bilibiliDynamicMessageDiscontributor);
    connect(m_biliBiliMessage, &BiliBiliMessage::newBilibiliLive, this, &QueryCenter::bilibiliLiveMessageDiscontributor);

    // 风控提醒
    connect(m_biliBiliMessage, &BiliBiliMessage::overLoadMessage, this, [this]() {
        overLoadMessageDiscontributor(0);
    });

    if (m_enableDouyin) {
        m_douyinThread = new QThread;
        m_douyinMessage = new DouyinMessage(m_logger, secIdList);
        m_douyinMessage->moveToThread(m_douyinThread);
        connect(m_douyinThread, &QThread::started, m_douyinMessage, &DouyinMessage::startQuery);
        connect(m_douyinThread, &QThread::finished, m_douyinMessage, &DouyinMessage::deleteLater);

        connect(m_douyinMessage, &DouyinMessage::newDouyinDynamic, this, &QueryCenter::douyinDynamicMessageDiscontributor);

        // 风控提醒
        connect(m_douyinMessage, &DouyinMessage::overLoadMessage, this, [this]() {
            overLoadMessageDiscontributor(1);
        });
    }
}

QueryCenter::~QueryCenter() {
}

void QueryCenter::startQuery() {
    // 启动查询进程
    m_bilibiliThread->start();
    if (m_enableDouyin) {
        m_douyinThread->start();
    }
}

void QueryCenter::bilibiliDynamicMessageDiscontributor(int uid, int type, const QString dynamic_id_str) {
    // B站动态消息分发
    QString message = m_biliBiliMemberMap[uid]["nickname"];
    QString avatar = m_dirPrefix + "avatar/" + m_biliBiliMemberMap[uid]["avatar"];
    QString url = BDYNAMICURLPREFIX + dynamic_id_str;

    switch (type) {
        case 8:
            message += "投稿了新视频";
            break;
        case 16:
            message += "投稿了新视频";
            break;
        case 64:
            message += "投稿了新专栏";
            break;
        default:
            message += "有了新动态";
            break;
    }

    emit newBilibiliDynamicMessage(message, avatar, url);
}

void QueryCenter::bilibiliLiveMessageDiscontributor(int uid, const QString title, const QString url) {
    // B站直播消息分发
    QString message = m_biliBiliMemberMap[uid]["nickname"] + "正在直播";
    QString avatar = m_dirPrefix + "avatar/" + m_biliBiliMemberMap[uid]["avatar"];

    emit newBilibiliLiveMessage(message, avatar, title, url);
}

void QueryCenter::douyinDynamicMessageDiscontributor(QString uid, int type, const QString aweme_id, QString desc) {
    // 抖音动态消息分发
    QString message = m_douyinMemberMap[uid]["nickname"];
    QString avatar = m_dirPrefix + "avatar/" + m_douyinMemberMap[uid]["avatar"];
    QString url = DOUYINDYNAMICPREFIX + aweme_id;

    switch (type) {
        case 4:
            message += "投稿了新视频";
            break;
        default:
            message += "有了新动态";
            break;
    }

    emit newDouyinDynamicMessage(message, avatar, desc, url);
}

// 风控消息
void QueryCenter::overLoadMessageDiscontributor(int type) {
    QString errorString;
    switch (type) {
        case 0:
            errorString = "【哔哩哔哩】";
            break;
        case 1:
            errorString = "【抖音】";
            break;
        default:
            break;
    }

    errorString += "检测到服务器风控，线程休眠30min。";
    emit newOverLoadMessage(errorString);
}
