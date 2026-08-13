#include "data/api_global.h"

#include <cmath>
#include <vector>
#include <QPixmap>

namespace CategoryIcon {

const QHash<QString, QString> kIconMap = {
    {"游戏",       "game"},
    {"电影",       "movie"},
    {"音乐",       "music"},
    {"三角洲行动",  "delta"},
    {"pubg",       "pubg"},
    {"csgo",       "csgo"},
    {"无畏契约",    "valorant"},
    {"暗区突围",    "AB"},
    {"apex",       "apex"},
    {"穿越火线",    "CF"},
};
const QString kDefaultBase = "game";

QString buildPath(const QString &t_base, bool t_selected, bool t_system)
{
    return QString(":/Skin/Images/Headphones/edit/%1%2.png")
        .arg(t_system ? "sys-" : "")
        .arg(t_base + (t_selected ? "-ch" : "-no"));
}

} // namespace CategoryIcon

namespace {

using WallpaperMap = QMap<int, QSharedPointer<UserInformation::UserInfo_Local::WallpaperEntry>>;
using WallpaperMapPtr = QSharedPointer<WallpaperMap>;

constexpr WallpaperStorageScope kWallpaperStorageScope = WallpaperStorageScope::AppLocalJson;
constexpr int kWallpaperConfigVersion = 1;

QJsonArray serializeWallpaperMap(const WallpaperMapPtr &map)
{
    QJsonArray t_arr;
    if (!map)
        return t_arr;
    for (auto it = map->constBegin(); it != map->constEnd(); ++it) {
        if (!it.value())
            continue;
        QJsonObject t_entry;
        t_entry["index"] = it.key();
        t_entry["path"] = it.value()->path;
        t_entry["mode"] = static_cast<int>(it.value()->mode);
        t_entry["is_selected"] = it.value()->is_selected;
        t_arr.append(t_entry);
    }
    return t_arr;
}

WallpaperMapPtr deserializeWallpaperMap(const QJsonArray &t_arr)
{
    auto t_map = WallpaperMapPtr::create();
    for (const auto &t_val : t_arr) {
        QJsonObject t_entry = t_val.toObject();
        auto t_we = QSharedPointer<UserInformation::UserInfo_Local::WallpaperEntry>::create();
        t_we->path = t_entry["path"].toString();
        t_we->mode = static_cast<BackgroundImageMode>(t_entry["mode"].toInt());
        t_we->is_selected = t_entry["is_selected"].toBool(false);
        t_map->insert(t_entry["index"].toInt(), t_we);
    }
    return t_map;
}

QJsonObject loadExistingUserJson(const QString &t_path)
{
    QJsonObject t_json = XIBERIA_X_HUB_Utils::loadJsonEncrypted(t_path);
    if (!t_json.isEmpty())
        return t_json;

    QFile t_file(t_path);
    if (!t_file.open(QIODevice::ReadOnly))
        return QJsonObject();
    QJsonDocument t_doc = QJsonDocument::fromJson(t_file.readAll());
    t_file.close();
    return t_doc.isObject() ? t_doc.object() : QJsonObject();
}

void preserveUserWallpaperConfig(QJsonObject &t_json, const QString &t_path)
{
    if (UserInformation::wallpaperStorageScope() != WallpaperStorageScope::AppLocalJson)
        return;

    QJsonObject t_local = t_json["local"].toObject();
    const QStringList t_keys = {
        "panel_opacity",
        "panel_blur_radius",
        "default_wallpaper_map",
        "system_wallpaper_map",
        "custom_wallpaper_map"
    };

    const QJsonObject t_existing = loadExistingUserJson(t_path);
    if (t_existing.isEmpty()) {
        for (const QString &t_key : t_keys)
            t_local.remove(t_key);
        t_json["local"] = t_local;
        return;
    }

    const QJsonObject t_existing_local = t_existing["local"].toObject();
    if (t_existing_local.isEmpty()) {
        for (const QString &t_key : t_keys)
            t_local.remove(t_key);
        t_json["local"] = t_local;
        return;
    }

    for (const QString &t_key : t_keys) {
        if (t_existing_local.contains(t_key))
            t_local[t_key] = t_existing_local[t_key];
        else
            t_local.remove(t_key);
    }
    t_json["local"] = t_local;
}

} // namespace

