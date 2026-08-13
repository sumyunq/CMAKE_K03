#ifndef API_GLOBAL_H
#define API_GLOBAL_H

#include <QCoreApplication>
#include <QCryptographicHash>
#include <QDir>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QList>
#include <QMap>
#include <QMessageAuthenticationCode>
#include <QNetworkAccessManager>
#include <QObject>
#include <QSet>
#include <QSettings>
#include <QSharedPointer>
#include <QStandardPaths>
#include <QString>
#include <QUrlQuery>
#include <algorithm>

/// DeSheng API相关 — 子模块各自定义 namespace DeSheng {}，此处聚合 include
/// 注：网络栈已迁移至新栈（HttpClient/ServerRouter/AuthStore），此处仅聚合接口定义层
/// 子模块
#include "data/ad/ad_api.h"
#include "data/audition/audition_api.h"
#include "data/device/device_info.h"
#include "data/drive/drive_api.h"
#include "data/feedback/feedback_api.h"
#include "data/firmware/firmware_api.h"
#include "data/googleOauth/google_oauth_api.h"
#include "data/schemes/schemes_api.h"
#include "data/user/user_api.h"
#include "data/userConfig/user_config_api.h"
#include "data/userDevice/user_device_api.h"
#include "data/userDeviceLog/user_device_log_api.h"
#include "data/userLevel/user_level_api.h"
#include "data/wechatOauth/wechat_oauth_api.h"

/// 默认参数常量
namespace defaultConfig {
/// 图片相关
constexpr qint64 MAX_IMAGE_SIZE = 2 * 1024 * 1024; // 2MB
constexpr double MAX_IMAGE_SIZE_MB = 2.0;          // 2MB
const QStringList SUPPORTED_FORMATS = {"png", "jpg", "jpeg", "bmp", "gif"};
} // namespace defaultConfig

namespace CategoryIcon {
/// user_tags → 图标 base 名（首个匹配生效，不含后缀/前缀）
extern const QHash<QString, QString> kIconMap;
extern const QString kDefaultBase;

/// 按 base + 选中态 + 是否系统图标，拼接完整资源路径
QString buildPath(const QString &t_base, bool t_selected, bool t_system);
} // namespace CategoryIcon

///
/// \brief The ScrollAreaDisplayMode enum
/// 显示模式：适应不同的 scrall 布局
enum class ScrollAreaDisplayMode {
    GridDisplay,         ///网格显示
    SingleColumnDisplay, ///单列显示
    SingleLineDisplay    ///单行显示
};

/// \brief 背景图片形态
enum class BackgroundImageMode {
    DefaultTheme = 0, ///< 默认主题背景 — 随主题切换
    SystemTheme,      ///< 系统自带壁纸预览
    Custom,           ///< 用户自定义壁纸（可删除）
    AddCustom,        ///< "添加背景"入口卡片，添加图片后切换为 Custom
    Reserved          ///< 预留
};

/// \brief 壁纸配置持久化范围
enum class WallpaperStorageScope {
    UserLocal,    ///< 跟随登录用户目录
    AppLocalJson  ///< 跟随应用本地 JSON，不随用户切换
};

#include <QByteArray>
#include <QFile>
#include <QFuture>
#include <QJsonDocument>
#include <QJsonObject>
#include <QtConcurrent/QtConcurrent>

/// 通用工具函数(仅对 XIBERIA_X_HUB )
namespace XIBERIA_X_HUB_Utils {

/// \brief 校验字段非空，为空时将错误信息写入 error
inline bool checkNotEmpty(const QString &value, const QString &fieldName, QString &error)
{
    if (value.isEmpty()) {
        error = QString("缺少必填字段: %1").arg(fieldName);
        return false;
    }
    return true;
}

/// \brief 将用户 ID 哈希为安全的目录名（含 ProgramData 前缀）
/// 使用 HMAC-SHA256，密钥固定，同一 ID 始终得到相同目录名
inline QString hashUserIdToDirName(const QString &userId)
{
    if (userId.isEmpty())
        return QString();
    const QByteArray t_key = "XIBERIA_X_HUB_2026_SecretKey";
    QByteArray t_hash = QMessageAuthenticationCode::hash(userId.toUtf8(),
                                                         t_key,
                                                         QCryptographicHash::Sha256);
    return QString::fromLatin1(t_hash.toHex().left(16));
}

/// \brief 获取程序 ProgramData 根目录
inline QString programDataPath()
{
    return QFileInfo(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation)).absolutePath()
           + "/XIBERIA X HUB/ProgramData";
}

