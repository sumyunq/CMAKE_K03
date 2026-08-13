#include "modules/HomePage/HomePageCustomUI/custom_QPushButton_single_plan.h"
#include "modules/Common/elide_text.h"  ///< DeSheng::elideTextWithDots

CustomQPushButtonSinglePlan::CustomQPushButtonSinglePlan(QWidget *parent)
    : QPushButton(parent)
{
    InitUIInformation();
    InitMember();
    InitConnect();
}

CustomQPushButtonSinglePlan::~CustomQPushButtonSinglePlan() {}

void CustomQPushButtonSinglePlan::setImages(const QString &normal,
                                            const QString &hover,
                                            const QString &checked)
{
    cl_normalImg_ = normal;
    cl_hover_img_ = hover;
    cl_checkedImg_ = checked;
    updateStyle();
}

void CustomQPushButtonSinglePlan::InitUIInformation()
{
    setCursor(Qt::PointingHandCursor); /// 手型光标
}

void CustomQPushButtonSinglePlan::InitMember()
{
    setFixedSize(cl_default_size_);
    // setMinimumSize (cl_default_size_);
    // setMaximumSize (cl_max_size_);
}

void CustomQPushButtonSinglePlan::InitConnect()
{

}

void CustomQPushButtonSinglePlan::updateStyle()
{
    QString style = QStringLiteral("QPushButton {"
                                   "    border: null;"
                                   "    color: #A1A8B3;"
                                   "    text-align: left;"
                                   "    padding-left: 8px;"
                                   "    border-image: url(%1);"
                                   "    font-family: \"Noto Sans S Chinese\";"
                                   "    font-weight: 500;"
                                   "    font-size: 10px;"
                                   "}"
                                   "QPushButton:hover { border-image: url(%2); }"
                                   "QPushButton:checked { border-image: url(%3); }")
                        .arg(cl_normalImg_, cl_hover_img_, cl_checkedImg_);

    setStyleSheet(style);
}

void CustomQPushButtonSinglePlan::setPlan_key(const QPair<QString, QString> &newPlan_key)
{
    cl_plan_key_ = newPlan_key;
}

void CustomQPushButtonSinglePlan::paintEvent(QPaintEvent *event)
{
    QPushButton::paintEvent(event); // 先画按钮背景和边框

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    QFont font("Noto Sans S Chinese");
    font.setPixelSize(10); // 明确的像素大小

    painter.setFont(font);
    painter.setPen(palette().buttonText().color());

    // 关键修改：添加文本省略
    cl_text_rect_ = QRect(6, 38, 44, 14);
    QFontMetrics fm(font);
    QString elidedText = DeSheng::elideTextWithDots(cl_plans_name_, font, cl_text_rect_.width());

    // 指定位置绘制文字
    // painter.drawText(cl_text_rect_, Qt::AlignLeft | Qt::AlignVCenter, elidedText);   ///暂时不需要文字
}