namespace XIBERIA_X_HUB_Utils {

/// XOR 异或 + Base64 加密
QByteArray encryptData(const QByteArray &data, char key)
{
    QByteArray result = data;
    // 逐字节进行异或加密
    for (int i = 0; i < result.size(); ++i) {
        result[i] = result[i] ^ key;
    }
    return result.toBase64();
}

/// XOR 异或 + Base64 解密
QByteArray decryptData(const QByteArray &encryptedData, char key)
{
    QByteArray raw = QByteArray::fromBase64(encryptedData);
    // 逐字节进行异或解密
    for (int i = 0; i < raw.size(); ++i) {
        raw[i] = raw[i] ^ key;
    }
    return raw;
}

/// 保存 JSON 到加密文件（同步）
bool saveJsonEncrypted(const QString &filePath, const QJsonObject &json, char key)
{
    // 将 JSON 转换为字节数组
    QByteArray jsonData = QJsonDocument(json).toJson();
    // 加密数据
    QByteArray encrypted = encryptData(jsonData, key);

    // 写入文件
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly)) {
        return false;
    }
    file.write(encrypted);
    file.close();
    return true;
}

/// 从加密文件读取 JSON（同步）
QJsonObject loadJsonEncrypted(const QString &filePath, char key)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        return QJsonObject();
    }

    // 读取加密数据
    QByteArray encrypted = file.readAll();
    file.close();

    // 解密并转换为 JSON
    QByteArray decrypted = decryptData(encrypted, key);
    return QJsonDocument::fromJson(decrypted).object();
}

/// 异步保存 JSON 到加密文件
void saveJsonEncryptedAsync(const QString &filePath, const QJsonObject &json, char key)
{
    QtConcurrent::run([filePath, json, key]() {
        // 将 JSON 转换为字节数组
        QByteArray jsonData = QJsonDocument(json).toJson();

        // 逐字节进行异或加密
        for (int i = 0; i < jsonData.size(); ++i) {
            jsonData[i] = jsonData[i] ^ key;
        }
        // 转换为 Base64
        QByteArray encrypted = jsonData.toBase64();

        // 写入文件
        QFile file(filePath);
        if (file.open(QIODevice::WriteOnly)) {
            file.write(encrypted);
            file.close();
        }
    });
}

/// \brief 真高斯模糊（截断核可分离卷积，σ = radius，与盒式 3 遍同视觉强度）
/// - 仅模糊 RGB，保持原始 Alpha（与 blurPixmap 一致）
/// - 边缘复制 clamp；double 累加防误差
/// - 数值正确性已用 Python 独立验证（DC 保持 / 脉冲对称 / σ 匹配 / 可分离）
static QPixmap blurPixmapGaussian(const QPixmap &t_source, int t_radius)
{
    QImage t_img = t_source.toImage().convertToFormat(QImage::Format_ARGB32);
    const int t_w = t_img.width();
    const int t_h = t_img.height();
    const int t_sigma = qBound(1, t_radius, 64);

    // 截断高斯核（3σ 截断），归一化
    const int t_kr = qMax(1, static_cast<int>(3.0 * t_sigma));
    std::vector<double> t_kernel(2 * t_kr + 1);
    double t_sum = 0.0;
    for (int t_i = -t_kr; t_i <= t_kr; ++t_i) {
        const double t_v = std::exp(-(static_cast<double>(t_i * t_i)) / (2.0 * t_sigma * t_sigma));
        t_kernel[t_i + t_kr] = t_v;
        t_sum += t_v;
    }
    for (double &t_v : t_kernel)
        t_v /= t_sum;

    // 一维卷积（边缘复制 clamp）
    auto t_convolve1d = [&t_kernel, t_kr](const std::vector<double> &t_in, std::vector<double> &t_out) {
        const int t_n = static_cast<int>(t_in.size());
        const int t_k = static_cast<int>(t_kernel.size());
        for (int t_i = 0; t_i < t_n; ++t_i) {
            double t_acc = 0.0;
            for (int t_j = 0; t_j < t_k; ++t_j) {
                const int t_idx = qBound(0, t_i - t_kr + t_j, t_n - 1);
                t_acc += t_kernel[t_j] * t_in[t_idx];
            }
            t_out[t_i] = t_acc;
        }
    };

    // 水平方向
    std::vector<double> t_r(t_w), t_g(t_w), t_b(t_w), t_out_r(t_w), t_out_g(t_w), t_out_b(t_w);
    for (int t_y = 0; t_y < t_h; ++t_y) {
        const quint32 *t_line = reinterpret_cast<const quint32 *>(t_img.constScanLine(t_y));
        for (int t_x = 0; t_x < t_w; ++t_x) {
            t_r[t_x] = (t_line[t_x] >> 16) & 0xFF;
            t_g[t_x] = (t_line[t_x] >> 8) & 0xFF;
            t_b[t_x] = t_line[t_x] & 0xFF;
        }
        t_convolve1d(t_r, t_out_r);
        t_convolve1d(t_g, t_out_g);
        t_convolve1d(t_b, t_out_b);
        quint32 *t_line_out = reinterpret_cast<quint32 *>(t_img.scanLine(t_y));
        for (int t_x = 0; t_x < t_w; ++t_x) {
            t_line_out[t_x] = (t_line_out[t_x] & 0xFF000000)  // 保持原始 Alpha
                            | (static_cast<quint32>(qRound(t_out_r[t_x])) << 16)
                            | (static_cast<quint32>(qRound(t_out_g[t_x])) << 8)
                            |  static_cast<quint32>(qRound(t_out_b[t_x]));
        }
    }

    // 垂直方向
    std::vector<double> t_c1(t_h), t_c2(t_h), t_c3(t_h), t_c1o(t_h), t_c2o(t_h), t_c3o(t_h);
    for (int t_x = 0; t_x < t_w; ++t_x) {
        for (int t_y = 0; t_y < t_h; ++t_y) {
            const quint32 t_p = reinterpret_cast<const quint32 *>(t_img.constScanLine(t_y))[t_x];
            t_c1[t_y] = (t_p >> 16) & 0xFF;
            t_c2[t_y] = (t_p >> 8) & 0xFF;
            t_c3[t_y] = t_p & 0xFF;
        }
        t_convolve1d(t_c1, t_c1o);
        t_convolve1d(t_c2, t_c2o);
        t_convolve1d(t_c3, t_c3o);
        for (int t_y = 0; t_y < t_h; ++t_y) {
            quint32 *t_line = reinterpret_cast<quint32 *>(t_img.scanLine(t_y));
            t_line[t_x] = (t_line[t_x] & 0xFF000000)  // 保持原始 Alpha
                        | (static_cast<quint32>(qRound(t_c1o[t_y])) << 16)
                        | (static_cast<quint32>(qRound(t_c2o[t_y])) << 8)
                        |  static_cast<quint32>(qRound(t_c3o[t_y]));
        }
    }

    return QPixmap::fromImage(t_img);
}