/**
 * @brief XOR 异或 + Base64 加密
 * @param data 原始数据
 * @param key 加密密钥，默认 0x5A
 * @return 加密后的 Base64 字符串
 */
QByteArray encryptData(const QByteArray &data, char key = 0x5A);

/**
 * @brief XOR 异或 + Base64 解密
 * @param encryptedData 加密后的 Base64 数据
 * @param key 解密密钥，需与加密密钥一致，默认 0x5A
 * @return 解密后的原始数据
 */
QByteArray decryptData(const QByteArray &encryptedData, char key = 0x5A);

/**
 * @brief 保存 JSON 到加密文件（同步）
 * @param filePath 文件路径
 * @param json JSON 对象
 * @param key 加密密钥，默认 0x5A
 * @return 成功返回 true，失败返回 false
 */
bool saveJsonEncrypted(const QString &filePath, const QJsonObject &json, char key = 0x5A);

/**
 * @brief 从加密文件读取 JSON（同步）
 * @param filePath 文件路径
 * @param key 解密密钥，需与加密密钥一致，默认 0x5A
 * @return 解密后的 JSON 对象，失败返回空对象
 */
QJsonObject loadJsonEncrypted(const QString &filePath, char key = 0x5A);

/**
 * @brief 异步保存 JSON 到加密文件
 * @param filePath 文件路径
 * @param json JSON 对象
 * @param key 加密密钥，默认 0x5A
 * @note 函数立即返回，在后台线程中执行加密和写入操作
 */
void saveJsonEncryptedAsync(const QString &filePath, const QJsonObject &json, char key = 0x5A);

/// \brief 获取下载临时文件目录
inline QString downloadTempDir()
{
    return programDataPath() + "/downloads/tempfiles";
}

/// \brief 确保下载临时文件目录存在
inline void ensureDownloadTempDir()
{
    QDir().mkpath(downloadTempDir());
}

/// \brief 获取默认截图保存路径
inline QString getDefaultScreenshotPath()
{
#ifdef Q_OS_WIN
    return QStandardPaths::writableLocation(QStandardPaths::PicturesLocation) + "/Screenshots";
#else
    return QStandardPaths::writableLocation(QStandardPaths::PicturesLocation);
#endif
}

/// \brief 对 QPixmap 应用快速模糊（三遍盒式模糊近似高斯），返回副本
/// \param t_source 源图像
/// \param t_radius 模糊半径（0 = 不处理，上限 254）
/// \note 使用滑动窗口累加器实现，O(n) 每遍，无堆分配，比 QGraphicsScene 方案轻 10 倍以上
QPixmap blurPixmap(const QPixmap &t_source, int t_radius);
} // namespace XIBERIA_X_HUB_Utils

typedef struct UserSystemSettingsConfigInfo
{
    std::atomic<bool> is_auto_start = false;             ///< 是否开机自启动
    std::atomic<bool> is_auto_start_show_widget = false; ///< 自启动时是否显示主界面
    std::atomic<bool> is_exit_directly = true;           ///< 关闭时是否直接退出
    std::atomic<bool> is_remember_choice = false;        ///< 是否记住我的选择

    UserSystemSettingsConfigInfo() = default;
    UserSystemSettingsConfigInfo(const UserSystemSettingsConfigInfo &other) { *this = other; }
    UserSystemSettingsConfigInfo &operator=(const UserSystemSettingsConfigInfo &other)
    {
        is_auto_start.store(other.is_auto_start.load());
        is_auto_start_show_widget.store(other.is_auto_start_show_widget.load());
        is_exit_directly.store(other.is_exit_directly.load());
        is_remember_choice.store(other.is_remember_choice.load());
        return *this;
    }
} UserSystemSettingsConfigInfo;

/// \brief API 服务器开关（每个模块独立控制 test/prod + domestic/overseas）
/// 使用方式：g_api_server_switch.userConfig.test = true; // userConfig 走测试服
struct ApiServerSwitch {
    bool test = false;     ///< true=测试服，false=正式服（全局默认，模块未设时回退）
    bool overseas = false; ///< true=海外服，false=国内服（全局默认）

