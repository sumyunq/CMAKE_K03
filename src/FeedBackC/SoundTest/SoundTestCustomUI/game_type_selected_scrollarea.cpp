#include "FeedBackC/SoundTest/SoundTestCustomUI/game_type_selected_scrollarea.h"

GameTypeSelectedScrollArea::GameTypeSelectedScrollArea(QWidget *parent)
    : QScrollArea(parent)
{
    InitUIInformation(); ///< 初始化UI的默认信息
    InitMember();        ///< 初始化内部成员
    InitConnect();       ///< 连接默认的信号槽

    updateView();
}

GameTypeSelectedScrollArea::~GameTypeSelectedScrollArea() {}

void GameTypeSelectedScrollArea::updateView()
{
    if (!cl_hBoxLayout_)
        return;

    if (!cl_all_games_type_buttons_)
        return;

    if (cl_all_game_type_.size() == 0)
        return;

    for (QPushButtonSingleGameType *btn : cl_all_game_type_) {
        if (btn->cl_is_show()) {
            btn->show();
        } else {
            btn->hide();
        }
    }
    update();
}

void GameTypeSelectedScrollArea::InitUIInformation()
{

    // this->setMinimumSize(300, 20);
    this->setStyleSheet(R"(
        QScrollArea {
            background: transparent;
            border: none;
        }
)");

    cl_content_widget_ = new QWidget(this);
    cl_content_widget_->setStyleSheet(R"(
        QWidget {

           border: none;
        }
)");

    cl_hBoxLayout_ = new QHBoxLayout(this); ///水平布局
    cl_hBoxLayout_->setSpacing(spacing_);
    cl_hBoxLayout_->setContentsMargins(left_margin_, top_margin_, right_margin_, bottom_margin_);

    cl_all_games_type_buttons_ = new QButtonGroup(this);
    cl_all_games_type_buttons_->setExclusive(true); /// 单选模式

    /// 将内容控件设置为滚动区域的内容
    this->setWidget(cl_content_widget_);
    this->setWidgetResizable(true); ///自适应大小

    /// 滚动条策略
    setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    /// 内容控件的尺寸策略
    cl_content_widget_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    /// 默认创建十个按键,更新时，只更新显示的内容，以及是否隐藏
    for (int var = 0; var < 10; ++var) {
        QPushButtonSingleGameType *btn = new QPushButtonSingleGameType(cl_content_widget_);

        btn->setCheckable(true);
        cl_all_game_type_.append(btn);
        cl_all_games_type_buttons_->addButton(btn, var);
    }

    /// 不修改布局内容，只更新对应的 按键的 hide /show
    for (int i = 0; i < cl_all_game_type_.size(); ++i) {
        cl_hBoxLayout_->addWidget(cl_all_game_type_.at(i));
    }
    cl_hBoxLayout_->addStretch();

}

void GameTypeSelectedScrollArea::InitMember() {}

void GameTypeSelectedScrollArea::InitConnect()
{
    connect(cl_all_games_type_buttons_,
            &QButtonGroup::idClicked,
            this,
            &GameTypeSelectedScrollArea::onGameTypeClicked,
            Qt::UniqueConnection);
}

void GameTypeSelectedScrollArea::resizeEvent(QResizeEvent *event)
{
    QScrollArea::resizeEvent(event);
    /// 延迟更新
    QTimer::singleShot(0, this, [this]() { this->updateView(); });
}

void GameTypeSelectedScrollArea::onGameTypeClicked(int index)
{
    /// 发射更新信号,通知 指定窗口更新滑动区域视频内容
    emit changeGameTypeVideos(index);
}
