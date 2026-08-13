#include "modules/HomePage/HomePageCustomUI/custom_QWidget_microphone_adjustment.h"
#include "ui_custom_QWidget_microphone_adjustment.h"

CustomQWidgetMicrophoneAdjustment::CustomQWidgetMicrophoneAdjustment(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::CustomQWidgetMicrophoneAdjustment)
{
    ui->setupUi(this);
    InitUIInformation();
    InitMember();
    InitConnect();
}

CustomQWidgetMicrophoneAdjustment::~CustomQWidgetMicrophoneAdjustment()
{
    delete ui;
}

void CustomQWidgetMicrophoneAdjustment::InitUIInformation()
{
    {
        cl_text_1_ = new QLabel(tr("麦克风"), this);
        cl_text_1_->move(cl_text_1_point_);
        cl_text_1_->setStyleSheet(R"(
            font-family: "Noto Sans S Chinese";
            font-weight: 500;
            font-size: 14px;
            color: #A1A8B3;
)");
    }
    {
        // 图标：麦克风图标
        cl_icon_ = new QLabel(this);
        cl_icon_->move(cl_icon_point_);

        cl_icon_->setPixmap(QPixmap(":/Skin/Images/homePage/microphone_icon_2x.png")
                                .scaled(cl_icon_min_size_,
                                        Qt::KeepAspectRatio, // 保持宽高比
                                        Qt::SmoothTransformation));
    }
    {
        cl_text_2_ = new QLabel(tr("人声清晰"), this);
        cl_text_2_->move(cl_text_2_point_);
        cl_text_2_->setStyleSheet(R"(
            font-family: "Noto Sans S Chinese";
                font-weight: 500;
            font-size: 12px;
            color: #A1A8B3;
)");
    }

    {
        cl_text_3_ = new QLabel(tr("人声浑厚"), this);
        cl_text_3_->move(cl_text_3_point_);
        cl_text_3_->setStyleSheet(R"(
            font-family: "Noto Sans S Chinese";
                font-weight: 500;
            font-size: 12px;
            color: #A1A8B3;
)");
    }

    {
        // 人声清晰 开关按键
        cl_pushButton_clear_voices_ = new CustomPushButton(this);
        cl_pushButton_clear_voices_->setCheckable(true);
        cl_pushButton_clear_voices_->move(cl_pushButton_clear_voices_point_);
    }
    {
        // 人声浑厚 开关按键
        cl_pushButton_deepPowerful_voice_ = new CustomPushButton(this);
        cl_pushButton_deepPowerful_voice_->setCheckable(true);
        cl_pushButton_deepPowerful_voice_->move(cl_pushButton_deepPowerful_voice_point_);
    }
    {
        // 说明按键
        cl_pBt_explain_ = new QPushButton(this);
        cl_pBt_explain_->setMinimumSize(cl_pBt_explain_default_size_);
        cl_pBt_explain_->setObjectName("MicrophoneAdjustment_cl_pBt_explain");
        cl_pBt_explain_->setCursor(Qt::PointingHandCursor);
        cl_pBt_explain_->move(cl_pBt_explain_default_point_);
        cl_pBt_explain_->setStyleSheet(R"(
        QPushButton{
            border-image: url(:/Skin/Images/homePage/annotation_13_13_2x.png);
        }
)");
        {
            clp_tip_explain_ = new NewCustomToolTip(cl_pBt_explain_);
            clp_tip_explain_->setLabelStyle(0);
            clp_tip_explain_->AddToolTip(cl_pBt_explain_,
                                         tr("针对麦克风输入的后期处理，包含人声清晰、人声浑厚两种风格"),
                                         Qt::AlignHCenter);
        }
    }
}

void CustomQWidgetMicrophoneAdjustment::retranslateTexts()
{
    cl_text_1_->setText(tr("麦克风"));
    cl_text_2_->setText(tr("人声清晰"));
    cl_text_3_->setText(tr("人声浑厚"));
}

void CustomQWidgetMicrophoneAdjustment::InitMember() {}

void CustomQWidgetMicrophoneAdjustment::InitConnect() {}

void CustomQWidgetMicrophoneAdjustment::resizeEvent(QResizeEvent *event)
{
    {
        cl_text_1_->setGeometry(cl_text_1_point_.x(),
                                cl_text_1_point_.y(),
                                cl_text_1_min_size_.width() + 26,
                                cl_text_1_min_size_.height());
    }
    {
        cl_icon_->setGeometry(rect().width() - 30 - cl_icon_min_size_.width(),
                              cl_icon_point_.y(),
                              cl_icon_min_size_.width(),
                              cl_icon_min_size_.height());
    }
    {
        cl_text_2_->setGeometry(cl_text_2_point_.x(),
                                cl_text_2_point_.y(),
                                cl_text_2_min_size_.width() + 26,
                                cl_text_2_min_size_.height());
    }

    {
        cl_text_3_->setGeometry(cl_text_3_point_.x(),
                                cl_text_3_point_.y(),
                                cl_text_3_min_size_.width() + 26,
                                cl_text_3_min_size_.height());
    }

    // // 设置文字样式
    // QString labelStyle = "font-size: 14px; color: #333; font-weight: bold;";
    // cl_text_1_->setStyleSheet(labelStyle);
    // cl_text_2_->setStyleSheet(labelStyle);
    // cl_text_3_->setStyleSheet(labelStyle);
    {
        // 人声清晰 开关按键
        cl_pushButton_clear_voices_->setGeometry(rect().width() - 25
                                                     - cl_pushButton_clear_voices_min_size_.width(),
                                                 cl_pushButton_clear_voices_point_.y(),
                                                 cl_pushButton_clear_voices_min_size_.width(),
                                                 cl_pushButton_clear_voices_min_size_.height());
    }
    {
        // 人声浑厚 开关按键
        cl_pushButton_deepPowerful_voice_
            ->setGeometry(rect().width() - 25 - cl_pushButton_deepPowerful_voice_min_size_.width(),
                          cl_pushButton_deepPowerful_voice_point_.y(),
                          cl_pushButton_deepPowerful_voice_min_size_.width(),
                          cl_pushButton_deepPowerful_voice_min_size_.height());
    }

    {
        // 麦克风调节 提示图标
        cl_pBt_explain_->setGeometry(cl_text_1_->x() + cl_text_1_->width() + 4 - 26,
                                     cl_text_1_->y() + 4,
                                     cl_pBt_explain_default_size_.width(),
                                     cl_pBt_explain_default_size_.height());
    }
    QWidget::resizeEvent(event);
}