/// \brief 对 QPixmap 应用快速模糊，返回副本
/// 使用盒式模糊滑动窗口累加器，O(n) 每遍
/// - 仅模糊 RGB，保持原始 Alpha（避免边缘伪影）
/// - r ≤ 25 用三遍近似高斯，r > 25 用六遍保持精度
/// - A/B 开关：kUseGaussianBlur = true 走 blurPixmapGaussian（真高斯），false 走盒式原实现
QPixmap blurPixmap(const QPixmap &t_source, int t_radius)
{
    if (t_radius <= 0 || t_source.isNull())
        return t_source;

    static const bool kUseGaussianBlur = true;  // A/B 试错开关：true=真高斯 false=盒式
    if (kUseGaussianBlur)
        return blurPixmapGaussian(t_source, t_radius);

    QImage t_img = t_source.toImage().convertToFormat(QImage::Format_ARGB32);
    const int t_w = t_img.width();
    const int t_h = t_img.height();
    const int t_r = qBound(1, t_radius, 254);
    const int t_passes = t_r <= 25 ? 3 : 6;
    const int t_window = t_r * 2 + 1;

    // 盒式模糊单遍 — 水平方向，只处理 RGB，保持 Alpha
    auto t_blurH = [](QImage &t_img, int t_w, int t_h, int t_r) {
        const int t_window = t_r * 2 + 1;
        std::vector<quint32> t_row(t_w);
        for (int t_y = 0; t_y < t_h; ++t_y) {
            quint32 *t_line = reinterpret_cast<quint32 *>(t_img.scanLine(t_y));
            int t_sr = 0, t_sg = 0, t_sb = 0, t_n = 0;
            for (int t_x = 0; t_x < t_w + t_r; ++t_x) {
                if (t_x < t_w) {
                    const quint32 t_p = t_line[t_x];
                    t_sr += (t_p >> 16) & 0xFF;
                    t_sg += (t_p >> 8)  & 0xFF;
                    t_sb += t_p & 0xFF;
                    ++t_n;
                }
                if (t_x >= t_window) {
                    const quint32 t_p = t_line[t_x - t_window];
                    t_sr -= (t_p >> 16) & 0xFF;
                    t_sg -= (t_p >> 8)  & 0xFF;
                    t_sb -= t_p & 0xFF;
                    --t_n;
                }
                if (t_x >= t_r) {
                    const int t_out = t_x - t_r;
                    const quint32 t_a = t_line[t_out] & 0xFF000000; // 保持原始 Alpha
                    t_row[t_out] = t_a
                                 | ((t_sr / t_n) << 16)
                                 | ((t_sg / t_n) << 8)
                                 |  (t_sb / t_n);
                }
            }
            memcpy(t_line, t_row.data(), t_w * sizeof(quint32));
        }
    };

    // 盒式模糊单遍 — 垂直方向，只处理 RGB，保持 Alpha
    auto t_blurV = [](QImage &t_img, int t_w, int t_h, int t_r) {
        const int t_window = t_r * 2 + 1;
        std::vector<quint32> t_col(t_h);
        for (int t_x = 0; t_x < t_w; ++t_x) {
            int t_sr = 0, t_sg = 0, t_sb = 0, t_n = 0;
            for (int t_y = 0; t_y < t_h + t_r; ++t_y) {
                if (t_y < t_h) {
                    const quint32 t_p = reinterpret_cast<quint32 *>(t_img.scanLine(t_y))[t_x];
                    t_sr += (t_p >> 16) & 0xFF;
                    t_sg += (t_p >> 8)  & 0xFF;
                    t_sb += t_p & 0xFF;
                    ++t_n;
                }
                if (t_y >= t_window) {
                    const quint32 t_p = reinterpret_cast<quint32 *>(t_img.scanLine(t_y - t_window))[t_x];
                    t_sr -= (t_p >> 16) & 0xFF;
                    t_sg -= (t_p >> 8)  & 0xFF;
                    t_sb -= t_p & 0xFF;
                    --t_n;
                }
                if (t_y >= t_r) {
                    const int t_out = t_y - t_r;
                    const quint32 t_a = reinterpret_cast<quint32 *>(t_img.scanLine(t_out))[t_x] & 0xFF000000;
                    t_col[t_out] = t_a
                                 | ((t_sr / t_n) << 16)
                                 | ((t_sg / t_n) << 8)
                                 |  (t_sb / t_n);
                }
            }
            for (int t_y = 0; t_y < t_h; ++t_y)
                reinterpret_cast<quint32 *>(t_img.scanLine(t_y))[t_x] = t_col[t_y];
        }
    };

    for (int t_pass = 0; t_pass < t_passes; ++t_pass) {
        t_blurH(t_img, t_w, t_h, t_r);
        t_blurV(t_img, t_w, t_h, t_r);
    }

    return QPixmap::fromImage(t_img);
}

} // namespace XIBERIA_X_HUB_Utils

