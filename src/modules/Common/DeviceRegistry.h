#ifndef DEVICE_REGISTRY_H
#define DEVICE_REGISTRY_H

#include <QMap>
#include <QMutex>
#include <QPair>
#include <QSharedPointer>
#include <QString>

#include "data/device/device_info.h" ///< DeSheng::DeviceInfo

/// \brief 全局设备注册表（饿汉单例，main() 前构造）
///
/// 持有全部支持设备及其颜色变体信息。通过 DeviceRegistry::instance() 访问。
/// 初始化链路：main() → init() → defaultConfig() 或 load()。
///
/// ## Thread Safety
/// - 读（cl_device_map_ 查询）：调用方应在 GUI 线程使用。
/// - 写（init / defaultConfig / save / load）：互斥锁保护。
///
/// \code
/// auto &t_reg = DeSheng::DeviceRegistry::instance();
/// auto t_info = t_reg.find("T10有线", 0);
/// \endcode
namespace DeSheng {

class DeviceRegistry
{
public:
    static DeviceRegistry &instance(); ///< 单例（饿汉）

    // 数据表
    using DeviceMap = QMap<QPair<QString, int>, std::shared_ptr<DeviceInfo>>;

    const DeviceMap &deviceMap() const; ///< 只读访问设备表
    std::shared_ptr<DeviceInfo> find(const QString &t_type, int t_colorIndex) const; ///< 查找设备
    QString deviceSysTypeName(ushort vid, ushort pid) const; ///< 根据 VID/PID 返回设备简写（T10/K03S/…），用于标签显示

    /// \brief 查硬编码表返回设备标签名称
    /// \param t_deviceId 设备 ID（服务器产品标识）
    /// \param t_isTest 是否测试服
    /// \return 标签名称，未查到返回空字符串
    static QString deviceLabel(int t_deviceId, bool t_isTest);

    /// \brief 机型标签映射：API 完整英文名称 → 中文标签名称（按 deviceLabel 表倒推）
    /// "XIBERIA T10G" → "T10有线"；未收录机型回退截断（如 "XIBERIA XXX" → "XXX"）
    static QString shortDisplayName(const QString &fullName);

    /// \brief 机型筛选项中文短名 → API device_name 英文全名（未收录原样返回）
    /// 如 "T10有线" → "XIBERIA T10G"；空串由上层空值守卫跳过
    static QString deviceNameParam(const QString &t_shortLabel);

    // 生命周期
    void init();                              ///< 启动时初始化：读磁盘 → 回退 defaultConfig
    void defaultConfig();                     ///< 写入默认设备表
    bool save(const QString &t_filePath, int t_mode = -1) const; ///< 持久化
    bool load(const QString &t_filePath);     ///< 从磁盘恢复

    static QString configFilePath(); ///< 延迟求值：ProgramData/deviceInfo/DevInfo.dat

    static constexpr int kConfigVersion = -9999; ///< 配置文件版本号（正式发布后,只增不减,避免版本重复导致的资源路径问题）。v2: DeviceInfo 新增 DeviceManualUrl 说明书 URL

private:
    DeviceRegistry();
    DeviceRegistry(const DeviceRegistry &) = delete;
    DeviceRegistry &operator=(const DeviceRegistry &) = delete;

private:
    mutable QMutex cl_mutex_;          ///< 保护 cl_device_map_ 读写

private:
    DeviceMap cl_device_map_;          ///< 设备注册表
    static DeviceRegistry s_instance_; ///< 饿汉单例实例
};

} // namespace DeSheng

#endif // DEVICE_REGISTRY_H
