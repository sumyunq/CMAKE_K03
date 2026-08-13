#include <QApplication>
#include <QStyleFactory>
#include "mainwindow.h"
#include "modules/Common/DeviceRegistry.h" ///< DeSheng::DeviceRegistry::instance().init()
#include "modules/CommunityModule/infrastructure/logger/logger.h" ///< Logger::init（spdlog）

#include <QLocalServer>
#include <QLocalSocket>

#include <QMainWindow>

#include <QMenu>
#include <QSystemTrayIcon>

#include <QFontDatabase>

#include <QSysInfo>

#include <windows.h>

#include <QCryptographicHash>
#include <QDateTime>
#include <QFuture>
#include <QSettings>
#include <QStandardPaths>
#include <QtConcurrent>

#include <QNetworkProxy>
#include <QNetworkProxyFactory>

#include "APOThread/ApoManager.h"
#include "LoadLib.h"
#include "data/api_global.h" ///< g_api_server_switch
#include "network/server_router.h"
//主窗体
QSharedMemory *m_sharedMemory = nullptr; //程序关闭时自动释放
MainWindow *m = nullptr;


#include <QSslSocket>
#include <QSslCertificate>
#include <QCryptographicHash>
#include <QSslKey>
//获得WebEngine SPKI
QString fetchSpkiFingerprint(const QString &hostName, quint16 port = 443)
{
    QSslSocket socket;
    socket.connectToHostEncrypted(hostName, port);
    if (!socket.waitForEncrypted(5000)) {
        qWarning() << "Connection failed:" << socket.errorString();
        return {};
    }

    QSslCertificate cert = socket.peerCertificate();
    if (cert.isNull()) {
        qWarning() << "No peer certificate";
        return {};
    }

    // 获取公钥 DER 格式
    QByteArray pubKeyDer = cert.publicKey().toDer();
    // 计算 SHA-256
    QByteArray hash = QCryptographicHash::hash(pubKeyDer, QCryptographicHash::Sha256);
    // 转为 Base64
    QString spki = hash.toBase64();

    qDebug() << "SPKI Fingerprint:" << spki;
    return spki;
}


#include <QApplication>
#include <QKeyEvent>
#include <QAbstractButton>
class SpaceKeyFilter : public QObject {
  protected:
    bool eventFilter(QObject *watched, QEvent *event) override {
        // 只拦截按钮上的空格键按下/释放事件
        if (qobject_cast<QAbstractButton*>(watched)) {
            if (event->type() == QEvent::KeyPress || event->type() == QEvent::KeyRelease) {
                auto *keyEvent = static_cast<QKeyEvent*>(event);
                if (keyEvent->key() == Qt::Key_Space) {
                    return true;  // 吃掉事件，不再向下传递
                }
            }
        }
        return QObject::eventFilter(watched, event);
    }
};

// 读取 exe 文件版本（version.rc，会话头日志用）
static QString exeVersion()
{
    wchar_t t_path[MAX_PATH] = {};
    GetModuleFileNameW(nullptr, t_path, MAX_PATH);
    DWORD t_dummy = 0;
    DWORD t_size = GetFileVersionInfoSizeW(t_path, &t_dummy);
    if (!t_size)
        return QString();
    QByteArray t_buf(static_cast<int>(t_size), Qt::Uninitialized);
    if (!GetFileVersionInfoW(t_path, 0, t_size, t_buf.data()))
        return QString();
    VS_FIXEDFILEINFO *t_ver = nullptr;
    UINT t_len = 0;
    if (!VerQueryValueW(t_buf.data(), L"\\", reinterpret_cast<void **>(&t_ver), &t_len)
        || !t_ver)
        return QString();
    return QString("%1.%2.%3.%4")
        .arg(HIWORD(t_ver->dwFileVersionMS))
        .arg(LOWORD(t_ver->dwFileVersionMS))
        .arg(HIWORD(t_ver->dwFileVersionLS))
        .arg(LOWORD(t_ver->dwFileVersionLS));
}