/// \brief 获取当前选中的壁纸，返回 {map指针, index}，无选中时 map 为 nullptr
QPair<QSharedPointer<QMap<int, QSharedPointer<UserInformation::UserInfo_Local::WallpaperEntry>>>, int>
UserInformation::UserInfo_Local::selectedWallpaper() const
{
    auto t_find = [](const QSharedPointer<QMap<int, QSharedPointer<WallpaperEntry>>> &t_map)
        -> QPair<QSharedPointer<QMap<int, QSharedPointer<WallpaperEntry>>>, int> {
        if (!t_map) return {nullptr, -1};
        for (auto t_it = t_map->constBegin(); t_it != t_map->constEnd(); ++t_it) {
            if (t_it.value()->is_selected)
                return {t_map, t_it.key()};
        }
        return {nullptr, -1};
    };
    auto t_result = t_find(default_wallpaper_map);
    if (t_result.first) return t_result;
    t_result = t_find(system_wallpaper_map);
    if (t_result.first) return t_result;
    return t_find(custom_wallpaper_map);
}

/// \brief 在指定 map 中选中 index 对应的壁纸，并清除其他所有 map 的选中状态
void UserInformation::UserInfo_Local::selectWallpaper(
    const QSharedPointer<QMap<int, QSharedPointer<WallpaperEntry>>> &map, int index)
{
    auto t_clearAll = [](QSharedPointer<QMap<int, QSharedPointer<WallpaperEntry>>> &m) {
        if (!m) return;
        for (auto it = m->begin(); it != m->end(); ++it)
            it.value()->is_selected = false;
    };
    t_clearAll(default_wallpaper_map);
    t_clearAll(system_wallpaper_map);
    t_clearAll(custom_wallpaper_map);

    if (map && map->contains(index))
        (*map)[index]->is_selected = true;
}

/// \brief 校验三张壁纸 map 中所有路径的有效性，移除磁盘不存在的条目
void UserInformation::UserInfo_Local::validateWallpaperPaths()
{
    auto t_removeInvalid = [](QSharedPointer<QMap<int, QSharedPointer<WallpaperEntry>>> &t_map) {
        if (!t_map) return;
        QList<int> t_keysToRemove;
        for (auto t_it = t_map->constBegin(); t_it != t_map->constEnd(); ++t_it) {
            if (!t_it.value() || !QFile::exists(t_it.value()->path))
                t_keysToRemove.append(t_it.key());
        }
        for (int t_key : t_keysToRemove)
            t_map->remove(t_key);
    };
    t_removeInvalid(default_wallpaper_map);
    t_removeInvalid(system_wallpaper_map);
    t_removeInvalid(custom_wallpaper_map);
}

// ============================================================================
// UserInformation 方法实现
// ============================================================================

