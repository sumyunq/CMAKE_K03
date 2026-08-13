#ifndef COMMUNITY_DELEGATE_H
#define COMMUNITY_DELEGATE_H

#include <QHash>
#include <QStyledItemDelegate>
#include <QVector>

#include "data/userConfig/user_config_api.h"  ///< Comment（原 entity/user_config.h 的 CommentItem）

/// \brief 社区卡片委托 — 自绘 hover zone + 展开栏 / Custom delegate with sub-region hit test
/// 布局与 K03 的 CustomQWidgetSinglePlans 保持一致（330×300，widget_01/02/03/04 分区）。
class CommunityDelegate : public QStyledItemDelegate {
    Q_OBJECT

  public:
    enum HoverZone {
        ZoneNone = 0,      ///< 无命中 / no hit
        ZoneAvatar = 1,    ///< 头像区域 / avatar area
        ZoneName = 2,      ///< 用户名区域 / name area
        ZonePlanName = 3,  ///< 方案名区域 / plan name area
        ZoneExpand = 5,    ///< 展开栏区域 / expand bar area (inside comments)
        ZoneLike = 6,      ///< 点赞区域 / like button area
        ZoneDislike = 7,   ///< 踩区域 / dislike button area
        ZoneDownload = 8,  ///< 下载区域 / download button area
        ZoneShare = 9,     ///< 分享区域 / share button area
        ZoneMore = 10      ///< 更多操作菜单 / more actions menu
    };

    /// \brief 构造社区卡片委托 / Construct community card delegate
    explicit CommunityDelegate(QObject* parent = nullptr);

    /// \brief 绘制卡片 / Paint the card
    void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const override;

    /// \brief 返回卡片尺寸提示 / Return size hint for the card
    QSize sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const override;

    /// \brief 处理编辑器事件（hover/click） / Handle editor events (hover/click)
    bool editorEvent(QEvent* event, QAbstractItemModel* model, const QStyleOptionViewItem& option,
                     const QModelIndex& index) override;

    /// \brief 公开的命中检测，供 View 使用 / Public hit test for view
    HoverZone hitTest(const QPoint& pos, const QStyleOptionViewItem& option) const;
    /// \brief 评论标签命中检测 / Comment tag hit test (returns tag id, -1 = miss)
    int commentTagHitTest(const QPoint& pos, int row) const;

    // ── 可选展示配置 / Optional display configuration ──
    enum ActionButton { ActionLike = 0, ActionDislike, ActionDownload, ActionShare };

    void setActionVisible(ActionButton btn, bool visible);
    bool actionVisible(ActionButton btn) const;
    void setStreamerBadgeEnabled(bool on);
    void setExpertBadgeEnabled(bool on);
    void setOfficialBadgeEnabled(bool on);
    void setProfessionalBadgeEnabled(bool on);
    void setCardBlurEnabled(bool on);                    ///< 卡片毛玻璃（弹窗内禁用） / card bg blur toggle
    void setShowSelectionBorder(bool on);                ///< 选中态边框开关（卡片+头像圆环，默认关闭） / selection border toggle
    bool streamerBadgeEnabled() const;
    bool expertBadgeEnabled() const;
    bool officialBadgeEnabled() const;
    bool professionalBadgeEnabled() const;
    void setMoreVisible(bool visible);
    bool moreVisible() const;
    /// \brief 昵称/方案名悬停效果开关（hover 变蓝 + 昵称下划线），默认关闭 / text hover toggle
    void setHoverTextEnabled(bool on);
    bool hoverTextEnabled() const;
    /// \brief 置顶条（widget_00）开关，默认开启；社区广场/大神/官方不显示置顶 / pinned bar toggle
    void setPinnedBarEnabled(bool on);
    bool pinnedBarEnabled() const;
    void setDownloadProgress(int userId, int percent);

  signals:
    void iconClicked(int userId);
    void avatarClicked(int userId);
    void likeClicked(int userId, bool liked);
    void dislikeClicked(int userId, bool disliked);
    void downloadClicked(int userId);
    void shareClicked(int userId);
    /// \param btnGlobalRect 更多按钮全局矩形（锚定菜单用）/ more button global rect
    void moreClicked(int userId, const QRect& btnGlobalRect);
    void commentTagClicked(int configId, int commentId, bool nowClicked);

