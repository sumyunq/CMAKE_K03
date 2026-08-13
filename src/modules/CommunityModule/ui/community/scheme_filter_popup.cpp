#include "modules/CommunityModule/ui/community/scheme_filter_popup.h"

#include <QCoreApplication>
#include <QGuiApplication>
#include <QHBoxLayout>
#include <QHideEvent>
#include <QLabel>
#include <QMouseEvent>
#include <QPen>
#include <QPolygonF>
#include <QPushButton>
#include <QScreen>
#include <QShowEvent>
#include <QVBoxLayout>

#include <algorithm>

namespace {
/// 递归清空布局：删除所有直接控件与嵌套子布局（含子布局内的控件）
/// 必须递归 — 选项行是子布局，只删直接项会残留孤儿控件
void clearLayout(QLayout* layout) {
  while (QLayoutItem* item = layout->takeAt(0)) {
    if (QWidget* widget = item->widget()) {
      widget->deleteLater();
    } else if (QLayout* subLayout = item->layout()) {
      clearLayout(subLayout);
    }
    delete item;
  }
}
}  // namespace

namespace {
/// 默认过滤分组 — 调整选项时改这里；运行时也可通过 setFilterGroups() 覆盖
/// sort 值对应后端 sort 参数（new/hot/download/like/collect/score）；
/// scene 值对应 user_tag；model 值对应 device_name（空串 = 全部，不传参）
QList<FilterGroup> defaultGroups() {
  const auto tr = [](const char* s) {
    return QCoreApplication::translate("SchemeFilterPopup", s);
  };
  return {
      {QStringLiteral("sort"), tr("排序依据"),
       {{tr("最新发布"), QStringLiteral("new")},
        {tr("最多点赞"), QStringLiteral("like")},
        {tr("最多下载"), QStringLiteral("download")},
        {tr("最多分享"), QStringLiteral("share")}}},
      {QStringLiteral("scene"), tr("场景分类"),
       {{tr("所有场景"), QString()},
        {tr("游戏"), QStringLiteral("游戏")},
        {tr("电影"), QStringLiteral("电影")},
        {tr("音乐"), QStringLiteral("音乐")},
        {tr("三角洲行动"), QStringLiteral("三角洲行动")},
        {tr("PUBG"), QStringLiteral("PUBG")},
        {tr("CSGO"), QStringLiteral("CSGO")},
        {tr("无畏契约"), QStringLiteral("无畏契约")},
        {tr("暗区突围"), QStringLiteral("暗区突围")},
        {tr("APEX"), QStringLiteral("APEX")},
        {tr("穿越火线"), QStringLiteral("穿越火线")}}},
      {QStringLiteral("model"), tr("机型分类"),
       {{tr("所有机型"), QString()},
        {tr("T10有线"), QStringLiteral("T10有线")},
        {tr("T10无线"), QStringLiteral("T10无线")},
        {tr("K03S超竞版"), QStringLiteral("K03S超竞版")},
        {tr("K03有线版二代"), QStringLiteral("K03有线版二代")},
        {tr("K06S"), QStringLiteral("K06S")},
        {tr("T7"), QStringLiteral("T7")},
        {tr("T7 GT"), QStringLiteral("T7 GT")},
        {tr("S21无线智充版"), QStringLiteral("S21无线智充版")}}},
  };
}
}  // namespace

SchemeFilterPopup::SchemeFilterPopup(QWidget* parent) : QWidget(parent) {
  initUi();
  setFilterGroups(defaultGroups());
  qApp->installEventFilter(this);  // 点击外部关闭
}

QString SchemeFilterPopup::value(const QString& key) const {
  return cl_values_.value(key);
}

bool SchemeFilterPopup::hasActiveFilters() const {
  for (auto it = cl_values_.constBegin(); it != cl_values_.constEnd(); ++it) {
    if (!it.value().isEmpty()) return true;
  }
  return false;
}