/// \brief 从 GetCurrentUser 应答更新网络字段
void UserInformation::updateFromServer(const DeSheng::GetCurrentUserResponse::ReturnData &t_data)
{
    network.id = t_data.id;
    network.username = t_data.username;
    network.email = t_data.email;
    network.nickname = t_data.nickname;
    network.avatar = t_data.avatar;
    network.status = t_data.status;
    network.login_type = t_data.login_type;
    network.created_at = t_data.created_at;
    network.last_login_at = t_data.last_login_at;
    network.favorite_games = t_data.favorite_games;
    network.activation_code = t_data.activation_code;
    network.city = t_data.city;
    network.login_ip = t_data.login_ip;
    network.os_info = t_data.os_info;
    network.bio = t_data.bio;
    network.roles = t_data.roles;
    network.titles = t_data.titles;
}

/// \brief 从登录应答的 UserInfo 更新网络字段
void UserInformation::updateFromServer(const DeSheng::UserLoginResponse::ReturnData::UserInfo &t_user)
{
    network.id = t_user.id;
    network.username = t_user.username;
    network.email = t_user.email;
    network.nickname = t_user.nickname;
    network.avatar = t_user.avatar;
    network.status = t_user.status;
    network.login_type = t_user.login_type;
    network.created_at = t_user.created_at;
    network.last_login_at = t_user.last_login_at;
    network.favorite_games = t_user.favorite_games;
    network.activation_code = t_user.activation_code;
    network.city = t_user.city;
    network.login_ip = t_user.login_ip;
    network.os_info = t_user.os_info;
    network.roles = t_user.roles;
    network.titles = t_user.titles;
}

WallpaperStorageScope UserInformation::wallpaperStorageScope()
{
    return kWallpaperStorageScope;
}

QString UserInformation::wallpaperConfigFilePath()
{
    return XIBERIA_X_HUB_Utils::programDataPath() + "/Config/wallpaper_config.json";
}

QString UserInformation::wallpaperCustomBackgroundDir()
{
    return XIBERIA_X_HUB_Utils::programDataPath() + "/Config/CustomBackground";
}

/// \brief 初始化默认壁纸（主题切换用，index 0 = DefaultTheme 无文件路径）
void UserInformation::initDefaultWallpaper()
{
    if (!local.default_wallpaper_map)
        local.default_wallpaper_map.reset(new QMap<int, QSharedPointer<UserInfo_Local::WallpaperEntry>>);
    local.default_wallpaper_map->clear();
    local.default_wallpaper_map->insert(0,
        QSharedPointer<UserInfo_Local::WallpaperEntry>::create(QString(), BackgroundImageMode::DefaultTheme));
}

/// \brief 从 exe 目录加载系统壁纸
void UserInformation::initSystemWallpapers()
{
    if (!local.system_wallpaper_map)
        local.system_wallpaper_map.reset(new QMap<int, QSharedPointer<UserInfo_Local::WallpaperEntry>>);
    local.system_wallpaper_map->clear();

    QString t_dir = QCoreApplication::applicationDirPath()
                    + "/Resources/images/systemBackground/";
    QDir t_sys_dir(t_dir);
    if (!t_sys_dir.exists())
        return;

    QStringList t_filters = {"*.png", "*.jpg", "*.jpeg", "*.bmp"};
    QStringList t_files = t_sys_dir.entryList(t_filters, QDir::Files, QDir::Name);

    // 从文件名末 '_' 后提取数字键值（如 system_background_2x_01.png → 1），
    // QMap 按 key 自动升序，保证 01 在 02 前
    auto t_extractKey = [](const QString &t_fileName) -> int {
        QString t_base = QFileInfo(t_fileName).completeBaseName();
        int t_pos = t_base.lastIndexOf('_');
        if (t_pos < 0 || t_pos + 1 >= t_base.length())
            return -1;
        bool t_ok = false;
        int t_key = t_base.midRef(t_pos + 1).toInt(&t_ok);
        return t_ok ? t_key : -1;
    };

    for (const QString &t_file : t_files) {
        int t_key = t_extractKey(t_file);
        if (t_key < 0) continue; // 无法提取键值，跳过
        local.system_wallpaper_map->insert(t_key,
            QSharedPointer<UserInfo_Local::WallpaperEntry>::create(t_dir + t_file, BackgroundImageMode::SystemTheme));
    }
}