int main(int argc, char *argv[])
{
    qputenv("QT_QPA_PLATFORM", "windows:fontengine=freetype");

    //关闭网络代理（否则有一些用户电脑继承了系统代理，当代理不可达时就会报 proxy connection refused）
    QNetworkProxyFactory::setUseSystemConfiguration(false);
    QNetworkProxy::setApplicationProxy(QNetworkProxy::NoProxy);
    //  必须先设置 OpenGL 共享上下文（WebEngine 必需）
    QCoreApplication::setAttribute(Qt::AA_ShareOpenGLContexts);

    // 强制使用 ANGLE，通过 Direct3D 11 实现硬件加速，避免第三方 DLL 挂钩
    QCoreApplication::setAttribute(Qt::AA_UseOpenGLES);

    // 设置高 DPI 策略（必须在 QApplication 创建之前）
    // 2026-08-06 统一取整 Floor（原第二处 PassThrough 覆盖了此设置，已删除）：
    // 125%/150% 等非整数缩放（DPR=1.25/1.5）下 Qt 5.15 物理/逻辑坐标换算错位，
    // 导致窗口边缘拉伸光标不显示、按钮点击无效果/错位；Floor 使 <200% 均按 DPR=1.0
    // 渲染（100% 逻辑像素），>=200% 正常整数倍，规避非整数 DPI 的全部坐标问题。
    QGuiApplication::setHighDpiScaleFactorRoundingPolicy(Qt::HighDpiScaleFactorRoundingPolicy::Floor);
    if (QT_VERSION >= QT_VERSION_CHECK(5, 6, 0))
    {
        QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling, true);
    }
    QApplication::setQuitOnLastWindowClosed(false); // 防止因弹窗关闭而退出
    QApplication a(argc, argv);

    // 安装全局过滤器
    SpaceKeyFilter filter;// 过滤全局空格按键
    a.installEventFilter(&filter);

    a.setStyleSheet("*:focus { outline: none; }"); //取消焦点虚线框
    a.setApplicationName("XIBERIA HUB");

    // 初始化 spdlog 异步日志（AppData/logs/ 每日滚动，保留 30 天）— 必须在 setApplicationName 之后
    Logger::init();

    // 会话头：版本 / 构建类型 / Qt / OS（Release 日志排查第一眼定位环境）
#ifdef NDEBUG
    LOG_INFO("========== 会话开始 (Release) ==========");
#else
    LOG_INFO("========== 会话开始 (Debug) ==========");