void SchemeFilterPopup::setValue(const QString& key, const QString& value) {
  applyValue(key, value);
}

void SchemeFilterPopup::reset() {
  // 恢复默认状态（每组第一个选项，即最新 / 所有场景 / 所有机型）；
  // 仅当当前状态与默认不同才通知刷新
  bool changed = false;
  for (const auto& group : cl_groups_) {
    const QString def =
        group.options.isEmpty() ? QString() : group.options.first().value;
    if (cl_values_.value(group.key) != def) {
      changed = true;
      applyValue(group.key, def);
    }
  }
  if (changed) emit filtersReset();
}

void SchemeFilterPopup::LanguageSet() {
  // 语言切换：用当前翻译重建分组标签（筛选值保持字面不变，不影响后端查询）；
  // 重建后恢复用户已选值，全程不发筛选信号
  const QHash<QString, QString> prev = cl_values_;
  setFilterGroups(defaultGroups());
  if (clp_reset_text_) {
    clp_reset_text_->setText(QCoreApplication::translate("SchemeFilterPopup", "重置筛选"));
  }
  for (auto it = prev.constBegin(); it != prev.constEnd(); ++it) {
    applyValue(it.key(), it.value());
  }
}

void SchemeFilterPopup::popupBelow(const QWidget* anchor) {
  if (!anchor) return;
  // 弹窗水平中心与锚点按钮中心对齐，位于锚点下方 8px
  const QPoint anchorCenter =
      anchor->mapToGlobal(QPoint(anchor->width() / 2, anchor->height() + 8));
  int x = anchorCenter.x() - width() / 2;
  // 限制在屏幕内（左右均不越界）
  if (QScreen* screen = QGuiApplication::screenAt(anchorCenter)) {
    const QRect avail = screen->availableGeometry();
    x = (std::min)((std::max)(x, avail.left()), avail.right() - width() + 1);
  }
  move(x, anchorCenter.y());
  show();
  raise();
}

bool SchemeFilterPopup::eventFilter(QObject* obj, QEvent* event) {
  // 非 Popup 窗口没有自动关闭：点击弹窗外部任意位置 → 关闭
  if (event->type() == QEvent::MouseButtonPress && isVisible()) {
    auto* me = static_cast<QMouseEvent*>(event);
    if (!frameGeometry().contains(me->globalPos())) {
      hide();
    }
  }
  return QWidget::eventFilter(obj, event);
}

void SchemeFilterPopup::showEvent(QShowEvent* event) {
  QWidget::showEvent(event);
  cl_visible_ = true;
}

void SchemeFilterPopup::hideEvent(QHideEvent* event) {
  QWidget::hideEvent(event);
  if (cl_visible_) {
    cl_visible_ = false;
    emit closed();
  }
}

void SchemeFilterPopup::mousePressEvent(QMouseEvent* event) {
  if (event->button() == Qt::LeftButton) {
    cl_drag_offset_ = event->globalPos() - frameGeometry().topLeft();
    event->accept();
    return;
  }
  QWidget::mousePressEvent(event);
}

void SchemeFilterPopup::mouseMoveEvent(QMouseEvent* event) {
  if ((event->buttons() & Qt::LeftButton) && !cl_drag_offset_.isNull()) {
    move(event->globalPos() - cl_drag_offset_);
    event->accept();
    return;
  }
  QWidget::mouseMoveEvent(event);
}

void SchemeFilterPopup::mouseReleaseEvent(QMouseEvent* event) {
  if (event->button() == Qt::LeftButton) {
    cl_drag_offset_ = QPoint();
    event->accept();
    return;
  }
  QWidget::mouseReleaseEvent(event);
}

void SchemeFilterPopup::setFilterGroups(const QList<FilterGroup>& groups) {
  cl_groups_ = groups;
  cl_values_.clear();
  rebuildGroups();
  // 默认选中每组第一个选项（静默，不发信号）：最新 / 所有场景 / 所有机型
  for (const auto& group : cl_groups_) {
    if (!group.options.isEmpty()) {
      applyValue(group.key, group.options.first().value);
    }
  }
}