  private:
    /// \brief 置顶偏移 — 置顶时 widget_01~04 整体下移 / Pinned content offset
    int pinnedOffset(const QStyleOptionViewItem& option) const;
    /// \brief 置顶条是否生效（开关 + IsPinnedRole 双条件） / Whether pinned bar applies
    bool pinnedBarActive(const QModelIndex& index) const;
    QRect pinnedBtnRect(const QStyleOptionViewItem& option) const;
    QRect eyeBtnRect(const QStyleOptionViewItem& option) const;
    /// \brief 眼睛按钮当前是否显示（与 paint 一致）/ Whether eye btn is visible
    bool eyeVisible(const QStyleOptionViewItem& option) const;
    QRect avatarRect(const QStyleOptionViewItem& option) const;
    QRect nameRect(const QStyleOptionViewItem& option) const;
    QRect planNameRect(const QStyleOptionViewItem& option) const;
    QRect planInfoRect(const QStyleOptionViewItem& option) const;
    QRect planIconRect(const QStyleOptionViewItem& option) const;
    QRect planDescRect(const QStyleOptionViewItem& option) const;
    QRect planBadgesRect(const QStyleOptionViewItem& option) const;
    QRect expandBtnRect(const QStyleOptionViewItem& option, int row) const;
    QVector<QPair<int, QRect>> visibleActionRects(const QStyleOptionViewItem& option) const;
    QRect moreRect(const QStyleOptionViewItem& option) const;
    QRect commentsAreaRect(const QStyleOptionViewItem& option) const;
    QRect actionBarRect(const QStyleOptionViewItem& option) const;

    enum class BadgeIcon { Mic, Crown, Star, Briefcase };

    /// \brief K03 CategoryIcon — tag→base 映射 + 路径构造（与 personal_center 统一）
    static const QHash<QString, QString>& categoryIconMap();
    static QString categoryIconPath(const QString& base, bool selected);
    static const char* kDefaultCategoryBase;
    void drawIconBadge(QPainter* p, const QRect& r, const QColor& bg, const QColor& fg, BadgeIcon icon,
                       const QString& text, const QString& bgImagePath = {}) const;

    int badgeWidthFor(const QString& bgImagePath, int height, int explicitW) const;
    int badgeWidthForText(const QString& bgPath, int height, int explicitW,
                          const QString& label, const QFontMetrics& fm) const;

    /// \brief 懒加载缓存 pixmap / Cached pixmap loader
    QPixmap cachedPixmap(const char* path) const;
    /// \brief 6 态图选择 — checked/unchecked × normal/hover/pressed
    /// \brief K03 三态图选择（pressed → checked, checked > hover > normal）
    QPixmap selectActionPixmap(const char* normal, const char* hover, const char* checked,
                               bool isChecked, bool isHovered) const;

    struct TagLayout {
        QVector<QRect> rects;
        QVector<int> ids;
        int rows = 0;
        int contentBottom = 0;
    };
    TagLayout layoutCommentTags(const QRect& area,
                                const QList<DeSheng::GetPublicConfigurationListResponse::Comment>& comments,
                                bool expanded) const;

    // ── Paint sub-methods ──
    void paintPinnedBar(QPainter* p, const QStyleOptionViewItem& opt) const;
    void paintMoreButton(QPainter* p, const QStyleOptionViewItem& opt, int zone) const;
    void paintEyeMark(QPainter* p, const QStyleOptionViewItem& opt) const;
    void paintCardBg(QPainter* p, const QStyleOptionViewItem& opt) const;
    void paintAvatar(QPainter* p, const QStyleOptionViewItem& opt, const QModelIndex& idx, int zone) const;
    void paintName(QPainter* p, const QStyleOptionViewItem& opt, const QModelIndex& idx, int zone) const;
    void paintPlanInfo(QPainter* p, const QStyleOptionViewItem& opt, const QModelIndex& idx, int zone) const;
    void paintPlanBadges(QPainter* p, const QStyleOptionViewItem& opt, const QModelIndex& idx) const;
    void paintActionBar(QPainter* p, const QStyleOptionViewItem& opt, const QModelIndex& idx, int zone) const;
    void paintComments(QPainter* p, const QStyleOptionViewItem& opt, const QModelIndex& idx, int zone) const;
    void paintCommentTags(QPainter* p, const QRect& area, const QModelIndex& idx) const;