    // 各模块独立开关（未设置时回退到全局）
    struct ModuleSwitch {
        bool test_override = false;
        bool test_set = false;       ///< 是否显式设置了 test
        bool overseas_override = false;
        bool overseas_set = false;
    };
    ModuleSwitch user;         ///< 用户系统：登录/注册/找回密码/获取信息/上传头像
    ModuleSwitch userConfig;   ///< 方案分享社区：方案CRUD/点赞踩/收藏/评论/下载
    ModuleSwitch schemes;      ///< 分享码系统：ys 8位码的创建与解析（预设库导出用）
    ModuleSwitch firmware;     ///< 固件升级：查询固件版本/下载升级包
    ModuleSwitch drive;        ///< 驱动更新：查询驱动版本/下载安装包
    ModuleSwitch ad;           ///< 广告系统：首页轮播广告列表/点击上报
    ModuleSwitch audition;     ///< 试听系统：游戏场景试听视频列表
    ModuleSwitch feedback;     ///< 用户反馈：提交反馈表单/上传截图
    ModuleSwitch userLevel;    ///< 用户等级：在线时长上报/等级查询
    ModuleSwitch userDevice;    ///< 设备绑定统计：月度/累计绑定人数
    ModuleSwitch userDeviceLog;///< 设备日志：设备绑定上报/活跃统计
    ModuleSwitch wechatOauth;  ///< 微信登录：获取二维码/轮询登录状态
    ModuleSwitch googleOauth;  ///< Google 登录：生成授权链接/回调/预授权/查询登录状态
};
extern ApiServerSwitch g_api_server_switch;

///
/// \brief UserLocalInformation
/// 用户本地信息,仅用于全局用户信息保存
/// 不同于网络回显的用户信息,按需保存。
/// 请求不同,只会更新该结构体的特定字段。
/// 按需 进行本地持久化
struct UserInformation
{
    static constexpr int kCurrentVersion = 1; ///< 当前数据格式版本

    /// 网络请求    回显相关
    struct UserInfo_NetWork
    {
        QString access_token;  ///< 访问令牌(持久化,用户主动退出时失效)
        QString refresh_token; ///< 刷新令牌

        QString id; ///< 用户 ID
        QString username; ///< 用户名（为兼容旧版 ini 文件，该字段保存和读取都照旧，取值为 ini 文件中的 Login/nickname）
        QString email;          ///< 邮箱地址
        QString nickname;       ///< 用户昵称(只用于保存原微信名称，项目中显示的昵称全取自 username)
        QString avatar;         ///< 头像 URL
        QString status;         ///< 状态
        QString login_type;     ///< 登录类型(邮箱 account、微信 wechat)
        QString created_at;     ///< 创建时间
        QString last_login_at;  ///< 最后登录时间
        QString favorite_games; ///< 喜爱游戏
        QString activation_code; ///< 激活码
        QString city;            ///< 城市
        QString login_ip;        ///< 登录 IP
        QString os_info;         ///< 操作系统信息
        QString bio;             ///< 个性签名
        QStringList roles;       ///< 用户角色（如 streamer / professional）
        QStringList titles;      ///< 用户头衔
    } network;

    /// 本地存储相关
    struct UserInfo_Local
    {
        QString user_psw; ///< 用户密码(勾选自动登录时,进行持久化)
        std::atomic<bool> is_get_userInfo_first
            = true; ///< 是否是第一次获取完整用户信息,获取后置为false,退出登录时需置为true

        double panel_opacity_ = 1.0;     ///< 面板透明度 (0.0 ~ 1.0)
        double panel_blur_radius_ = 0.02; ///< 面板模糊半径 归一化 (0.0 ~ 1.0，映射 ≤25px)
        bool is_encrypted_save_ = false;  ///< 是否加密保存（仅调试用，发布时始终为 true）

        /// 壁纸条目
        struct WallpaperEntry
        {
            QString path;                                    ///< 壁纸文件路径
            BackgroundImageMode mode = BackgroundImageMode::SystemTheme; ///< 背景形态
            bool is_selected = false;                        ///< 是否当前选中

