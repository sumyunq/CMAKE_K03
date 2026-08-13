#include "modules/Common/DeviceRegistry.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMutexLocker>
#include <QRegularExpression>
#include <QStandardPaths>

#include "data/api_global.h" ///< XIBERIA_X_HUB_Utils
#include "modules/CommunityModule/infrastructure/logger/logger.h" ///< LOG_INFO 调试日志

namespace {

// 机型标签硬编码表：键<设备ID, 是否测试服> → {标签名称, API 完整名称}
// deviceLabel() 与 shortDisplayName() 共用，避免两处硬编码漂移
struct DeviceLabel {
    QString label;
    QString fullName;
};
const QHash<QPair<int, bool>, DeviceLabel> kDeviceLabelMap = {
    {{13,  true },  {"T10有线",        "XIBERIA T10G"}},
    {{-99, false},  {"T10有线",        "XIBERIA T10G"}},
    {{34,  true },  {"K03有线版二代",  "XIBERIA K03"}},
    {{29,  false},  {"K03有线版二代",  "XIBERIA K03"}},
    {{11,  true },  {"T10无线",        "XIBERIA T10 Wireless"}},
    {{11,  false},  {"T10无线",        "XIBERIA T10 Wireless"}},
    {{15,  true },  {"T10有线",        "XIBERIA T10"}},
    {{12,  false},  {"T10有线",        "XIBERIA T10"}},
    {{20,  true },  {"K03S超竞版",     "XIBERIA K03S"}},
    {{14,  false},  {"K03S超竞版",     "XIBERIA K03S"}},
    {{21,  true },  {"K06S 无线",      "XIBERIA K06S Wireless"}},
    {{19,  false},  {"K06S 无线",      "XIBERIA K06S Wireless"}},
    {{25,  true },  {"T7",             "CROCIRIS T7"}},
    {{20,  false},  {"T7",             "CROCIRIS T7"}},
    {{35,  true },  {"T7 GT",          "CROCIRIS T7 GT"}},
    {{30,  false},  {"T7 GT",          "CROCIRIS T7 GT"}},
    {{36,  true },  {"S21无线智充版",  "XIBERIA S21 Wireless 7.1"}},
};

} // namespace

namespace DeSheng {

// 饿汉单例
DeviceRegistry DeviceRegistry::s_instance_;

DeviceRegistry::DeviceRegistry() {}

DeviceRegistry &DeviceRegistry::instance()
{
    return s_instance_;
}

// 数据表
const DeviceRegistry::DeviceMap &DeviceRegistry::deviceMap() const
{
    return cl_device_map_;
}

std::shared_ptr<DeviceInfo> DeviceRegistry::find(const QString &t_type, int t_colorIndex) const
{
    auto t_it = cl_device_map_.constFind({t_type, t_colorIndex});
    if (t_it != cl_device_map_.constEnd())
        return t_it.value();
    return nullptr;
}

QString DeviceRegistry::deviceSysTypeName(ushort vid, ushort pid) const
{
    QMutexLocker t_locker(&cl_mutex_);
    for (auto t_it = cl_device_map_.constBegin(); t_it != cl_device_map_.constEnd(); ++t_it) {
        const auto &t_info = *t_it.value();
        if (t_info.SelDev_VID == vid && t_info.SelDev_PID == pid && !t_info.DeviceSysTypeName.isEmpty())
            return t_info.DeviceSysTypeName;
    }
    return QString();
}

QString DeviceRegistry::deviceLabel(int t_deviceId, bool t_isTest)
{
    auto t_it = kDeviceLabelMap.constFind({t_deviceId, t_isTest});
    return t_it != kDeviceLabelMap.constEnd() ? t_it->label : QString();
}

QString DeviceRegistry::deviceNameParam(const QString &t_shortLabel)
{
    // 机型筛选项中文短名（CMake SchemeFilterPopup 选项）→ API device_name 英文全名
    // 未收录的短名原样返回（不猜测）；空串由上层空值守卫跳过
    // key 统一 toCaseFolded() 归一化，与 shortDisplayName 风格一致（兼容英文短名大小写变体）
    static const QHash<QString, QString> t_map = [] {
        QHash<QString, QString> t_map;
        t_map.insert(QStringLiteral("T10有线").toCaseFolded(), QStringLiteral("XIBERIA T10"));
        t_map.insert(QStringLiteral("T10无线").toCaseFolded(), QStringLiteral("XIBERIA T10 Wireless"));
        t_map.insert(QStringLiteral("K03S超竞版").toCaseFolded(), QStringLiteral("XIBERIA K03S"));
        t_map.insert(QStringLiteral("K03有线版二代").toCaseFolded(), QStringLiteral("XIBERIA K03"));
        t_map.insert(QStringLiteral("K06S").toCaseFolded(), QStringLiteral("XIBERIA K06S Wireless"));
        t_map.insert(QStringLiteral("T7").toCaseFolded(), QStringLiteral("CROCIRIS T7"));
        t_map.insert(QStringLiteral("T7 GT").toCaseFolded(), QStringLiteral("CROCIRIS T7 GT"));  ///< 待确认：kLabelMap 无此条目
        t_map.insert(QStringLiteral("S21无线智充版").toCaseFolded(), QStringLiteral("XIBERIA S21 Wireless 7.1"));
        return t_map;
    }();
    return t_map.value(t_shortLabel.toCaseFolded(), t_shortLabel);
}

QString DeviceRegistry::shortDisplayName(const QString &fullName)
{
    LOG_INFO("shortDisplayName 进入, fullName={}", fullName.toStdString());  ///< 调试日志

    // 机型标签映射：API 完整英文名称 → 中文标签名称（按 kDeviceLabelMap 倒推）
    // "XIBERIA T10G" → "T10有线"；未收录机型回退正则截断，保持可读性
    // key 统一 toCaseFolded() 归一化，使机型名匹配不区分大小写（如 "xiberia t10g" 也能命中）
    static const QHash<QString, QString> t_reverse = [] {
        QHash<QString, QString> t_map;
        for (auto t_it = kDeviceLabelMap.constBegin(); t_it != kDeviceLabelMap.constEnd(); ++t_it) {
            if (!t_it->fullName.isEmpty())
                t_map.insert(t_it->fullName.toCaseFolded(), t_it->label);
        }
        return t_map;
    }();
    QString t_label = t_reverse.value(fullName.toCaseFolded());
    if (!t_label.isEmpty())
        return t_label;

    // fallback: 正则截断（大小写不敏感，兼容全小写机型名）
    QString t_name = fullName;
    static const QRegularExpression t_prefix(R"(^[A-Z]{2,}\s+)",
                                             QRegularExpression::CaseInsensitiveOption);
    t_name.remove(t_prefix);
    static const QRegularExpression t_suffix(R"(\s*[（(]\d+[）)])");
    t_name.remove(t_suffix);

    LOG_INFO("shortDisplayName 结束, result={}", t_name.toStdString());  ///< 调试日志
    return t_name;
}

// 配置文件路径
QString DeviceRegistry::configFilePath()
{
    return QFileInfo(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation)).absolutePath()
           + "/XIBERIA X HUB/ProgramData/deviceInfo/DevInfo.dat";
}

