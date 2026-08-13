#include "modules/UserSetting/UserSettingSubModule/PersonalCenterSettings/PersonalCenterSettingsCustomUI/custom_QWidget_grade_status.h"

/// \brief 构造函数
CustomQWidgetGradeStatus::CustomQWidgetGradeStatus(QWidget *parent)
    : QWidget(parent)
{
    InitUIInformation(); ///< 初始化UI的默认信息
    InitMember();        ///< 初始化内部成员
    InitConnect();       ///< 连接默认的信号槽
}

/// \brief 按主题更新样式
void CustomQWidgetGradeStatus::applyTheme(int theme)
{
    cl_theme_ = theme;

    if (theme == 0) {
        clp_grade_progressBar_->setStyleSheet(R"(
            QProgressBar#GradeStatus_progressBar {
                border: none;
                border-radius: 2px;
                background: #2A2F38;
                text-align: center;
                color: #009FEF;
                height: 20px;
            }
            QProgressBar::chunk {
                background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                                            stop:0 #2196F3, stop:1 #21CBF3);
                border-radius: 2px;
            }
        )");
    } else {
        clp_grade_progressBar_->setStyleSheet(R"(
            QProgressBar#GradeStatus_progressBar {
                border: 1px solid #ccc;
                border-radius: 2px;
                background: #f0f0f0;
                text-align: center;
                color: #333;
                height: 20px;
            }
            QProgressBar::chunk {
                background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                                            stop:0 #2196F3, stop:1 #21CBF3);
                border-radius: 2px;
            }
        )");
    }
}

/// \brief 初始化UI的默认信息
void CustomQWidgetGradeStatus::InitUIInformation()
{
    {
        this->setMinimumSize(332, 14);
    }
    {
        // 等级文字
        clp_grade_ = new QLabel(this);
        clp_grade_->setObjectName("GradeStatus_gradeLabel");
        clp_grade_->setMinimumSize(cl_grade_min_size_);
        clp_grade_->setAlignment(Qt::AlignCenter);
        clp_grade_->move(cl_grade_default_point_);
        clp_grade_->setStyleSheet(R"(
            QLabel#GradeStatus_gradeLabel {
                font-family: "ZQKfreefont";
                font-weight: 500;
                font-size: 12px;
                color: #FFFFFF;
                border-image: url(:/Skin/Images/more/personal_center_settings/Level_background.png);
            }
        )");
    }
    {
        // 经验进度条
        clp_grade_progressBar_ = new QProgressBar(this);
        clp_grade_progressBar_->setObjectName("GradeStatus_progressBar");
        clp_grade_progressBar_->setMinimumSize(cl_grade_progressBar_min_size_);
        clp_grade_progressBar_->move(cl_grade_progressBar_default_point_);
        // clp_grade_progressBar_->setRange(0, 1500);
        // clp_grade_progressBar_->setValue(1080);
        clp_grade_progressBar_->setTextVisible(false);
        clp_grade_progressBar_->setOrientation(Qt::Horizontal);
    }
    {
        // 经验比例文字
        clp_grade_empirical_value_proportion_ = new QLabel(this);
        clp_grade_empirical_value_proportion_->setObjectName("GradeStatus_empiricalLabel");
        clp_grade_empirical_value_proportion_->setMinimumSize(
            cl_grade_empirical_value_proportion_min_size_);
        clp_grade_empirical_value_proportion_->move(
            cl_grade_empirical_value_proportion_default_point_);
        clp_grade_empirical_value_proportion_->setStyleSheet(R"(
            QLabel#GradeStatus_empiricalLabel {
                font-family: "Noto Sans S Chinese";
                font-weight: 500;
                font-size: 12px;
                color: #616871;
                background: transparent;
            }
        )");
    }
    {
        // 应用默认主题样式
        applyTheme(cl_theme_);
    }
}

/// \brief 初始化内部成员
void CustomQWidgetGradeStatus::InitMember()
{
}

/// \brief 连接默认的信号槽
void CustomQWidgetGradeStatus::InitConnect()
{
}

/// \brief 设置等级文字
void CustomQWidgetGradeStatus::setCl_grade_level(int level)
{
    clp_grade_->setText(QString("Lv.%1").arg(level));
}

/// \brief 设置经验进度
void CustomQWidgetGradeStatus::setCl_progress(int value, int maximum)
{
    clp_grade_progressBar_->setMaximum(maximum);
    clp_grade_progressBar_->setValue(value);
}

/// \brief 设置经验比例文字
void CustomQWidgetGradeStatus::setCl_empirical_text(const QString &text)
{
    clp_grade_empirical_value_proportion_->setText(text);
}

/// \brief 设置经验数值（自动格式化为 "currentXp/requiredXp"）
void CustomQWidgetGradeStatus::setCl_empirical_value(int currentXp, int requiredXp)
{
    clp_grade_empirical_value_proportion_->setText(
        QString("%1/%2").arg(currentXp).arg(requiredXp));
}

/// \brief 窗口大小变化时更新子控件布局
void CustomQWidgetGradeStatus::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);

    {
        clp_grade_->setGeometry(QRect(cl_grade_default_point_, cl_grade_current_size_));
    }
    {
        clp_grade_progressBar_->setGeometry(cl_grade_progressBar_default_point_.x(),
                                            cl_grade_progressBar_default_point_.y(),
                                            rect().width() - 74 - 92,
                                            cl_grade_progressBar_current_size_.height());
    }
    {
        clp_grade_empirical_value_proportion_
            ->setGeometry(rect().width() - 40
                              - cl_grade_empirical_value_proportion_current_size_.width(),
                          cl_grade_empirical_value_proportion_default_point_.y(),
                          cl_grade_empirical_value_proportion_current_size_.width(),
                          cl_grade_empirical_value_proportion_current_size_.height());
    }
}