    // ── 布局常量（K03 CustomQWidgetSinglePlans） / Layout constants ──
    static constexpr int kCardMargin = 6;            ///< 卡片白边宽度 / card white border
    static constexpr int kAvatarLeftMargin = 24;      ///< 头像左边距 / avatar left margin
    static constexpr int kAvatarTopMargin = 22;       ///< 头像上边距 / avatar top margin
    static constexpr int kAvatarSize = 48;            ///< 头像尺寸 / avatar size
    static constexpr int kNameLeftMargin = 82;        ///< 名字左边距 / name left margin
    static constexpr int kNameRightMargin = 21;       ///< 名字右边距（右侧无元素时）/ name right margin
    static constexpr int kNameEyeGap = 8;             ///< 昵称与眼睛按钮间距（眼睛可见时）/ gap to eye btn
    static constexpr int kNameMoreGap = 12;           ///< 昵称与更多按钮间距（仅 more 可见时）/ gap to more btn
    static constexpr int kNameTop = 26;               ///< 名字 Y / name Y from card top
    static constexpr int kNameHeight = 17;            ///< 昵称行高 / nickname line height
    static constexpr int kLevelX = 82;                ///< 等级 X / level badge X
    static constexpr int kLevelY = 52;                ///< 等级 Y / level badge Y
    static constexpr int kLevelW = 25;                ///< 等级宽度 / level badge width
    static constexpr int kLevelBadgeH = 13;           ///< 等级徽章高度 / level badge height
    static constexpr int kNameLevelSpacing = 5;       ///< 等级与徽章间距 / level-badge spacing
    static constexpr int kBadgeIconSize = 8;          ///< 徽章图标尺寸 / badge icon size
    static constexpr int kMoreSize = 20;              ///< 更多按钮尺寸 / more button size
    static constexpr int kMoreRightMargin = 21;       ///< 更多按钮右边距 / more btn right margin
    // ── 仅自己可见标记（小眼睛，more 左侧，底部与 more 对齐）/ Private eye mark ──
    static constexpr int kEyeBtnW = 18;               ///< 眼睛按钮宽度 / eye btn width
    static constexpr int kEyeBtnH = 16;               ///< 眼睛按钮高度 / eye btn height
    static constexpr int kEyeMoreGap = 10;            ///< 眼睛与 more 水平距离 / gap to more btn
    static constexpr const char* kEyePrivatePath = ":/Skin/Images/Community/private.png";  ///< 仅自己可见图标
    static constexpr const char* kPinPath = ":/Skin/Images/Community/pin.png";             ///< 置顶标记图标
    static constexpr int kTopInfoHeight = 48;         ///< 顶部信息区高度 / top info height
    static constexpr int kSpacerAfterTopInfo = 20;    ///< 顶部区下方间距 / spacer below top info
    // ── 置顶栏（widget_00）/ Pinned bar ──
    // 置顶时在 widget_01 上方绘制置顶条：右侧 46×19 按钮（内容坐标 260,14，右缘距内容右 24px 与评论区对齐）
    static constexpr int kPinnedBtnW = 46;            ///< 置顶按钮宽度 / pinned btn width
    static constexpr int kPinnedBtnH = 19;            ///< 置顶按钮高度 / pinned btn height
    static constexpr int kPinnedBtnX = 260;           ///< 置顶按钮 X（内容坐标）/ pinned btn X
    static constexpr int kPinnedBtnY = 14;            ///< 置顶按钮 Y（内容坐标，底边 y=33）/ pinned btn Y
    static constexpr int kPinnedGap = 8;              ///< 按钮底边与头像顶边间距 / gap to avatar top
    static constexpr int kPinnedOffset = kPinnedBtnY + kPinnedBtnH + kPinnedGap - kAvatarTopMargin;  ///< 置顶时内容下移量 = 19 / content down-shift
    static constexpr int kCommentHMargin = 24;        ///< 评论区水平边距 / comments h-margin
    static constexpr int kCommentAreaMinHeight = 46;  ///< 评论区最小高度 / comments min height
    static constexpr int kSpacerAfterComments = 12;   ///< 评论区下方间距 / spacer below comments
    static constexpr int kPlanInfoHeight = 95;        ///< 方案信息区高度 / plan info height
    static constexpr int kPlanInfoLeftMargin = 22;    ///< 方案信息区左边距 / plan info L margin
    static constexpr int kSpacerAfterPlanInfo = 27;   ///< 方案信息区下方间距 / spacer below plan info
    static constexpr int kActionBarHeight = 14;       ///< 操作栏高度 / action bar height
    // ── 操作栏布局：四等分，首左/中中/末右，图标+间隔+计数整体对齐 ──
    // 内容区宽度 = iconSize + spacing + countMaxWidth，需 ≤ slotW 才能居中生效
    // slotW = bar.width() / 4 ≈ (330-80)/4 = 62.5px（4 按钮，330 宽卡片）
    static constexpr int kActionBarMargin = 37;        ///< 操作栏左右边距 / action bar L/R margin
    static constexpr int kLikeIconSize = 14;           ///< 点赞图标 / like icon px
    static constexpr int kDislikeIconSize = 14;        ///< 踩图标 / dislike icon px
    static constexpr int kDownloadIconSize = 14;       ///< 下载图标 / download icon px
    static constexpr int kShareIconSize = 14;          ///< 分享图标 / share icon px
    static constexpr int kActionIconTextSpacing = 4;    ///< 图标到计数间距 / icon-count gap
    static constexpr int kActionCountWidth = 46;        ///< 计数区域固定宽度（14+4+46=64=slotW）/ count area width
    // 实际内容区 = kXxxIconSize + kActionIconTextSpacing + kActionCountMaxWidth
    static constexpr int kActionCountFontSize = 11;    ///< 操作栏计数字号 / count font px
    static constexpr int kTagFontSize = 11;            ///< 评论标签字号 / tag font px
    static constexpr int kNameFontSize = 12;           ///< 昵称字号 / name font px
    static constexpr int kPlanTitleFontSize = 14;      ///< 方案名字号 / plan title font px
    static constexpr int kDescFontSize = 10;           ///< 描述字号 / desc font px
    static constexpr int kBadgeFontSize = 10;          ///< 徽章文字字号 / badge text font px
    static constexpr int kExpandBtnFontSize = 12;      ///< 展开/收起字号 / expand btn font px
    static constexpr int kMoreFontSize = 12;           ///< 更多按钮字号 / more btn font px
    static constexpr int kActionBtnWidth = 44;        ///< 操作按钮宽度（倍数基数） / btn width base
    static constexpr int kActionBtnSpacing = 31;      ///< 操作按钮间距 / btn spacing
    static constexpr int kBottomSpacing = 31;         ///< 卡片底部间距 / bottom spacing
    static constexpr int kItemMinWidth = 342;         ///< 卡片最小宽度 / min card width
    static constexpr int kItemMinHeight = 325;        ///< 卡片最小高度 / min card height
    static constexpr int kAvatarRadius = 20;          ///< 头像圆角半径 / avatar corner radius
    static constexpr int kCardRadius = 10;            ///< 卡片圆角半径 / card corner radius
    static constexpr int kTagHeight = 20;             ///< 评论标签高度 / comment tag height
    static constexpr int kTagHPadding = 14;           ///< 标签文字水平内边距 / tag text h-pad
    static constexpr int kTagHSpacing = 6;            ///< 标签水平间距 / tag h-spacing
    static constexpr int kTagVSpacing = 6;            ///< 标签垂直间距 / tag v-spacing
    static constexpr int kExpandBtnW = 40;            ///< 展开按钮宽度 / expand btn width
    static constexpr int kExpandBtnH = 20;            ///< 展开按钮高度 / expand btn height
    static constexpr int kMaxCollapsedTagRows = 2;    ///< 收起态最大标签行数 / max collapsed rows
    static constexpr int kPlanBoxRadius = 8;          ///< 方案信息框圆角 / plan box radius
    static constexpr int kPlanIconSize = 71;          ///< 分类图标尺寸 / category icon size
    static constexpr int kPlanIconMargin = 12;        ///< 图标距框边距 / icon margin in box
    static constexpr int kPlanTextX = 95;             ///< 文本列起始 X（框内） / text column X in box
    static constexpr int kPlanTextRight = 12;         ///< 文本列右边距 / text right margin
    static constexpr int kPlanNameY = 14;             ///< 方案名 Y（框内） / plan name Y
    static constexpr int kPlanNameHeight = 20;        ///< 方案名高度 / plan name height
    static constexpr int kPlanDescY = 38;             ///< 描述 Y（框内） / desc Y
    static constexpr int kPlanDescHeight = 14;        ///< 描述高度 / desc height
    static constexpr int kPlanBadgesY = 65;           ///< 徽章 Y（框内） / badges Y
    static constexpr int kBadgeHeight = 16;           ///< 徽章高度 / badge height
    static constexpr int kBadgeHPadding = 12;         ///< 徽章水平内边距 / badge h-padding
    static constexpr int kBadgeSpacing = 6;           ///< 徽章间距 / badge spacing

