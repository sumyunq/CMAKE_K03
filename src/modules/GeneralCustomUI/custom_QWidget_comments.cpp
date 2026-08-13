#include "modules/GeneralCustomUI/custom_QWidget_comments.h"
#include "modules/GeneralCustomUI/custom_QLabel_tag.h"

#include <QEvent>
#include <QMouseEvent>
#include <QPainter>
#include <QResizeEvent>

namespace {

constexpr int kExpandButtonWidth = 40;
constexpr int kExpandButtonHeight = 20;
constexpr int kMaxCollapsedTagRows = 2;
constexpr int kTagHorizontalPadding = 14;

int tagFlowWidth(const CustomQLabelTag *tag)
{
    if (!tag)
        return 0;
    return QFontMetrics(tag->font()).horizontalAdvance(tag->text()) + kTagHorizontalPadding;
}

} // namespace

CustomQWidgetComments::CustomQWidgetComments(QWidget *parent, int theme)
    : QWidget(parent)
    , cl_theme_(theme)
{
    InitUIInformation(theme);
    InitMember();
    InitConnect();
}

CustomQWidgetComments::~CustomQWidgetComments() {}

void CustomQWidgetComments::InitUIInformation(int theme)
{
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    setMouseTracking(true);

    clp_arrow_icon_ = new QLabel(this);
    clp_arrow_icon_->setFixedSize(8, 4);
    clp_arrow_icon_->setAttribute(Qt::WA_TransparentForMouseEvents);
    // 初始可见，paintEvent 中定位和更新图标

    applyTheme(theme);
}

void CustomQWidgetComments::applyTheme(int theme)
{
    cl_theme_ = theme;
    switch (theme) {
    case 0: {
    } break;
    }
}

void CustomQWidgetComments::InitMember() {}

void CustomQWidgetComments::InitConnect() {}

void CustomQWidgetComments::addTag(int key, CustomQLabelTag *tag)
{
    removeTag(key);
    tag->setParent(this);
    tag->show();
    connect(tag, &CustomQLabelTag::clicked, this, [this, key]() {
        emit tagClicked(key);
    }, Qt::UniqueConnection);
    cl_tag_map_.insert(key, tag);
    cl_tag_order_.append(key);
    doLayout();
}

void CustomQWidgetComments::removeTag(int key)
{
    auto t_it = cl_tag_map_.find(key);
    cl_tag_order_.removeAll(key);
    if (t_it != cl_tag_map_.end()) {
        delete t_it.value();
        cl_tag_map_.erase(t_it);
        doLayout();
    }
}

void CustomQWidgetComments::clearTags()
{
    if (cl_tag_map_.isEmpty()) {
        cl_tag_order_.clear();
        return;
    }

    // 先取出全部标签指针再清空 map，防止 delete QWidget 触发 doLayout 重入访问空悬指针
    QList<CustomQLabelTag *> t_tags = cl_tag_map_.values();
    cl_tag_map_.clear();
    cl_tag_order_.clear();

    for (auto *t_tag : t_tags) {
        delete t_tag; ///< 此时 doLayout 重入看到空 map，安全
    }

    cl_expanded_ = false;
    doLayout();
}

CustomQLabelTag *CustomQWidgetComments::tag(int key) const
{
    return cl_tag_map_.value(key, nullptr);
}

bool CustomQWidgetComments::cl_expanded() const { return cl_expanded_; }

void CustomQWidgetComments::setCl_expanded(bool expanded)
{
    if (cl_expanded_ == expanded) return;
    cl_expanded_ = expanded;
    updateArrowIcon();
    doLayout();
    update();
    emit expandedChanged(expanded);
}

void CustomQWidgetComments::updateArrowIcon()
{
    if (!clp_arrow_icon_) return;
    clp_arrow_icon_->setPixmap(QPixmap(cl_expanded_
        ? (cl_is_hover_expand_ ? QStringLiteral(":/Skin/Images/GeneralIcon/arrow_down_hover.png")
                               : QStringLiteral(":/Skin/Images/GeneralIcon/arrow_down_normal.png"))
        : (cl_is_hover_expand_ ? QStringLiteral(":/Skin/Images/GeneralIcon/arrow_up_hover.png")
                               : QStringLiteral(":/Skin/Images/GeneralIcon/arrow_up_normal.png"))));
}

int CustomQWidgetComments::cl_h_spacing() const { return cl_h_spacing_; }
void CustomQWidgetComments::setCl_h_spacing(int s)
{
    if (cl_h_spacing_ == s) return;
    cl_h_spacing_ = s;
    doLayout();
}

