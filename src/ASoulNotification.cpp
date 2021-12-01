#include "BilibiliMessage.h"
#include "wintoastlib.h"
#include "json.hpp"

#include <QCoreApplication>
#include <QProcess>
#include <QEventLoop>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QTimer>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <chrono>

#pragma comment( linker, "/subsystem:\"windows\" /entry:\"mainCRTStartup\"" ) // Hide app

using namespace WinToastLib;

const QString VERSION = "1.5.0";

static int errCount = 0;

class CustomHandler : public IWinToastHandler {
public:
    CustomHandler(const QString url = "") : m_url(url)
	{}

    void toastActivated() const {
        std::wcout << L"The user clicked in this toast" << std::endl;
    }

    void toastActivated(int actionIndex) const {
        std::wcout << L"The user clicked on button #" << actionIndex << L" in this toast" << std::endl;
    	QProcess::startDetached("explorer " + m_url);
    }

    void toastFailed() const {
        std::wcout << L"Error showing current toast" << std::endl;
    }
    void toastDismissed(WinToastDismissalReason state) const {
        switch (state) {
        case UserCanceled:
            std::wcout << L"The user dismissed this toast" << std::endl;
            break;
        case ApplicationHidden:
            std::wcout << L"The application hid the toast using ToastNotifier.hide()" << std::endl;
            break;
        case TimedOut:
            std::wcout << L"The toast has timed out" << std::endl;
            break;
        default:
            std::wcout << L"Toast not activated" << std::endl;
            break;
        }
    }
private:
    QString m_url;
};

QString checkForUpdate(const QString& version, std::shared_ptr<spdlog::logger> &logger)
{
    QString retUrlStr;
    Json j;
    QNetworkAccessManager manager;
    QNetworkRequest request;
    request.setUrl(QUrl("https://cdn.jsdelivr.net/gh/skykeyjoker/A-Soul-Notification/version.json"));

    QEventLoop eventLoop;
    QObject::connect(&manager, SIGNAL(finished(QNetworkReply*)), &eventLoop, SLOT(quit()));
    QNetworkReply* reply = manager.get(request);
    eventLoop.exec();

    if(reply->error()!=QNetworkReply::NoError)
    {
        qDebug() << "Can not get update from github.";
        logger->error("Update Error. Can not get update from github.");

        return retUrlStr;
    }
    else
    {
        QByteArray buf = reply->readAll();
        if (buf.isNull())
        {
            qDebug() << "Update info is null.";
            logger->error("Update Error. Update info is null.");
            return retUrlStr;
        }
        else
        {
            j = Json::parse(buf.data(), nullptr, false);  // 不抛出异常
            if (j.is_null())
            {
                qDebug() << "Update info json is null.";
                logger->error("Update Error. Update info json is null.");
                return retUrlStr;
            }
            else
            {

                QString remoteVersion;
                QString remoteUrl;
                try
                {
                    remoteVersion = QString::fromStdString(j["versions"][0]["version"].get<std::string>());
                    remoteUrl = QString::fromStdString(j["versions"][0]["url"].get<std::string>());
                }
                catch (...)
                {
                    qDebug() << "Update info json can not be parsed.";
                    logger->error("Update Error. Update info json can not be parsed.");
                    return retUrlStr;
                }

                qDebug() << remoteUrl << remoteVersion<< QString("v" + version);
                logger->info("Romote Version: {}, Remote Url: {}", remoteVersion.toStdString(), remoteUrl.toStdString());
                if (QString("v" + version).compare(remoteVersion) != 0) // 本地版本与远程版本不同
                {
                    retUrlStr = remoteUrl;
                }
            	
            }
        }
    }

    return retUrlStr;
}