void SchemeFilterPopup::initUi() {
  // Dialog + 半透明 + 子容器圆角（Qt::Popup 上透明不可靠，四角露白块）
  // 点击外部关闭由 eventFilter 手动实现
  setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint);
  setAttribute(Qt::WA_TranslucentBackground);
  setFixedSize(1018, 315);  // 调整 UI 时：与分组数量/选项数量配套修改

  auto* shell = new QWidget(this);
  shell->setObjectName(QStringLiteral("schemeFilterPopup"));
  shell->setStyleSheet(R"(
    QWidget#schemeFilterPopup { background:#0D0F14; border-radius:16px; }
    QLabel { color:#7e8796; background:transparent; font-size:12px; }
    QPushButton { border:none; }
  )");
  auto* rootLayout = new QVBoxLayout(this);
  rootLayout->setContentsMargins(0, 0, 0, 0);
  rootLayout->addWidget(shell);

  auto* root = new QVBoxLayout(shell);
  root->setContentsMargins(30, 14, 22, 18);
  root->setSpacing(0);

  // 顶栏：首组标签（左）+ 重置按钮（右对齐，右侧让位给手动定位的关闭按钮）
  auto* topRow = new QHBoxLayout();
  topRow->setContentsMargins(0, 0, 44, 0);
  topRow->setSpacing(0);
  clp_title_label_ = new QLabel(this);
  clp_title_label_->setFixedSize(56, 20);
  topRow->addWidget(clp_title_label_, 0, Qt::AlignVCenter);
  topRow->addStretch();
  auto* resetButton = new QPushButton(this);
  resetButton->setFixedSize(110, 32);
  resetButton->setCursor(Qt::PointingHandCursor);
  resetButton->setStyleSheet(QStringLiteral(
      "QPushButton{background:transparent;border:none;border-radius:16px;}"
      "QPushButton:hover{background:rgba(223, 243, 255, 0.2);}"));
  auto* resetLayout = new QHBoxLayout(resetButton);
  resetLayout->setContentsMargins(15, 0, 15, 0);  // 图标 (15,8,16,17) + 间距 8 + 文字 (39,6,56,20)
  resetLayout->setSpacing(8);
  auto* resetIcon = new QLabel(resetButton);
  resetIcon->setAttribute(Qt::WA_TransparentForMouseEvents);
  resetIcon->setFixedSize(16, 17);
  resetIcon->setPixmap(QPixmap(QStringLiteral(":/Skin/Images/Community/FilterReset.png")));
  resetLayout->addWidget(resetIcon);
  clp_reset_text_ = new QLabel(QCoreApplication::translate("SchemeFilterPopup", "重置筛选"),
                               resetButton);
  clp_reset_text_->setAttribute(Qt::WA_TransparentForMouseEvents);
  clp_reset_text_->setStyleSheet(
      "QLabel{color:#009FEF;background:transparent;font-size:14px;font-weight:500;}");
  resetLayout->addWidget(clp_reset_text_);
  topRow->addWidget(resetButton);
  root->addLayout(topRow);

  // 选项区：setFilterGroups() / rebuildGroups() 动态填充
  clp_options_layout_ = new QVBoxLayout();
  clp_options_layout_->setContentsMargins(0, 0, 0, 0);
  clp_options_layout_->setSpacing(0);
  root->addLayout(clp_options_layout_);
  root->addStretch();

  // 关闭按钮：手动定位右上角（距上 15 / 距右 15）
  auto* closeButton = new QPushButton(this);
  closeButton->setCursor(Qt::PointingHandCursor);
  closeButton->setFixedSize(31, 31);
  closeButton->setStyleSheet(R"(
      QPushButton
      {
        border-radius:0px;
        border-image: url(:/Skin/Images/Popup/close-no.png);
        background:transparent;
      }
      QPushButton:hover
      {
        border-image: url(:/Skin/Images/Popup/close-ho.png);
      }
  )");
  closeButton->move(1018 - 15 - 31, 15);

  connect(resetButton, &QPushButton::clicked, this, &SchemeFilterPopup::reset);
  connect(closeButton, &QPushButton::clicked, this, &QWidget::hide);
}

