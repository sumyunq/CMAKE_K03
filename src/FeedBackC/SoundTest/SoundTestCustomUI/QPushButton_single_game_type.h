#ifndef QPUSHBUTTON_SINGLE_GAME_TYPE_H
#define QPUSHBUTTON_SINGLE_GAME_TYPE_H

#include <QObject>
#include <QPushButton>
#include <QWidget>

class QPushButtonSingleGameType : public QPushButton
{
    Q_OBJECT
public:
    QPushButtonSingleGameType(QWidget *parent = nullptr);
    ~QPushButtonSingleGameType();

    QString cl_game_name() const;
    void setCl_game_name(const QString &newCl_game_name);
    int cl_is_show() const;
    void setCl_is_show(const int &newCl_is_show);

private:
    void InitUIInformation(); ///< 初始化UI的默认信息
    void InitMember();        ///< 初始化内部成员
    void InitConnect();       ///< 连接默认的信号槽

private:
    /// 整体 Pushbutton 的尺寸
    std::atomic<int> minWidth = 91;
    std::atomic<int> minHeight = 36;
    std::atomic<int> maxWidth = 91;
    std::atomic<int> maxHeight = 36;

    QString cl_game_name_ = "";          ///游戏名称,默认空
    std::atomic<int> cl_is_show_ = false; ///是否在布局中显示,默认false
};

#endif // QPUSHBUTTON_SINGLE_GAME_TYPE_H