    // ── 颜色常量 / Color constants ──
    // item 整体背景：Qt 5.15 自绘用 QRgb 整数 alpha（0.2×255≈51）
    static constexpr const char* kColorAccent = "#2d9df0";                ///< 强调蓝 / accent blue
    static constexpr const char* kColorNameText = "#ffffff";              ///< 昵称/方案名 / name & title
    static constexpr QRgb kColorCardBg = qRgba(81, 96, 122, 51);        ///< item 整体背景 / card bg（与 FrostedPanel 面板 tint 同色号同浓度，压住盒式模糊纹理）
    static constexpr QRgb kColorCardHover = qRgba(81, 96, 122, 51);     ///< item 悬停 — 与常态同色（需求：hover 不变色，保留逻辑）
    static constexpr const char* kColorAvatarBg = "#e8eaed";              ///< 头像占位 / avatar placeholder
    static constexpr const char* kColorLevelBg = "#2d9df0";               ///< 等级徽章背景 / level badge bg
    static constexpr const char* kColorLevelText = "#ffffff";             ///< 等级徽章文字 / level badge text
    static constexpr const char* kColorTagNormal = "#333d4d";             ///< 标签普通背景 / tag normal bg
    static constexpr const char* kColorTagNormalText = "#9aa4b2";         ///< 标签普通文字 / tag normal text
    static constexpr const char* kColorTagSelectedText = "#ffffff";       ///< 标签选中文字 / tag selected
    static constexpr const char* kColorActionInactive = "#8a94a6";        ///< 操作图标灰 / action gray
    static constexpr const char* kColorMoreInactive = "#8a94a6";          ///< 更多按钮灰 / more btn gray
    static constexpr const char* kColorExpandCollapsed = "#2d9df0";       ///< 展开按钮 / expand btn
    static constexpr const char* kColorExpandCollapsedHover = "#5cb3f5";  ///< 展开按钮悬停
    static constexpr const char* kColorExpandExpanded = "#9cabb4";        ///< 收起按钮 / collapse
    static constexpr const char* kColorExpandExpandedHover = "#c3cdd4";   ///< 收起按钮悬停
    static constexpr const char* kColorBadgeText = "#aab2c0";             ///< 徽章文字 / badge text
    static constexpr const char* kColorPlanIconText = "#8a94a6";          ///< 图标占位文字 / icon text
    static constexpr const char* kColorStreamerBg = "#e6498f";            ///< 主播徽章背景 / streamer bg
    static constexpr const char* kColorStreamerText = "#ffffff";          ///< 主播徽章文字 / streamer text
    static constexpr const char* kColorExpertBg = "#f5c211";              ///< 大神徽章背景 / expert bg
    static constexpr const char* kColorExpertText = "#3d2e00";            ///< 大神徽章文字 / expert text
    static constexpr const char* kColorOfficialBg = "#10b981";            ///< 官方徽章背景 / official bg
    static constexpr const char* kColorOfficialText = "#ffffff";          ///< 官方徽章文字 / official text
    static constexpr const char* kColorProfessionalBg = "#8b5cf6";        ///< 职业徽章背景 / professional bg
    static constexpr const char* kColorProfessionalText = "#ffffff";      ///< 职业徽章文字 / professional text

