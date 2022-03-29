#ifndef ESSENTIAL_H
#define ESSENTIAL_H

#include <QHash>
#include <QString>
#include <QStringList>

const QString VERSION = "3.0.0";

const QString BDYNAMICQUERY("https://api.vc.bilibili.com/dynamic_svr/v1/dynamic_svr/space_history?host_uid=");
const QString BLIVEQUERY("https://api.bilibili.com/x/space/acc/info?mid=");
const QString BDYNAMICURLPREFIX("https://t.bilibili.com/");
const QString DOUYINDYNAMICQUERY("https://www.iesdouyin.com/web/api/v2/aweme/post/?sec_uid=");
const QString DOUYINDYNAMICPREFIX("https://www.douyin.com/video/");

const int CRON = 30; // 30s

const int MAX_LOG_SIZE = 1048576;  // 1MB
const int MAX_LOG_NUM= 10;  // 10 Log files

using BiliBiliMemberMap = QHash<int, QHash<QString, QString>>;  // B站用户组{uid:{"nickname", "avatar"}}
using DouyinMemberMap = QHash<QString, QHash<QString, QString>>;// 抖音用户组{"uid":{"nickname", "avatar"}}
using UidList = QStringList;                                    // B站用户uid列表
using SecUidList = QStringList;                                 // 抖音用户sec_uid列表

typedef struct BilibiliDynamicCard {
    QString nickname;
    QString dynamic_id_str;
    int uid;
    int type;
} BilibiliDynamicCard;

typedef struct BilibiliLiveCard {
    QString nickname;
    QString title;
    int mid;
    int status;
    QString url;
    bool is_null;
} BilibiliLiveCard;

typedef struct DouyinDynamicCard {
    QString nickname;
    QString uid;
    int type;
    QString aweme_id;
    QString desc;
} DouyinDynamicCard;

#endif /* ESSENTIAL_H */