// 初始化
void DeviceRegistry::init()
{
    const QString t_path = configFilePath();

    QDir t_dir;
    t_dir.mkpath(QFileInfo(t_path).absolutePath());

    QFile t_file(t_path);
    if (!t_file.exists()) {
        defaultConfig();
        save(t_path, 0);
    } else {
        if (!load(t_path)) {
            defaultConfig();
            save(t_path, 0);
        }
    }
}

// 持久化
bool DeviceRegistry::save(const QString &t_filePath, int t_mode) const
{
    QMutexLocker t_locker(&cl_mutex_);

    QJsonObject t_root;
    t_root["version"] = kConfigVersion;

    for (auto t_it = cl_device_map_.constBegin(); t_it != cl_device_map_.constEnd(); ++t_it) {
        QString t_key = t_it.key().first + "_" + QString::number(t_it.key().second);
        t_root[t_key] = t_it.value()->toJson();
    }

    if (t_mode != -1) {
        char t_xor = 0x5A;
        QByteArray t_data = QJsonDocument(t_root).toJson();
        for (int t_i = 0; t_i < t_data.size(); ++t_i)
            t_data[t_i] = t_data[t_i] ^ t_xor;
        QFile t_file(t_filePath);
        if (t_file.open(QIODevice::WriteOnly)) {
            t_file.write(t_data.toBase64());
            t_file.close();
        }
    } else {
        XIBERIA_X_HUB_Utils::saveJsonEncryptedAsync(t_filePath, t_root);
    }
    return true;
}

bool DeviceRegistry::load(const QString &t_filePath)
{
    QMutexLocker t_locker(&cl_mutex_);

    QJsonObject t_root = XIBERIA_X_HUB_Utils::loadJsonEncrypted(t_filePath);
    if (t_root.isEmpty())
        return false;

    int t_ver = t_root["version"].toInt(-1);
    if (t_ver != kConfigVersion) {
        qWarning() << "DeviceRegistry: config version mismatch, file=" << t_ver
                   << "current=" << kConfigVersion;
        return false;
    }

    cl_device_map_.clear();

    for (auto t_it = t_root.begin(); t_it != t_root.end(); ++t_it) {
        if (t_it.key() == "version") continue;
        QJsonObject t_obj = t_it.value().toObject();
        int t_underscore = t_it.key().lastIndexOf('_');
        if (t_underscore < 0) continue;
        QString t_type = t_it.key().left(t_underscore);
        int t_idx = t_it.key().midRef(t_underscore + 1).toInt();
        cl_device_map_.insert({t_type, t_idx},
                              std::make_shared<DeviceInfo>(DeviceInfo::fromJson(t_obj)));
    }
    return true;
}