int CustomQWidgetComments::cl_v_spacing() const { return cl_v_spacing_; }
void CustomQWidgetComments::setCl_v_spacing(int s)
{
    if (cl_v_spacing_ == s) return;
    cl_v_spacing_ = s;
    doLayout();
}

int CustomQWidgetComments::cl_tag_height() const { return cl_tag_height_; }
void CustomQWidgetComments::setCl_tag_height(int h)
{
    if (cl_tag_height_ == h) return;
    cl_tag_height_ = h;
    doLayout();
}

QSize CustomQWidgetComments::sizeHint() const
{
    if (cl_cached_height_ > 0)
        return QSize(100, cl_cached_height_);
    return QSize(200, cl_tag_height_);
}

QSize CustomQWidgetComments::minimumSizeHint() const
{
    return QSize(0, cl_tag_height_);
}

bool CustomQWidgetComments::event(QEvent *event)
{
    if (event->type() == QEvent::LayoutRequest) {
        doLayout();
        return true;
    }
    return QWidget::event(event);
}

void CustomQWidgetComments::mousePressEvent(QMouseEvent *event)
{
    if (!cl_expand_rect_.isNull() && cl_expand_rect_.contains(event->pos())) {
        setCl_expanded(!cl_expanded_);
        event->accept();
        return;
    }
    QWidget::mousePressEvent(event);
}

void CustomQWidgetComments::mouseMoveEvent(QMouseEvent *event)
{
    if (!cl_expand_rect_.isNull() && cl_expand_rect_.contains(event->pos())) {
        if (!cl_is_hover_expand_) {
            cl_is_hover_expand_ = true;
            update();
            updateArrowIcon();
            setCursor(Qt::PointingHandCursor);
        }
    } else {
        if (cl_is_hover_expand_) {
            cl_is_hover_expand_ = false;
            update();
            updateArrowIcon();
            setCursor(Qt::ArrowCursor);
        }
    }
    QWidget::mouseMoveEvent(event);
}

void CustomQWidgetComments::leaveEvent(QEvent *event)
{
    if (cl_is_hover_expand_) {
        cl_is_hover_expand_ = false;
        update();
        updateArrowIcon();
        setCursor(Qt::ArrowCursor);
    }
    QWidget::leaveEvent(event);
}

void CustomQWidgetComments::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    doLayout();
}

void CustomQWidgetComments::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)

    if (cl_tag_map_.isEmpty())
        return;

    QPainter t_painter(this);
    t_painter.setRenderHint(QPainter::Antialiasing, true);

    // 展开/收起按钮：右下角 20×14 文字 + 8×4 箭头图标
    const bool t_is_expanded = cl_expanded_;
    const bool t_hover = cl_is_hover_expand_;
    const QColor t_color = t_is_expanded
        ? (t_hover ? QColor("#C3CDD4") : QColor("#9CABB4"))
        : (t_hover ? QColor("#5CB3F5") : QColor("#2D9DF0"));
    const QString t_text = t_is_expanded ? QStringLiteral("收起") : QStringLiteral("展开");

    QFont t_font("Noto Sans S Chinese");
    t_font.setWeight(QFont::Normal);
    t_font.setPixelSize(12);
    t_painter.setFont(t_font);

    // 文字：左对齐按钮区域，24×16，垂直居中（按钮高 20，文字高 16 → 上偏移 2px）
    QRect t_text_rect(cl_expand_rect_.left(), cl_expand_rect_.top() + 2, 24, 16);
    t_painter.setPen(t_color);
    t_painter.drawText(t_text_rect, Qt::AlignLeft | Qt::AlignVCenter, t_text);

    // 箭头图标已在 doLayout 中定位并显示

    // ── DEBUG: 区域可视化 ──
    // {
    //     // 红色半透明 — 文字绘制区域
    //     t_painter.setPen(Qt::NoPen);
    //     t_painter.setBrush(QColor(255, 0, 0, 60));
    //     t_painter.drawRect(t_text_rect);

    //     // 绿色半透明 — 箭头图标区域
    //     if (clp_arrow_icon_) {
    //         t_painter.setBrush(QColor(0, 255, 0, 60));
    //         t_painter.drawRect(QRect(clp_arrow_icon_->pos(), clp_arrow_icon_->size()));
    //     }

    //     // 蓝色边框 — 统一的点击/悬停按钮区域
    //     t_painter.setBrush(Qt::NoBrush);
    //     t_painter.setPen(QPen(QColor(0, 145, 218), 1, Qt::DashLine));
    //     t_painter.drawRect(t_btn_rect);
    // }
}

