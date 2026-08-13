#ifndef UI_COMMUNITY_COMMUNITY_STATE_OVERLAY_H
#define UI_COMMUNITY_COMMUNITY_STATE_OVERLAY_H

#include <QWidget>

class QLabel;
class QResizeEvent;
class QShowEvent;

/// \brief 社区列表空态/错误态覆盖层 — 图标 + 居中文字提示，可带"点击重试"
///
/// 使用方式（CommunityPanel 持有，5 个列表视图共用）：
///   panel->stateOverlay()->showState(tr("暂无数据"));
///   panel->stateOverlay()->showState(tr("加载失败，点击重试"), true);
///   panel->stateOverlay()->hideState();
///
/// 布局：手动绝对定位（2026-08-07 起，原 QVBoxLayout 移除）——每类图标的
/// y/宽/高查表（值来自 tools/ui_mockups/01-03 用户 Designer 调整，画布 y−56 页头），
/// x 一律按当前面板宽度水平居中（容器宽不同也能居中）。
class CommunityStateOverlay : public QWidget {
  Q_OBJECT

public:
  explicit CommunityStateOverlay(QWidget* parent = nullptr);

  /// \brief 显示状态文案；retryEnabled = true 时整层可点击触发 retryClicked；
  /// iconPath 非空时按图标类型查表布局（图标+文字+可选按钮），空时纯文字态
  void showState(const QString& text, bool retryEnabled = false,
                 const QString& iconPath = QString());
  /// \brief 隐藏覆盖层
  void hideState();

  /// \brief 语言切换：刷新"刷新"按钮文本（提示文字由调用方动态传入，触发时已按当前语言翻译）
  void LanguageSet();
  /// \brief 父页面 show/resize 后强制按当前状态重算几何
  void refreshLayout();

signals:
  /// \brief 点击重试（仅 retryEnabled 时发射）
  void retryClicked();

protected:
  void mousePressEvent(QMouseEvent* event) override;
  void mouseMoveEvent(QMouseEvent* event) override;
  void showEvent(QShowEvent* event) override;
  void resizeEvent(QResizeEvent* event) override;  ///< 面板尺寸变化时重新居中（x 依赖宽度）

private:
  /// \brief 按当前 cl_icon_path_ 查表布置图标/文字/按钮（x 水平居中）
  void cl_apply_layout_();

  QLabel* clp_icon_label_ = nullptr;  ///< 图标（空态图，按查表尺寸显示）
  QLabel* clp_text_label_ = nullptr;  ///< 提示文字
  class QPushButton* clp_retry_button_ = nullptr;  ///< 刷新按钮（错误态显示）
  bool cl_retry_enabled_ = false;     ///< 是否可点击重试
  bool cl_state_active_ = false;      ///< 当前是否持有空态/错误态内容
  QString cl_icon_path_;              ///< 当前图标路径（空 = 纯文字态）
  QString cl_text_;                   ///< 当前文案（resize 重布置时沿用）
};

#endif  // UI_COMMUNITY_COMMUNITY_STATE_OVERLAY_H
