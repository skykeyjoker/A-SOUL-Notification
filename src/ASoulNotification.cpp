#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/spdlog.h>

#include <QCoreApplication>
#include <QDebug>
#include <QDir>
#include <QEventLoop>
#include <QFile>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QProcess>
#include <chrono>

#include "Essential.h"
#include "QueryCenter.h"
#include "json.hpp"
#include "wintoastlib.h"

#pragma comment(linker, "/subsystem:\"windows\" /entry:\"mainCRTStartup\"")// Hide app

using namespace WinToastLib;

class CustomHandler : public IWinToastHandler {
public:
    CustomHandler(const QString url = "") : m_url(url) {}

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

QString checkForUpdate(const QString& version, std::shared_ptr<spdlog::logger>& logger) {
    QString retUrlStr;
    Json j;
    QNetworkAccessManager manager;
    QNetworkRequest request;
    request.setUrl(QUrl("https://cdn.jsdelivr.net/gh/skykeyjoker/A-Soul-Notification/version.json"));

    QEventLoop eventLoop;
    QObject::connect(&manager, SIGNAL(finished(QNetworkReply*)), &eventLoop, SLOT(quit()));
    QNetworkReply* reply = manager.get(request);
    eventLoop.exec();

    if (reply->error() != QNetworkReply::NoError) {
        qDebug() << "Can not get update from github.";
        logger->error("Update Error. Can not get update from github.");

        return retUrlStr;
    } else {
        QByteArray buf = reply->readAll();
        reply->deleteLater();

        if (buf.isNull()) {
            qDebug() << "Update info is null.";
            logger->error("Update Error. Update info is null.");
            return retUrlStr;
        } else {
            j = Json::parse(buf.data(), nullptr, false);// 不抛出异常
            if (j.is_null()) {
                qDebug() << "Update info json is null.";
                logger->error("Update Error. Update info json is null.");
                return retUrlStr;
            } else {

                QString remoteVersion;
                QString remoteUrl;
                try {
                    remoteVersion = QString::fromStdString(j["versions"][0]["version"].get<std::string>());
                    remoteUrl = QString::fromStdString(j["versions"][0]["url"].get<std::string>());
                } catch (Json::exception& ex) {
                    qDebug() << "Update info json can not be parsed. Exception occurred:" << ex.what();
                    logger->error("Update Error. Update info json can not be parsed. Exception Occurred: {}", ex.what());
                    return retUrlStr;
                } catch (...) {
                    qDebug() << "Update info json can not be parsed.";
                    logger->error("Update Error. Update info json can not be parsed.");
                    return retUrlStr;
                }

                qDebug() << remoteUrl << remoteVersion << QString("v" + version);
                logger->info("Remote Version: {}, Remote Url: {}", remoteVersion.toStdString(), remoteUrl.toStdString());
                if (QString("v" + version).compare(remoteVersion) != 0)// 本地版本与远程版本不同
                {
                    retUrlStr = remoteUrl;
                }
            }
        }
    }

    return retUrlStr;
}

int main(int argc, char* argv[]) {
    QCoreApplication app(argc, argv);

    // 初始化logs文件夹
    QDir logDir;
    logDir.setPath(app.applicationDirPath());
    if (logDir.exists("logs")) {
        if (logDir.exists("logs/log.txt")) {
            logDir.remove("logs/log.txt");
        }
    } else
        logDir.mkdir("logs");

    // 初始化spdlog
    auto main_logger = spdlog::rotating_logger_mt("main_logger", "logs/log.txt", MAX_LOG_SIZE, MAX_LOG_NUM);
    main_logger->set_level(spdlog::level::debug);
    main_logger->set_pattern("[%Y-%m-%d %H:%M:%S] [%^%L%$] [thread %t] %v");
    spdlog::flush_every(std::chrono::seconds(10));

    qDebug() << "A-SOUL Notification is Running!";
    main_logger->info("A-SOUL提醒小助手启动！");

    /* 检测重复运行*/
    QProcess process;
    QString processName = "ASoulNotification.exe";
    process.start("tasklist", QStringList() << "-fi"
                                            << "imagename eq " + processName);
    process.waitForFinished();
    QString outputStr = QString::fromLocal8Bit(process.readAllStandardOutput());
    if (outputStr.count(processName) > 1) {
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
    if (!retUrlStr.isNull())// 本地版本与远程版本不同
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
    BiliBiliMemberMap bilibiliMemberMap;
    DouyinMemberMap douyinMemberMap;
    UidList uid_list{};
    SecUidList sec_uid_list{};
    if (m_file.open(QFile::ReadOnly)) {
        Json m_json;
        QByteArray buf = m_file.readAll();
        if (buf.isNull()) {
            qDebug() << "Member json File is null.";
            main_logger->error("Read Member json Error. Read Member json file is null.");
            m_file.close();

            main_logger->flush();
            exit(1);// 立即退出程序
        }

        m_json = Json::parse(buf.data(), nullptr, false);// 不抛出异常
        if (m_json.is_null()) {
            qDebug() << "Member json is null.";
            main_logger->error("Read Member json Error. Read Member json is null.");
            m_file.close();

            main_logger->flush();
            exit(1);// 立即退出程序
        } else {
            try {
                // 赋值memberMap
                auto bilibiliArr = m_json["Bilibili"]["member"];
                for (int i = 0; i < bilibiliArr.size(); ++i) {
                    int uid = bilibiliArr[i]["uid"].get<int>();
                    QString nickName = QString::fromStdString(bilibiliArr[i]["nickname"].get<std::string>());
                    QString avatar = QString::fromStdString(bilibiliArr[i]["avatar"].get<std::string>());
                    uid_list << QString::number(uid);
                    bilibiliMemberMap[uid]["nickname"] = nickName;
                    bilibiliMemberMap[uid]["avatar"] = avatar;
                    qDebug() << uid << nickName << avatar;
                }

                auto enableDouyin = m_json["Douyin"]["enable"].get<bool>();
                if (enableDouyin) {
                    auto douyinArr = m_json["Douyin"]["member"];
                    for (int i = 0; i < douyinArr.size(); ++i) {
                        QString uid = QString::fromStdString(douyinArr[i]["uid"].get<std::string>());
                        QString sec_uid = QString::fromStdString(douyinArr[i]["sec_uid"].get<std::string>());
                        QString nickName = QString::fromStdString(douyinArr[i]["nickname"].get<std::string>());
                        QString avatar = QString::fromStdString(douyinArr[i]["avatar"].get<std::string>());
                        sec_uid_list << sec_uid;
                        douyinMemberMap[uid]["nickname"] = nickName;
                        douyinMemberMap[uid]["avatar"] = avatar;
                        qDebug() << uid << sec_uid << nickName << avatar;
                    }
                }

                main_logger->info("Member Json loaded successfully. Douyin Mode:{}", enableDouyin);
            } catch (Json::exception& ex) {
                qDebug() << "解析member.json失败，发生异常" << ex.what();
                main_logger->error("解析member.json失败，发生异常：{}", ex.what());
                m_file.close();

                main_logger->flush();
                exit(1);
            } catch (...) {
                qDebug() << "解析member.json失败，发生未知异常";
                main_logger->error("解析member.json失败，发生未知异常");
                m_file.close();

                main_logger->flush();
                exit(1);
            }
        }
    } else {
        qDebug() << "Member json can not be opened.";
        main_logger->error("Read Member json Error. Read Member json can not be opened.");
        m_file.close();

        main_logger->flush();
        exit(1);// 立即退出程序
    }
    m_file.close();

    // 启动监控中心
    QueryCenter queryCenter(main_logger, uid_list, sec_uid_list, bilibiliMemberMap, douyinMemberMap);

    // 直播消息提醒
    QObject::connect(&queryCenter, &QueryCenter::newBilibiliLiveMessage, [&](QString message, QString avatar, QString title, QString url) {
        main_logger->info("【哔哩哔哩】新直播消息信号，启动直播提醒Wintoast。成员：{}，标题：{}，网址：{}", message.toStdString(), title.toStdString(), url.toStdString());
        qDebug() << "【哔哩哔哩】直播：" << message << title << url << avatar;
        WinToastTemplate templ = WinToastTemplate(WinToastTemplate::ImageAndText04);

        templ.setImagePath(avatar.toStdWString());
        templ.setTextField(L"成员直播提醒", WinToastTemplate::FirstLine);
        templ.setTextField(message.toStdWString(), WinToastTemplate::SecondLine);
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
    QObject::connect(&queryCenter, &QueryCenter::newBilibiliDynamicMessage, [&](QString message, QString avatar, QString url) {
        main_logger->info("【哔哩哔哩】新动态消息信号，启动动态提醒Wintoast。成员：{}，动态链接：{}", message.toStdString(), url.toStdString());
        qDebug() << "【哔哩哔哩】动态" << message << avatar << url;

        WinToastTemplate templ = WinToastTemplate(WinToastTemplate::ImageAndText02);
        templ.setImagePath(avatar.toStdWString());
        templ.setTextField(L"成员动态提醒", WinToastTemplate::FirstLine);
        templ.setTextField(message.toStdWString(), WinToastTemplate::SecondLine);
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
    QObject::connect(&queryCenter, &QueryCenter::newDouyinDynamicMessage, [&](QString message, QString avatar, QString title, QString url) {
        main_logger->info("【抖音】新动态消息信号，启动动态提醒Wintoast。成员：{}，标题：{}，动态链接：{}", message.toStdString(), title.toStdString(), url.toStdString());
        qDebug() << "【抖音】动态" << message << avatar << title << url;

        WinToastTemplate templ = WinToastTemplate(WinToastTemplate::ImageAndText04);
        templ.setImagePath(avatar.toStdWString());
        templ.setTextField(L"成员动态提醒", WinToastTemplate::FirstLine);
        templ.setTextField(message.toStdWString(), WinToastTemplate::SecondLine);
        templ.setTextField(title.toStdWString(), WinToastTemplate::ThirdLine);
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

    // 风控消息提醒
    QObject::connect(&queryCenter, &QueryCenter::newOverLoadMessage, [&](QString message) {
        main_logger->error("风控信号，启动风控提醒Wintoast。错误信息：{}", message.toStdString());

        WinToastTemplate templ = WinToastTemplate(WinToastTemplate::ImageAndText02);
        QString imagePath = app.applicationDirPath() + "/avatar/sleep.png";
        templ.setImagePath(imagePath.toStdWString());
        templ.setTextField(L"检测到服务器风控", WinToastTemplate::FirstLine);
        templ.setTextField(message.toStdWString(), WinToastTemplate::SecondLine);
        templ.setExpiration(0);
        templ.setAudioPath(WinToastTemplate::AudioSystemFile::DefaultSound);
        templ.setAudioOption(WinToastTemplate::AudioOption::Default);

        if (WinToast::instance()->showToast(templ, new CustomHandler(0)) < 0) {
            //QMessageBox::warning(this, "Error", "Could not launch your toast notification!");
            qDebug() << "Could not launch your toast notification!";
            main_logger->error("风控提醒WinToast启动失败！");
        }
    });

    queryCenter.startQuery();

    return app.exec();
}