void CustomQWidgetComments::doLayout()
{
    int t_w = width();
    if (t_w <= 0) return;

    if (cl_tag_map_.isEmpty()) {
        cl_cached_height_ = 0;
        cl_expand_rect_ = QRect();
        if (clp_arrow_icon_)
            clp_arrow_icon_->hide();
        updateGeometry();
        return;
    }

    QList<int> t_keys;
    t_keys.reserve(cl_tag_order_.size());
    for (int t_key : cl_tag_order_) {
        if (cl_tag_map_.contains(t_key))
            t_keys.append(t_key);
    }

    const int t_row_h = cl_tag_height_ + cl_v_spacing_;
    const int t_btn_reserve = kExpandButtonWidth + cl_h_spacing_;
    const int t_max_rows = cl_expanded_ ? t_keys.size() : kMaxCollapsedTagRows;
    const int t_area_left = 0;
    const int t_area_end = t_area_left + t_w; // exclusive end; QRect::right() is inclusive
    int t_x = t_area_left;
    int t_y = 0;
    int t_rows = 0;
    int t_content_bottom = 0;
    bool t_stop_layout = false;

    for (int t_i = 0; t_i < t_keys.size(); ++t_i) {
        CustomQLabelTag *t_tag = cl_tag_map_.value(t_keys.at(t_i), nullptr);
        if (!t_tag)
            continue;

        if (t_stop_layout) {
            t_tag->hide();
            continue;
        }

        const int t_tag_w = tagFlowWidth(t_tag);
        int t_cur_row = t_y / t_row_h;
        if (t_cur_row >= t_max_rows) {
            t_tag->hide();
            continue;
        }

        const bool t_is_last_tag = (t_i == t_keys.size() - 1);
        auto rowEnd = [&](int row) {
            const bool t_reserve_button = (!cl_expanded_ && row == kMaxCollapsedTagRows - 1)
                                      || (cl_expanded_ && t_is_last_tag);
            return t_area_end - (t_reserve_button ? t_btn_reserve : 0);
        };

        int t_row_end = rowEnd(t_cur_row);
        if (t_x > t_area_left && t_x + t_tag_w > t_row_end) {
            t_x = t_area_left;
            t_y += t_row_h;
            t_cur_row = t_y / t_row_h;
            if (t_cur_row >= t_max_rows) {
                t_stop_layout = true;
                t_tag->hide();
                continue;
            }
            t_row_end = rowEnd(t_cur_row);
        }

        if (!cl_expanded_ && t_cur_row == kMaxCollapsedTagRows - 1
            && t_tag_w > t_w - t_btn_reserve) {
            t_stop_layout = true;
            t_tag->hide();
            continue;
        }

        t_tag->setGeometry(t_x, t_y, t_tag_w, cl_tag_height_);
        t_tag->show();
        t_rows = qMax(t_rows, t_cur_row + 1);
        t_content_bottom = t_y + cl_tag_height_;
        t_x += t_tag_w + cl_h_spacing_;
    }

    int t_disp_rows = qMax(t_rows, kMaxCollapsedTagRows);
    int t_new_h = t_disp_rows * cl_tag_height_ + (t_disp_rows - 1) * cl_v_spacing_;

    if (t_new_h != cl_cached_height_) {
        cl_cached_height_ = t_new_h;
        setFixedHeight(t_new_h);
        updateGeometry();
        emit heightChanged(t_new_h);
    }

    // 展开/收起按钮定位（在高度确定后）
    if (clp_arrow_icon_ && !cl_tag_map_.isEmpty()) {
        const int t_btn_y = qMax(0, t_content_bottom - kExpandButtonHeight);
        cl_expand_rect_ = QRect(qMax(0, width() - kExpandButtonWidth), t_btn_y,
                                kExpandButtonWidth, kExpandButtonHeight);
        clp_arrow_icon_->move(cl_expand_rect_.right() - 11,
                              cl_expand_rect_.top() + (kExpandButtonHeight - clp_arrow_icon_->height()) / 2);
        clp_arrow_icon_->show();
        updateArrowIcon(); ///< 初始设置 pixmap（否则只靠 setCl_expanded/hover 触发，初始为空）
    }

    update();
}
