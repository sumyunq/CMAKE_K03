#ifndef UI_COMMUNITY_SCHEME_FILTER_POPUP_H
#define UI_COMMUNITY_SCHEME_FILTER_POPUP_H

#include <QHash>
#include <QList>
#include <QPoint>
#include <QString>
#include <QWidget>

class QLabel;
class QPushButton;
class QVBoxLayout;

/// 单个过滤选项
struct FilterOption {
  QString text;  ///< 按钮文字
  QString value; ///< 筛选值；空串 = "全部"（默认选中的选项）
};

/// 过滤分组（一行选项，标签在选项上方）
struct FilterGroup {
  QString key;                 ///< 分组标识，如 "sort" / "scene" / "model"
  QString title;               ///< 行标签文字
  QList<FilterOption> options; ///< 选项列表（可动态增删）
};

/// \brief 方案筛选弹窗 — 分组式过滤条件，布局按 FilterGroup 数据动态生成
///
/// 设计约定（迁移自 WidgetCMake，详见参考文档）：
/// - 筛选状态唯一来源在本组件内（QHash key → value），页面不镜像
/// - setFilterGroups() 可随时重建选项区（运行时增删分组/选项），重建后值清空
/// - 选项变更通过 filterChanged(key, value) 通知，仅在实际值变化时发射
/// - 按钮宽度按文字自适应，内容边距 (10, 6, 10, 6)，统一圆角 4px
/// - 用 Qt::Dialog（非 Qt::Popup）+ 全局事件过滤器实现点击外部关闭
///   （WA_TranslucentBackground 在 Qt::Popup 上会被静默忽略，四角露白块）
class SchemeFilterPopup : public QWidget {
  Q_OBJECT

public:
  explicit SchemeFilterPopup(QWidget* parent = nullptr);

  /// 设置过滤分组并重建选项区（构造时已内置 sort/scene/model 默认分组）
  void setFilterGroups(const QList<FilterGroup>& groups);

  // ── 当前筛选状态（空串 = 全部） ──
  QString value(const QString& key) const;
  bool hasActiveFilters() const;

  // ── 外部恢复状态（静默，不触发信号；需要刷新请自行处理） ──
  void setValue(const QString& key, const QString& value);
  void reset();                ///< 恢复默认状态（每组第一个选项）；仅当状态与默认不同才发 filtersReset()

  /// 在锚点控件下方弹出（弹窗水平中心与锚点中心对齐，位于锚点下方 8px，屏幕内不越界）
  void popupBelow(const QWidget* anchor);

  /// 语言切换：按当前翻译重建分组标签，保持已选筛选值不变（不发筛选信号）
  void LanguageSet();

signals:
  void filterChanged(const QString& key, const QString& value);
  void filtersReset();
  void closed();               ///< 弹窗关闭（点击外部 / 关闭按钮）

protected:
  bool eventFilter(QObject* obj, QEvent* event) override;
  void showEvent(QShowEvent* event) override;
  void hideEvent(QHideEvent* event) override;
  // 按住面板空白处可拖动弹窗（按钮区域由子控件处理，不参与拖动）
  void mousePressEvent(QMouseEvent* event) override;
  void mouseMoveEvent(QMouseEvent* event) override;
  void mouseReleaseEvent(QMouseEvent* event) override;

private:
  void initUi();
  /// 按 cl_groups_ 重建选项区（首组标签在顶栏，其余组标签行 + 选项行）
  void rebuildGroups();
  QPushButton* makeOptionButton(const FilterOption& option);
  /// 更新值 + 选中态，返回是否实际变化
  bool applyValue(const QString& key, const QString& value);
  /// 按钮点击入口：仅在实际变化时发 filterChanged
  void selectValue(const QString& key, const QString& value);
  void selectOption(QList<QPushButton*>& buttons, const QString& value);

  QList<FilterGroup> cl_groups_;
  QVBoxLayout* clp_options_layout_ = nullptr;
  QLabel* clp_title_label_ = nullptr;  ///< 首组标签（位于顶栏）
  QLabel* clp_reset_text_ = nullptr;   ///< 顶栏"重置筛选"文字（语言切换时需刷新）
  QHash<QString, QList<QPushButton*>> cl_group_buttons_;
  QHash<QString, QString> cl_values_;
  QPoint cl_drag_offset_;  ///< 拖动时鼠标与窗口左上角的偏移
  bool cl_visible_ = false;  ///< 弹窗可见标记（hideEvent 区分外部关闭与构造期隐藏）
};

#endif  // UI_COMMUNITY_SCHEME_FILTER_POPUP_H
