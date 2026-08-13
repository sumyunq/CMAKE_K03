#ifndef AVATARBUTTON_H
#define AVATARBUTTON_H

#include <QPushButton>
#include <QLabel>
#include <QPixmap>

class QHideEvent;

class AvatarButton : public QPushButton
{
    Q_OBJECT
    Q_PROPERTY(SlotType slotType READ slotType WRITE setSlotType DESIGNABLE true)

public:
    enum SlotType {
        LikeNo1,
        LikeNo2,
        LikeNo3,
        DownloadNo1,
        DownloadNo2,
        DownloadNo3
    };
    Q_ENUM(SlotType)

    // 默认构造函数，用于在 UI 设计器提升时自动调用
    explicit AvatarButton(QWidget *parent = nullptr);

    // 带位置类型的构造函数，也可直接使用
    AvatarButton(SlotType slot, QWidget *parent = nullptr);

    ~AvatarButton() override = default;

    // 设置头像图片（用户真实头像）
    void setAvatar(const QString &imagePath);
    // 设置头像位图（网络下载头像用）
    void setAvatarPixmap(const QPixmap &pm);

    // 设置位置类型（决定边框和角标样式）
    void setSlotType(SlotType slot);
    SlotType slotType() const { return m_slot; }

signals:
    void clicked();  ///< 点击头像位（左键按下并释放于按钮内）

protected:
    void resizeEvent(QResizeEvent *event) override;

    void moveEvent(QMoveEvent *event) override;
    void showEvent(QShowEvent *event) override;
    void hideEvent(QHideEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

private:
    void applySlotResources();              // 根据 m_slot 加载边框/角标资源
    void updateIcon();                      // 合成圆形头像+边框，设为图标
    void repositionBadge();                 // 调整角标位置

    SlotType m_slot = LikeNo1;              // 默认为第一名
    QString m_borderPath;
    QString m_badgePath;
    QString m_avatarPath;
    QPixmap m_avatarPixmap;
    QLabel *m_badgeLabel = nullptr;


    // 尺寸常量
    static constexpr int BUTTON_SIZE = 42;// 按钮固定大小
    static constexpr int AVATAR_SIZE = 33;// 头像圆直径
    static constexpr int BORDER_SIZE = 39;// 边框圆环直径
    static constexpr int BADGE_SIZE  = 18;//数字角标圆直径
    static constexpr int OFFSET = 3;   // 数字角标偏移，使角标部分伸出按钮
};

#endif // AVATARBUTTON_H
