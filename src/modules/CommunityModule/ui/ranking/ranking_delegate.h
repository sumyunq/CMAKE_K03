#ifndef UI_RANKING_RANKING_DELEGATE_H
#define UI_RANKING_RANKING_DELEGATE_H

#include <QModelIndex>
#include <QStyledItemDelegate>

/// \brief 排行榜行委托 — 自绘 360×55 行（排名/头像/方案名/用户名/热度/点赞按钮）
class RankingDelegate : public QStyledItemDelegate {
  Q_OBJECT

 public:
  explicit RankingDelegate(QObject *parent = nullptr);

  QSize sizeHint(const QStyleOptionViewItem &option,
                 const QModelIndex &index) const override;
  void paint(QPainter *painter, const QStyleOptionViewItem &option,
             const QModelIndex &index) const override;
  bool editorEvent(QEvent *event, QAbstractItemModel *model,
                   const QStyleOptionViewItem &option,
                   const QModelIndex &index) override;

  /// \brief 设置当前榜类型：0=点赞榜（Like 图标系列） 1=下载榜（DL 系列）
  void setRankingType(int type);

 signals:
  /// \brief 行内按钮点击（点赞榜=点赞，下载榜=下载） / Row button clicked
  void buttonClicked(const QModelIndex &index);
  /// \brief 方案名点击 / Plan name clicked
  void planNameClicked(const QModelIndex &index, const QPoint &viewportPos);  ///< 点击方案名（viewport 坐标）

 private:
  static QFont makeFont(int pointSize, int weight);
  static QColor rankColor(int rank);
  static QString formatHeat(int count);

  QRect rankRect(const QRect &row) const;
  QRect avatarRect(const QRect &row) const;
  QRect planRect(const QRect &row) const;
  QRect usernameRect(const QRect &row) const;
  QRect heatRect(const QRect &row) const;
  QRect likeRect(const QRect &row) const;
  /// \brief 视口坐标下的鼠标位置（子区域 hover 判定，无 view 时返回无效点）
  QPoint cursorInItem(const QStyleOptionViewItem &option) const;
  /// \brief 点赞/下载按钮图标路径（按榜类型 × 排名段 × 选中态 × hover）
  QString likeIconPath(int rank, bool liked, bool hovered) const;

 private:
  static constexpr int kRowWidth = 360;   ///< 行宽
  static constexpr int kRowHeight = 55;   ///< 行高

  int cl_ranking_type_ = 0;  ///< 0=点赞榜 1=下载榜
};

#endif  // UI_RANKING_RANKING_DELEGATE_H