/// \brief 版本更新时刷新系统壁纸（与 exe 目录比对，清理已删除的，新增的追加）
void UserInformation::refreshSystemWallpapers()
{
    // 获取当前 exe 目录下的系统壁纸文件列表
    QString t_dir = QCoreApplication::applicationDirPath()
                    + "/Resources/images/systemBackground/";
    QDir t_sys_dir(t_dir);
    QStringList t_filters = {"*.png", "*.jpg", "*.jpeg", "*.bmp"};
    QSet<QString> t_current_paths;
    if (t_sys_dir.exists()) {
        QStringList t_files = t_sys_dir.entryList(t_filters, QDir::Files, QDir::Name);
        for (const QString &t_file : t_files) {
            t_current_paths.insert(t_dir + t_file);
        }
    }

    // 清理 system_wallpaper_map 中不存在的路径
    QList<int> t_remove_keys;
    for (auto it = local.system_wallpaper_map->constBegin();
         it != local.system_wallpaper_map->constEnd(); ++it) {
        if (!t_current_paths.contains(it.value()->path))
            t_remove_keys.append(it.key());
    }
    for (int t_key : t_remove_keys)
        local.system_wallpaper_map->remove(t_key);

    // 追加新文件 — 从文件名末 '_' 后提取数字键值（如 system_background_2x_01.png → 1）
    for (const QString &t_path : t_current_paths) {
        bool t_exists = false;
        for (auto it = local.system_wallpaper_map->constBegin();
             it != local.system_wallpaper_map->constEnd(); ++it) {
            if (it.value()->path == t_path) { t_exists = true; break; }
        }
        if (!t_exists) {
            bool t_ok = false;
            QString t_base = QFileInfo(t_path).completeBaseName();
            int t_pos = t_base.lastIndexOf('_');
            int t_key = -1;
            if (t_pos >= 0 && t_pos + 1 < t_base.length())
                t_key = t_base.midRef(t_pos + 1).toInt(&t_ok);
            if (!t_ok || t_key < 0) continue; // 无法提取键值，跳过
            local.system_wallpaper_map->insert(t_key,
                QSharedPointer<UserInfo_Local::WallpaperEntry>::create(t_path, BackgroundImageMode::SystemTheme, false));
        }
    }
}

/// \brief 从用户 CustomBackground 目录加载自定义壁纸
void UserInformation::initCustomWallpapers()
{
    if (!local.custom_wallpaper_map)
        local.custom_wallpaper_map.reset(new QMap<int, QSharedPointer<UserInfo_Local::WallpaperEntry>>);
    local.custom_wallpaper_map->clear();
    QString t_dir = customBackgroundDir();
    if (t_dir.isEmpty())
        return;
    QDir t_custom_dir(t_dir);
    if (!t_custom_dir.exists())
        return;

    QStringList t_filters = {"*.png", "*.jpg", "*.jpeg", "*.bmp"};
    QStringList t_files = t_custom_dir.entryList(t_filters, QDir::Files, QDir::Name);
    int t_idx = 0;
    for (const QString &t_file : t_files) {
        local.custom_wallpaper_map->insert(t_idx,
            QSharedPointer<UserInfo_Local::WallpaperEntry>::create(t_dir + "/" + t_file, BackgroundImageMode::Custom));
        ++t_idx;
    }
}

/// \brief 序列化为 JSON
QJsonObject UserInformation::toJson() const
{
    QJsonObject t_root;
    t_root["version"] = kCurrentVersion; ///< 数据格式版本

    // network
    QJsonObject t_net;
    t_net["access_token"] = network.access_token;
    t_net["refresh_token"] = network.refresh_token;
    t_net["id"] = network.id;
    t_net["username"] = network.username;
    t_net["email"] = network.email;
    t_net["nickname"] = network.nickname;
    t_net["avatar"] = network.avatar;
    t_net["status"] = network.status;
    t_net["login_type"] = network.login_type;
    t_net["created_at"] = network.created_at;
    t_net["last_login_at"] = network.last_login_at;
    t_net["favorite_games"] = network.favorite_games;
    t_net["activation_code"] = network.activation_code;
    t_net["city"] = network.city;
    t_net["login_ip"] = network.login_ip;
    t_net["os_info"] = network.os_info;
    t_net["bio"] = network.bio;
    t_root["network"] = t_net;

    // local
    QJsonObject t_loc;
    t_loc["user_psw"] = local.user_psw;
    t_loc["is_get_userInfo_first"] = local.is_get_userInfo_first.load();
    t_loc["panel_opacity"] = local.panel_opacity_;
    t_loc["panel_blur_radius"] = local.panel_blur_radius_;
    t_loc["is_encrypted_save"] = local.is_encrypted_save_;

    t_loc["default_wallpaper_map"] = serializeWallpaperMap(local.default_wallpaper_map);
    t_loc["system_wallpaper_map"] = serializeWallpaperMap(local.system_wallpaper_map);
    t_loc["custom_wallpaper_map"] = serializeWallpaperMap(local.custom_wallpaper_map);
    t_root["local"] = t_loc;

    return t_root;
}

