#include "FeedBackC/SoundTest/SoundTestCustomUI/video_scroll_area.h"

VideoScrollArea::VideoScrollArea(ScrollAreaDisplayMode showMode, QObject *parent)
    : cl_showMode(showMode)
{
    InitUIInformation(); ///< 初始化UI的默认信息
    InitMember();        ///< 初始化内部成员
    InitConnect();       ///< 连接默认的信号槽
}

VideoScrollArea::~VideoScrollArea() {}

void VideoScrollArea::InitUIInformation()
{
    setStyleSheet(R"(
        QScrollArea {
           background: transparent;
            background: none;
        }
        QScrollBar:vertical {
           background: rgba(0, 0, 0, 0);
           border-radius: 5px;
           width: 10px;
           margin: 0;
        }
        QScrollBar::handle:vertical {
           background: rgba(0, 0, 0, 0.2);
           border-radius: 5px;
           min-height: 30px;
        }
        QScrollBar::add-line:vertical,
        QScrollBar::sub-line:vertical {
           border: none;
           background: none;
           height: 0;
        }
        QScrollBar::add-page:vertical,
        QScrollBar::sub-page:vertical {
           background: none;
        }
)");
    this->viewport()->setAutoFillBackground(false); // 视口需要单独设置透明


    cl_content_widget_ = new QWidget(this);
    cl_content_widget_->setObjectName("cl_content_widget_");
    cl_content_widget_->setStyleSheet(R"(
        QWidget#cl_content_widget_ {
            background: transparent;
            border: none;
        }
)");



    switch (cl_showMode) {
    case ScrollAreaDisplayMode::GridDisplay: {
        this->setMinimumWidth(SingleVideoInfo::getMinWidth());

        cl_gridLayout_ = new QGridLayout(cl_content_widget_);
        cl_gridLayout_->setSpacing(20);
        cl_gridLayout_->setContentsMargins(30, 0, 30, 0);

        cl_hBoxLayout_ = new QHBoxLayout(); ///水平布局
        cl_vBoxLayout_ = new QVBoxLayout(); ///垂直布局
        break;
    }
    case ScrollAreaDisplayMode::SingleColumnDisplay: {
        this->setMinimumWidth(SingleVideoInfo::getMinWidth());

        cl_vBoxLayout_ = new QVBoxLayout(cl_content_widget_); ///垂直布局
        cl_vBoxLayout_->setSpacing(0);
        cl_vBoxLayout_->setContentsMargins(0, 0, 0, 0);

        cl_gridLayout_ = new QGridLayout(); ///网格布局
        cl_hBoxLayout_ = new QHBoxLayout(); ///水平布局
        break;
    }
    default: {
        cl_gridLayout_ = new QGridLayout(cl_content_widget_);
        cl_gridLayout_->setSpacing(20);
        cl_gridLayout_->setContentsMargins(0, 0, 0, 0);

        cl_hBoxLayout_ = new QHBoxLayout(); ///水平布局
        cl_vBoxLayout_ = new QVBoxLayout(); ///垂直布局
        break;
    }
    }

    /// 将内容控件设置为滚动区域的内容
    this->setWidget(cl_content_widget_);
    this->setWidgetResizable(true); ///自适应大小

    /// 滚动条策略
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);

    /// 内容控件的尺寸策略
    cl_content_widget_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    ///测试布局
    // for (int i = 0; i < 3; ++i) {
    //     SingleVideoInfo *p_b = new SingleVideoInfo(cl_showMode, cl_content_widget_);
    //     cl_all_videos_.append(p_b);
    // }

    updateView();
}

void VideoScrollArea::InitMember() {}

void VideoScrollArea::InitConnect() {}

void VideoScrollArea::updateView()
{
    ///网格布局显示
    if (cl_showMode == ScrollAreaDisplayMode::GridDisplay) {
        // cl_hBoxLayout_->setParent(nullptr); ///水平布局
        // cl_vBoxLayout_->setParent(nullptr); ///垂直布局
        // cl_gridLayout_->setParent(cl_content_widget_);

        if (!cl_gridLayout_)
            return;
        ///清除布局内部 item
        while (QLayoutItem *item = cl_gridLayout_->takeAt(0)) {
            delete item;
        }

        int widgetWidth = this->viewport()->width();
        int leftMargin = 30;    //左边距
        int rightMargin = 20;   //右边距
        int inMargin = 20;      //内部部件边距
        int rightComWidth = 10; //右侧滑动部件宽度

        ///单列数 取 (视口的实际可用宽度- 左边距 - 右边距 + 右侧空出的部件边距 + 右侧滚动条宽度) 除以 内部控件最小宽度 + 部件边距（网格布局： 238 + inMargin ）
        this->cl_columnCount_ = (widgetWidth - leftMargin - rightMargin + inMargin + rightComWidth)
                                / (238 + inMargin);

        if (cl_columnCount_ < 1)
            cl_columnCount_ = 1;
        /// 设置网格布局从左上开始
        cl_gridLayout_->setAlignment(Qt::AlignTop | Qt::AlignLeft);
        for (int i = 0; i < cl_all_videos_.size(); ++i) {
            int row = i / cl_columnCount_;
            int col = i % cl_columnCount_ + 1;
            cl_gridLayout_->addWidget(cl_all_videos_.at(i), row, col);
        }
    }

    ///垂直布局（单列显示）
    if (cl_showMode == ScrollAreaDisplayMode::SingleColumnDisplay) {
        if (!cl_vBoxLayout_)
            return;
        ///清除布局内部 item
        while (QLayoutItem *item = cl_vBoxLayout_->takeAt(0)) {
            delete item;
        }

        for (int i = 0; i < cl_all_videos_.size(); ++i) {
            cl_vBoxLayout_->addWidget(cl_all_videos_.at(i));
        }
        cl_vBoxLayout_->addStretch();
    }
}


void VideoScrollArea::dealwithSingleVideo(QString fileName)
{
    SingleVideoInfo *current = qobject_cast<SingleVideoInfo *>(sender());
    if (!current)
        return;

    emit openVideo(fileName);

    update();
}

void VideoScrollArea::resizeEvent(QResizeEvent *event)
{
    QScrollArea::resizeEvent(event);
    /// 延迟更新
    QTimer::singleShot(0, this, [this]() { this->updateView(); });
}