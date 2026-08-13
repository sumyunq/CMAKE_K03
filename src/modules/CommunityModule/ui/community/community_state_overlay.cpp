#include "modules/CommunityModule/ui/community/community_state_overlay.h"

#include <QLabel>
#include <QMouseEvent>
#include <QPainter>
#include <QPixmap>
#include <QPushButton>
#include <QResizeEvent>
#include <QShowEvent>

namespace {

/// \brief 各空态/错误态图标布局（y 为覆盖层面板内坐标；x 一律运行时水平居中）
///
/// 值来源：tools/ui_mockups/01-03，2026-08-07 用户 Designer 调整，
/// 画布 y − 56（社区页工具栏高 = tabArea 44 + 上下边距 6+6）换算为面板内坐标。
struct StateLayout {
  int iconY = 0;          // 图标 y
  QSize icon;             // 图标显示尺寸（@2x 源图缩小一倍）
  int textY = 0;          // 文字 y
  int textW = 0;          // 文字宽（0 = 铺满面板宽）
  int textH = 0;          // 文字高
  int buttonY = 0;        // 刷新按钮 y（104×30，0 = 无按钮）
};

const StateLayout kTextOnlyLayout{0, QSize(), 150, 0, 30, 0};  // 纯文字态（防御，当前无调用方）

StateLayout layoutForIcon(const QString& iconPath) {
  if (iconPath.contains(QStringLiteral("NetError")))
    return {90, QSize(236, 185), 311, 500, 20, 396};
  if (iconPath.contains(QStringLiteral("searchPlanEmpty")))
    return {116, QSize(242, 192), 358, 224, 23, 0};
  // PlanEmpty（默认）
  return {141, QSize(249, 187), 378, 500, 20, 0};
}

QRect alphaBounds(const QImage& image) {
  if (image.isNull() || !image.hasAlphaChannel()) return image.rect();

  int minX = image.width();
  int minY = image.height();
  int maxX = -1;
  int maxY = -1;
  for (int y = 0; y < image.height(); ++y) {
    for (int x = 0; x < image.width(); ++x) {
      if (qAlpha(image.pixel(x, y)) <= 0) continue;
      minX = qMin(minX, x);
      minY = qMin(minY, y);
      maxX = qMax(maxX, x);
      maxY = qMax(maxY, y);
    }
  }

  if (maxX < minX || maxY < minY) return image.rect();
  return QRect(QPoint(minX, minY), QPoint(maxX, maxY));
}

QPixmap centeredStateIconPixmap(const QString& iconPath, const QSize& displaySize) {
  const QImage image(iconPath);
  if (image.isNull() || displaySize.isEmpty()) return QPixmap();

  const QPixmap scaled = QPixmap::fromImage(image).scaled(
      displaySize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
  QPixmap canvas(displaySize);
  canvas.fill(Qt::transparent);

  const QRect alphaRect = alphaBounds(image);
  const QPointF imageCenter(image.width() / 2.0, image.height() / 2.0);
  const QPointF alphaCenter(alphaRect.x() + alphaRect.width() / 2.0,
                            alphaRect.y() + alphaRect.height() / 2.0);
  const qreal scaleX = scaled.width() / static_cast<qreal>(image.width());
  const qreal scaleY = scaled.height() / static_cast<qreal>(image.height());
  const QPointF visibleOffset((alphaCenter.x() - imageCenter.x()) * scaleX,
                              (alphaCenter.y() - imageCenter.y()) * scaleY);

  const QPointF imageTopLeft((displaySize.width() - scaled.width()) / 2.0,
                             (displaySize.height() - scaled.height()) / 2.0);
  QPainter painter(&canvas);
  painter.drawPixmap((imageTopLeft - visibleOffset).toPoint(), scaled);
  return canvas;
}

int iconTextGap(const StateLayout& layout) {
  if (layout.icon.isEmpty()) return 0;
  return qMax(0, layout.textY - layout.iconY - layout.icon.height());
}

int textButtonGap(const StateLayout& layout, bool retryEnabled) {
  if (!retryEnabled || layout.buttonY <= 0) return 0;
  return qMax(0, layout.buttonY - layout.textY - layout.textH);
}

int stateContentHeight(const StateLayout& layout, bool retryEnabled) {
  int contentHeight = layout.textH;
  if (!layout.icon.isEmpty()) {
    contentHeight += layout.icon.height() + iconTextGap(layout);
  }
  if (retryEnabled && layout.buttonY > 0) {
    contentHeight += textButtonGap(layout, retryEnabled) + 30;
  }
  return contentHeight;
}

}  // namespace

CommunityStateOverlay::CommunityStateOverlay(QWidget* parent) : QWidget(parent) {
  // objectName 供 QSS 挂背景图（后续扩展）
  setObjectName(QStringLiteral("communityStateOverlay"));
  setAttribute(Qt::WA_TransparentForMouseEvents, false);
  hide();

  // 图标（空态图，按查表尺寸 setGeometry）
  clp_icon_label_ = new QLabel(this);
  clp_icon_label_->setAlignment(Qt::AlignCenter);
  clp_icon_label_->hide();

  // 提示文字（16px，x 运行时居中）
  clp_text_label_ = new QLabel(this);
  clp_text_label_->setAlignment(Qt::AlignCenter);
  clp_text_label_->setStyleSheet(
      "QLabel{color:#747880;background:transparent;font-size:16px;"
      "font-family:\"Noto Sans S Chinese\";font-weight:500;}");

  // 刷新按钮（错误态显示；样式与"上传方案"一致 104×30 confirm 图）
  clp_retry_button_ = new QPushButton(tr("刷新"), this);
  clp_retry_button_->setCursor(Qt::PointingHandCursor);
  clp_retry_button_->setStyleSheet(
      "QPushButton{font-family:\"Noto Sans S Chinese\";font-weight:500;font-size:12px;"
      "color:#FFFFFF;border-image:url(:/Skin/Images/Popup/confirm-no.png);}"
      "QPushButton:hover{border-image:url(:/Skin/Images/Popup/confirm-ho.png);}");
  clp_retry_button_->hide();
  connect(clp_retry_button_, &QPushButton::clicked, this, [this] { emit retryClicked(); });
}

void CommunityStateOverlay::showState(const QString& text, bool retryEnabled,
                                      const QString& iconPath) {
  cl_icon_path_ = iconPath;
  cl_text_ = text;
  cl_retry_enabled_ = retryEnabled;
  cl_state_active_ = true;
  if (parentWidget()) setGeometry(parentWidget()->rect());
  refreshLayout();
  setCursor(retryEnabled ? Qt::PointingHandCursor : Qt::ArrowCursor);
  show();
  raise();
}

void CommunityStateOverlay::refreshLayout() {
  if (!cl_state_active_) return;
  cl_apply_layout_();
}

void CommunityStateOverlay::cl_apply_layout_() {
  const StateLayout t_layout =
      cl_icon_path_.isEmpty() ? kTextOnlyLayout : layoutForIcon(cl_icon_path_);
  const int t_content_h = stateContentHeight(t_layout, cl_retry_enabled_);
  int t_next_y = (height() - t_content_h) / 2;
  if (t_next_y < 0) t_next_y = 0;

  // 图标：按查表尺寸，x 水平居中；纯文字态隐藏
  if (t_layout.icon.height() > 0) {
    const QPixmap t_pm = centeredStateIconPixmap(cl_icon_path_, t_layout.icon);
    if (!t_pm.isNull()) {
      clp_icon_label_->setGeometry((width() - t_layout.icon.width()) / 2, t_next_y,
                                   t_layout.icon.width(), t_layout.icon.height());
      clp_icon_label_->setPixmap(t_pm);
      clp_icon_label_->show();
      t_next_y += t_layout.icon.height() + iconTextGap(t_layout);
    } else {
      clp_icon_label_->hide();
    }
  } else {
    clp_icon_label_->hide();
  }

  // 文字：y/宽/高查表，x 水平居中（0 宽 = 铺满）
  const int t_text_w = t_layout.textW > 0 ? t_layout.textW : width();
  clp_text_label_->setGeometry((width() - t_text_w) / 2, t_next_y, t_text_w,
                               t_layout.textH);
  clp_text_label_->setText(cl_text_);
  t_next_y += t_layout.textH;

  // 刷新按钮：仅错误态显示；x 水平居中
  const bool t_show_button = (t_layout.buttonY > 0 && cl_retry_enabled_);
  if (t_show_button) {
    t_next_y += textButtonGap(t_layout, cl_retry_enabled_);
    clp_retry_button_->setGeometry((width() - 104) / 2, t_next_y, 104, 30);
  }
  clp_retry_button_->setVisible(t_show_button);
}

void CommunityStateOverlay::hideState() {
  hide();
  cl_retry_enabled_ = false;
  cl_state_active_ = false;
  clp_retry_button_->hide();
}

void CommunityStateOverlay::LanguageSet() {
  // "刷新"按钮文本为构造时一次性设置，语言切换后重设
  if (clp_retry_button_) {
    clp_retry_button_->setText(tr("刷新"));
  }
}

void CommunityStateOverlay::mousePressEvent(QMouseEvent* event) {
  if (event->button() == Qt::LeftButton && cl_retry_enabled_) {
    emit retryClicked();
    event->accept();
    return;
  }
  QWidget::mousePressEvent(event);
}

void CommunityStateOverlay::mouseMoveEvent(QMouseEvent* event) {
  // 保持手型光标（子标签不拦截时生效）
  setCursor(cl_retry_enabled_ ? Qt::PointingHandCursor : Qt::ArrowCursor);
  QWidget::mouseMoveEvent(event);
}

void CommunityStateOverlay::showEvent(QShowEvent* event) {
  QWidget::showEvent(event);
  refreshLayout();
}

void CommunityStateOverlay::resizeEvent(QResizeEvent* event) {
  QWidget::resizeEvent(event);
  refreshLayout();
}