/// \brief 从 JSON 反序列化
void UserInformation::fromJson(QJsonObject t_root) // 按值传递，迁移链可修改
{
    int t_version = t_root["version"].toInt(0);

    // 数据版本迁移链
    switch (t_version) {
    case 0: {
        // v0 → v1：三表持久化 + WallpaperEntry 新增 is_selected
        if (t_root.contains("local")) {
            QJsonObject t_loc = t_root["local"].toObject();
            auto t_addSelected = [](QJsonArray &t_arr) {
                for (int i = 0; i < t_arr.size(); ++i) {
                    QJsonObject t_entry = t_arr[i].toObject();
                    if (!t_entry.contains("is_selected")) {
                        t_entry["is_selected"] = false;
                        t_arr[i] = t_entry;
                    }
                }
            };
            // 旧版只有 custom_wallpaper_map，补 default / system
            if (!t_loc.contains("default_wallpaper_map"))
                t_loc["default_wallpaper_map"] = QJsonArray();
            if (!t_loc.contains("system_wallpaper_map"))
                t_loc["system_wallpaper_map"] = QJsonArray();
            QJsonArray t_custom = t_loc["custom_wallpaper_map"].toArray();
            t_addSelected(t_custom);
            t_loc["custom_wallpaper_map"] = t_custom;
            t_root["local"] = t_loc;
        }
        t_root["version"] = 1;
        t_version = 1;
    } // fallthrough
    default: break;
    }

    // network
    if (t_root.contains("network")) {
        QJsonObject t_net = t_root["network"].toObject();
        network.access_token = t_net["access_token"].toString();
        network.refresh_token = t_net["refresh_token"].toString();
        network.id = t_net["id"].toString();
        network.username = t_net["username"].toString();
        network.email = t_net["email"].toString();
        network.nickname = t_net["nickname"].toString();
        network.avatar = t_net["avatar"].toString();
        network.status = t_net["status"].toString();
        network.login_type = t_net["login_type"].toString();
        network.created_at = t_net["created_at"].toString();
        network.last_login_at = t_net["last_login_at"].toString();
        network.favorite_games = t_net["favorite_games"].toString();
        network.activation_code = t_net["activation_code"].toString();
        network.city = t_net["city"].toString();
        network.login_ip = t_net["login_ip"].toString();
        network.os_info = t_net["os_info"].toString();
        network.bio = t_net["bio"].toString();
    }

    // local
    if (t_root.contains("local")) {
        QJsonObject t_loc = t_root["local"].toObject();
        local.user_psw = t_loc["user_psw"].toString();
        if (t_loc.contains("is_get_userInfo_first"))
            local.is_get_userInfo_first.store(t_loc["is_get_userInfo_first"].toBool());
        if (t_loc.contains("panel_opacity"))
            local.panel_opacity_ = t_loc["panel_opacity"].toDouble(1.0);
        if (t_loc.contains("panel_blur_radius"))
            local.panel_blur_radius_ = t_loc["panel_blur_radius"].toDouble(0.0);
        if (t_loc.contains("is_encrypted_save"))
            local.is_encrypted_save_ = t_loc["is_encrypted_save"].toBool();

        local.default_wallpaper_map = deserializeWallpaperMap(
            t_loc["default_wallpaper_map"].toArray());
        local.system_wallpaper_map = deserializeWallpaperMap(
            t_loc["system_wallpaper_map"].toArray());
        local.custom_wallpaper_map = deserializeWallpaperMap(
            t_loc["custom_wallpaper_map"].toArray());
    }
}

QJsonObject UserInformation::wallpaperConfigToJson() const
{
    QJsonObject t_root;
    t_root["version"] = kWallpaperConfigVersion;

    QJsonObject t_local;
    t_local["panel_opacity"] = local.panel_opacity_;
    t_local["panel_blur_radius"] = local.panel_blur_radius_;
    t_local["default_wallpaper_map"] = serializeWallpaperMap(local.default_wallpaper_map);
    t_local["system_wallpaper_map"] = serializeWallpaperMap(local.system_wallpaper_map);
    t_local["custom_wallpaper_map"] = serializeWallpaperMap(local.custom_wallpaper_map);
    t_root["local"] = t_local;
    return t_root;
}

void UserInformation::wallpaperConfigFromJson(QJsonObject t_root)
{
    QJsonObject t_local = t_root["local"].toObject();
    if (t_local.contains("panel_opacity"))
        local.panel_opacity_ = t_local["panel_opacity"].toDouble(1.0);
    if (t_local.contains("panel_blur_radius"))
        local.panel_blur_radius_ = t_local["panel_blur_radius"].toDouble(0.0);
    local.default_wallpaper_map = deserializeWallpaperMap(
        t_local["default_wallpaper_map"].toArray());
    local.system_wallpaper_map = deserializeWallpaperMap(
        t_local["system_wallpaper_map"].toArray());
    local.custom_wallpaper_map = deserializeWallpaperMap(
        t_local["custom_wallpaper_map"].toArray());
}

