#ifndef COMMUNITY_FLOW_VIEW_H
#define COMMUNITY_FLOW_VIEW_H

#include <QAbstractItemView>
#include <QHash>

/// \brief 布局缓存条目 / Cached item layout in content coordinates
struct FlowItemLayout {
  QRect rect;
  int count_ = 0;
};

/// \brief 社区流式视图 — QAbstractItemView 子类，多垂直列并列布局 / Custom flow layout view
class CommunityFlowView : public QAbstractItemView {
  Q_OBJECT

public:
  /// \brief 构造流式视图 / Construct flow view
  explicit CommunityFlowView(QWidget* parent = nullptr);
  /// \brief 默认析构 / Default destructor
  ~CommunityFlowView() override = default;

  // ── QAbstractItemView 必须实现 / Required overrides ──
  /// \brief 返回 index 对应的视觉矩形 / Return visual rect for index
  QRect visualRect(const QModelIndex& index) const override;
  /// \brief 滚动到指定 index / Scroll to index
  void scrollTo(const QModelIndex& index, ScrollHint hint = EnsureVisible) override;
  /// \brief 返回指定点下的 model index / Return model index at point
  QModelIndex indexAt(const QPoint& point) const override;

  /// \brief 键盘移动光标 / Move cursor via keyboard
  QModelIndex moveCursor(CursorAction cursorAction, Qt::KeyboardModifiers modifiers) override;

  /// \brief 返回水平滚动偏移 / Return horizontal scroll offset
  int horizontalOffset() const override;
  /// \brief 返回垂直滚动偏移 / Return vertical scroll offset
  int verticalOffset() const override;
  /// \brief 指定 index 是否隐藏 / Whether index is hidden
  bool isIndexHidden(const QModelIndex& index) const override;

  /// \brief 按矩形设置选区 / Set selection by rect
  void setSelection(const QRect& rect, QItemSelectionModel::SelectionFlags command) override;
  /// \brief 返回 selection 对应的可视区域 / Visual region for selection
  QRegion visualRegionForSelection(const QItemSelection& selection) const override;

  /// \brief 设置数据模型并触发布局重算 / Set model and recalculate layout
  void setModel(QAbstractItemModel* model) override;

protected:
  /// \brief 更新子项几何（模型变化时调用） / Update item geometries on model change
  void updateGeometries() override;
  /// \brief modelReset 时 Qt 内置回调 / Qt built-in callback on model reset
  void reset() override;
  /// \brief 绘制所有可见卡片 / Paint all visible cards
  void paintEvent(QPaintEvent* event) override;
  /// \brief 窗口大小变化时重算布局 / Recalculate layout on resize
  void resizeEvent(QResizeEvent* event) override;
  /// \brief 显示时重算布局（隐藏面板首次 show 尺寸从 0 就位）
  void showEvent(QShowEvent* event) override;
  /// \brief 滚动内容 / Scroll content by delta
  void scrollContentsBy(int dx, int dy) override;
  /// \brief 鼠标按下事件 / Mouse press event
  void mousePressEvent(QMouseEvent* event) override;
  /// \brief 鼠标移动事件（更新 hover 状态） / Mouse move event (update hover)
  void mouseMoveEvent(QMouseEvent* event) override;
  /// \brief 鼠标释放事件 / Mouse release event
  void mouseReleaseEvent(QMouseEvent* event) override;
  /// \brief 滚轮事件 / Wheel event
  void wheelEvent(QWheelEvent* event) override;
  /// \brief 鼠标离开事件（清除 hover） / Leave event (clear hover)
  void leaveEvent(QEvent* event) override;

signals:
  /// \brief 滚动到底部 / Scroll reached bottom
  void loadMoreRequested();

public:
  /// \brief 计算全部卡片的流式布局 + 重绘（供 Panel 主动刷新） / Full relayout + repaint
  void calculateLayout();
  /// \brief 内容宽度上限（0=不限制；弹窗内视口加宽但卡片宽度不扩张） / Max content width, 0 = unlimited
  void setMaxContentWidth(int w);

private:
  /// \brief 根据内容高度更新滚动条范围 / Update scroll bar range
  void updateScrollBar();
  /// \brief 恢复 scheduleDelayedItemsLayout 前保存的滚动位置 / Restore saved scroll
  void restoreSavedScroll();
  /// \brief 根据视口宽度计算列数 / Calculate column count for viewport width
  int columnCountForWidth(int vpWidth) const;

  /// \brief 将内容坐标矩形映射到视口坐标 / Map content rect to viewport coords
  QRect viewportRect(const QRect& contentRect) const;
  /// \brief 查找 index 对应的布局缓存 / Find layout cache for index
  const FlowItemLayout* layoutForIndex(const QModelIndex& index) const;

  // ── 常量 / Constants ──
  static constexpr int kItemMinWidth = 342;   ///< 卡片最小宽度 / min card width
  static constexpr int kItemMinHeight = 325;  ///< 卡片最小高度 / min card height

  // ── 布局缓存（以 row 为键——internalId 默认为 0，不可靠） ──
  // ── Layout cache keyed by row (internalId defaults to 0, unreliable) ──
  QHash<int, FlowItemLayout> cl_layout_cache_;  ///< 布局缓存 / layout cache
  int cl_columns_ = 1;                          ///< 当前列数 / current column count
  int cl_content_width_ = 0;                    ///< 内容总宽度 / total content width
  int cl_content_height_ = 0;                   ///< 内容总高度 / total content height
  int cl_max_content_width_ = 0;                ///< 内容宽度上限（0=不限制）/ max content width, 0 = unlimited
  int cl_cursor_row_ = -1;                      ///< 键盘光标所在行 / keyboard cursor row
  QPersistentModelIndex cl_hovered_index_;      ///< 当前悬停的 index / currently hovered index
  int cl_saved_scroll_ = 0;                     ///< 布局前滚动位置（doItemsLayout 会重置）
};

#endif  // COMMUNITY_FLOW_VIEW_H
