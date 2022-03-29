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
    m_dirPrefix = QDir::currentPath() + "/";  // 目录前缀
    for (int i = 0; i < 2; ++i)  // 初始化错误计数
    {
        m_errCountArr[i] = 0;
    }

    // 初始化查询进程
    m_bilibiliThread = new QThread;
    m_biliBiliMessage = new BiliBiliMessage(m_logger, uidList);
    m_biliBiliMessage->moveToThread(m_bilibiliThread);

    m_douyinThread = new QThread;
    m_douyinMessage = new DouyinMessage(m_logger, secIdList);
    m_douyinMessage->moveToThread(m_douyinThread);

    connect(m_bilibiliThread, &QThread::started, m_biliBiliMessage, &BiliBiliMessage::startQuery);
    connect(m_bilibiliThread, &QThread::finished, m_biliBiliMessage, &BiliBiliMessage::deleteLater);
    connect(m_douyinThread, &QThread::started, m_douyinMessage, &DouyinMessage::startQuery);
    connect(m_douyinThread, &QThread::finished, m_douyinMessage, &DouyinMessage::deleteLater);

    // 绑定新动态/直播信号
    connect(m_biliBiliMessage, &BiliBiliMessage::newBilibiliMessage, this, &QueryCenter::bilibiliDynamicMessageDiscontributor);
    connect(m_biliBiliMessage, &BiliBiliMessage::newBilibiliLive, this, &QueryCenter::bilibiliLiveMessageDiscontributor);
    connect(m_douyinMessage, &DouyinMessage::newDouyinDynamic, this, &QueryCenter::douyinDynamicMessageDiscontributor);

    // 绑定错误信号
    connect(m_biliBiliMessage, &BiliBiliMessage::errorOccurred, this, [this](QString error) {
        errorMessageDiscontributor(0, error);
    });
    connect(m_douyinMessage, &DouyinMessage::errorOccurred, this, [this](QString error) {
        errorMessageDiscontributor(1, error);
    });
}

QueryCenter::~QueryCenter() {
}

void QueryCenter::startQuery() {
    // 启动查询进程
    m_bilibiliThread->start();
    m_douyinThread->start();
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

void QueryCenter::errorMessageDiscontributor(int type, const QString& message) {
    QString typeString;
    switch (type) {
        case 0:
            typeString = "【哔哩哔哩】";
            break;
        case 1:
            typeString = "【抖音】";
            break;
        default:
            break;
    }

    qDebug() << typeString << "Error Occurred Signal...";

    m_errCountArr[type]++;
    qDebug() << m_errCountArr[type];

    if (m_errCountArr[type] < 3) {
        emit newErrorMessage(message);
    } else if (m_errCountArr[type] == 3) {
        // 错误次数达到3次及以上，停止发送错误卡片
        qDebug() << "Type:" << typeString
                 << "错误信号数已达到3次，停止发送错误提醒。";
        m_logger->error(QString(typeString + "错误信号数已达到3次，停止发送错误提醒。").toStdString());
        emit newErrorMessage("插件错误达到3次，停止错误提醒。\n10分钟后错误提醒计数会自动归零，此后错误提醒会正常发送。");

        qDebug() << typeString << "错误信号数清零计时开始";
        m_logger->info(QString(typeString + "错误信号数清零计时开始").toStdString());
        QTimer::singleShot(1000*60*3, [type, typeString, this]() {
            qDebug() << typeString << "错误信号数清零计时达10分钟";
            m_logger->info(QString(typeString + "错误信号数清零计时达10分钟").toStdString());
            m_errCountArr[type] = 0;
        });

    } else if (m_errCountArr[type] >= 6) {
        // 错误次数达到6次及以上，退出程序
        qDebug() << typeString << "错误信号数已达到6次及以上，发送错误提醒并退出程序。";
        m_logger->error(QString(typeString + "错误信号数已达到6次及以上，发送错误提醒并退出程序。").toStdString());
        emit newErrorMessage("错误信号数已达到6次及以上，发送错误提醒并退出程序。");
        emit sayGoodbye();
    }
}
