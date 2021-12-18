#ifndef BILIBILIMESSAGE_H
#define BILIBILIMESSAGE_H

#include "Essential.h"
#include "json.hpp"

#include <QObject>
#include <QtCore>
#include <QEventLoop>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QTimer>
#include <QDebug>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/basic_file_sink.h>

using Json = nlohmann::json;
using BiliBiliMessageRes=QVector<BilibiliMessageCard>;

class BiliBiliMessage: public QObject
{
	Q_OBJECT
public:
	BiliBiliMessage(std::shared_ptr<spdlog::logger>& logger, QStringList &uidList, QObject* parent = nullptr);
	void startQuery();

signals:
	void newBilibiliMessage(int user, int type, const QString dynamic_id_str);
	void newBilibiliLive(int user, const QString title, const QString url);
	void errorOccurred(const QString errorString);

private:
	BiliBiliMessageRes messageQuery(const QString& url);  // ÿ�β�ѯ����һ�鶯̬
	BilibiliLiveCard liveQuery(const QString& url);
	Json getJson(const QString& url);

private:
	QMap<int, QSet<QString>> m_oldMessageCardMap; // ÿ���û���Ӧһ��ɶ�̬
	QMap<int, int> m_liveStatusMap;  // ÿ���û���Ӧһ��ֱ��״̬
	std::shared_ptr<spdlog::logger>& m_logger;
	QStringList &m_uidList;
	int m_timeOut=5;
};




#endif /* BILIBILIMESSAGE_H */