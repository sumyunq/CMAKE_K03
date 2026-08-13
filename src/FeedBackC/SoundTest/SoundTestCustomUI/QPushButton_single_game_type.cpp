#include "FeedBackC/SoundTest/SoundTestCustomUI/QPushButton_single_game_type.h"

QPushButtonSingleGameType::QPushButtonSingleGameType(QWidget *parent)
    : QPushButton(parent)
{
    setCursor(Qt::PointingHandCursor);
    InitUIInformation(); ///< 初始化UI的默认信息
    InitMember();        ///< 初始化内部成员
    InitConnect();       ///< 连接默认的信号槽
}

QPushButtonSingleGameType::~QPushButtonSingleGameType() {}

void QPushButtonSingleGameType::InitUIInformation()
{
    /// 设置尺寸限制
    setMinimumSize(minWidth, minHeight);
    setMaximumSize(maxWidth, maxHeight);

    /// 文字居中

    ///添加样式：默认、悬停、选中
    setStyleSheet(R"(

QPushButton {
    font-family: "Noto Sans S Chinese";
                font-weight: 500;
    font-size: 14px;
    text-align: center;
    color: #A1A8B3;
    border: none;
}
QPushButton:hover {
    font-family: "Noto Sans S Chinese";
                font-weight: 500;
    font-size: 14px;
    text-align: center;
    color: #FFFFFF;
    border-image: url(:/Skin/Images/soundTest/game_type_btn_hover.png);
    border: none;
}
QPushButton:checked {
    font-family: "Noto Sans S Chinese";
                font-weight: 500;
    font-size: 14px;
    text-align: center;
    color: #FFFFFF;
    border-image: url(:/Skin/Images/soundTest/game_type_btn_checked.png);
    border: none;
}

)");

    setText(""); ///默认为null
    hide();      ///默认不显示
}

void QPushButtonSingleGameType::InitMember() {}

void QPushButtonSingleGameType::InitConnect() {}

int QPushButtonSingleGameType::cl_is_show() const
{
    return cl_is_show_.load();
}

void QPushButtonSingleGameType::setCl_is_show(const int &newCl_is_show)
{
    cl_is_show_.store(newCl_is_show);
}

QString QPushButtonSingleGameType::cl_game_name() const
{
    return cl_game_name_;
}

void QPushButtonSingleGameType::setCl_game_name(const QString &newCl_game_name)
{
    cl_game_name_ = newCl_game_name;
    setText(cl_game_name_);
}
