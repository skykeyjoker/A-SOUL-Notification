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
#include <spdlog/spdlog.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <chrono>

#pragma comment( linker, "/subsystem:\"windows\" /entry:\"mainCRTStartup\"" ) // Hide app

using namespace WinToastLib;



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
                logger->info("Romote Version: {}, Remote Url: {}", version.toStdString(), remoteUrl.toStdString());
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

	qDebug() << "A-Soul Notification is Running!";
    main_logger->info("A-Soul提醒小助手启动！");

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
    app.setApplicationName("A-Soul Notification");
    app.setApplicationVersion("1.3.0");
    /* init wintoast */
    WinToast::instance()->setAppName(L"A-Soul Notification");
    WinToast::instance()->setAppUserModelId(
        WinToast::configureAUMI(L"Skykey", L"A-Soul Notification", L"A-Soul Notification", L"1.3.0"));
    if (!WinToast::instance()->initialize()) {
        qDebug() << "Error, your system in not compatible!";
        main_logger->error("系统不支持WinToast！");
    }

    /* 应用启动显示提示信息 */
    //ImageAndText01
    WinToastTemplate runInfo = WinToastTemplate(WinToastTemplate::ImageAndText01);
    runInfo.setImagePath(QString(app.applicationDirPath() + "/" + "avatar/icon.png").toStdWString());
    runInfo.setTextField(L"A-Soul提醒小助手运行中！", WinToastTemplate::FirstLine);
    runInfo.setExpiration(0);
    runInfo.setAudioPath(WinToastTemplate::AudioSystemFile::DefaultSound);
    runInfo.setAudioOption(WinToastTemplate::AudioOption::Default);
    if (WinToast::instance()->showToast(runInfo, new CustomHandler(0)) < 0) {
        //QMessageBox::warning(this, "Error", "Could not launch your toast notification!");
        qDebug() << "Could not launch your toast notification!";
        main_logger->error("启动信息Wintoast启动失败！");
    }
    main_logger->info("A-Soul提醒小助手启动成功！");

    /* 检查更新 */
    QString retUrlStr = checkForUpdate(app.applicationVersion(), main_logger);
    //QString retUrlStr = checkForUpdate("1.0.0");
    if (!retUrlStr.isNull())  // 本地版本与远程版本不同
    {
        main_logger->info("检测到A-Soul提醒小助手新版本，启动更新提醒Wintoast。");
	    WinToastTemplate updateNotification = WinToastTemplate(WinToastTemplate::ImageAndText01);
        updateNotification.setImagePath(QString(app.applicationDirPath() + "/" + "avatar/update.png").toStdWString());
        updateNotification.setTextField(L"检测到A-Soul提醒小助手新版本！", WinToastTemplate::FirstLine);
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

	BiliBiliMessage bilibiliMessager(main_logger);

    QObject::connect(&bilibiliMessager, &BiliBiliMessage::newBilibiliLive, [&](int user, const QString title, const QString url)
        {
            main_logger->info("新直播消息信号，启动直播提醒Wintoast。成员：{}，标题：{}，网址：{}", user, title.toStdString(), url.toStdString());
            qDebug() << "直播：" << user << title << url;
            WinToastTemplate templ = WinToastTemplate(WinToastTemplate::ImageAndText04);
            QString imagePath = app.applicationDirPath() + "/";
            QString userName;
            switch (user)
            {
            case AVAUID:
                imagePath += "avatar/ava.jpg";
                userName = "向晚";
                break;
            case BELLAUID:
                imagePath += "avatar/bella.jpg";
                userName = "贝拉";
                break;
            case CAROLUID:
                imagePath += "avatar/carol.jpg";
                userName = "珈乐";
                break;
            case DIANAUID:
                imagePath += "avatar/diana.jpg";
                userName = "嘉然";
                break;
            case EILEENUID:
                imagePath += "avatar/eileen.jpg";
                userName = "乃琳";
                break;
            case OFFICIALUID:
                imagePath += "avatar/official.jpg";
                userName = "A-Soul Official";
                break;
            }
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
    QObject::connect(&bilibiliMessager, &BiliBiliMessage::newBilibiliMessage, [&](int user, int type, const QString dynamic_id_str)
        {
            main_logger->info("新动态消息信号，启动动态提醒Wintoast。成员：{}，类型：{}，动态id：{}", user, type, dynamic_id_str.toStdString());
            qDebug() << "动态" << user << type << dynamic_id_str;
            WinToastTemplate templ = WinToastTemplate(WinToastTemplate::ImageAndText02);
            QString imagePath = app.applicationDirPath() + "/";
            QString userName;
            QString url = BDYNAMICURLPREFIX + dynamic_id_str;
            switch (user)
            {
            case AVAUID:
                imagePath += "avatar/ava.jpg";
                userName = "向晚";
                break;
            case BELLAUID:
                imagePath += "avatar/bella.jpg";
                userName = "贝拉";
                break;
            case CAROLUID:
                imagePath += "avatar/carol.jpg";
                userName = "珈乐";
                break;
            case DIANAUID:
                imagePath += "avatar/diana.jpg";
                userName = "嘉然";
                break;
            case EILEENUID:
                imagePath += "avatar/eileen.jpg";
                userName = "乃琳";
                break;
            case OFFICIALUID:
                imagePath += "avatar/official.jpg";
                userName = "A-Soul Official";
                break;
            }
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
    QObject::connect(&bilibiliMessager, &BiliBiliMessage::errorOccurred, [&](const QString errorString)
        {
            qDebug() << "Error Occurred Signal...";
            main_logger->info("新错误信号，启动错误提醒Wintoast。错误信息：{}", errorString.toStdString());
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
        });

	bilibiliMessager.startQuery();

	return app.exec();
}