void DeviceRegistry::defaultConfig()
{
    cl_device_map_.clear();

    const QString QR_T10  = ":/Skin/Images/DevSel/QrCode/T10.png";
    const QString QR_K03  = ":/Skin/Images/DevSel/QrCode/K03.png";
    const QString QR_K06S = ":/Skin/Images/DevSel/QrCode/K06S.png";
    const QString QR_T7   = ":/Skin/Images/DevSel/QrCode/T7.png";
    const QString QR_T7GT = ":/Skin/Images/DevSel/QrCode/T7.png"; ///< T7 GT 暂复用 T7 二维码

    // 设备说明书 URL（h-por.html?groupId=xx，型号级；K03S 普通版未提供暂留空）
    const QString MANUAL_T10_Wired    = "https://www.xiberia.net/h-por.html?groupId=16";
    const QString MANUAL_T10_Wireless = "https://www.xiberia.net/h-por.html?groupId=20";
    const QString MANUAL_K03S_Super   = "https://www.xiberia.net/h-por.html?groupId=24";
    const QString MANUAL_K03          = "https://www.xiberia.net/h-por.html?groupId=26";
    const QString MANUAL_K06S         = "https://www.xiberia.net/h-por.html?groupId=18";
    const QString MANUAL_T7           = "https://www.xiberia.net/h-por.html?groupId=22";
    const QString MANUAL_T7GT         = "https://www.xiberia.net/h-col-188.html";
    const QString MANUAL_S21          = "https://www.xiberia.net/h-por.html?groupId=27";

    {
        // T10有线
        {
            auto t_info = std::make_shared<DeSheng::DeviceInfo>();
            t_info->DeviceTypeName = "T10有线"; t_info->DeviceColorName = "玄墨黑";
            t_info->DeviceColorPixmapPath = ":/Skin/Images/DevSel/selectionPage/T10wired_RGB_000000_0.png";
            t_info->DeviceColorRGB = "#000000"; t_info->isChecked = true;
            t_info->DeviceHomePagePixmapPath = ":/Skin/Images/DevSel/homePageDevice/T10wired_homePage_RGB_000000_0.png";
            t_info->DeviceHomePageTopLeftPixmapPath_normal = ":/Skin/Images/DevSel/leftTopIcon/T10wired_RGB_000000_0_normal.png";
            t_info->DeviceHomePageTopLeftPixmapPath_abnormal = ":/Skin/Images/DevSel/leftTopIcon/T10wired_RGB_000000_0_abnormal.png";
            t_info->DeviceMoreSetPixmapPath = ":/Skin/Images/DevSel/moreSetIPageDeviceIcon/T10wired_moreSetPage_RGB_000000_0.png";
            t_info->DeviceMoreSetQrCodePixmapPath = QR_T10;
            t_info->DeviceManualUrl = MANUAL_T10_Wired;
            t_info->DeviceSysTypeName = "T10";
            cl_device_map_.insert({"T10有线", 0}, t_info);
        }
        {
            auto t_info = std::make_shared<DeSheng::DeviceInfo>();
            t_info->DeviceTypeName = "T10有线"; t_info->DeviceColorName = "蚀金黑";
            t_info->DeviceColorPixmapPath = ":/Skin/Images/DevSel/selectionPage/T10wired_RGB_FFE251_1.png";
            t_info->DeviceColorRGB = "#FFE251"; t_info->isChecked = false;
            t_info->DeviceHomePagePixmapPath = ":/Skin/Images/DevSel/homePageDevice/T10wired_homePage_RGB_FFE251_1.png";
            t_info->DeviceHomePageTopLeftPixmapPath_normal = ":/Skin/Images/DevSel/leftTopIcon/T10wired_RGB_FFE251_1_normal.png";
            t_info->DeviceHomePageTopLeftPixmapPath_abnormal = ":/Skin/Images/DevSel/leftTopIcon/T10wired_RGB_FFE251_1_abnormal.png";
            t_info->DeviceMoreSetPixmapPath = ":/Skin/Images/DevSel/moreSetIPageDeviceIcon/T10wired_moreSetPage_RGB_FFE251_1.png";
            t_info->DeviceMoreSetQrCodePixmapPath = QR_T10;
            t_info->DeviceManualUrl = MANUAL_T10_Wired;
            t_info->DeviceSysTypeName = "T10";
            cl_device_map_.insert({"T10有线", 1}, t_info);
        }
        {
            auto t_info = std::make_shared<DeSheng::DeviceInfo>();
            t_info->DeviceTypeName = "T10有线"; t_info->DeviceColorName = "凌空灰";
            t_info->DeviceColorPixmapPath = ":/Skin/Images/DevSel/selectionPage/T10wired_RGB_919393_2.png";
            t_info->DeviceColorRGB = "#919393"; t_info->isChecked = false;
            t_info->DeviceHomePagePixmapPath = ":/Skin/Images/DevSel/homePageDevice/T10wired_homePage_RGB_919393_2.png";
            t_info->DeviceHomePageTopLeftPixmapPath_normal = ":/Skin/Images/DevSel/leftTopIcon/T10wired_RGB_919393_2_normal.png";
            t_info->DeviceHomePageTopLeftPixmapPath_abnormal = ":/Skin/Images/DevSel/leftTopIcon/T10wired_RGB_919393_2_abnormal.png";
            t_info->DeviceMoreSetPixmapPath = ":/Skin/Images/DevSel/moreSetIPageDeviceIcon/T10wired_moreSetPage_RGB_919393_2.png";
            t_info->DeviceMoreSetQrCodePixmapPath = QR_T10;
            t_info->DeviceManualUrl = MANUAL_T10_Wired;
            t_info->DeviceSysTypeName = "T10";
            cl_device_map_.insert({"T10有线", 2}, t_info);
        }
        {
            auto t_info = std::make_shared<DeSheng::DeviceInfo>();
            t_info->DeviceTypeName = "T10有线"; t_info->DeviceColorName = "皓月银";
            t_info->DeviceColorPixmapPath = ":/Skin/Images/DevSel/selectionPage/T10wired_RGB_B7B7B7_3.png";
            t_info->DeviceColorRGB = "#B7B7B7"; t_info->isChecked = false;
            t_info->DeviceHomePagePixmapPath = ":/Skin/Images/DevSel/homePageDevice/T10wired_homePage_RGB_B7B7B7_3.png";
            t_info->DeviceHomePageTopLeftPixmapPath_normal = ":/Skin/Images/DevSel/leftTopIcon/T10wired_RGB_B7B7B7_3_normal.png";
            t_info->DeviceHomePageTopLeftPixmapPath_abnormal = ":/Skin/Images/DevSel/leftTopIcon/T10wired_RGB_B7B7B7_3_abnormal.png";
            t_info->DeviceMoreSetPixmapPath = ":/Skin/Images/DevSel/moreSetIPageDeviceIcon/T10wired_moreSetPage_RGB_B7B7B7_3.png";
            t_info->DeviceMoreSetQrCodePixmapPath = QR_T10;
            t_info->DeviceManualUrl = MANUAL_T10_Wired;
            t_info->DeviceSysTypeName = "T10";
            cl_device_map_.insert({"T10有线", 3}, t_info);
        }

        // T10无线
        {
            auto t_info = std::make_shared<DeSheng::DeviceInfo>();
            t_info->DeviceTypeName = "T10无线"; t_info->DeviceColorName = "玄墨黑";
            t_info->DeviceColorPixmapPath = ":/Skin/Images/DevSel/selectionPage/T10wireless_RGB_000000_0.png";
            t_info->DeviceColorRGB = "#000000"; t_info->isChecked = true;
            t_info->DeviceHomePagePixmapPath = ":/Skin/Images/DevSel/homePageDevice/T10wireless_homePage_RGB_000000_0.png";
            t_info->DeviceHomePageTopLeftPixmapPath_normal = ":/Skin/Images/DevSel/leftTopIcon/T10wireless_RGB_000000_0_normal.png";
            t_info->DeviceHomePageTopLeftPixmapPath_abnormal = ":/Skin/Images/DevSel/leftTopIcon/T10wireless_RGB_000000_0_abnormal.png";
            t_info->DeviceMoreSetPixmapPath = ":/Skin/Images/DevSel/moreSetIPageDeviceIcon/T10wireless_moreSetPage_RGB_000000_0.png";
            t_info->DeviceMoreSetQrCodePixmapPath = QR_T10;
            t_info->DeviceManualUrl = MANUAL_T10_Wireless;
            t_info->DeviceSysTypeName = "T10Wireless";
            cl_device_map_.insert({"T10无线", 0}, t_info);
        }
        {
            auto t_info = std::make_shared<DeSheng::DeviceInfo>();
            t_info->DeviceTypeName = "T10无线"; t_info->DeviceColorName = "香槟金";
            t_info->DeviceColorPixmapPath = ":/Skin/Images/DevSel/selectionPage/T10wireless_RGB_FCDEB2_1.png";
            t_info->DeviceColorRGB = "#FCDEB2"; t_info->isChecked = false;
            t_info->DeviceHomePagePixmapPath = ":/Skin/Images/DevSel/homePageDevice/T10wireless_homePage_RGB_FCDEB2_1.png";
            t_info->DeviceHomePageTopLeftPixmapPath_normal = ":/Skin/Images/DevSel/leftTopIcon/T10wireless_RGB_FCDEB2_1_normal.png";
            t_info->DeviceHomePageTopLeftPixmapPath_abnormal = ":/Skin/Images/DevSel/leftTopIcon/T10wireless_RGB_FCDEB2_1_abnormal.png";
            t_info->DeviceMoreSetPixmapPath = ":/Skin/Images/DevSel/moreSetIPageDeviceIcon/T10wireless_moreSetPage_RGB_FCDEB2_1.png";
            t_info->DeviceMoreSetQrCodePixmapPath = QR_T10;
            t_info->DeviceManualUrl = MANUAL_T10_Wireless;
            t_info->DeviceSysTypeName = "T10Wireless";
            cl_device_map_.insert({"T10无线", 1}, t_info);
        }
        {
            auto t_info = std::make_shared<DeSheng::DeviceInfo>();
            t_info->DeviceTypeName = "T10无线"; t_info->DeviceColorName = "皓月银";
            t_info->DeviceColorPixmapPath = ":/Skin/Images/DevSel/selectionPage/T10wireless_RGB_B7B7B7_2.png";
            t_info->DeviceColorRGB = "#B7B7B7"; t_info->isChecked = false;
            t_info->DeviceHomePagePixmapPath = ":/Skin/Images/DevSel/homePageDevice/T10wireless_homePage_RGB_B7B7B7_2.png";
            t_info->DeviceHomePageTopLeftPixmapPath_normal = ":/Skin/Images/DevSel/leftTopIcon/T10wireless_RGB_B7B7B7_2_normal.png";
            t_info->DeviceHomePageTopLeftPixmapPath_abnormal = ":/Skin/Images/DevSel/leftTopIcon/T10wireless_RGB_B7B7B7_2_abnormal.png";
            t_info->DeviceMoreSetPixmapPath = ":/Skin/Images/DevSel/moreSetIPageDeviceIcon/T10wireless_moreSetPage_RGB_B7B7B7_2.png";
            t_info->DeviceMoreSetQrCodePixmapPath = QR_T10;
            t_info->DeviceManualUrl = MANUAL_T10_Wireless;
            t_info->DeviceSysTypeName = "T10Wireless";
            cl_device_map_.insert({"T10无线", 2}, t_info);
        }

        // K03S
        {
            auto t_info = std::make_shared<DeSheng::DeviceInfo>();
            t_info->DeviceTypeName = "K03S"; t_info->DeviceColorName = "武士黑";
            t_info->DeviceColorPixmapPath = ":/Skin/Images/DevSel/selectionPage/K03S_RGB_000000_0.png";
            t_info->DeviceColorRGB = "#000000"; t_info->isChecked = true;
            t_info->DeviceHomePagePixmapPath = ":/Skin/Images/DevSel/homePageDevice/K03S_homePage_RGB_000000_0.png";
            t_info->DeviceHomePageTopLeftPixmapPath_normal = ":/Skin/Images/DevSel/leftTopIcon/K03S_RGB_000000_0_normal.png";
            t_info->DeviceHomePageTopLeftPixmapPath_abnormal = ":/Skin/Images/DevSel/leftTopIcon/K03S_RGB_000000_0_abnormal.png";
            t_info->DeviceMoreSetPixmapPath = ":/Skin/Images/DevSel/moreSetIPageDeviceIcon/K03S_moreSetPage_RGB_000000_0.png";
            t_info->DeviceMoreSetQrCodePixmapPath = QR_K03;
            t_info->DeviceSysTypeName = "K03S";
            cl_device_map_.insert({"K03S", 0}, t_info);
        }
        {
            auto t_info = std::make_shared<DeSheng::DeviceInfo>();
            t_info->DeviceTypeName = "K03S"; t_info->DeviceColorName = "熊猫白";
            t_info->DeviceColorPixmapPath = ":/Skin/Images/DevSel/selectionPage/K03S_RGB_FFFFFF_1.png";
            t_info->DeviceColorRGB = "#FFFFFF"; t_info->isChecked = false;
            t_info->DeviceHomePagePixmapPath = ":/Skin/Images/DevSel/homePageDevice/K03S_homePage_RGB_FFFFFF_1.png";
            t_info->DeviceHomePageTopLeftPixmapPath_normal = ":/Skin/Images/DevSel/leftTopIcon/K03S_RGB_FFFFFF_1_normal.png";
            t_info->DeviceHomePageTopLeftPixmapPath_abnormal = ":/Skin/Images/DevSel/leftTopIcon/K03S_RGB_FFFFFF_1_abnormal.png";
            t_info->DeviceMoreSetPixmapPath = ":/Skin/Images/DevSel/moreSetIPageDeviceIcon/K03S_moreSetPage_RGB_FFFFFF_1.png";
            t_info->DeviceMoreSetQrCodePixmapPath = QR_K03;
            t_info->DeviceSysTypeName = "K03S";
            cl_device_map_.insert({"K03S", 1}, t_info);
        }
        {
            auto t_info = std::make_shared<DeSheng::DeviceInfo>();
            t_info->DeviceTypeName = "K03S"; t_info->DeviceColorName = "极光粉";
            t_info->DeviceColorPixmapPath = ":/Skin/Images/DevSel/selectionPage/K03S_RGB_F9CAD6_2.png";
            t_info->DeviceColorRGB = "#F9CAD6"; t_info->isChecked = false;
            t_info->DeviceHomePagePixmapPath = ":/Skin/Images/DevSel/homePageDevice/K03S_homePage_RGB_F9CAD6_2.png";
            t_info->DeviceHomePageTopLeftPixmapPath_normal = ":/Skin/Images/DevSel/leftTopIcon/K03S_RGB_F9CAD6_2_normal.png";
            t_info->DeviceHomePageTopLeftPixmapPath_abnormal = ":/Skin/Images/DevSel/leftTopIcon/K03S_RGB_F9CAD6_2_abnormal.png";
            t_info->DeviceMoreSetPixmapPath = ":/Skin/Images/DevSel/moreSetIPageDeviceIcon/K03S_moreSetPage_RGB_F9CAD6_2.png";
            t_info->DeviceMoreSetQrCodePixmapPath = QR_K03;
            t_info->DeviceSysTypeName = "K03S";
            cl_device_map_.insert({"K03S", 2}, t_info);
        }
        {
            auto t_info = std::make_shared<DeSheng::DeviceInfo>();
            t_info->DeviceTypeName = "K03S"; t_info->DeviceColorName = "宝石蓝";
            t_info->DeviceColorPixmapPath = ":/Skin/Images/DevSel/selectionPage/K03S_RGB_6BA6C6_3.png";
            t_info->DeviceColorRGB = "#6BA6C6"; t_info->isChecked = false;
            t_info->DeviceHomePagePixmapPath = ":/Skin/Images/DevSel/homePageDevice/K03S_homePage_RGB_6BA6C6_3.png";
            t_info->DeviceHomePageTopLeftPixmapPath_normal = ":/Skin/Images/DevSel/leftTopIcon/K03S_RGB_6BA6C6_3_normal.png";
            t_info->DeviceHomePageTopLeftPixmapPath_abnormal = ":/Skin/Images/DevSel/leftTopIcon/K03S_RGB_6BA6C6_3_abnormal.png";
            t_info->DeviceMoreSetPixmapPath = ":/Skin/Images/DevSel/moreSetIPageDeviceIcon/K03S_moreSetPage_RGB_6BA6C6_3.png";
            t_info->DeviceMoreSetQrCodePixmapPath = QR_K03;
            t_info->DeviceSysTypeName = "K03S";
            cl_device_map_.insert({"K03S", 3}, t_info);
        }

        // K03有线版
        {
            auto t_info = std::make_shared<DeSheng::DeviceInfo>();
            t_info->DeviceTypeName = "K03有线版"; t_info->DeviceColorName = "武士黑";
            t_info->DeviceColorPixmapPath = ":/Skin/Images/DevSel/selectionPage/K03S_wired_RGB_000000_0.png";
            t_info->DeviceColorRGB = "#000000"; t_info->isChecked = true;
            t_info->DeviceHomePagePixmapPath = ":/Skin/Images/DevSel/homePageDevice/K03S_wired_homePage_RGB_000000_0.png";
            t_info->DeviceHomePageTopLeftPixmapPath_normal = ":/Skin/Images/DevSel/leftTopIcon/K03S_wired_RGB_000000_0_normal.png";
            t_info->DeviceHomePageTopLeftPixmapPath_abnormal = ":/Skin/Images/DevSel/leftTopIcon/K03S_wired_RGB_000000_0_abnormal.png";
            t_info->DeviceMoreSetPixmapPath = ":/Skin/Images/DevSel/moreSetIPageDeviceIcon/K03wired_moreSetPage_RGB_000000_0.png";
            t_info->DeviceMoreSetQrCodePixmapPath = QR_K03;
            t_info->DeviceManualUrl = MANUAL_K03;
            t_info->DeviceSysTypeName = "K03有线版";
            cl_device_map_.insert({"K03有线版", 0}, t_info);
        }
        {
            auto t_info = std::make_shared<DeSheng::DeviceInfo>();
            t_info->DeviceTypeName = "K03有线版"; t_info->DeviceColorName = "熊猫白";
            t_info->DeviceColorPixmapPath = ":/Skin/Images/DevSel/selectionPage/K03S_wired_RGB_FFFFFF_1.png";
            t_info->DeviceColorRGB = "#FFFFFF"; t_info->isChecked = false;
            t_info->DeviceHomePagePixmapPath = ":/Skin/Images/DevSel/homePageDevice/K03S_wired_homePage_RGB_FFFFFF_1.png";
            t_info->DeviceHomePageTopLeftPixmapPath_normal = ":/Skin/Images/DevSel/leftTopIcon/K03S_wired_RGB_FFFFFF_1_normal.png";
            t_info->DeviceHomePageTopLeftPixmapPath_abnormal = ":/Skin/Images/DevSel/leftTopIcon/K03S_wired_RGB_FFFFFF_1_abnormal.png";
            t_info->DeviceMoreSetPixmapPath = ":/Skin/Images/DevSel/moreSetIPageDeviceIcon/K03wired_moreSetPage_RGB_FFFFFF_1.png";
            t_info->DeviceMoreSetQrCodePixmapPath = QR_K03;
            t_info->DeviceManualUrl = MANUAL_K03;
            t_info->DeviceSysTypeName = "K03有线版";
            cl_device_map_.insert({"K03有线版", 1}, t_info);
        }
        {
            auto t_info = std::make_shared<DeSheng::DeviceInfo>();
            t_info->DeviceTypeName = "K03有线版"; t_info->DeviceColorName = "极光粉";
            t_info->DeviceColorPixmapPath = ":/Skin/Images/DevSel/selectionPage/K03S_wired_RGB_F9CAD6_2.png";
            t_info->DeviceColorRGB = "#F9CAD6"; t_info->isChecked = false;
            t_info->DeviceHomePagePixmapPath = ":/Skin/Images/DevSel/homePageDevice/K03S_wired_homePage_RGB_F9CAD6_2.png";
            t_info->DeviceHomePageTopLeftPixmapPath_normal = ":/Skin/Images/DevSel/leftTopIcon/K03S_wired_RGB_F9CAD6_2_normal.png";
            t_info->DeviceHomePageTopLeftPixmapPath_abnormal = ":/Skin/Images/DevSel/leftTopIcon/K03S_wired_RGB_F9CAD6_2_abnormal.png";
            t_info->DeviceMoreSetPixmapPath = ":/Skin/Images/DevSel/moreSetIPageDeviceIcon/K03wired_moreSetPage_RGB_F9CAD6_2.png";
            t_info->DeviceMoreSetQrCodePixmapPath = QR_K03;
            t_info->DeviceManualUrl = MANUAL_K03;
            t_info->DeviceSysTypeName = "K03有线版";
            cl_device_map_.insert({"K03有线版", 2}, t_info);
        }
        {
            auto t_info = std::make_shared<DeSheng::DeviceInfo>();
            t_info->DeviceTypeName = "K03有线版"; t_info->DeviceColorName = "冰川白";
            t_info->DeviceColorPixmapPath = ":/Skin/Images/DevSel/selectionPage/K03S_wired_RGB_BBBEBF_3.png";
            t_info->DeviceColorRGB = "#BBBEBF"; t_info->isChecked = false;
            t_info->DeviceHomePagePixmapPath = ":/Skin/Images/DevSel/homePageDevice/K03S_wired_homePage_RGB_BBBEBF_3.png";
            t_info->DeviceHomePageTopLeftPixmapPath_normal = ":/Skin/Images/DevSel/leftTopIcon/K03S_wired_RGB_BBBEBF_3_normal.png";
            t_info->DeviceHomePageTopLeftPixmapPath_abnormal = ":/Skin/Images/DevSel/leftTopIcon/K03S_wired_RGB_BBBEBF_3_abnormal.png";
            t_info->DeviceMoreSetPixmapPath = ":/Skin/Images/DevSel/moreSetIPageDeviceIcon/K03wired_moreSetPage_RGB_BBBEBF_3.png";
            t_info->DeviceMoreSetQrCodePixmapPath = QR_K03;
            t_info->DeviceManualUrl = MANUAL_K03;
            t_info->DeviceSysTypeName = "K03有线版";
            cl_device_map_.insert({"K03有线版", 3}, t_info);
        }

        // K03S超竞版
        {
            auto t_info = std::make_shared<DeSheng::DeviceInfo>();
            t_info->DeviceTypeName = "K03S超竞版"; t_info->DeviceColorName = "武士黑";
            t_info->DeviceColorPixmapPath = ":/Skin/Images/DevSel/selectionPage/K03S_RGB_000000_0.png";
            t_info->DeviceColorRGB = "#000000"; t_info->isChecked = true;
            t_info->DeviceHomePagePixmapPath = ":/Skin/Images/DevSel/homePageDevice/K03S_homePage_RGB_000000_0.png";
            t_info->DeviceHomePageTopLeftPixmapPath_normal = ":/Skin/Images/DevSel/leftTopIcon/K03S_Super_RGB_000000_0_normal.png";
            t_info->DeviceHomePageTopLeftPixmapPath_abnormal = ":/Skin/Images/DevSel/leftTopIcon/K03S_Super_RGB_000000_0_abnormal.png";
            t_info->DeviceMoreSetPixmapPath = ":/Skin/Images/DevSel/moreSetIPageDeviceIcon/K03S_Super_moreSetPage_RGB_000000_0.png";
            t_info->DeviceMoreSetQrCodePixmapPath = QR_K03;
            t_info->DeviceManualUrl = MANUAL_K03S_Super;
            t_info->DeviceSysTypeName = "K03S超竞版";
            cl_device_map_.insert({"K03S超竞版", 0}, t_info);
        }
        {
            auto t_info = std::make_shared<DeSheng::DeviceInfo>();
            t_info->DeviceTypeName = "K03S超竞版"; t_info->DeviceColorName = "熊猫白";
            t_info->DeviceColorPixmapPath = ":/Skin/Images/DevSel/selectionPage/K03S_RGB_FFFFFF_1.png";
            t_info->DeviceColorRGB = "#FFFFFF"; t_info->isChecked = false;
            t_info->DeviceHomePagePixmapPath = ":/Skin/Images/DevSel/homePageDevice/K03S_homePage_RGB_FFFFFF_1.png";
            t_info->DeviceHomePageTopLeftPixmapPath_normal = ":/Skin/Images/DevSel/leftTopIcon/K03S_Super_RGB_FFFFFF_1_normal.png";
            t_info->DeviceHomePageTopLeftPixmapPath_abnormal = ":/Skin/Images/DevSel/leftTopIcon/K03S_Super_RGB_FFFFFF_1_abnormal.png";
            t_info->DeviceMoreSetPixmapPath = ":/Skin/Images/DevSel/moreSetIPageDeviceIcon/K03S_Super_moreSetPage_RGB_FFFFFF_1.png";
            t_info->DeviceMoreSetQrCodePixmapPath = QR_K03;
            t_info->DeviceManualUrl = MANUAL_K03S_Super;
            t_info->DeviceSysTypeName = "K03S超竞版";
            cl_device_map_.insert({"K03S超竞版", 1}, t_info);
        }
        {
            auto t_info = std::make_shared<DeSheng::DeviceInfo>();
            t_info->DeviceTypeName = "K03S超竞版"; t_info->DeviceColorName = "极光粉";
            t_info->DeviceColorPixmapPath = ":/Skin/Images/DevSel/selectionPage/K03S_RGB_F9CAD6_2.png";
            t_info->DeviceColorRGB = "#F9CAD6"; t_info->isChecked = false;
            t_info->DeviceHomePagePixmapPath = ":/Skin/Images/DevSel/homePageDevice/K03S_homePage_RGB_F9CAD6_2.png";
            t_info->DeviceHomePageTopLeftPixmapPath_normal = ":/Skin/Images/DevSel/leftTopIcon/K03S_Super_RGB_F9CAD6_2_normal.png";
            t_info->DeviceHomePageTopLeftPixmapPath_abnormal = ":/Skin/Images/DevSel/leftTopIcon/K03S_Super_RGB_F9CAD6_2_abnormal.png";
            t_info->DeviceMoreSetPixmapPath = ":/Skin/Images/DevSel/moreSetIPageDeviceIcon/K03S_Super_moreSetPage_RGB_F9CAD6_2.png";
            t_info->DeviceMoreSetQrCodePixmapPath = QR_K03;
            t_info->DeviceManualUrl = MANUAL_K03S_Super;
            t_info->DeviceSysTypeName = "K03S超竞版";
            cl_device_map_.insert({"K03S超竞版", 2}, t_info);
        }
        {
            auto t_info = std::make_shared<DeSheng::DeviceInfo>();
            t_info->DeviceTypeName = "K03S超竞版"; t_info->DeviceColorName = "宝石蓝";
            t_info->DeviceColorPixmapPath = ":/Skin/Images/DevSel/selectionPage/K03S_RGB_6BA6C6_3.png";
            t_info->DeviceColorRGB = "#6BA6C6"; t_info->isChecked = false;
            t_info->DeviceHomePagePixmapPath = ":/Skin/Images/DevSel/homePageDevice/K03S_homePage_RGB_6BA6C6_3.png";
            t_info->DeviceHomePageTopLeftPixmapPath_normal = ":/Skin/Images/DevSel/leftTopIcon/K03S_Super_RGB_6BA6C6_3_normal.png";
            t_info->DeviceHomePageTopLeftPixmapPath_abnormal = ":/Skin/Images/DevSel/leftTopIcon/K03S_Super_RGB_6BA6C6_3_abnormal.png";
            t_info->DeviceMoreSetPixmapPath = ":/Skin/Images/DevSel/moreSetIPageDeviceIcon/K03S_Super_moreSetPage_RGB_6BA6C6_3.png";
            t_info->DeviceMoreSetQrCodePixmapPath = QR_K03;
            t_info->DeviceManualUrl = MANUAL_K03S_Super;
            t_info->DeviceSysTypeName = "K03S超竞版";
            cl_device_map_.insert({"K03S超竞版", 3}, t_info);
        }

        // K06S
        {
            auto t_info = std::make_shared<DeSheng::DeviceInfo>();
            t_info->DeviceTypeName = "K06S"; t_info->DeviceColorName = "黑色";
            t_info->DeviceColorPixmapPath = ":/Skin/Images/DevSel/selectionPage/K06S_RGB_000000_0.png";
            t_info->DeviceColorRGB = "#000000"; t_info->isChecked = true;
            t_info->DeviceHomePagePixmapPath = ":/Skin/Images/DevSel/homePageDevice/K06S_homePage_RGB_000000_0.png";
            t_info->DeviceHomePageTopLeftPixmapPath_normal = ":/Skin/Images/DevSel/leftTopIcon/K06S_RGB_000000_0_normal.png";
            t_info->DeviceHomePageTopLeftPixmapPath_abnormal = ":/Skin/Images/DevSel/leftTopIcon/K06S_RGB_000000_0_abnormal.png";
            t_info->DeviceMoreSetPixmapPath = ":/Skin/Images/DevSel/moreSetIPageDeviceIcon/K06S_moreSetPage_RGB_000000_0.png";
            t_info->DeviceMoreSetQrCodePixmapPath = QR_K06S;
            t_info->DeviceManualUrl = MANUAL_K06S;
            t_info->DeviceSysTypeName = "K06S";
            cl_device_map_.insert({"K06S", 0}, t_info);
        }
        {
            auto t_info = std::make_shared<DeSheng::DeviceInfo>();
            t_info->DeviceTypeName = "K06S"; t_info->DeviceColorName = "云贝白";
            t_info->DeviceColorPixmapPath = ":/Skin/Images/DevSel/selectionPage/K06S_RGB_FFFFFF_1.png";
            t_info->DeviceColorRGB = "#FFFFFF"; t_info->isChecked = false;
            t_info->DeviceHomePagePixmapPath = ":/Skin/Images/DevSel/homePageDevice/K06S_homePage_RGB_FFFFFF_1.png";
            t_info->DeviceHomePageTopLeftPixmapPath_normal = ":/Skin/Images/DevSel/leftTopIcon/K06S_RGB_FFFFFF_1_normal.png";
            t_info->DeviceHomePageTopLeftPixmapPath_abnormal = ":/Skin/Images/DevSel/leftTopIcon/K06S_RGB_FFFFFF_1_abnormal.png";
            t_info->DeviceMoreSetPixmapPath = ":/Skin/Images/DevSel/moreSetIPageDeviceIcon/K06S_moreSetPage_RGB_FFFFFF_1.png";
            t_info->DeviceMoreSetQrCodePixmapPath = QR_K06S;
            t_info->DeviceManualUrl = MANUAL_K06S;
            t_info->DeviceSysTypeName = "K06S";
            cl_device_map_.insert({"K06S", 1}, t_info);
        }

        // T7
        {
            auto t_info = std::make_shared<DeSheng::DeviceInfo>();
            t_info->DeviceTypeName = "T7"; t_info->DeviceColorName = "玄墨黑";
            t_info->DeviceColorPixmapPath = ":/Skin/Images/DevSel/selectionPage/T7_RGB_000000_0.png";
            t_info->DeviceColorRGB = "#000000"; t_info->isChecked = true;
            t_info->DeviceHomePagePixmapPath = ":/Skin/Images/DevSel/homePageDevice/T7_homePage_RGB_000000_0.png";
            t_info->DeviceHomePageTopLeftPixmapPath_normal = ":/Skin/Images/DevSel/leftTopIcon/T7_RGB_000000_0_normal.png";
            t_info->DeviceHomePageTopLeftPixmapPath_abnormal = ":/Skin/Images/DevSel/leftTopIcon/T7_RGB_000000_0_abnormal.png";
            t_info->DeviceMoreSetPixmapPath = ":/Skin/Images/DevSel/moreSetIPageDeviceIcon/T7_moreSetPage_RGB_000000_0.png";
            t_info->DeviceMoreSetQrCodePixmapPath = QR_T7;
            t_info->DeviceManualUrl = MANUAL_T7;
            t_info->DeviceSysTypeName = "T7";
            cl_device_map_.insert({"T7", 0}, t_info);
        }
        {
            auto t_info = std::make_shared<DeSheng::DeviceInfo>();
            t_info->DeviceTypeName = "T7"; t_info->DeviceColorName = "香槟金";
            t_info->DeviceColorPixmapPath = ":/Skin/Images/DevSel/selectionPage/T7_RGB_C7C0B8_1.png";
            t_info->DeviceColorRGB = "#C7C0B8"; t_info->isChecked = false;
            t_info->DeviceHomePagePixmapPath = ":/Skin/Images/DevSel/homePageDevice/T7_homePage_RGB_C7C0B8_1.png";
            t_info->DeviceHomePageTopLeftPixmapPath_normal = ":/Skin/Images/DevSel/leftTopIcon/T7_RGB_C7C0B8_1_normal.png";
            t_info->DeviceHomePageTopLeftPixmapPath_abnormal = ":/Skin/Images/DevSel/leftTopIcon/T7_RGB_C7C0B8_1_abnormal.png";
            t_info->DeviceMoreSetPixmapPath = ":/Skin/Images/DevSel/moreSetIPageDeviceIcon/T7_moreSetPage_RGB_C7C0B8_1.png";
            t_info->DeviceMoreSetQrCodePixmapPath = QR_T7;
            t_info->DeviceManualUrl = MANUAL_T7;
            t_info->DeviceSysTypeName = "T7";
            cl_device_map_.insert({"T7", 1}, t_info);
        }
        {
            auto t_info = std::make_shared<DeSheng::DeviceInfo>();
            t_info->DeviceTypeName = "T7"; t_info->DeviceColorName = "雪山粉";
            t_info->DeviceColorPixmapPath = ":/Skin/Images/DevSel/selectionPage/T7_RGB_D7C3CF_2.png";
            t_info->DeviceColorRGB = "#D7C3CF"; t_info->isChecked = false;
            t_info->DeviceHomePagePixmapPath = ":/Skin/Images/DevSel/homePageDevice/T7_homePage_RGB_D7C3CF_2.png";
            t_info->DeviceHomePageTopLeftPixmapPath_normal = ":/Skin/Images/DevSel/leftTopIcon/T7_RGB_D7C3CF_2_normal.png";
            t_info->DeviceHomePageTopLeftPixmapPath_abnormal = ":/Skin/Images/DevSel/leftTopIcon/T7_RGB_D7C3CF_2_abnormal.png";
            t_info->DeviceMoreSetPixmapPath = ":/Skin/Images/DevSel/moreSetIPageDeviceIcon/T7_moreSetPage_RGB_D7C3CF_2.png";
            t_info->DeviceMoreSetQrCodePixmapPath = QR_T7;
            t_info->DeviceManualUrl = MANUAL_T7;
            t_info->DeviceSysTypeName = "T7";
            cl_device_map_.insert({"T7", 2}, t_info);
        }
        {
            auto t_info = std::make_shared<DeSheng::DeviceInfo>();
            t_info->DeviceTypeName = "T7"; t_info->DeviceColorName = "闪电黄";
            t_info->DeviceColorPixmapPath = ":/Skin/Images/DevSel/selectionPage/T7_RGB_FFC652_3.png";
            t_info->DeviceColorRGB = "#FFC652"; t_info->isChecked = false;
            t_info->DeviceHomePagePixmapPath = ":/Skin/Images/DevSel/homePageDevice/T7_homePage_RGB_FFC652_3.png";
            t_info->DeviceHomePageTopLeftPixmapPath_normal = ":/Skin/Images/DevSel/leftTopIcon/T7_RGB_FFC652_3_normal.png";
            t_info->DeviceHomePageTopLeftPixmapPath_abnormal = ":/Skin/Images/DevSel/leftTopIcon/T7_RGB_FFC652_3_abnormal.png";
            t_info->DeviceMoreSetPixmapPath = ":/Skin/Images/DevSel/moreSetIPageDeviceIcon/T7_moreSetPage_RGB_FFC652_3.png";
            t_info->DeviceMoreSetQrCodePixmapPath = QR_T7;
            t_info->DeviceManualUrl = MANUAL_T7;
            t_info->DeviceSysTypeName = "T7";
            cl_device_map_.insert({"T7", 3}, t_info);
        }
        {
            auto t_info = std::make_shared<DeSheng::DeviceInfo>();
            t_info->DeviceTypeName = "T7"; t_info->DeviceColorName = "云杉绿";
            t_info->DeviceColorPixmapPath = ":/Skin/Images/DevSel/selectionPage/T7_RGB_50756A_4.png";
            t_info->DeviceColorRGB = "#50756A"; t_info->isChecked = false;
            t_info->DeviceHomePagePixmapPath = ":/Skin/Images/DevSel/homePageDevice/T7_homePage_RGB_50756A_4.png";
            t_info->DeviceHomePageTopLeftPixmapPath_normal = ":/Skin/Images/DevSel/leftTopIcon/T7_RGB_50756A_4_normal.png";
            t_info->DeviceHomePageTopLeftPixmapPath_abnormal = ":/Skin/Images/DevSel/leftTopIcon/T7_RGB_50756A_4_abnormal.png";
            t_info->DeviceMoreSetPixmapPath = ":/Skin/Images/DevSel/moreSetIPageDeviceIcon/T7_moreSetPage_RGB_50756A_4.png";
            t_info->DeviceMoreSetQrCodePixmapPath = QR_T7;
            t_info->DeviceManualUrl = MANUAL_T7;
            t_info->DeviceSysTypeName = "T7";
            cl_device_map_.insert({"T7", 4}, t_info);
        }
        {
            auto t_info = std::make_shared<DeSheng::DeviceInfo>();
            t_info->DeviceTypeName = "T7"; t_info->DeviceColorName = "卡布里蓝";
            t_info->DeviceColorPixmapPath = ":/Skin/Images/DevSel/selectionPage/T7_RGB_4BC3D7_5.png";
            t_info->DeviceColorRGB = "#4BC3D7"; t_info->isChecked = false;
            t_info->DeviceHomePagePixmapPath = ":/Skin/Images/DevSel/homePageDevice/T7_homePage_RGB_4BC3D7_5.png";
            t_info->DeviceHomePageTopLeftPixmapPath_normal = ":/Skin/Images/DevSel/leftTopIcon/T7_RGB_4BC3D7_5_normal.png";
            t_info->DeviceHomePageTopLeftPixmapPath_abnormal = ":/Skin/Images/DevSel/leftTopIcon/T7_RGB_4BC3D7_5_abnormal.png";
            t_info->DeviceMoreSetPixmapPath = ":/Skin/Images/DevSel/moreSetIPageDeviceIcon/T7_moreSetPage_RGB_4BC3D7_5.png";
            t_info->DeviceMoreSetQrCodePixmapPath = QR_T7;
            t_info->DeviceManualUrl = MANUAL_T7;
            t_info->DeviceSysTypeName = "T7";
            cl_device_map_.insert({"T7", 5}, t_info);
        }
        {
            auto t_info = std::make_shared<DeSheng::DeviceInfo>();
            t_info->DeviceTypeName = "T7"; t_info->DeviceColorName = "赤芒红";
            t_info->DeviceColorPixmapPath = ":/Skin/Images/DevSel/selectionPage/T7_RGB_BD353F_6.png";
            t_info->DeviceColorRGB = "#BD353F"; t_info->isChecked = false;
            t_info->DeviceHomePagePixmapPath = ":/Skin/Images/DevSel/homePageDevice/T7_homePage_RGB_BD353F_6.png";
            t_info->DeviceHomePageTopLeftPixmapPath_normal = ":/Skin/Images/DevSel/leftTopIcon/T7_RGB_BD353F_6_normal.png";
            t_info->DeviceHomePageTopLeftPixmapPath_abnormal = ":/Skin/Images/DevSel/leftTopIcon/T7_RGB_BD353F_6_abnormal.png";
            t_info->DeviceMoreSetPixmapPath = ":/Skin/Images/DevSel/moreSetIPageDeviceIcon/T7_moreSetPage_RGB_BD353F_6.png";
            t_info->DeviceMoreSetQrCodePixmapPath = QR_T7;
            t_info->DeviceManualUrl = MANUAL_T7;
            t_info->DeviceSysTypeName = "T7";
            cl_device_map_.insert({"T7", 6}, t_info);
        }

        // T7 GT
        {
            auto t_info = std::make_shared<DeSheng::DeviceInfo>();
            t_info->DeviceTypeName = "T7 GT"; t_info->DeviceColorName = "玄墨黑";
            t_info->DeviceColorPixmapPath = ":/Skin/Images/DevSel/selectionPage/T7GT_RGB_000000_0.png";
            t_info->DeviceColorRGB = "#000000"; t_info->isChecked = true;
            t_info->DeviceHomePagePixmapPath = ":/Skin/Images/DevSel/homePageDevice/T7GT_homePage_RGB_000000_0.png";
            t_info->DeviceHomePageTopLeftPixmapPath_normal = ":/Skin/Images/DevSel/leftTopIcon/T7GT_RGB_000000_0_normal.png";
            t_info->DeviceHomePageTopLeftPixmapPath_abnormal = ":/Skin/Images/DevSel/leftTopIcon/T7GT_RGB_000000_0_abnormal.png";
            t_info->DeviceMoreSetPixmapPath = ":/Skin/Images/DevSel/moreSetIPageDeviceIcon/T7GT_moreSetPage_RGB_000000_0.png";
            t_info->DeviceMoreSetQrCodePixmapPath = QR_T7GT;
            t_info->DeviceManualUrl = MANUAL_T7GT;
            t_info->DeviceSysTypeName = "T7GT";
            cl_device_map_.insert({"T7 GT", 0}, t_info);
        }
        {
            auto t_info = std::make_shared<DeSheng::DeviceInfo>();
            t_info->DeviceTypeName = "T7 GT"; t_info->DeviceColorName = "太空银";
            t_info->DeviceColorPixmapPath = ":/Skin/Images/DevSel/selectionPage/T7GT_RGB_ACB1B9_1.png";
            t_info->DeviceColorRGB = "#ACB1B9"; t_info->isChecked = false;
            t_info->DeviceHomePagePixmapPath = ":/Skin/Images/DevSel/homePageDevice/T7GT_homePage_RGB_ACB1B9_1.png";
            t_info->DeviceHomePageTopLeftPixmapPath_normal = ":/Skin/Images/DevSel/leftTopIcon/T7GT_RGB_ACB1B9_1_normal.png";
            t_info->DeviceHomePageTopLeftPixmapPath_abnormal = ":/Skin/Images/DevSel/leftTopIcon/T7GT_RGB_ACB1B9_1_abnormal.png";
            t_info->DeviceMoreSetPixmapPath = ":/Skin/Images/DevSel/moreSetIPageDeviceIcon/T7GT_moreSetPage_RGB_ACB1B9_1.png";
            t_info->DeviceMoreSetQrCodePixmapPath = QR_T7GT;
            t_info->DeviceManualUrl = MANUAL_T7GT;
            t_info->DeviceSysTypeName = "T7GT";
            cl_device_map_.insert({"T7 GT", 1}, t_info);
        }
        {
            // S21无线智充版（铁灰色 E1E3D6，单色默认选中；二维码暂无留空）
            auto t_info = std::make_shared<DeSheng::DeviceInfo>();
            t_info->DeviceTypeName = "S21无线智充版"; t_info->DeviceColorName = "铁灰色";
            t_info->DeviceColorPixmapPath = ":/Skin/Images/DevSel/selectionPage/S21Wireless_7_1_RGB_E1E3D6_0.png";
            t_info->DeviceColorRGB = "#E1E3D6"; t_info->isChecked = true;
            t_info->DeviceHomePagePixmapPath = ":/Skin/Images/DevSel/homePageDevice/S21Wireless_7_1_homePage_RGB_E1E3D6_0.png";
            t_info->DeviceHomePageTopLeftPixmapPath_normal = ":/Skin/Images/DevSel/leftTopIcon/S21Wireless_7_1_RGB_E1E3D6_0_normal.png";
            t_info->DeviceHomePageTopLeftPixmapPath_abnormal = ":/Skin/Images/DevSel/leftTopIcon/S21Wireless_7_1_RGB_E1E3D6_0_abnormal.png";
            t_info->DeviceMoreSetPixmapPath = ":/Skin/Images/DevSel/moreSetIPageDeviceIcon/S21Wireless_7_1_moreSetPage_RGB_E1E3D6_0.png";
            t_info->DeviceMoreSetQrCodePixmapPath = QString();  ///< 二维码暂无，后续补充
            t_info->DeviceManualUrl = MANUAL_S21;
            t_info->DeviceSysTypeName = "XIBERIA S21 Wireless 7.1";
            cl_device_map_.insert({"S21无线智充版", 0}, t_info);
        }
    }
}

} // namespace DeSheng
