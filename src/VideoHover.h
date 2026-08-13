#ifndef VIDEOHOVER_H
#define VIDEOHOVER_H

#include <QWidget>
#include <QPainter>
#include <atomic>


namespace Ui {
class VideoHover;
}

class VideoHover : public QWidget
{
    Q_OBJECT

public:
    explicit VideoHover(QWidget *parent = nullptr);
    ~VideoHover();

    QWidget* hoverWav;

    // 添加获取和设置相对位置的方法
    QPointF getRelativePosition() const;
    void setRelativePosition(const QPointF &relativePos);
    void updatePositionRelativeToParent();

    void setPlayEn(bool en);

    void requestRestore(); ///用于主动请求恢复至主窗口
    void set_is_pause(bool is_pause);   ///启用画中画时，更新对应图标状态
    void setAcceptingFrames(bool accept); ///视频切换时拒绝接收旧帧

signals:
    void closeRequested(); ///点击关闭按键
    void restoreRequested(); ///恢复至主窗口
    void video_pause_requested(bool pause);   /// 暂停true / 播放false

private:
    void InitUIInformation(); ///< 初始化UI的默认信息

public slots:
    void on_pBt_close_clicked();
    void updateMinWidget_Frame(const QImage &img);  ///更新视频帧图片

public:
    QImage currentImage; ///用于显示的图片

private:
    Ui::VideoHover *ui;
    bool m_dragging  = false;   // 拖动状态标志
    QPoint m_dragPosition;       // 记录拖动时的鼠标位置

    std::atomic<bool> accepting_frames_{true}; ///视频切换过渡期间拒绝旧帧

    // 记录相对于父窗口的相对位置 (0.0到1.0之间)
    QPointF m_relativePosition = QPointF(1.0, 0.5); // 默认靠右居中
    // 限制移动范围
    QRect getMoveArea() const;
    void updateRelativePosition();

protected:
    // bool eventFilter(QObject* obj, QEvent* event) override;
    void mousePressEvent(QMouseEvent *event) override;  // 鼠标按下事件
    void mouseMoveEvent(QMouseEvent *event) override;   // 鼠标移动事件
    void mouseReleaseEvent(QMouseEvent *event) override;// 鼠标释放事件

signals:
    void videoClose();
    void videoBack();
    void videoPlay();


private slots:
    void on_pBt_back_clicked();
    void on_pBt_play_clicked();

    // QWidget interface
protected:
    virtual void paintEvent(QPaintEvent *event) override;
};

#endif // VIDEOHOVER_H
