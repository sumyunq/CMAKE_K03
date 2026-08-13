#include "modules/CommunityModule/ui/community/community_flow_view.h"

#include <QMouseEvent>
#include <QPaintEvent>
#include <QPainter>
#include <QResizeEvent>
#include <QScrollBar>
#include <QWheelEvent>

#include <algorithm>

#include "modules/CommunityModule/ui/community/community_delegate.h"
#include "modules/CommunityModule/ui/community/community_model.h"
#include "modules/CommunityModule/infrastructure/logger/logger.h"

CommunityFlowView::CommunityFlowView(QWidget* parent) : QAbstractItemView(parent) {
  setMouseTracking(true);
  setAutoFillBackground(false);
  setAttribute(Qt::WA_OpaquePaintEvent, false);
  // 关键：viewport 默认不透明（绘制 palette 背景 = 黑色），必须显式透明透出 K03 背景
  viewport()->setAutoFillBackground(false);
  viewport()->setAttribute(Qt::WA_OpaquePaintEvent, false);
  viewport()->setStyleSheet(QStringLiteral("background: transparent;"));
  setEditTriggers(QAbstractItemView::NoEditTriggers);  // 禁 base 的 editorEvent

  horizontalScrollBar()->setRange(0, 0);
  horizontalScrollBar()->hide();

  verticalScrollBar()->setSingleStep(kItemMinHeight / 2);

  // 滚动触底 → 自动加载下一页（一次性连接，不依赖 model 切换）
  connect(verticalScrollBar(), &QScrollBar::valueChanged, this, [this](int v) {
    const int m = verticalScrollBar()->maximum();
    if (v == m && m > 0) {
      LOG_DEBUG("[FlowView] loadMore trigger: v={} max={}", v, m);
      emit loadMoreRequested();
    }
  });

  // 视图背景透明（透出 K03 深色主题）
  setObjectName("myCustomView");
  setStyleSheet(R"(
        QAbstractItemView#myCustomView {
            background: transparent;
            border: none;
            outline: 0;
        }
    )");

  // 滚动条样式 — 直接对实例设置（QAbstractItemView 的后代选择器不生效）
  const QString kScrollBarStyle = R"(
        QScrollBar:vertical {
            background: transparent;
            width: 8px;
            margin: 0;
        }
        QScrollBar::handle:vertical {
            background: rgba(0, 0, 0, 0.2);
            border: 1px solid rgba(154, 154, 154, 0.2);
            border-radius: 3px;
            min-height: 30px;
        }
        QScrollBar::handle:vertical:hover {
            background: rgba(0, 0, 0, 0.2);
            border: 1px solid rgba(154, 154, 154, 0.2);
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
    )";
  verticalScrollBar()->setStyleSheet(kScrollBarStyle);

}

/// \brief 设置内容宽度上限并重算布局 / Set max content width and relayout
void CommunityFlowView::setMaxContentWidth(int w) {
  if (cl_max_content_width_ == w) return;
  cl_max_content_width_ = w;
  calculateLayout();
}

/// \brief 列数计算

int CommunityFlowView::columnCountForWidth(int vpWidth) const {
  if (vpWidth <= 0) return 1;
  return (std::max)(1, vpWidth / kItemMinWidth);
}

/// \brief 列式布局算法（核心）