    // ── 徽章背景图 + 独立 W/H/Y / Badge image paths + per-badge dimensions ──
    static constexpr const char* kStreamerBadgeBgPath = ":/Skin/Images/modules/community/host.png";  ///< 主播徽章背景
    static constexpr int kStreamerBadgeW = 46;                                          ///< 主播徽章宽
    static constexpr int kStreamerBadgeH = 19;                                          ///< 主播徽章高
    static constexpr int kStreamerBadgeY = 33;                                          ///< 主播徽章 Y
    static constexpr const char* kExpertBadgeBgPath = ":/Skin/Images/modules/community/god.png";  ///< 大神徽章背景
    static constexpr int kExpertBadgeW = 53;                                            ///< 大神徽章宽
    static constexpr int kExpertBadgeH = 20;                                            ///< 大神徽章高
    static constexpr int kExpertBadgeY = 32;                                            ///< 大神徽章 Y
    static constexpr const char* kOfficialBadgeBgPath = ":/Skin/Images/modules/community/official.png";  ///< 官方徽章背景
    static constexpr int kOfficialBadgeW = 56;                                          ///< 官方徽章宽
    static constexpr int kOfficialBadgeH = 18;                                          ///< 官方徽章高
    static constexpr int kOfficialBadgeY = 34;                                          ///< 官方徽章 Y
    static constexpr const char* kProfessionalBadgeBgPath = ":/Skin/Images/modules/community/professional.png";  ///< 职业徽章背景
    static constexpr int kProfessionalBadgeW = 50;                                      ///< 职业徽章宽
    static constexpr int kProfessionalBadgeH = 18;                                      ///< 职业徽章高
    static constexpr int kProfessionalBadgeY = 34;                                      ///< 职业徽章 Y

