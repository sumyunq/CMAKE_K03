#include "modules/UserSetting/UserSettingCustomUI/custom_QPushButton_single_settings_type.h"

CustomQPushButtonSingleSettingsType::CustomQPushButtonSingleSettingsType(QWidget *parent)
    : QPushButton(parent)
{
    setCursor(Qt::PointingHandCursor);
    InitUIInformation(); // 初始化UI的默认信息
    InitMember();        // 初始化内部成员
    InitConnect();       // 连接默认的信号槽
}

CustomQPushButtonSingleSettingsType::~CustomQPushButtonSingleSettingsType() {}

void CustomQPushButtonSingleSettingsType::InitUIInformation()
{
    // 设置尺寸限制
    setMinimumSize(cl_min_size_);
    setMaximumSize(cl_max_size_);

    // 文字居中

    //添加样式：默认、悬停、选中
    setStyleSheet(R"(

QPushButton {
    font-family: "Noto Sans S Chinese";
    font-weight: 500;
    font-size: 14px;
    text-align: center;
    color: #A1A8B3;
    border-radius: 6px;
}
QPushButton:hover {
    font-family: "Noto Sans S Chinese";
    font-weight: 500;
    font-size: 14px;
    text-align: center;
    color: #FFFFFF;
    border-radius: 6px;
    background: #009FEF;
}
QPushButton:checked {
    font-family: "Noto Sans S Chinese";
    font-weight: 500;
    font-size: 14px;
    text-align: center;
    color: #FFFFFF;
    border-radius: 6px;
    background: #009FEF;
}

)");

    setText(""); // 默认为null
    hide();      // 默认不显示
}

void CustomQPushButtonSingleSettingsType::InitMember() {}

void CustomQPushButtonSingleSettingsType::InitConnect() {}

bool CustomQPushButtonSingleSettingsType::cl_is_show() const
{
    return cl_is_show_;
}

void CustomQPushButtonSingleSettingsType::setCl_is_show(const bool newCl_is_show)
{
    cl_is_show_ = newCl_is_show;
}

QString CustomQPushButtonSingleSettingsType::cl_settings_type() const
{
    return cl_settings_type_;
}

void CustomQPushButtonSingleSettingsType::setCl_settings_type(const QString &newCl_settings_type)
{
    cl_settings_type_ = newCl_settings_type;
    this->setText(cl_settings_type_);
}