bool UserInformation::saveWallpaperConfig() const
{
    if (wallpaperStorageScope() == WallpaperStorageScope::UserLocal)
        return saveToDisk();

    const QString t_path = wallpaperConfigFilePath();
    QDir().mkpath(QFileInfo(t_path).absolutePath());
    QFile t_file(t_path);
    if (!t_file.open(QIODevice::WriteOnly))
        return false;
    t_file.write(QJsonDocument(wallpaperConfigToJson()).toJson(QJsonDocument::Indented));
    t_file.close();
    return true;
}

void UserInformation::saveWallpaperConfigAsync() const
{
    if (wallpaperStorageScope() == WallpaperStorageScope::UserLocal) {
        saveToDiskAsync();
        return;
    }

    const QString t_path = wallpaperConfigFilePath();
    QDir().mkpath(QFileInfo(t_path).absolutePath());
    const QJsonObject t_json = wallpaperConfigToJson();
    QtConcurrent::run([t_path, t_json]() {
        QFile t_file(t_path);
        if (t_file.open(QIODevice::WriteOnly)) {
            t_file.write(QJsonDocument(t_json).toJson(QJsonDocument::Indented));
            t_file.close();
        }
    });
}

bool UserInformation::loadWallpaperConfig()
{
    if (wallpaperStorageScope() == WallpaperStorageScope::UserLocal)
        return loadFromDisk();

    QFile t_file(wallpaperConfigFilePath());
    if (!t_file.open(QIODevice::ReadOnly))
        return false;
    QJsonDocument t_doc = QJsonDocument::fromJson(t_file.readAll());
    t_file.close();
    if (t_doc.isNull() || !t_doc.isObject())
        return false;

    wallpaperConfigFromJson(t_doc.object());
    return true;
}

/// \brief 保存到用户目录（根据 is_encrypted_save_ 决定加密或明文）
bool UserInformation::saveToDisk() const
{
    QString t_path = userInfoFilePath();
    if (t_path.isEmpty())
        return false;
    QDir().mkpath(QFileInfo(t_path).absolutePath());
    if (local.is_encrypted_save_) {
        QJsonObject t_json = toJson();
        preserveUserWallpaperConfig(t_json, t_path);
        return XIBERIA_X_HUB_Utils::saveJsonEncrypted(t_path, t_json);
    }
    // 调试模式：明文保存
    QJsonObject t_json = toJson();
    preserveUserWallpaperConfig(t_json, t_path);
    QFile t_file(t_path);
    if (!t_file.open(QIODevice::WriteOnly))
        return false;
    t_file.write(QJsonDocument(t_json).toJson());
    t_file.close();
    return true;
}

/// \brief 保存到用户目录（根据 is_encrypted_save_ 决定加密或明文，异步）
void UserInformation::saveToDiskAsync() const
{
    QString t_path = userInfoFilePath();
    if (t_path.isEmpty())
        return;
    QDir().mkpath(QFileInfo(t_path).absolutePath());
    QJsonObject t_json = toJson();
    preserveUserWallpaperConfig(t_json, t_path);
    if (local.is_encrypted_save_) {
        XIBERIA_X_HUB_Utils::saveJsonEncryptedAsync(t_path, t_json);
        return;
    }
    // 调试模式：明文异步保存
    QtConcurrent::run([t_path, t_json]() {
        QFile t_file(t_path);
        if (t_file.open(QIODevice::WriteOnly)) {
            t_file.write(QJsonDocument(t_json).toJson());
            t_file.close();
        }
    });
}

/// \brief 从用户目录读取（先尝试加密读，失败则明文读；自动迁移并回写旧版本数据）
bool UserInformation::loadFromDisk()
{
    QString t_path = userInfoFilePath();
    if (t_path.isEmpty())
        return false;

    QJsonObject t_json = XIBERIA_X_HUB_Utils::loadJsonEncrypted(t_path);
    if (t_json.isEmpty()) {
        // 加密读取失败 → 尝试明文读取（调试模式）
        QFile t_file(t_path);
        if (!t_file.open(QIODevice::ReadOnly))
            return false;
        QJsonDocument t_doc = QJsonDocument::fromJson(t_file.readAll());
        t_file.close();
        if (t_doc.isNull() || !t_doc.isObject())
            return false;
        t_json = t_doc.object();
        if (t_json.isEmpty())
            return false;
    }
    int t_old_version = t_json["version"].toInt(0); ///< 记录旧版本号
    fromJson(t_json);                                ///< 迁移链在此执行
    // 版本落后则立刻回写，保证磁盘文件与当前结构一致
    if (t_old_version < UserInformation::kCurrentVersion) {
        saveToDisk();
    }
    return true;
}
