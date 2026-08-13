#ifndef DEVICE_INFO_H
#define DEVICE_INFO_H

#include <QJsonObject>
#include <QString>

namespace DeSheng {

/*************************************************************************************  设备信息  ************************************************************************************************/
typedef struct DeviceInfo
{
    // QString — 设备基础
    QString DeviceTypeName;                            ///< 设备型号
    QString DeviceColorName;                           ///< 设备颜色中文名称
    QString DeviceColorPixmapPath;                     ///< 设备对应的资源路径
    QString DeviceColorRGB;                            ///< 设备颜色对应的 RGB 数值

    // bool
    bool isChecked = false;                            ///< 该设备颜色是否选中

    // QString — 首页图片
    QString DeviceHomePagePixmapPath;                  ///< 设备对应的首页图片资源路径
    QString DeviceHomePageTopLeftPixmapPath_normal;    ///< 首页左上角图片(正常状态)
    QString DeviceHomePageTopLeftPixmapPath_abnormal;  ///< 首页左上角图片(异常状态)

    // QString — 更多设置图片
    QString DeviceMoreSetPixmapPath;                   ///< 更多设置 — 耳机图片资源路径
    QString DeviceMoreSetQrCodePixmapPath;             ///< 更多设置 — 耳机二维码图片资源路径
    QString DeviceManualUrl;                           ///< 设备说明书 URL（型号级，h-por.html?groupId=xx）

    // QString — 系统
    QString DeviceSysTypeName;                         ///< 设备系统显示名称
    QString DeviceGuid;                                ///< windows 设备 GUID（APO 参数）

    // unsigned short
    unsigned short SelDev_VID = 0;                     ///< 选中设备的 VID
    unsigned short SelDev_PID = 0;                     ///< 选中设备的 PID

    /**
     * @brief 将结构体转换为 JSON 对象
     * @return QJsonObject JSON 对象
     */
    QJsonObject toJson() const
    {
        QJsonObject obj;
        obj["DeviceTypeName"] = DeviceTypeName;
        obj["DeviceColorName"] = DeviceColorName;
        obj["DeviceColorPixmapPath"] = DeviceColorPixmapPath;
        obj["DeviceColorRGB"] = DeviceColorRGB;
        obj["isChecked"] = isChecked;
        obj["DeviceHomePagePixmapPath"] = DeviceHomePagePixmapPath;
        obj["DeviceHomePageTopLeftPixmapPath_normal"] = DeviceHomePageTopLeftPixmapPath_normal;
        obj["DeviceHomePageTopLeftPixmapPath_abnormal"] = DeviceHomePageTopLeftPixmapPath_abnormal;
        obj["DeviceMoreSetPixmapPath"] = DeviceMoreSetPixmapPath;
        obj["DeviceMoreSetQrCodePixmapPath"] = DeviceMoreSetQrCodePixmapPath;
        obj["DeviceManualUrl"] = DeviceManualUrl;
        obj["DeviceSysTypeName"] = DeviceSysTypeName;
        obj["SelDev_VID"] = SelDev_VID;
        obj["SelDev_PID"] = SelDev_PID;
        obj["DeviceGuid"] = DeviceGuid;
        return obj;
    }

    /**
     * @brief 从 JSON 对象解析结构体
     * @param obj JSON 对象
     * @return DeviceInfo 结构体
     */
    static DeviceInfo fromJson(const QJsonObject &obj)
    {
        DeviceInfo info;
        info.DeviceTypeName = obj["DeviceTypeName"].toString();
        info.DeviceColorName = obj["DeviceColorName"].toString();
        info.DeviceColorPixmapPath = obj["DeviceColorPixmapPath"].toString();
        info.DeviceColorRGB = obj["DeviceColorRGB"].toString();
        info.isChecked = obj["isChecked"].toBool(false);
        info.DeviceHomePagePixmapPath = obj["DeviceHomePagePixmapPath"].toString();
        info.DeviceHomePageTopLeftPixmapPath_normal = obj["DeviceHomePageTopLeftPixmapPath_normal"]
                                                          .toString();
        info.DeviceHomePageTopLeftPixmapPath_abnormal
            = obj["DeviceHomePageTopLeftPixmapPath_abnormal"].toString();
        info.DeviceMoreSetPixmapPath = obj["DeviceMoreSetPixmapPath"].toString();
        info.DeviceMoreSetQrCodePixmapPath = obj["DeviceMoreSetQrCodePixmapPath"].toString();
        info.DeviceManualUrl = obj["DeviceManualUrl"].toString();
        info.DeviceSysTypeName = obj["DeviceSysTypeName"].toString();
        info.SelDev_VID = static_cast<unsigned short>(obj["SelDev_VID"].toInt(0));
        info.SelDev_PID = static_cast<unsigned short>(obj["SelDev_PID"].toInt(0));
        info.DeviceGuid = obj["DeviceGuid"].toString();
        return info;
    }
} DeviceInfo;

} // namespace DeSheng

#endif // DEVICE_INFO_H