void CommunityFlowView::calculateLayout() {
  // 未显示/尺寸未就绪时跳过 — 0 宽布局会污染 cache，首次点击重算导致内容跳动
  // （社区页非当前页加载数据、个人中心隐藏页 initView 均会触发此场景）
  if (viewport()->width() <= 0 || viewport()->height() <= 0) return;

  cl_layout_cache_.clear();

  auto* m = model();
  if (!m || m->rowCount() == 0) {
    cl_content_width_ = 0;
    cl_content_height_ = 0;
    cl_columns_ = 1;
    cl_saved_scroll_ = 0;  // 空模型无需恢复
    updateScrollBar();
    viewport()->update();
    return;
  }

  const int vpWidth = viewport()->width();
  // 内容宽度上限（弹窗：视口加宽但卡片宽度不扩张；0=不限制）
  const int t_content_w = (cl_max_content_width_ > 0 && vpWidth > cl_max_content_width_)
                              ? cl_max_content_width_
                              : vpWidth;
  cl_columns_ = columnCountForWidth(t_content_w);
  const int colWidth = t_content_w / cl_columns_;
  // 末列槽宽：让卡片右缘落在距滚动条 1px 处（UI 要求滑块与 item 间距 1px）
  // 卡片右白边与 CommunityDelegate::kCardMargin（=6）保持一致
  const int lastColWidth = t_content_w - 1 + 6 - (cl_columns_ - 1) * colWidth;

  // 每列的当前 Y 偏移（列顶部位置）
  QVector<int> columnY(cl_columns_, 0);

  QStyleOptionViewItem baseOption;
  // 传真实列宽 — delegate sizeHint 按可用宽度模拟标签流式布局计算展开高度
  baseOption.rect = QRect(0, 0, colWidth, 0);

  for (int row = 0; row < m->rowCount(); ++row) {
    const int col = row % cl_columns_;
    const auto idx = m->index(row, 0);

    const QSize itemHint = itemDelegate()->sizeHint(baseOption, idx);
    const int itemW = (col == cl_columns_ - 1) ? lastColWidth : colWidth;
    const int itemH = (std::max)(itemHint.height(), kItemMinHeight);

    // item 在列内居中
    const int offsetX = (colWidth - itemW) / 2;
    const int x = col * colWidth + offsetX;
    const int y = columnY[col];

    FlowItemLayout layout;
    layout.rect = QRect(x, y, itemW, itemH);
    cl_layout_cache_[row] = layout;

    // 仅更新当前列的 Y 偏移，其他列不受影响
    columnY[col] = y + itemH;
  }

  // Effective STL Item 34: std::max_element 替代手写循环
  cl_content_height_ = *std::max_element(columnY.cbegin(), columnY.cend());

  cl_content_width_ = t_content_w;

  // 底部弹簧：内容高度不足 viewport 时撑满
  const int vpHeight = viewport()->height();
  if (cl_content_height_ < vpHeight) cl_content_height_ = vpHeight;

  updateScrollBar();
  restoreSavedScroll();
  viewport()->update();  // 布局变化后必须重绘视口
}

void CommunityFlowView::restoreSavedScroll() {
  if (cl_saved_scroll_ <= 0) {
    cl_saved_scroll_ = 0;
    return;
  }
  // 仅当滚动条被 Qt 基类置 0（doItemsLayout 强制 setValue(0)）时恢复 —
  // 用户正常滚动到其他位置时不清除保存值，避免后续异步 layout 重复恢复
  if (verticalScrollBar()->value() != 0) {
    cl_saved_scroll_ = 0;
    return;
  }
  const int vMax = verticalScrollBar()->maximum();
  if (cl_saved_scroll_ <= vMax) {
    verticalScrollBar()->setValue(cl_saved_scroll_);
  } else {
    verticalScrollBar()->setValue(vMax);
  }
  cl_saved_scroll_ = 0;
}

void CommunityFlowView::updateScrollBar() {
  const int vpHeight = viewport()->height();
  const int vpWidth = viewport()->width();

  verticalScrollBar()->setPageStep(vpHeight);
  verticalScrollBar()->setRange(0, (std::max)(0, cl_content_height_ - vpHeight));
  // clamp: 内容缩小后 scrollbar 不越界
  if (verticalScrollBar()->value() > verticalScrollBar()->maximum())
    verticalScrollBar()->setValue(verticalScrollBar()->maximum());

  horizontalScrollBar()->setPageStep(vpWidth);
  horizontalScrollBar()->setRange(0, (std::max)(0, cl_content_width_ - vpWidth));
  if (horizontalScrollBar()->value() > horizontalScrollBar()->maximum())
    horizontalScrollBar()->setValue(horizontalScrollBar()->maximum());
}

/// \brief updateGeometries — Qt 标准入口，model 变更后自动调用

void CommunityFlowView::updateGeometries() {
  calculateLayout();
  QAbstractItemView::updateGeometries();
}

/// \brief modelReset 回调 — Qt 架构级，绕过用户层信号分发
void CommunityFlowView::reset() {
  // 数据重置保护：保存当前滚动（Qt 基类 doItemsLayout 会 setValue(0)）
  if (cl_saved_scroll_ == 0 && verticalScrollBar()->maximum() > 0)
    cl_saved_scroll_ = verticalScrollBar()->value();
  QAbstractItemView::reset();  // 内部 scheduleDelayedItemsLayout（pending=true）
  // 同步消化异步布局：立即执行 doItemsLayout（setValue(0) + updateGeometries），
  // 清除 pending — 杜绝"数据加载时排队、用户交互后才执行 → 滚动条归零"的竞态
  // （与已上传/社区一致：加载在用户交互前完成布局）
  doItemsLayout();
  // doItemsLayout 内部 updateGeometries → calculateLayout → restoreSavedScroll
}