int main(int argc, char* argv[])
{
	QCoreApplication app(argc, argv);

    // 初始化logs文件夹
    QDir logDir;
    logDir.setPath(app.applicationDirPath());
    if (logDir.exists("logs"))
    {
        if (logDir.exists("logs/log.txt"))
        {
            logDir.remove("logs/log.txt");
        }
    }
    else
        logDir.mkdir("logs");

    // 初始化spdlog
    auto main_logger = spdlog::basic_logger_mt("main_logger", "logs/log.txt");
    main_logger->set_level(spdlog::level::debug);
    main_logger->set_pattern("[%Y-%m-%d %H:%M:%S] [%^%L%$] [thread %t] %v");
    spdlog::flush_every(std::chrono::seconds(3));

	qDebug() << "A-SOUL Notification is Running!";
    main_logger->info("A-SOUL提醒小助手启动！");

    /* 检测重复运行*/
    QProcess process;
    QString processName = "ASoulNotification.exe";
    process.start("tasklist", QStringList() << "-fi" << "imagename eq " + processName);
    process.waitForFinished();
    QString outputStr = QString::fromLocal8Bit(process.readAllStandardOutput());
    if(outputStr.count(processName)>1)
    {
        qDebug("已有另外一个实例处于运行中！");
        main_logger->error("已有另外一个实例处于运行中！");
        main_logger->flush();

        app.exit(1);
        return 1;
    }

    /* 初始化应用信息 */
    app.setApplicationName("A-SOUL Notification");
    app.setApplicationVersion(VERSION);
    /* init wintoast */
    WinToast::instance()->setAppName(L"A-SOUL Notification");
    WinToast::instance()->setAppUserModelId(
        WinToast::configureAUMI(L"Skykey", L"A-SOUL Notification", L"A-SOUL Notification", VERSION.toStdWString()));
    if (!WinToast::instance()->initialize()) {
        qDebug() << "Error, your system in not compatible!";
        main_logger->error("系统不支持WinToast！");
    }

    /* 应用启动显示提示信息 */
    //ImageAndText01
    WinToastTemplate runInfo = WinToastTemplate(WinToastTemplate::ImageAndText01);
    runInfo.setImagePath(QString(app.applicationDirPath() + "/" + "avatar/icon.png").toStdWString());
    runInfo.setTextField(L"A-SOUL提醒小助手运行中！", WinToastTemplate::FirstLine);
    runInfo.setExpiration(0);
    runInfo.setAudioPath(WinToastTemplate::AudioSystemFile::DefaultSound);
    runInfo.setAudioOption(WinToastTemplate::AudioOption::Default);
    if (WinToast::instance()->showToast(runInfo, new CustomHandler(0)) < 0) {
        //QMessageBox::warning(this, "Error", "Could not launch your toast notification!");
        qDebug() << "Could not launch your toast notification!";
        main_logger->error("启动信息Wintoast启动失败！");
    }
    main_logger->info("A-SOUL提醒小助手启动成功！");

    /* 检查更新 */
    QString retUrlStr = checkForUpdate(app.applicationVersion(), main_logger);
    //QString retUrlStr = checkForUpdate("1.0.0");
    if (!retUrlStr.isNull())  // 本地版本与远程版本不同
    {
        main_logger->info("检测到A-SOUL提醒小助手新版本，启动更新提醒Wintoast。");
	    WinToastTemplate updateNotification = WinToastTemplate(WinToastTemplate::ImageAndText01);
        updateNotification.setImagePath(QString(app.applicationDirPath() + "/" + "avatar/update.png").toStdWString());
        updateNotification.setTextField(L"检测到A-SOUL提醒小助手新版本！", WinToastTemplate::FirstLine);
        updateNotification.setExpiration(0);
        updateNotification.setAudioPath(WinToastTemplate::AudioSystemFile::DefaultSound);
        updateNotification.setAudioOption(WinToastTemplate::AudioOption::Default);
        updateNotification.addAction(L"前往下载");

        if (WinToast::instance()->showToast(updateNotification, new CustomHandler(retUrlStr)) < 0) {
            //QMessageBox::warning(this, "Error", "Could not launch your toast notification!");
            qDebug() << "Could not launch your toast notification!";
            main_logger->error("更新提醒Wintoast启动失败！");
        }
    }

    /* 读取配置文件 */
    QFile m_file("member.json");
    QHash<int, QHash<QString, QString>> memberMap;
    QStringList uid_list;
    if (m_file.open(QFile::ReadOnly))
    {
        Json m_json;
        QByteArray buf = m_file.readAll();
        if(buf.isNull())
        {
            qDebug() << "Member json File is null.";
            main_logger->error("Read Member json Error. Read Member json file is null.");
            m_file.close();

            exit(1);  // 立即退出程序
        }

        m_json = Json::parse(buf.data(), nullptr, false);  // 不抛出异常
        if (m_json.is_null())
        {
            qDebug() << "Member json is null.";
            main_logger->error("Read Member json Error. Read Member json is null.");
            m_file.close();

        	exit(1);  // 立即退出程序
        }
        else
        {
            // 赋值memberMap
            auto arr = m_json["member"];
            for(int i=0; i<arr.size(); ++i)
            {
                int uid = arr[i]["uid"].get<int>();
                QString nickName = QString::fromStdString(arr[i]["nickname"].get<std::string>());
                QString avatar = QString::fromStdString(arr[i]["avatar"].get<std::string>());
                uid_list << QString::number(uid);
                memberMap[uid]["nickname"] = nickName;
                memberMap[uid]["avatar"] = avatar;
                qDebug() << uid << nickName<< avatar;
            }
            main_logger->info("Member Json loaded successfully.");
        }
    }
    else
    {
        qDebug() << "Member json can not be opened.";
        main_logger->error("Read Member json Error. Read Member json can not be opened.");
        m_file.close();

        exit(1);  // 立即退出程序
    }
    m_file.close();

	BiliBiliMessage bilibiliMessager(main_logger, uid_list);

    // 直播消息提醒
    QObject::connect(&bilibiliMessager, &BiliBiliMessage::newBilibiliLive, [&](int user, const QString title, const QString url)
        {
            main_logger->info("新直播消息信号，启动直播提醒Wintoast。成员：{}，标题：{}，网址：{}", user, title.toStdString(), url.toStdString());
            qDebug() << "直播：" << user << title << url;
            WinToastTemplate templ = WinToastTemplate(WinToastTemplate::ImageAndText04);
            QString imagePath = app.applicationDirPath() + "/";
            QString userName;

            imagePath += "avatar/" + memberMap[user]["avatar"];
            userName = memberMap[user]["nickname"];
            userName += "正在直播";
            qDebug() << imagePath;
            templ.setImagePath(imagePath.toStdWString());
            templ.setTextField(L"成员直播提醒", WinToastTemplate::FirstLine);
            templ.setTextField(userName.toStdWString(), WinToastTemplate::SecondLine);
            templ.setTextField(title.toStdWString(), WinToastTemplate::ThirdLine);
            templ.setExpiration(0);
            templ.setAudioPath(WinToastTemplate::AudioSystemFile::DefaultSound);
            templ.setAudioOption(WinToastTemplate::AudioOption::Default);
            templ.addAction(L"前往直播间");

            if (WinToast::instance()->showToast(templ, new CustomHandler(url)) < 0) {
                //QMessageBox::warning(this, "Error", "Could not launch your toast notification!");
                qDebug() << "Could not launch your toast notification!";
                main_logger->error("直播提醒WinToast启动失败！");
            }
        });

    // 动态消息提醒
    QObject::connect(&bilibiliMessager, &BiliBiliMessage::newBilibiliMessage, [&](int user, int type, const QString dynamic_id_str)
        {
            main_logger->info("新动态消息信号，启动动态提醒Wintoast。成员：{}，类型：{}，动态id：{}", user, type, dynamic_id_str.toStdString());
            qDebug() << "动态" << user << type << dynamic_id_str;
            WinToastTemplate templ = WinToastTemplate(WinToastTemplate::ImageAndText02);
            QString imagePath = app.applicationDirPath() + "/";
            QString userName;
            QString url = BDYNAMICURLPREFIX + dynamic_id_str;
            userName = memberMap[user]["nickname"];

            switch (type)
            {
            case 8:
                userName += "投稿了新视频";
                break;
            case 16:
                userName += "投稿了新视频";
                break;
            case 64:
                userName += "投稿了新专栏";
                break;
            default:
                userName += "有了新动态";
            }
            imagePath += "avatar/" + memberMap[user]["avatar"];
            qDebug() << imagePath;
            templ.setImagePath(imagePath.toStdWString());
            templ.setTextField(L"成员动态提醒", WinToastTemplate::FirstLine);
            templ.setTextField(userName.toStdWString(), WinToastTemplate::SecondLine);
            templ.setExpiration(0);
            templ.setAudioPath(WinToastTemplate::AudioSystemFile::DefaultSound);
            templ.setAudioOption(WinToastTemplate::AudioOption::Default);
            templ.addAction(L"前往动态");

            if (WinToast::instance()->showToast(templ, new CustomHandler(url)) < 0) {
                //QMessageBox::warning(this, "Error", "Could not launch your toast notification!");
                qDebug() << "Could not launch your toast notification!";
                main_logger->error("动态提醒WinToast启动失败！");
            }
        });

    // 错误提醒
    QObject::connect(&bilibiliMessager, &BiliBiliMessage::errorOccurred, [&](const QString errorString)
        {
            qDebug() << "Error Occurred Signal...";
            errCount++;
            qDebug() << errCount;

            if (errCount < 3)
            {
                main_logger->error("新错误信号，启动错误提醒Wintoast。错误信息：{}", errorString.toStdString());
                WinToastTemplate templ = WinToastTemplate(WinToastTemplate::ImageAndText02);
                QString imagePath = app.applicationDirPath() + "/avatar/error.png";
                templ.setImagePath(imagePath.toStdWString());
                templ.setTextField(L"插件发生错误", WinToastTemplate::FirstLine);
                templ.setTextField(errorString.toStdWString(), WinToastTemplate::SecondLine);
                templ.setExpiration(0);
                templ.setAudioPath(WinToastTemplate::AudioSystemFile::DefaultSound);
                templ.setAudioOption(WinToastTemplate::AudioOption::Default);

                if (WinToast::instance()->showToast(templ, new CustomHandler(0)) < 0) {
                    //QMessageBox::warning(this, "Error", "Could not launch your toast notification!");
                    qDebug() << "Could not launch your toast notification!";
                    main_logger->error("错误提醒WinToast启动失败！");
                }
            }
            else if (errCount == 3)
            {
                // 错误次数达到3次及以上，停止发送错误卡片
                qDebug()<< "错误信号数已达到3次，停止发送错误提醒。";
                main_logger->error("错误信号数已达到3次，停止发送错误提醒。");
                WinToastTemplate templ = WinToastTemplate(WinToastTemplate::ImageAndText02);
                QString imagePath = app.applicationDirPath() + "/avatar/error.png";
                templ.setImagePath(imagePath.toStdWString());
                templ.setTextField(L"插件发生错误", WinToastTemplate::FirstLine);
                templ.setTextField(L"插件错误达到3次，停止错误提醒。\n10分钟后错误提醒计数会自动归零，此后错误提醒会正常发送。", WinToastTemplate::SecondLine);
                templ.setExpiration(0);
                templ.setAudioPath(WinToastTemplate::AudioSystemFile::DefaultSound);
                templ.setAudioOption(WinToastTemplate::AudioOption::Default);

                if (WinToast::instance()->showToast(templ, new CustomHandler(0)) < 0) {
                    //QMessageBox::warning(this, "Error", "Could not launch your toast notification!");
                    qDebug() << "Could not launch your toast notification!";
                    main_logger->error("错误提醒WinToast启动失败！");
                }

                qDebug() << "错误信号数清零计时开始"; 
                main_logger->info("错误信号数清零计时开始");
                QTimer::singleShot(600000, [&]() {
                    qDebug() << "错误信号数清零计时达10分钟";
                    main_logger->info("错误信号数清零计时达10分钟");
                    errCount = 0;
                });

            }
            else if (errCount >= 6)
            {
                // 错误次数达到6次及以上，退出程序
                qDebug() << "错误信号数已达到6次及以上，发送错误提醒并退出程序。";
                main_logger->error("错误信号数已达到6次及以上，发送错误提醒并退出程序。");
                main_logger->flush();
                WinToastTemplate templ = WinToastTemplate(WinToastTemplate::ImageAndText02);
                QString imagePath = app.applicationDirPath() + "/avatar/exit.png";
                templ.setImagePath(imagePath.toStdWString());
                templ.setTextField(L"插件发生错误", WinToastTemplate::FirstLine);
                templ.setTextField(L"插件错误达到6次，插件已退出。", WinToastTemplate::SecondLine);
                templ.setExpiration(0);
                templ.setAudioPath(WinToastTemplate::AudioSystemFile::DefaultSound);
                templ.setAudioOption(WinToastTemplate::AudioOption::Default);

                if (WinToast::instance()->showToast(templ, new CustomHandler(0)) < 0) {
                    //QMessageBox::warning(this, "Error", "Could not launch your toast notification!");
                    qDebug() << "Could not launch your toast notification!";
                    main_logger->error("错误提醒WinToast启动失败！");
                }

                exit(1);
            }
        });

	bilibiliMessager.startQuery();

	return app.exec();
}