void SchemeFilterPopup::rebuildGroups() {
  // 清空旧选项区（递归删除子布局中的按钮/标签，避免二次重建时控件残留）
  clearLayout(clp_options_layout_);
  cl_group_buttons_.clear();

  bool first = true;
  for (const auto& group : cl_groups_) {
    if (first) {
      // 首组标签位于顶栏
      clp_title_label_->setText(group.title);
    } else {
      // 其余组：标签行（顶部间距 28，与上方选项行隔开）
      auto* labelRow = new QHBoxLayout();
      labelRow->setContentsMargins(0, 28, 0, 0);
      auto* label = new QLabel(group.title, this);
      label->setFixedSize(56, 20);
      labelRow->addWidget(label, 0, Qt::AlignVCenter);
      labelRow->addStretch();
      clp_options_layout_->addLayout(labelRow);
    }

    // 选项行（顶部间距 14；按钮自动宽度，文字居中，内容边距 (10,6,10,6)）
    auto* optionRow = new QHBoxLayout();
    optionRow->setContentsMargins(0, 14, 0, 0);
    optionRow->setSpacing(8);
    QList<QPushButton*> buttons;
    const QString key = group.key;
    for (const auto& option : group.options) {
      auto* button = makeOptionButton(option);
      buttons.append(button);
      optionRow->addWidget(button, 0, Qt::AlignVCenter);
      const QString value = option.value;
      connect(button, &QPushButton::clicked, this, [this, key, value] {
        selectValue(key, value);
      });
    }
    optionRow->addStretch();
    clp_options_layout_->addLayout(optionRow);
    cl_group_buttons_.insert(key, buttons);
    first = false;
  }
}

QPushButton* SchemeFilterPopup::makeOptionButton(const FilterOption& option) {
  auto* button = new QPushButton(option.text, this);
  button->setCheckable(true);
  button->setCursor(Qt::PointingHandCursor);
  button->setProperty("filterValue", option.value);  // selectOption 按此属性匹配
  button->setMinimumHeight(32);
  // 所有选项按钮统一圆角 4px；内容边距 (10, 6, 10, 6) = padding 6px 10px
  button->setStyleSheet(QStringLiteral(
      "QPushButton{background:transparent;color:#a4adba;border:none;border-radius:4px;"
      "padding:6px 10px;font-size:12px;}"
      "QPushButton:hover{color:#ffffff;background:#18202b;}"
      "QPushButton:checked{background:#009FEF;color:#ffffff;}"));
  return button;
}

bool SchemeFilterPopup::applyValue(const QString& key, const QString& value) {
  // 守卫仅在 key 已存在时比较：缺失 key 的 value() 返回空串，
  // 会把"默认选中空值选项"误判为"值未变化"而跳过选中
  if (cl_values_.contains(key) && cl_values_.value(key) == value) return false;
  cl_values_.insert(key, value);
  QList<QPushButton*> buttons = cl_group_buttons_.value(key);
  selectOption(buttons, value);
  return true;
}

void SchemeFilterPopup::selectValue(const QString& key, const QString& value) {
  if (applyValue(key, value)) {
    emit filterChanged(key, value);
    return;
  }
  // 重复点击已选中的选项：checkable 按钮已被 Qt 自动翻转选中态，恢复正确显示
  QList<QPushButton*> buttons = cl_group_buttons_.value(key);
  selectOption(buttons, value);
}

void SchemeFilterPopup::selectOption(QList<QPushButton*>& buttons, const QString& value) {
  for (auto* button : qAsConst(buttons)) {
    if (!button) continue;
    button->setChecked(button->property("filterValue").toString() == value);
  }
}