    // ── 操作按钮资源图（K03 QSS 三态：normal / hover / checked） ──
    static constexpr const char* kLikeNormalPath   = ":/Skin/Images/GeneralIcon/liked_1x_normal_darkBlue.png";
    static constexpr const char* kLikeHoverPath    = ":/Skin/Images/GeneralIcon/liked_1x_hover_darkBlue.png";
    static constexpr const char* kLikeCheckedPath  = ":/Skin/Images/GeneralIcon/liked_1x_checked_darkBlue.png";
    static constexpr const char* kDislikeNormalPath  = ":/Skin/Images/GeneralIcon/disliked_1x_normal_darkBlue.png";
    static constexpr const char* kDislikeHoverPath   = ":/Skin/Images/GeneralIcon/disliked_1x_hover_darkBlue.png";
    static constexpr const char* kDislikeCheckedPath = ":/Skin/Images/GeneralIcon/disliked_1x_checked_darkBlue.png";
    static constexpr const char* kDownloadNormalPath = ":/Skin/Images/GeneralIcon/download_1x_normal_darkBlue.png";
    static constexpr const char* kDownloadHoverPath = ":/Skin/Images/GeneralIcon/download_1x_hover_darkBlue.png";
    static constexpr const char* kShareNormalPath = ":/Skin/Images/GeneralIcon/share_1x_normal_darkBlue.png";
    static constexpr const char* kShareHoverPath = ":/Skin/Images/GeneralIcon/share_1x_hover_darkBlue.png";
    static constexpr const char* kDownloadRingColor = "#009FEF";
    static constexpr int kDownloadRingWidth = 2;
    // ── 更多按钮资源图（menu normal / hover） ──
    static constexpr const char* kMoreNormalPath = ":/Skin/Images/Community/menu-no.png";
    static constexpr const char* kMoreHoverPath  = ":/Skin/Images/Community/menu-hover.png";

    mutable QHash<int, HoverZone> cl_hover_zones_;       ///< 行悬停区域缓存 / hover zone cache
    mutable QHash<int, HoverZone> cl_pressed_zones_;     ///< 行按下区域缓存 / pressed zone cache
    mutable QHash<int, QVector<QRect>> cl_tag_rects_;    ///< 行标签矩形缓存 / tag rect cache
    mutable QHash<int, QVector<int>> cl_tag_ids_;        ///< 行标签 ID 缓存 / tag id cache
    mutable QHash<int, int> cl_tag_content_bottom_;      ///< 末行标签底部 Y / tag content bottom
    mutable QHash<QString, QPixmap> cl_badge_bg_cache_;  ///< 徽章背景图缓存 / badge bg cache
    mutable QHash<QString, QPixmap> cl_btn_img_cache_;   ///< 按钮图标缓存 / btn img cache
    mutable QHash<int, int> cl_download_progress_;       ///< 下载进度 0~100（每行） / download progress
    QHash<int, bool> cl_action_visibility_;              ///< 操作按钮显隐 / action visibility
    bool cl_streamer_badge_enabled_ = true;              ///< 主播徽章开关 / streamer badge
    bool cl_expert_badge_enabled_ = true;                ///< 大神徽章开关 / expert badge
    bool cl_official_badge_enabled_ = true;              ///< 官方徽章开关 / official badge
    bool cl_professional_badge_enabled_ = true;          ///< 职业徽章开关 / professional badge
    bool cl_card_blur_enabled_ = true;                   ///< 卡片毛玻璃（弹窗内 setCardBlurEnabled(false) 禁用，默认开启） / card bg blur
    bool cl_show_selection_border_ = false;              ///< 选中态边框开关（卡片+头像圆环，setShowSelectionBorder(true) 开启，默认关闭） / selection border
    bool cl_more_visible_ = false;                       ///< 更多按钮显隐 / more button visible
    bool cl_hover_text_enabled_ = false;                 ///< 昵称/方案名悬停效果开关 / text hover toggle
    bool cl_pinned_bar_enabled_ = false;                 ///< 置顶条开关（默认关闭；仅个人中心已上传显式开启，按服务器 is_pinned 渲染）
};

#endif  // COMMUNITY_DELEGATE_H
