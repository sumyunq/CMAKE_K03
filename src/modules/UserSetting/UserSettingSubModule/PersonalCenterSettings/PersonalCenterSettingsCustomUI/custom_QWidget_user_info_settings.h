#ifndef CUSTOM_QWIDGET_USER_INFO_SETTINGS_H
#define CUSTOM_QWIDGET_USER_INFO_SETTINGS_H

#include <QLabel>
#include <QPushButton>
#include <QTextEdit>
#include <QWidget>

#include "modules/UserSetting/UserSettingSubModule/PersonalCenterSettings/PersonalCenterSettingsCustomUI/custom_QWidget_grade_status.h" ///< 子控件，等级状态

namespace Ui {
class CustomQWidgetUserInfoSettings;
}

///
/// \brief The CustomQWidgetUserInfoSettings class
/// UserInfo 用户信息设置
/// 子控件：
///     QLabel 显示 ID:12345678
///     QLabel 显示 头像
///     QLabel 显示 昵称
///     CustomQWidgetGradeStatus 等级状态 自定义控件
///     QLabel 显示 个性签名
///     QLabel 显示 个性签名（用户输入）
///     QPushButton  编辑资料
///     QPushButton  退出登录

class CustomQWidgetUserInfoSettings : public QWidget
{
    Q_OBJECT

public:
    explicit CustomQWidgetUserInfoSettings(QWidget *parent = nullptr);
    ~CustomQWidgetUserInfoSettings();

    void LanguageSet();                           ///< 语言切换：刷新代码内一次性 setText 文本

    void setId(const QString &id);                ///< 设置用户 ID
    void setNickname(const QString &nickname);     ///< 设置用户昵称
    void setSignature(const QString &signature);   ///< 设置个性签名
    void setAvatar(const QPixmap &pixmap);         ///< 设置头像
    void setGrade(int level, int currentXp, int requiredXp); ///< 设置等级与经验进度
    /// \brief 设置用户角色（"streamer"→主播 / "professional"→职业，徽章尺寸与社区卡一致）
    void setRoles(const QStringList &roles);
    /// \brief 设置用户头衔（目前仅有大神 expert，徽章与社区卡一致）
    void setTitles(const QStringList &titles);

private:
    void InitUIInformation(); ///< 初始化UI的默认信息
    void InitMember();        ///< 初始化内部成员
    void InitConnect();       ///< 连接默认的信号槽

public:
    QLabel *clp_id_label_ = nullptr; ///< ID
    QSize cl_id_label_current_size_ = QSize(66, 14);
    QSize cl_id_label_min_size_ = QSize(66, 14);
    QSize cl_id_label_max_size_ = QSize(200, 14);
    QPoint cl_id_label_current_point_ = QPoint(242, 23);
    QPoint cl_id_label_default_point_ = QPoint(242, 23);

    QLabel *clp_icon_head_portrait_ = nullptr; ///< 头像
    QSize cl_icon_head_portrait_current_size_ = QSize(80, 80);
    QSize cl_icon_head_portrait_min_size_ = QSize(80, 80);
    QSize cl_icon_head_portrait_max_size_ = QSize(80, 80);
    QPoint cl_icon_head_portrait_current_point_ = QPoint(126, 81);
    QPoint cl_icon_head_portrait_default_point_ = QPoint(126, 81);

    QLabel *clp_nickname_label_ = nullptr; ///< 昵称
    QString cl_nickname_text_;             ///< 昵称原文（省略显示用，宽度变化时重新截断）
    QSize cl_nickname_label_current_size_ = QSize(239, 23);
    QSize cl_nickname_label_min_size_ = QSize(239, 23);
    QSize cl_nickname_label_max_size_ = QSize(239, 23);
    QPoint cl_nickname_label_current_point_ = QPoint(0, 175);
    QPoint cl_nickname_label_default_point_ = QPoint(0, 175);

    QLabel *clp_role_label_ = nullptr; ///< 角色徽章（主播/职业）
    QSize cl_role_label_current_size_ = QSize(46, 19);
    QSize cl_role_label_min_size_ = QSize(46, 19);
    QSize cl_role_label_max_size_ = QSize(56, 20);
    QPoint cl_role_label_current_point_ = QPoint(116, 204);
    QPoint cl_role_label_default_point_ = QPoint(116, 204);

    QLabel *clp_title_label_ = nullptr; ///< 头衔徽章
    QSize cl_title_label_current_size_ = QSize(56, 18);
    QSize cl_title_label_min_size_ = QSize(56, 18);
    QSize cl_title_label_max_size_ = QSize(120, 18);
    QPoint cl_title_label_current_point_ = QPoint(168, 204);
    QPoint cl_title_label_default_point_ = QPoint(168, 204);

    CustomQWidgetGradeStatus *clp_grade_status_ = nullptr; ///< 等级状态
    QSize cl_grade_status_current_size_ = QSize(332, 14);
    QSize cl_grade_status_min_size_ = QSize(332, 14);
    QSize cl_grade_status_max_size_ = QSize(332, 14);
    QPoint cl_grade_status_current_point_ = QPoint(0, 243);
    QPoint cl_grade_status_default_point_ = QPoint(0, 243);

    QLabel *clp_signature_label_ = nullptr; ///< 个性签名
    QSize cl_signature_label_current_size_ = QSize(56, 20);
    QSize cl_signature_label_min_size_ = QSize(56, 20);
    QSize cl_signature_label_max_size_ = QSize(56, 20);
    QPoint cl_signature_label_current_point_ = QPoint(39, 287);
    QPoint cl_signature_label_default_point_ = QPoint(39, 287);

    QTextEdit *clp_signature_text_ = nullptr; ///< 个性签名（用户输入，只读）
    bool cl_signature_has_value_ = false;     ///< 签名是否有用户数据（false → 显示"无"占位，随语言刷新）
    QSize cl_signature_text_current_size_ = QSize(168, 34);
    QSize cl_signature_text_min_size_ = QSize(168, 150);
    QSize cl_signature_text_max_size_ = QSize(168, 250);
    QPoint cl_signature_text_current_point_ = QPoint(125, 287);
    QPoint cl_signature_text_default_point_ = QPoint(125, 287);

    QPushButton *clp_edit_profile_button_ = nullptr; ///< 编辑资料
    QSize cl_edit_profile_button_current_size_ = QSize(48, 17);
    QSize cl_edit_profile_button_min_size_ = QSize(48, 17);
    QSize cl_edit_profile_button_max_size_ = QSize(48, 17);
    QPoint cl_edit_profile_button_current_point_ = QPoint(76, 551);
    QPoint cl_edit_profile_button_default_point_ = QPoint(76, 551);

    QPushButton *clp_logout_button_ = nullptr; ///< 退出登录
    QSize cl_logout_button_current_size_ = QSize(48, 17);
    QSize cl_logout_button_min_size_ = QSize(48, 17);
    QSize cl_logout_button_max_size_ = QSize(48, 17);
    QPoint cl_logout_button_current_point_ = QPoint(208, 551);
    QPoint cl_logout_button_default_point_ = QPoint(208, 551);

private:
    Ui::CustomQWidgetUserInfoSettings *ui;

    // QWidget interface
protected:
    virtual void resizeEvent(QResizeEvent *event) override;
};

#endif // CUSTOM_QWIDGET_USER_INFO_SETTINGS_H
