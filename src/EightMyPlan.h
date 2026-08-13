#ifndef EIGHTMYPLAN_H
#define EIGHTMYPLAN_H

#include <QWidget>
#include <QPushButton>
#include <QGridLayout>
#include <QButtonGroup>

namespace Ui {
class EightMyPlan;
}

class EightMyPlan : public QWidget
{
    Q_OBJECT

public:
    explicit EightMyPlan(QWidget *parent = nullptr);
    ~EightMyPlan();

    // 设置按钮列表（用于与UI设计中的按钮关联）
    QList<QPushButton*> buttonList;
    QButtonGroup *buttonGroup;
    void ShowEightFavorite(bool signalEn);
    void TruncateText(QString text,QPushButton *pbt,int PlanMode);
    void Rename(int idx);

    void updateAllButtonsLayout();
    //void updateAllButtonsLayout(QGridLayout *gLayout,QList<QPushButton*> btnList);
    void createPlaceholder(int index);
    void movePlaceholder(int fromIndex, int toIndex);
    void completeDrag();
    void updateAllFavIndices();

    // 同步用接口：根据索引列表更新按钮顺序
    void updateLayoutFromOrder(const QList<int>& newOrder);

    void PlanCheckedUpdate(int id);

public slots:
    void updateChecked(int id);
    void AllDisChecked();
    void AllBtnDisChecked();

signals:
    void layoutChanged(const QList<int>& newOrder);  // 发出当前按钮新顺序
    void btnCheckedChanged(int id);
    void btnAllDisChecked();
    void PlanSave_F();
    void FavToEq();//点击收藏方案跳转到EQ


private:
    Ui::EightMyPlan *ui;


    bool m_potentialDrag = false;   // 替代原来的 m_dragging 作为候选状态
    QPoint m_pressGlobalPos;        // 按下时的全局坐标



    QGridLayout *gridLayout;
    bool m_dragging = false; // 标记是否正在拖动
    int m_dragIndex = -1;    // 当前拖动的按钮索引
    QPoint m_dragStartPos;   // 拖动起始位置

    QPoint m_dragOffset;          // 鼠标在按钮内部的偏移量
    QPushButton* m_draggedButton = nullptr; // 当前被拖动的按钮
    QWidget* m_placeholder = nullptr;       // 布局中的占位符

    QList<int> m_lastEmittedOrder;  // 记录上次发出的顺序

protected:
    bool eventFilter(QObject *obj, QEvent *event) override;

signals:
    void ShowBtn(bool signalEn);
    void RenameBtn(int idx);


};

#endif // EIGHTMYPLAN_H
