#ifndef UI_RANKING_RANKING_LIST_VIEW_H
#define UI_RANKING_RANKING_LIST_VIEW_H

#include <QListView>

/// \brief 排行榜列表视图 — 暴露 QAbstractScrollArea::setViewportMargins（Qt 5.15 为 protected）
class RankingListView : public QListView
{
    Q_OBJECT

 public:
    explicit RankingListView(QWidget *parent = nullptr)
        : QListView(parent)
    {
    }

    using QAbstractScrollArea::setViewportMargins;
};

#endif  // UI_RANKING_RANKING_LIST_VIEW_H
