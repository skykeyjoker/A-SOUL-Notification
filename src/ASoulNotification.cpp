#include "BilibiliMessage.h"
#include "wintoastlib.h"

#include <QCoreApplication>
#include <QProcess>
#include <QDebug>

#pragma comment( linker, "/subsystem:\"windows\" /entry:\"mainCRTStartup\"" ) // Hide app

using namespace WinToastLib;

class CustomHandler : public IWinToastHandler {
public:
    CustomHandler(int uid, int type = 0, const QString url = "") : m_uid(uid), m_type(type), m_url(url)
	{}

    void toastActivated() const {
        std::wcout << L"The user clicked in this toast" << std::endl;
    }

    void toastActivated(int actionIndex) const {
        std::wcout << L"The user clicked on button #" << actionIndex << L" in this toast" << std::endl;
        if (m_type == 0)
            QProcess::startDetached("explorer " + m_url);
        else
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
    int m_uid;
    int m_type;
    QString m_url;
};

int main(int argc, char* argv[])
{
	QCoreApplication app(argc, argv);
	qDebug() << "A-Soul Notification is Running!";

    /* 检测重复运行*/
    QProcess process;
    QString processName = "ASoulNotification.exe";
    process.start("tasklist", QStringList() << "-fi" << "imagename eq " + processName);
    process.waitForFinished();
    QString outputStr = QString::fromLocal8Bit(process.readAllStandardOutput());
    if(outputStr.count(processName)>1)
    {
        qDebug("已有另外一个实例处于运行中！");
        app.exit(1);
        return 1;
    }

    /* 初始化应用信息 */
    app.setApplicationName("A-Soul Notification");
    app.setApplicationVersion("1.1.0");
    /* init wintoast */
    WinToast::instance()->setAppName(L"A-Soul Notification");
    WinToast::instance()->setAppUserModelId(
        WinToast::configureAUMI(L"Skykey", L"A-Soul Notification", L"A-Soul Notification", L"1.1.0"));
    if (!WinToast::instance()->initialize()) {
        qDebug() << "Error, your system in not compatible!";
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
    }

	BiliBiliMessage bilibiliMessager;

    QObject::connect(&bilibiliMessager, &BiliBiliMessage::newBilibiliLive, [&](int user, const QString title, const QString url)
        {
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

            if (WinToast::instance()->showToast(templ, new CustomHandler(user, 1, url)) < 0) {
                //QMessageBox::warning(this, "Error", "Could not launch your toast notification!");
                qDebug() << "Could not launch your toast notification!";
            }
        });
    QObject::connect(&bilibiliMessager, &BiliBiliMessage::newBilibiliMessage, [&](int user, int type, const QString dynamic_id_str)
        {
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

            if (WinToast::instance()->showToast(templ, new CustomHandler(user, 1, url)) < 0) {
                //QMessageBox::warning(this, "Error", "Could not launch your toast notification!");
                qDebug() << "Could not launch your toast notification!";
            }
        });
    QObject::connect(&bilibiliMessager, &BiliBiliMessage::errorOccurred, [&](const QString errorString)
        {
            qDebug() << "Error Occurred Signal...";
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
            }
        });

	bilibiliMessager.startQuery();

	return app.exec();
}
