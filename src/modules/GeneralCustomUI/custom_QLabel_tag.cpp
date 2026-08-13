#include "modules/GeneralCustomUI/custom_QLabel_tag.h"

#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>

CustomQLabelTag::CustomQLabelTag(QWidget *parent, int theme)
    : QLabel(parent)
    , cl_theme_(theme)
{
    InitUIInformation(theme);
    InitMember();
    InitConnect();
}

CustomQLabelTag::~CustomQLabelTag() {}

void CustomQLabelTag::InitUIInformation(int theme)
{
    {
        setAlignment(Qt::AlignCenter);
        setContentsMargins(7, 0, 7, 0);
        cl_font_.setPixelSize(11);
        cl_font_.setWeight(QFont::Normal);
        setFont(cl_font_);
        setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    }

    refreshLabel();
    applyTheme(theme);
}

void CustomQLabelTag::applyTheme(int theme)
{
    cl_theme_ = theme;
    switch (theme) {
    case 0: {
    } break;
    }
}

void CustomQLabelTag::InitMember() {}

void CustomQLabelTag::InitConnect() {}

void CustomQLabelTag::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)
    QPainter t_painter(this);
    t_painter.setRenderHint(QPainter::Antialiasing);

    QRectF t_rect = QRectF(rect()).adjusted(0.5, 0.5, -0.5, -0.5);
    QPainterPath t_path;
    t_path.addRoundedRect(t_rect, height() / 2.0, height() / 2.0);

    // 背景
    t_painter.fillPath(t_path, (cl_tag_style_ == TagLabelStyle::selected)
                                   ? QColor("#2D9DF0") : QColor("#333D4D"));

    // 文字
    t_painter.setPen(cl_tag_style_ == TagLabelStyle::selected ? Qt::white : QColor("#9AA4B2"));
    t_painter.setFont(font());
    t_painter.drawText(rect(), Qt::AlignCenter, text());
}

void CustomQLabelTag::mousePressEvent(QMouseEvent *event)
{
    Q_UNUSED(event)
    // 乐观更新：先切样式，计数由外部 API 回调驱动
    if (cl_tag_style_ == TagLabelStyle::selected) {
        setCl_tag_style(TagLabelStyle::not_selected);
        if (cl_tag_number_ > 0)
            setCl_tag_number(cl_tag_number_ - 1);
    } else {
        setCl_tag_style(TagLabelStyle::selected);
        setCl_tag_number(cl_tag_number_ + 1);
    }
    emit clicked();
}

QString CustomQLabelTag::formatNumber(int n)
{
    if (n < 10000)
        return QString::number(n);

    int t_w = n / 1000;
    int t_whole = t_w / 10;
    int t_dec   = t_w % 10;
    return QString::number(t_whole) + "." + QString::number(t_dec) + "w";
}

void CustomQLabelTag::refreshLabel()
{
    setText(cl_tag_text_ + " " + formatNumber(cl_tag_number_));
    adjustSize();
    updateGeometry();
    cl_cached_width_ = 0;
}

QFont CustomQLabelTag::cl_font() const { return cl_font_; }

void CustomQLabelTag::setCl_font(const QFont &font)
{
    cl_font_ = font;
    setFont(font);
    refreshLabel();
}

int CustomQLabelTag::cachedWidth() const
{
    if (cl_cached_width_ <= 0)
        cl_cached_width_ = sizeHint().width();
    return cl_cached_width_;
}

QString CustomQLabelTag::cl_tag_text() const { return cl_tag_text_; }

void CustomQLabelTag::setCl_tag_text(const QString &text)
{
    if (cl_tag_text_ == text) return;
    cl_tag_text_ = text;
    refreshLabel();
}

int CustomQLabelTag::cl_tag_number() const { return cl_tag_number_; }

void CustomQLabelTag::setCl_tag_number(int number)
{
    if (cl_tag_number_ == number) return;
    cl_tag_number_ = number;
    refreshLabel();
}

TagLabelStyle CustomQLabelTag::cl_tag_style() const { return cl_tag_style_; }

void CustomQLabelTag::setCl_tag_style(TagLabelStyle style)
{
    if (cl_tag_style_ == style) return;
    cl_tag_style_ = style;
    refreshLabel();
    update();
}

void CustomQLabelTag::updateTag(const QString &text, int number)
{
    cl_tag_text_   = text;
    cl_tag_number_ = number;
    refreshLabel();
}
