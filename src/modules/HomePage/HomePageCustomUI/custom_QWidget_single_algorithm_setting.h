#ifndef CUSTOM_QWIDGET_SINGLE_ALGORITHM_SETTING_H
#define CUSTOM_QWIDGET_SINGLE_ALGORITHM_SETTING_H

#include <QLabel>
#include <QPushButton>
#include <QSlider>
#include <QWidget>


#include "CustomControl/CustomSlider/GearSlider.h"   ///< 自定义水平条

enum class AlgorithmType {
    FootstepEnhance,    // 脚步增强
    GunshotWeakening,   // 枪声弱化
    SoundFieldControl,  // 声场控制
    Clarity,            // 清晰度
    Unknown             // 未知
};

namespace Ui {
class CustomQWidgetSingleAlgorithmSetting;
}
///
/// \brief The CustomQWidgetSingleAlgorithmSetting class
/// 子控件：
///     算法类型 图标
///     算法效果 文字
///     算法数值 文字
///     算法增强 按键
///     算法削弱 按键
///     算法值 水平进度条
class CustomQWidgetSingleAlgorithmSetting : public QWidget
{
    Q_OBJECT

public:
    explicit CustomQWidgetSingleAlgorithmSetting(QWidget *parent = nullptr);
    ~CustomQWidgetSingleAlgorithmSetting();

    // 构造后，需设置以下参数：算法名字（算法效果）、
    void updateAlgorithmEffect(QString algorithm_effect);    ///算法类型（算法效果）
    void updateValueRange(int min_value,int max_value);    /// 更新数值范围
    void updateValue(int value);    /// 更新一次数值

    ///
    /// \brief setEditStatus    设置编辑状态
    /// \param status           true:可编辑; false:不可编辑
    ///
    void setEditStatus(bool status);

private:
    void InitUIInformation(); ///< 初始化UI的默认信息
    void InitMember();        ///< 初始化内部成员
    void InitConnect();       ///< 连接默认的信号槽

    AlgorithmType stringToAlgorithmType(const QString &str);

public:
    /******************** UI ********************/
    QLabel *cl_icon_algorithm_type_ = nullptr; ///< 图标： 算法类型
    QSize cl_icon_algorithm_type_size_ = QSize(22, 22);
    QPoint cl_icon_algorithm_type_point_ = QPoint(0, 0);

    QLabel *cl_text_algorithm_effect_ = nullptr; ///< 文字： 算法效果
    QSize cl_text_algorithm_effect_size_ = QSize(150, 17);
    QPoint cl_text_algorithm_effect_point_ = QPoint(33, 2);

    QLabel *cl_text_algorithm_value_ = nullptr; ///< 文字： 算法数值
    QSize cl_text_algorithm_value_size_ = QSize(20, 20);
    QPoint cl_text_algorithm_value_point_ = QPoint(264, 2);

    QPushButton *cl_del_algorithm_value_ = nullptr; ///< 按键： 削弱算法数值
    QSize cl_del_algorithm_value_size_ = QSize(20, 20);
    QPoint cl_del_algorithm_value_point_ = QPoint(0, 28);

    GearSlider *cl_algorithm_value_hSlider_ = nullptr; ///< 水平进度条： 算法数值
    QSize cl_algorithm_value_hSlider__size_ = QSize(218, 20);
    QPoint cl_algorithm_value_hSlider_point_ = QPoint(33, 28);

    QPushButton *cl_add_algorithm_value_ = nullptr; ///< 按键： 提高算法数值
    QSize cl_add_algorithm_value_size_ = QSize(20, 20);
    QPoint cl_add_algorithm_value_point_ = QPoint(264, 28);

    int cl_minValue_ = 0;     ///< 最大值
    int cl_maxValue_ = 6;   ///< 最小值

private:
    Ui::CustomQWidgetSingleAlgorithmSetting *ui;

    // QWidget interface
protected:
    virtual void resizeEvent(QResizeEvent *event) override;
};

#endif // CUSTOM_QWIDGET_SINGLE_ALGORITHM_SETTING_H