/// \brief 坐标转换

QRect CommunityFlowView::viewportRect(const QRect& contentRect) const {
  return contentRect.translated(-horizontalScrollBar()->value(), -verticalScrollBar()->value());
}

QRect CommunityFlowView::visualRect(const QModelIndex& index) const {
  const auto* lo = layoutForIndex(index);
  if (!lo) return {};
  return viewportRect(lo->rect);
}

/// \brief indexAt

QModelIndex CommunityFlowView::indexAt(const QPoint& point) const {
  const QPoint contentPos(point.x() + horizontalScrollBar()->value(),
                          point.y() + verticalScrollBar()->value());

  for (auto it = cl_layout_cache_.constBegin(); it != cl_layout_cache_.constEnd(); ++it) {
    if (it->rect.contains(contentPos)) return model()->index(it.key(), 0);  // key == row
  }
  return {};
}

/// \brief scrollTo

void CommunityFlowView::scrollTo(const QModelIndex& index, ScrollHint hint) {
  const QRect vr = visualRect(index);
  if (vr.isEmpty()) return;

  const int vpH = viewport()->height();
  const int vpW = viewport()->width();
  int newV = verticalScrollBar()->value();
  int newH = horizontalScrollBar()->value();

  if (hint == EnsureVisible) {
    // 与视口有交集（含部分可见/完全可见）→ 不滚动
    // 防止点击边缘 item 时 Qt 基类 currentChanged→scrollTo 微调滚动导致内容跳动（"首次点击晃动"）
    if (vr.intersects(QRect(0, 0, vpW, vpH))) return;

    if (vr.top() < 0)
      newV += vr.top();
    else if (vr.bottom() > vpH)
      newV += vr.bottom() - vpH;
    if (vr.left() < 0)
      newH += vr.left();
    else if (vr.right() > vpW)
      newH += vr.right() - vpW;
  } else {
    newV = vr.center().y() - vpH / 2;
  }

  // EnsureVisible 跳转幅度 > viewport → 用户已滚远，拒绝自动回拉
  if (hint == EnsureVisible && qAbs(newV - verticalScrollBar()->value()) > vpH)
    return;
  verticalScrollBar()->setValue(newV);
  horizontalScrollBar()->setValue(newH);
}

/// \brief moveCursor

QModelIndex CommunityFlowView::moveCursor(CursorAction cursorAction,
                                          Qt::KeyboardModifiers /*modifiers*/) {
  auto* m = model();
  if (!m || m->rowCount() == 0) return {};

  int cur = std::clamp(cl_cursor_row_, 0, m->rowCount() - 1);
  if (cur < 0) cur = 0;

  switch (cursorAction) {
    case MoveUp:
      cur -= cl_columns_;
      break;
    case MoveDown:
      cur += cl_columns_;
      break;
    case MoveLeft:
      cur -= 1;
      break;
    case MoveRight:
      cur += 1;
      break;
    case MoveHome:
      cur = 0;
      break;
    case MoveEnd:
      cur = m->rowCount() - 1;
      break;
    case MovePageUp:
      cur -= cl_columns_ * (viewport()->height() / kItemMinHeight);
      break;
    case MovePageDown:
      cur += cl_columns_ * (viewport()->height() / kItemMinHeight);
      break;
    default:
      break;
  }

  cur = std::clamp(cur, 0, m->rowCount() - 1);
  cl_cursor_row_ = cur;

  auto newIdx = m->index(cur, 0);
  scrollTo(newIdx, EnsureVisible);
  selectionModel()->setCurrentIndex(newIdx, QItemSelectionModel::ClearAndSelect);
  return newIdx;
}

/// \brief 偏移

int CommunityFlowView::horizontalOffset() const { return horizontalScrollBar()->value(); }
int CommunityFlowView::verticalOffset() const { return verticalScrollBar()->value(); }
bool CommunityFlowView::isIndexHidden(const QModelIndex&) const { return false; }

/// \brief 选择

