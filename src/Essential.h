#ifndef ESSENTIAL_H
#define ESSENTIAL_H

#include <QStringList>

const QString BMURLPREFIX("https://api.vc.bilibili.com/dynamic_svr/v1/dynamic_svr/space_history?host_uid=");
const QString BLURLPREFIX("https://api.bilibili.com/x/space/acc/info?mid=");
const QString BDYNAMICURLPREFIX("https://t.bilibili.com/");

typedef struct BilibiliMessageCard
{
	QString nickname;
	QString dynamic_id_str;
	int uid;
	int type;
	bool is_null;
}BilibiliMessageCard;

typedef struct BilibiliLiveCard
{
	QString nickname;
	QString title;
	int mid;
	int status;
	QString url;
	bool is_null;
}BilibiliLiveCard;


#endif /* ESSENTIAL_H */