            WallpaperEntry() = default;
            WallpaperEntry(const QString &p, BackgroundImageMode m, bool s = false)
                : path(p), mode(m), is_selected(s) {}
        };
        QSharedPointer<QMap<int, QSharedPointer<WallpaperEntry>>> default_wallpaper_map;
        QSharedPointer<QMap<int, QSharedPointer<WallpaperEntry>>> system_wallpaper_map;
        QSharedPointer<QMap<int, QSharedPointer<WallpaperEntry>>> custom_wallpaper_map;

        void selectWallpaper(const QSharedPointer<QMap<int, QSharedPointer<WallpaperEntry>>> &map, int index);

        /// \brief 校验三张壁纸 map 中所有路径的有效性，移除磁盘不存在的条目
        void validateWallpaperPaths();

        /// \brief 获取当前选中的壁纸，返回 {map指针, index}，无选中时 map 为 nullptr
        QPair<QSharedPointer<QMap<int, QSharedPointer<WallpaperEntry>>>, int> selectedWallpaper() const;

    } local;

    /// \brief 从 GetCurrentUser 应答更新网络字段
    void updateFromServer(const DeSheng::GetCurrentUserResponse::ReturnData &t_data);

    /// \brief 从登录应答的 UserInfo 更新网络字段
    void updateFromServer(const DeSheng::UserLoginResponse::ReturnData::UserInfo &t_user);

    /// \brief 获取当前用户对应的哈希目录名
    QString userDirName() const
    {
        return XIBERIA_X_HUB_Utils::programDataPath() + "/Users/"
               + XIBERIA_X_HUB_Utils::hashUserIdToDirName(network.id);
    }

    QString avatarFilePath() const
    {
        if (network.id.isEmpty())
            return QString();
        return userDirName() + "/head_portrait.png";
    }

    QString userInfoFilePath() const
    {
        if (network.id.isEmpty())
            return QString();
        return userDirName() + "/user_info.json";
    }

    QString customBackgroundDir() const
    {
        if (wallpaperStorageScope() == WallpaperStorageScope::AppLocalJson)
            return wallpaperCustomBackgroundDir();
        if (network.id.isEmpty())
            return QString();
        return userDirName() + "/CustomBackground";
    }

    /// \brief 当前壁纸配置存储范围；临时需求默认应用本地，后续可切回 UserLocal
    static WallpaperStorageScope wallpaperStorageScope();

    /// \brief 应用本地壁纸配置文件路径
    static QString wallpaperConfigFilePath();

    /// \brief 应用本地自定义壁纸目录
    static QString wallpaperCustomBackgroundDir();

    /// \brief 初始化默认壁纸（主题切换用，index 0 = DefaultTheme 无文件路径）
    void initDefaultWallpaper();

    /// \brief 从 exe 目录加载系统壁纸
    void initSystemWallpapers();

    /// \brief 版本更新时刷新系统壁纸（与 exe 目录比对，清理已删除的，新增的追加）
    void refreshSystemWallpapers();

    /// \brief 从用户 CustomBackground 目录加载自定义壁纸
    void initCustomWallpapers();

    /// \brief 序列化壁纸配置为独立 JSON
    QJsonObject wallpaperConfigToJson() const;

    /// \brief 从独立 JSON 恢复壁纸配置
    void wallpaperConfigFromJson(QJsonObject t_root);

    /// \brief 保存壁纸配置，按 wallpaperStorageScope 决定落盘位置
    bool saveWallpaperConfig() const;

    /// \brief 异步保存壁纸配置，按 wallpaperStorageScope 决定落盘位置
    void saveWallpaperConfigAsync() const;

    /// \brief 从独立壁纸配置恢复，AppLocalJson 模式下成功返回 true
    bool loadWallpaperConfig();

    /// \brief 序列化为 JSON
    QJsonObject toJson() const;

    /// \brief 从 JSON 反序列化
    void fromJson(QJsonObject t_root); // 按值传递，迁移链可修改

    /// \brief 保存到用户目录（根据 is_encrypted_save_ 决定加密或明文）
    bool saveToDisk() const;

    /// \brief 保存到用户目录（根据 is_encrypted_save_ 决定加密或明文，异步）
    void saveToDiskAsync() const;

    /// \brief 从用户目录读取（先尝试加密读，失败则明文读；自动迁移并回写旧版本数据）
    bool loadFromDisk();

};

/// \brief 全局用户信息（LoginAndActivationCode.cpp 定义）
extern UserInformation g_user_information;



#endif // API_GLOBAL_H