void CommunityFlowView::setSelection(const QRect& rect,
                                     QItemSelectionModel::SelectionFlags command) {
  if (rect.isNull() || !model()) return;

  const QRect contentSel =
      QRect(rect.left() + horizontalScrollBar()->value(), rect.top() + verticalScrollBar()->value(),
            rect.width(), rect.height())
          .normalized();

  QItemSelection sel;
  for (int r = 0; r < model()->rowCount(); ++r) {
    auto idx = model()->index(r, 0);
    const auto* lo = layoutForIndex(idx);
    if (lo && lo->rect.intersects(contentSel)) sel.select(idx, idx);
  }
  selectionModel()->select(sel, command);
}

QRegion CommunityFlowView::visualRegionForSelection(const QItemSelection& selection) const {
  QRegion region;
  for (const auto& range : selection)
    for (int r = range.top(); r <= range.bottom(); ++r) region += visualRect(model()->index(r, 0));
  return region;
}

/// \brief setModel

void CommunityFlowView::setModel(QAbstractItemModel* newModel) {
  if (auto* old = model()) {
    disconnect(old, nullptr, this, nullptr);
  }

  QAbstractItemView::setModel(newModel);

  if (newModel) {
    connect(newModel, &QAbstractItemModel::modelAboutToBeReset, this,
            [this]() { cl_layout_cache_.clear(); });
    // modelReset / rowsInserted / rowsRemoved 走 Qt 内置 reset() 虚函数，
    // 不再手动 connect — 避免信号分发时序问题
    // 直接同步重算 — 不走 scheduleDelayedItemsLayout：
    // Qt 异步 doItemsLayout 会强制 setValue(0)（滚动条归零），
    // 且数据加载时的异步 layout 会与用户滚动/点击竞态（已点赞列表首次点击归零的根因）
    auto delayedLayout = [this] {
      calculateLayout();
    };
    connect(newModel, &QAbstractItemModel::rowsInserted, this, delayedLayout);
    connect(newModel, &QAbstractItemModel::rowsRemoved, this, delayedLayout);
    connect(newModel, &QAbstractItemModel::dataChanged, this,
            [this, delayedLayout](const QModelIndex&, const QModelIndex&, const QVector<int>& roles) {
              // IsPinnedRole 变更影响卡片高度（置顶 +19px），需重算布局
              if (roles.contains(CommunityModel::ExpandedRole) || roles.contains(CommunityModel::CommentsRole) ||
                  roles.contains(CommunityModel::IsPinnedRole))
                delayedLayout();
              else
                viewport()->update();
            });
    connect(newModel, &QAbstractItemModel::layoutChanged, this, delayedLayout);
  }

  calculateLayout();
  viewport()->update();
}

/// \brief paintEvent

void CommunityFlowView::paintEvent(QPaintEvent* event) {
  QPainter painter(viewport());
  painter.setRenderHint(QPainter::Antialiasing);

  // 背景
  // 透出父级 Widget 背景 — 不填充 viewport

  auto* m = model();
  if (!m || m->rowCount() == 0) return;

  const QRect visibleContent(horizontalScrollBar()->value(), verticalScrollBar()->value(),
                             viewport()->width(), viewport()->height());

  QStyleOptionViewItem option;
  option.widget = viewport();

  for (int r = 0; r < m->rowCount(); ++r) {
    const auto idx = m->index(r, 0);
    const auto* lo = layoutForIndex(idx);
    if (!lo || !lo->rect.intersects(visibleContent)) continue;

    option.rect = viewportRect(lo->rect);
    option.state = QStyle::State_None;
    option.index = idx;

    if (selectionModel()->isSelected(idx)) option.state |= QStyle::State_Selected;
    if (idx == cl_hovered_index_) option.state |= QStyle::State_MouseOver;

    itemDelegate()->paint(&painter, option, idx);
  }
}

/// \brief 事件

void CommunityFlowView::resizeEvent(QResizeEvent* event) {
  QAbstractItemView::resizeEvent(event);
  // updateGeometries 不自动调，resize 时手动触发布局重算
  calculateLayout();
}

/// \brief 显示时重算布局 — 覆盖"隐藏时 initView/加载数据"场景
/// 隐藏面板首次 show 时尺寸从 0 变为实际值，必须重算（守卫跳过的不算数）。
/// 基类 showEvent 可能 scheduleDelayedItemsLayout（懒布局）→ 同步 doItemsLayout 消化，
/// 防止"首次点击时异步 doItemsLayout 才执行 → 滚动条置 0"（已点赞列表场景根因）
void CommunityFlowView::showEvent(QShowEvent* event) {
  QAbstractItemView::showEvent(event);
  doItemsLayout();
  calculateLayout();
}