#endif
    LOG_INFO("版本: {}  Qt: {}  OS: {} ({})", exeVersion().toStdString(), qVersion(),
             QSysInfo::prettyProductName().toStdString(),
             QSysInfo::currentCpuArchitecture().toStdString());

    // QString spki = fetchSpkiFingerprint("hubsys.xiberia.net");
    // 设置 WebEngine SPKI 固定,不能动态获取，容易被攻击，目的是为了防止，客户电脑系统证书库不完整，报错该网址的证书不可靠等
    const char* pinnedSpki = "rh5ySHhJ3lZURCQTq1+xQ/Zn8x9sosF1Z83sxgY8F0E=";
    qputenv("QTWEBENGINE_CHROMIUM_FLAGS",
            QString("--ignore-certificate-errors-spki-list=%1").arg(pinnedSpki).toUtf8());

    // // 初始化 SSL 证书管理（自动探测系统证书、加载/更新本地证书）
    // SslCertManager certManager;
    // bool sslReady = certManager.initialize();
    // if (!sslReady) {
    //     qWarning() << "SSL initialization failed, some network features may not work.";
    // }


    // 获取exe所在目录
    QString appDir = QCoreApplication::applicationDirPath();
    // 注册并加载TTC字体
    int fontId = QFontDatabase::addApplicationFont(appDir + "/Resources/Font/NotoSansHans-Regular.otf"); //400 "Noto Sans S Chinese"
    QFontDatabase::addApplicationFont(appDir + "/Resources/Font/NotoSansHans-Black.otf"); //900 "Noto Sans S Chinese Black"
    QFontDatabase::addApplicationFont(appDir + "/Resources/Font/NotoSansHans-Bold.otf");      //700
    QFontDatabase::addApplicationFont(appDir + "/Resources/Font/NotoSansHans-DemiLight.otf"); //350
    QFontDatabase::addApplicationFont(appDir + "/Resources/Font/NotoSansHans-Light.otf");     //300
    QFontDatabase::addApplicationFont(appDir + "/Resources/Font/NotoSansHans-Medium.otf");    //500
    QFontDatabase::addApplicationFont(appDir+ "/Resources/Font/NotoSansHans-Thin-Windows.otf"); //100
    if (fontId == -1) {
        qWarning() << "Failed to load font: SourceHanSans-Medium.ttc";
    } else {
        QStringList fontFamilies = QFontDatabase::applicationFontFamilies(fontId);
        if (!fontFamilies.isEmpty()) {
            QString fontFamily = fontFamilies.first();
            // 使用自定义字体
            QFont font(fontFamily);
            font.setPixelSize(12);                             // 明确的像素大小
            font.setHintingPreference(QFont::PreferNoHinting); //不加该句，字体模糊，笔画深浅不一
            QApplication::setFont(font);
        }
    }

    QString appDir2 = QCoreApplication::applicationDirPath();
    // 注册并加载 优设标题黑 字体
    int fontId2 = QFontDatabase::addApplicationFont(
        appDir2 + "/Resources/Font/优设标题黑.otf");
    if (fontId2 == -1) {
        qWarning() << "Failed to load font: 优设标题黑.otf";
    } else {
        QStringList fontFamilies = QFontDatabase::applicationFontFamilies(fontId2);//YouSheBiaoTiHei
    }

    QString appDir3 = QCoreApplication::applicationDirPath();
    // 注册并加载 ZQKfreefont 字体
    int fontId3 = QFontDatabase::addApplicationFont(
        appDir3 + "/Resources/Font/ZQKfreefont-2.ttf");
    if (fontId3 == -1) {
        qWarning() << "Failed to load font: ZQKfreefont-2.ttf";
    } else {
        QStringList fontFamilies = QFontDatabase::applicationFontFamilies(fontId3);
    }


    /*// 加载自定义字体文件
    int fontId = QFontDatabase::addApplicationFont("resources/Font/NotoSansHans-Regular.otf");

    // 获取字体族名称
    QString fontName = QFontDatabase::applicationFontFamilies(fontId).at(0);

    // 使用自定义字体
    QFont font(fontName, 12);
    QApplication::setFont(font);*/

    //a.setStyle(QStyleFactory::create("fusion"));// 设置 Fusion 风格

    //定义共享内存，使软件只打开一次（这个名称不可修改，旧版本使用的这个名称。不管后续软件名称叫什么，该处 和 自启动和在线时长上报 和 本地配置文件目录这些地方都不要更改）
    m_sharedMemory = new QSharedMemory("XIBERIA X HUB.exe"); //该共享内存键值为 XIBERIA X HUB.exe
    if (!m_sharedMemory->create(1) && m_sharedMemory->error() == QSharedMemory::AlreadyExists) {
        //通过本地套接字唤起现有实例
        QLocalSocket socket;
        socket.connectToServer("XiberiaGPServer");
        if (socket.waitForConnected(500)) { // 设置超时时间为500ms
            socket.write("activate");       // 发送唤醒指令
            socket.waitForBytesWritten();   //阻塞直到指令数据确实写入socket
            socket.close();                 //关闭socket连接
        }

        qDebug("程序已运行\n");
        return 0;
    }

    // 初始化背景表现配置（不依赖用户登录）
    g_user_information.initDefaultWallpaper();
    g_user_information.initSystemWallpapers();
    if (!g_user_information.loadWallpaperConfig()) {
        g_user_information.initCustomWallpapers();
        g_user_information.saveWallpaperConfigAsync();
    } else {
        g_user_information.local.validateWallpaperPaths();
        g_user_information.refreshSystemWallpapers();
        g_user_information.saveWallpaperConfigAsync();
    }

    ApoManager::instance()->start(); // 启动 APO 工作线程
    m = new MainWindow();            // 动态分配，长期有效

    // 初始化全局设备信息表（UI 创建前即可用）
    DeSheng::DeviceRegistry::instance().init();

    // API 服务器开关

    g_api_server_switch.test = true; ///<  默认 true 走测试服

    // 注册 4 服到 ServerRouter（URL 与 ApiConfig 旧栈一致）
    {
        auto &router = ServerRouter::instance();
        router.registerServer("domestic",   "https://hubsys.xiberia.net/api/v1");
        router.registerServer("domestic-t", "https://hubsystest.xiberia.net/api/v1");
        router.registerServer("overseas",   "");
        router.registerServer("overseas-t", "");

        // 默认服务器：根据全局 test/overseas 决定
        if (g_api_server_switch.test && g_api_server_switch.overseas)
            router.setDefaultServer("overseas-t");
        else if (g_api_server_switch.overseas)
            router.setDefaultServer("overseas");
        else if (g_api_server_switch.test)
            router.setDefaultServer("domestic-t");
        else
            router.setDefaultServer("domestic");

        // 13 个模块 tag 绑定：逻辑等价于旧 ApiServerSwitch::serverKey()
        auto computeKey = [](const ApiServerSwitch::ModuleSwitch &m) -> QString {
            bool t = m.test_set ? m.test_override : g_api_server_switch.test;
            bool o = m.overseas_set ? m.overseas_override : g_api_server_switch.overseas;
            if (t && o)      return "overseas-t";
            if (o)           return "overseas";
            if (t)           return "domestic-t";
            return "domestic";
        };
        router.setTagDefault("user",         computeKey(g_api_server_switch.user));
        router.setTagDefault("userConfig",   computeKey(g_api_server_switch.userConfig));
        router.setTagDefault("schemes",      computeKey(g_api_server_switch.schemes));
        router.setTagDefault("firmware",     computeKey(g_api_server_switch.firmware));
        router.setTagDefault("drive",        computeKey(g_api_server_switch.drive));
        router.setTagDefault("ad",           computeKey(g_api_server_switch.ad));
        router.setTagDefault("audition",     computeKey(g_api_server_switch.audition));
        router.setTagDefault("feedback",     computeKey(g_api_server_switch.feedback));
        router.setTagDefault("userLevel",    computeKey(g_api_server_switch.userLevel));
        router.setTagDefault("userDeviceLog", computeKey(g_api_server_switch.userDeviceLog));
        router.setTagDefault("wechatOauth",  computeKey(g_api_server_switch.wechatOauth));
        router.setTagDefault("googleOauth",  computeKey(g_api_server_switch.googleOauth));
        router.setTagDefault("userDevice",   computeKey(g_api_server_switch.userDevice));
    }

    // 提前初始化 globalSettings，确保后续可读取配置
    if (!globalSettings) {
        QString t_file_path = QFileInfo(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation))
                                  .absolutePath()
                              + "/XIBERIA X HUB/ProgramData/setting.ini";
        globalSettings = new QSettings(t_file_path, QSettings::IniFormat);
    }

    bool t_is_autostart = qApp->arguments().contains("/autostart");

    if (t_is_autostart) {
        // 开机自启动：根据配置决定显示或后台运行
        if (globalSettings->value("AutoStartShowWidget", true).toBool()) {
            m->show();
        } else {
            m->hide();
        }
    } else {
        m->show();
    }

    {
        UserSystemSettingsConfigInfo t_config_info;
        t_config_info.is_auto_start.store(globalSettings->value("AutoStart").toBool());
        t_config_info.is_auto_start_show_widget.store(
            globalSettings->value("AutoStartShowWidget").toBool());
        t_config_info.is_exit_directly.store(globalSettings->value("bExitDirectly", true).toBool());
        t_config_info.is_remember_choice.store(globalSettings->value("bRemember", false).toBool());
        m->UpdateUserSettingsConfig(t_config_info);
    }

    // ---------- 设置任务栏托盘图标 ----------

    // 创建系统托盘图标对象
    QSystemTrayIcon *pSystemTray = new QSystemTrayIcon(m);

    // 判断系统托盘图标是否创建成功
    if (NULL != pSystemTray) {
        // 设置托盘图标
        const QIcon trayIcon(":/Skin/Images/Tool.ico");
        if (trayIcon.isNull()) {
            qWarning() << "System tray icon resource is missing";
        }
        pSystemTray->setIcon(trayIcon);

        // 设置托盘图标的提示信息
        pSystemTray->setToolTip("XIBERIA Audio Center");

        // 显示托盘图标
        pSystemTray->show();

        // QObject::connect(pSystemTray, &QSystemTrayIcon::activated, m, &MainWindow::showPanel);

        QObject::connect(pSystemTray,
                         &QSystemTrayIcon::activated,
                         m,
                         [](QSystemTrayIcon::ActivationReason reason) {
                             if (reason == QSystemTrayIcon::Trigger) {
                                 // 通常是左键单击
                                 m->showPanel();
                             }
                             // 其他情况（如右键、双击）不处理
                         });
    }
    // ---------- 创建托盘菜单 ----------

    // 创建托盘菜单对象
    auto *trayMenu = new QMenu(m);
    trayMenu->setStyleSheet("border-image:NULL;");

    // 创建“显示面板”菜单项
    auto *showPanelAction = new QAction(QObject::tr("显示面板"), m);

    // 连接菜单项的触发信号与主窗口的showPanel槽函数
    QObject::connect(showPanelAction, &QAction::triggered, m, &MainWindow::showPanel);

    // 将菜单项添加到托盘菜单中
    trayMenu->addAction(showPanelAction);

    // 创建“退出”菜单项
    auto *quitAction = new QAction(QObject::tr("退出"), m);

    // 连接菜单项的触发信号与 QApplication 的 quit() 槽函数
    // QObject::connect(quitAction,&QAction::triggered,m,&QApplication::quit);
    QObject::connect(quitAction, &QAction::triggered, m, &MainWindow::closeSoftWare);

    // 将菜单项添加到托盘菜单中
    trayMenu->addAction(quitAction);

    // 设置托盘图标的上下文菜单
    pSystemTray->setContextMenu(trayMenu);

    // 显示托盘图标
    pSystemTray->show();

    // 创建本地服务器接收激活请求
    QLocalServer server;
    QObject::connect(&server, &QLocalServer::newConnection, [&]() {
        QLocalSocket *client = server.nextPendingConnection();
        if (client->waitForReadyRead(500)) {
            if (QString(client->readAll()) == "activate") {
                m->showNormal();     //将窗口从最小化或最大化状态恢复为原始尺寸
                m->activateWindow(); //激活窗口使其获得焦点
                m->raise();          //处于视觉最上层
            }
        }
        client->close();
    });

    if (!server.listen("XiberiaGPServer")) {
        QLocalServer::removeServer("XiberiaGPServer");
        server.listen("XiberiaGPServer");
    }

    int result = a.exec(); // 这里卡住了！

    ApoManager::instance()->stop(); // 安全停止

    Logger::shutdown(); // flush 异步日志队列

    return result;
}