void CommunityFlowView::mousePressEvent(QMouseEvent* event) {
  QAbstractItemView::mousePressEvent(event);
  // 选择移到 mouseReleaseEvent — press 时 setCurrentIndex 触发 selection
  // change → view 重绘 → release 时 option.rect 可能偏移，导致首 item 需双击
}

void CommunityFlowView::mouseMoveEvent(QMouseEvent* event) {
  const auto idx = indexAt(event->pos());

  if (idx != cl_hovered_index_) {
    // 旧区域 → 新区域各触发一次重绘
    auto repaintIndex = [this](const QModelIndex& i) {
      const auto vr = visualRect(i);
      if (!vr.isEmpty()) viewport()->update(vr);
    };
    repaintIndex(cl_hovered_index_);
    cl_hovered_index_ = idx;
    repaintIndex(idx);
  }

  // 转交给 delegate 做子区域 hover（坐标保持视口系，与 option.rect 一致）
  if (idx.isValid()) {
    QStyleOptionViewItem option;
    option.rect = visualRect(idx);
    option.widget = viewport();
    option.index = idx;

    QMouseEvent adjusted(QEvent::MouseMove, event->pos(), event->button(),
                         event->buttons(), event->modifiers());
    itemDelegate()->editorEvent(&adjusted, model(), option, idx);
    // 始终重绘 — 同 item 内子区域变化时也需刷新
    viewport()->update(option.rect);

    // 可点击区域显示手型光标（含评论标签）
    auto* d = qobject_cast<CommunityDelegate*>(itemDelegate());
    if (d) {
      const auto zone = d->hitTest(event->pos(), option);
      const bool onTag = (d->commentTagHitTest(event->pos(), idx.row()) >= 0);
      setCursor((zone != CommunityDelegate::ZoneNone || onTag)
                    ? Qt::PointingHandCursor
                    : Qt::ArrowCursor);
    }
  } else {
    setCursor(Qt::ArrowCursor);
  }

  QAbstractItemView::mouseMoveEvent(event);
}

void CommunityFlowView::mouseReleaseEvent(QMouseEvent* event) {
  const auto idx = indexAt(event->pos());
  if (idx.isValid()) {
    // NoEditTriggers 下基类不会调 delegate::editorEvent
    // （Qt 5.15: mouseReleaseEvent 仅 EditingState 才 sendDelegateEvent），
    // 手动转发 release — 坐标保持视口系，与 option.rect/hitTest 一致
    QStyleOptionViewItem option;
    option.rect = visualRect(idx);
    option.widget = viewport();
    option.index = idx;

    QMouseEvent adjusted(QEvent::MouseButtonRelease, event->pos(),
                         event->button(), event->buttons(), event->modifiers());
    itemDelegate()->editorEvent(&adjusted, model(), option, idx);

    cl_cursor_row_ = idx.row();
    selectionModel()->setCurrentIndex(idx, QItemSelectionModel::ClearAndSelect);
  }
  // 不调 QAbstractItemView::mouseReleaseEvent —
  // 其内部路径会对当前 item 重复触发 editorEvent（8c5128e 教训）
}

void CommunityFlowView::scrollContentsBy(int /*dx*/, int /*dy*/) { viewport()->update(); }

void CommunityFlowView::wheelEvent(QWheelEvent* event) {
  const int delta = event->angleDelta().y();
  verticalScrollBar()->setValue(verticalScrollBar()->value() - delta);
  event->accept();
}

void CommunityFlowView::leaveEvent(QEvent* event) {
  QAbstractItemView::leaveEvent(event);
  if (cl_hovered_index_.isValid()) {
    const auto vr = visualRect(cl_hovered_index_);
    cl_hovered_index_ = QModelIndex();
    if (!vr.isEmpty()) viewport()->update(vr);
  }
}

/// \brief 辅助

const FlowItemLayout* CommunityFlowView::layoutForIndex(const QModelIndex& index) const {
  if (!index.isValid()) return nullptr;
  const auto it = cl_layout_cache_.constFind(index.row());
  if (it == cl_layout_cache_.constEnd()) return nullptr;
  return &(*it);
}
