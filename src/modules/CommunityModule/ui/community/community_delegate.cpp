#include "modules/CommunityModule/ui/community/community_delegate.h"

#include <QAbstractItemView>
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>
#include <QtMath>

#include <limits>

#include "modules/CommunityModule/ui/community/community_model.h"
#include "modules/CommunityModule/infrastructure/logger/logger.h"

// ── K03 CategoryIcon 统一映射（与 personal_center_settings_main_page 一致）──
// 使用 data/api_global.h 的 CategoryIcon::kIconMap + buildPath，
// 资源路径 :/Skin/Images/Headphones/edit/ 由 K03 统一管理。
#include "data/api_global.h"

// ── 毛玻璃背景 — K03 AppImageCache 整窗模糊快照 ──
#include "modules/Common/AppImageCache.h"
#include "modules/Common/elide_text.h"  ///< DeSheng::elideTextWithDots

const char* CommunityDelegate::kDefaultCategoryBase = "game";

const QHash<QString, QString>& CommunityDelegate::categoryIconMap() {
    return CategoryIcon::kIconMap;  // 与 K03 个人中心/方案卡片共用同一映射
}

QString CommunityDelegate::categoryIconPath(const QString& base, bool selected) {
    return CategoryIcon::buildPath(base, selected, /*t_system=*/false);
}

namespace {
/// \brief Return a copy of the painter's font with the given pixel size / 字体工具
QFont makeFont(const QPainter* p, int pixelSize) {
    QFont f = p->font();
    f.setPixelSize(pixelSize);
    return f;
}
/// \brief K03 计数格式：<10000 原样，≥10000 显示 X.Yw / tag count format
QString tagCountText(int count) {
    return count >= 10000 ? QString::asprintf("%.1fw", count / 10000.0) : QString::number(count);
}
}  // namespace

CommunityDelegate::CommunityDelegate(QObject* parent) : QStyledItemDelegate(parent) {}

// ── 可选展示配置 ──

void CommunityDelegate::setActionVisible(ActionButton btn, bool visible) {
    cl_action_visibility_[static_cast<int>(btn)] = visible;
}

bool CommunityDelegate::actionVisible(ActionButton btn) const {
    return cl_action_visibility_.value(static_cast<int>(btn), true);
}

void CommunityDelegate::setStreamerBadgeEnabled(bool on) { cl_streamer_badge_enabled_ = on; }

void CommunityDelegate::setExpertBadgeEnabled(bool on) { cl_expert_badge_enabled_ = on; }

void CommunityDelegate::setOfficialBadgeEnabled(bool on) { cl_official_badge_enabled_ = on; }

void CommunityDelegate::setProfessionalBadgeEnabled(bool on) { cl_professional_badge_enabled_ = on; }
void CommunityDelegate::setCardBlurEnabled(bool on) { cl_card_blur_enabled_ = on; }  ///< 卡片毛玻璃（弹窗内禁用）
void CommunityDelegate::setShowSelectionBorder(bool on) { cl_show_selection_border_ = on; }  ///< 选中态边框开关（默认关闭，需显式开启）
void CommunityDelegate::setMoreVisible(bool visible) { cl_more_visible_ = visible; }
bool CommunityDelegate::moreVisible() const { return cl_more_visible_; }

void CommunityDelegate::setHoverTextEnabled(bool on) { cl_hover_text_enabled_ = on; }
bool CommunityDelegate::hoverTextEnabled() const { return cl_hover_text_enabled_; }

void CommunityDelegate::setPinnedBarEnabled(bool on) { cl_pinned_bar_enabled_ = on; }
bool CommunityDelegate::pinnedBarEnabled() const { return cl_pinned_bar_enabled_; }

bool CommunityDelegate::streamerBadgeEnabled() const { return cl_streamer_badge_enabled_; }

bool CommunityDelegate::expertBadgeEnabled() const { return cl_expert_badge_enabled_; }

bool CommunityDelegate::officialBadgeEnabled() const { return cl_official_badge_enabled_; }

bool CommunityDelegate::professionalBadgeEnabled() const { return cl_professional_badge_enabled_; }

void CommunityDelegate::setDownloadProgress(int userId, int percent) { cl_download_progress_[userId] = percent; }

// ── sizeHint ──

/// \brief 卡片尺寸 — 收起态 342×325（含 6px 白边），置顶 +19，展开态 + 标签行数增量
/// 内容区 330×313，K03: baseH = wp_01~04 + 5 spacers = 315
QSize CommunityDelegate::sizeHint(const QStyleOptionViewItem& opt, const QModelIndex& index) const {
    const bool expanded = index.data(CommunityModel::ExpandedRole).toBool();
    // 置顶条生效时 widget_00 占顶部，widget_01~04 整体下移 kPinnedOffset（与 pinnedOffset 同一判定）
    const int pinnedH = pinnedBarActive(index) ? kPinnedOffset : 0;
    // baseH = K03 9-layer fixed height: 22+48+20+46+12+95+27+14+31 = 315
    if (!expanded) return {kItemMinWidth, kItemMinHeight + pinnedH};

    // expanded: 用与 paint 完全相同的流式布局算行数，保证高度与绘制一致
    // availW 必须与 commentsAreaRect 的实际宽度一致：opt.rect.width() - 2*kCardMargin - 2*kCommentHMargin
    const int availW = opt.rect.width() > 0 ? opt.rect.width() - kCardMargin * 2 - kCommentHMargin * 2
                                            : kItemMinWidth - kCardMargin * 2 - kCommentHMargin * 2;
    const auto* model = qobject_cast<const CommunityModel*>(index.model());
    if (!model) return {kItemMinWidth, kItemMinHeight + pinnedH};
    const auto itemOpt = model->findById(index.data(CommunityModel::UserIdRole).toInt());
    if (!itemOpt.has_value() || itemOpt->comments.isEmpty()) return {kItemMinWidth, kItemMinHeight + pinnedH};

    const auto layout = layoutCommentTags(QRect(0, 0, availW, 100000), itemOpt->comments,
                                          /*expanded=*/true);
    // baseH 已含 2 行（46px），超出部分按行高递增
    const int rowH = kTagHeight + kTagVSpacing;
    const int extraH = (std::max)(0, (layout.rows - kMaxCollapsedTagRows) * rowH);
    return {kItemMinWidth, kItemMinHeight + extraH + pinnedH};
}

// ── paint (orchestrator) ──

/// \brief 卡片绘制编排 — 按 K03 widget_01→02→03→04 顺序调用子方法，若是置顶方案，则先调用widget_00
void CommunityDelegate::paint(QPainter* p, const QStyleOptionViewItem& opt, const QModelIndex& idx) const {
    p->save();
    p->setRenderHint(QPainter::Antialiasing);
    const auto zone = static_cast<int>(cl_hover_zones_.value(idx.row(), ZoneNone));

    paintCardBg(p, opt);
    if (pinnedOffset(opt) > 0) paintPinnedBar(p, opt);  // K03 widget_00: 置顶栏（纯展示）
    paintAvatar(p, opt, idx, zone);
    paintName(p, opt, idx, zone);
    paintMoreButton(p, opt, zone);  // K03 widget_01 right: more 按钮（默认隐藏）
    paintEyeMark(p, opt);           // K03: 仅自己可见标记（more 左侧）
    paintComments(p, opt, idx, zone);   // K03 widget_02: expandable comment tags
    paintPlanInfo(p, opt, idx, zone);   // K03 widget_03: plan info box
    paintActionBar(p, opt, idx, zone);  // K03 widget_04: bottom bar
    p->restore();
}

// ── paint sub-methods ──

/// \brief 置顶条是否生效（开关 + IsPinnedRole 双条件）— sizeHint 与 pinnedOffset 共用
bool CommunityDelegate::pinnedBarActive(const QModelIndex& index) const {
    return cl_pinned_bar_enabled_ && index.data(CommunityModel::IsPinnedRole).toBool();
}

/// \brief 置顶偏移 — 置顶条生效时 widget_01~04 整体下移 kPinnedOffset，否则返回 0
int CommunityDelegate::pinnedOffset(const QStyleOptionViewItem& opt) const {
    return pinnedBarActive(opt.index) ? kPinnedOffset : 0;
}

/// \brief K03 widget_01 — more 按钮（默认隐藏）：menu-no/menu-hover 背景图，缺图回退 ⋯ 文本
void CommunityDelegate::paintMoreButton(QPainter* p, const QStyleOptionViewItem& opt,
                                        int zone) const {
    if (!cl_more_visible_) return;
    const bool h = (zone == ZoneMore);
    const QPixmap pm = cachedPixmap(h ? kMoreHoverPath : kMoreNormalPath);
    const QRect r = moreRect(opt);
    if (!pm.isNull()) {
        p->drawPixmap(r, pm.scaled(r.size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
    } else {
        p->setPen(QColor(h ? kColorAccent : kColorMoreInactive));
        p->setFont(makeFont(p, kMoreFontSize));
        p->drawText(r, Qt::AlignCenter, QString::fromUtf8("\xe2\x8b\xaf"));
    }
}

/// \brief 仅自己可见标记（more 左侧 18×16，眼睛位）— visibility 为 private 时绘制
void CommunityDelegate::paintEyeMark(QPainter* p, const QStyleOptionViewItem& opt) const {
    if (!eyeVisible(opt)) return;
    const QPixmap pm = cachedPixmap(kEyePrivatePath);
    const QRect r = eyeBtnRect(opt);
    if (!pm.isNull())
        p->drawPixmap(r, pm.scaled(r.size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
}

/// \brief 置顶按钮矩形（内容坐标 260,14，46×19，右缘距内容右 24px 与评论区对齐）
QRect CommunityDelegate::pinnedBtnRect(const QStyleOptionViewItem& opt) const {
    return {opt.rect.left() + kCardMargin + kPinnedBtnX,
            opt.rect.top() + kCardMargin + kPinnedBtnY, kPinnedBtnW, kPinnedBtnH};
}

/// \brief 状态标记矩形（18×16，more 左侧 kEyeMoreGap，底部与 more 底部对齐）/ eye btn rect
QRect CommunityDelegate::eyeBtnRect(const QStyleOptionViewItem& opt) const {
    const QRect m = moreRect(opt);
    return {m.left() - kEyeMoreGap - kEyeBtnW, m.bottom() + 1 - kEyeBtnH, kEyeBtnW, kEyeBtnH};
}

/// \brief 眼睛按钮当前是否显示（仅已上传页 + private 状态，与 paint 一致）/ Whether eye btn visible
bool CommunityDelegate::eyeVisible(const QStyleOptionViewItem& opt) const {
    return cl_more_visible_ &&
           opt.index.data(CommunityModel::VisibilityRole).toString() == DeSheng::kVisibilityPrivate;
}

/// \brief K03 widget_00 — 置顶栏（纯展示，不响应鼠标）：右侧 46×19 置顶按钮（仅 pin.png 图标）
/// 按钮底边 y=33 与头像顶边 y=41 间距 kPinnedGap=8
void CommunityDelegate::paintPinnedBar(QPainter* p, const QStyleOptionViewItem& opt) const {
    const QRect r = pinnedBtnRect(opt);
    const QPixmap pm = cachedPixmap(kPinPath);
    if (!pm.isNull())
        p->drawPixmap(r, pm.scaled(r.size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
    // 缺图不绘制（无颜色/文本回退）
}

/// \brief K03 卡片背景 — 圆角矩形，悬停/选中态切换
void CommunityDelegate::paintCardBg(QPainter* p, const QStyleOptionViewItem& opt) const {
    const bool hovered = (opt.state & QStyle::State_MouseOver);
    const QRect cardRect = opt.rect.adjusted(kCardMargin, kCardMargin, -kCardMargin, -kCardMargin);
    QPainterPath path;
    path.addRoundedRect(cardRect, kCardRadius, kCardRadius);

    // ① 毛玻璃 — FrostedPanel 同款模式：translate(-viewport窗口位置) 使 painter 坐标 = 窗口坐标
    // 注意：不用 clippedBlur（其对降采样小图不缩放且坐标偏移，结果错误）
    // 关键：平移的是 viewport 原点在窗口的位置（不是卡片位置！），否则越靠右下偏移越大
    // 实例 cl_card_blur_enabled_（弹窗内禁用），关闭时回退纯色 tint
    auto& cache = AppImageCache::instance();
    if (cl_card_blur_enabled_ && !cache.cl_background_blurred_cache_.isNull()) {
        if (QAbstractItemView* view = qobject_cast<QAbstractItemView*>(parent())) {
            QWidget* win = view->window();
            QWidget* vp = view->viewport();
            if (win && vp) {
                const QPoint vpWinPos = vp->mapTo(win, QPoint(0, 0));  // viewport 原点 → 窗口坐标
                p->save();
                p->setClipPath(path);  // 视口坐标圆角裁剪
                p->setRenderHint(QPainter::SmoothPixmapTransform);  // 平滑放大，避免 4× 降采样颗粒
                p->translate(-vpWinPos);  // painter 坐标 = 窗口坐标
                p->drawPixmap(QRect(0, 0, win->width(), win->height()),
                              cache.cl_background_blurred_cache_);
                p->restore();
            }
        }
        // ② item 整体背景 tint — normal: rgba(81,96,122,0.2), hover: rgba(223,243,255,0.2)
        p->fillPath(path, QColor::fromRgba(hovered ? kColorCardHover : kColorCardBg));
    } else {
        // 模糊快照未就绪 → 回退纯色背景
        p->fillPath(path, QColor::fromRgba(hovered ? kColorCardHover : kColorCardBg));
    }

    if (cl_show_selection_border_ && (opt.state & QStyle::State_Selected)) {
        p->setPen(QPen(QColor(kColorAccent), 2));
        p->setBrush(Qt::NoBrush);
        p->drawRoundedRect(opt.rect.adjusted(kCardMargin + 1, kCardMargin + 1, -kCardMargin - 1, -kCardMargin - 1),
                           kCardRadius, kCardRadius);
    }
}

/// \brief K03 widget_01 — 40×40 圆形头像，悬停时描边
void CommunityDelegate::paintAvatar(QPainter* p, const QStyleOptionViewItem& opt, const QModelIndex& idx,
                                    int zone) const {
    const QRect r = avatarRect(opt);
    QPixmap px = idx.data(CommunityModel::AvatarRole).value<QPixmap>();
    // 头像为空 → 系统默认头像（@2x 图按显示尺寸缩放，等价缩小一倍）
    if (px.isNull())
        px = QPixmap(QStringLiteral(":/Skin/Images/system/system_avatar/system_avatar_2x_01.png"));
    if (!px.isNull()) {
        p->save();
        QPainterPath path;
        path.addEllipse(r);  // K03: circular avatar
        p->setClipPath(path);
        p->drawPixmap(r, px.scaled(r.size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
        p->restore();
    } else {
        p->setBrush(QColor(kColorAvatarBg));
        p->setPen(Qt::NoPen);
        p->drawEllipse(r);
    }
    if (cl_show_selection_border_ && zone == ZoneAvatar) {
        p->setPen(QPen(QColor(kColorAccent), 2));
        p->setBrush(Qt::NoBrush);
        p->drawEllipse(r.adjusted(1, 1, -1, -1));
    }
}

/// \brief K03 widget_01 — 昵称（12px）+ 徽章行（Lv.X / 主播 / 大神，跟随文字末端）
/// 徽章显隐由 setStreamerBadgeEnabled / setExpertBadgeEnabled 控制
void CommunityDelegate::paintName(QPainter* p, const QStyleOptionViewItem& opt, const QModelIndex& idx,
                                  int zone) const {
    const int cardLeft = opt.rect.left() + kCardMargin;
    const int cardTop = opt.rect.top() + kCardMargin;
    const int offset = pinnedOffset(opt);  // 置顶时 widget_01 整体下移

    // ── Row 1: Name (82, 26), elide end ──
    // 与 nameRect()（命中区域）统一：右边界始终避让 眼睛/更多 图标，保持固定间距
    const QRect nameR = nameRect(opt);
    p->setFont(makeFont(p, kNameFontSize));
    const QFontMetrics nameFm(p->font());
    const QString name = idx.data(CommunityModel::NameRole).toString();
    const QString shown = DeSheng::elideTextWithDots(name, p->font(), nameR.width());
    const bool hovered = cl_hover_text_enabled_ && (zone == ZoneName);  ///< 悬停效果受开关控制（默认关闭）
    p->setPen(QColor(hovered ? kColorAccent : kColorNameText));
    p->drawText(nameR, Qt::AlignLeft | Qt::AlignVCenter, shown);
    if (hovered) {
        const int tw = nameFm.horizontalAdvance(shown);
        p->drawLine(QPoint(nameR.left(), nameR.bottom() - 2), QPoint(nameR.left() + tw, nameR.bottom() - 2));
    }

    // ── Row 2: Level (82, 52, 25, 13) + badges ──
    const int level = idx.data(CommunityModel::AuthorLevelRole).toInt();
    const bool streamer = cl_streamer_badge_enabled_ && idx.data(CommunityModel::IsStreamerRole).toBool();
    const bool expert = cl_expert_badge_enabled_ && idx.data(CommunityModel::IsExpertRole).toBool();
    const bool official = cl_official_badge_enabled_ && idx.data(CommunityModel::IsOfficialRole).toBool();
    const bool professional = cl_professional_badge_enabled_ && idx.data(CommunityModel::IsProfessionalRole).toBool();

    p->setFont(makeFont(p, kBadgeFontSize));
    const QFontMetrics badgeFm(p->font());
    int badgeX = cardLeft + kLevelX;

    auto drawBadge = [&](int w, int h, int yOff, auto fn) {
        fn(QRect(badgeX, cardTop + yOff, w, h));
        badgeX += w + kNameLevelSpacing;
    };

    // Level（等级 0 也显示 Lv.0，部件不隐藏）
    drawBadge(kLevelW, kLevelBadgeH, kLevelY + offset, [&](const QRect& r) {
        p->setPen(Qt::NoPen);
        p->setBrush(QColor(kColorLevelBg));
        p->drawRoundedRect(r, 3, 3);
        p->setPen(QColor(kColorLevelText));
        p->drawText(r, Qt::AlignCenter, QStringLiteral("Lv.%1").arg(level));
    });
    const int levelBottom = kLevelY + kLevelBadgeH + offset;  // badge bottom-align reference

    // Streamer
    if (streamer) {
        const int w = badgeWidthForText(kStreamerBadgeBgPath, kStreamerBadgeH, kStreamerBadgeW, QStringLiteral("主播"), badgeFm);
        drawBadge(w, kStreamerBadgeH, levelBottom - kStreamerBadgeH, [&](const QRect& r) {
            drawIconBadge(p, r, QColor(kColorStreamerBg), QColor(kColorStreamerText), BadgeIcon::Mic,
                          QStringLiteral("主播"), kStreamerBadgeBgPath);
        });
    }
    // Official
    if (official) {
        const int w = badgeWidthForText(kOfficialBadgeBgPath, kOfficialBadgeH, kOfficialBadgeW, QStringLiteral("官方"), badgeFm);
        drawBadge(w, kOfficialBadgeH, levelBottom - kOfficialBadgeH, [&](const QRect& r) {
            drawIconBadge(p, r, QColor(kColorOfficialBg), QColor(kColorOfficialText), BadgeIcon::Star,
                          QStringLiteral("官方"), kOfficialBadgeBgPath);
        });
    }
    // Professional
    if (professional) {
        const int w = badgeWidthForText(kProfessionalBadgeBgPath, kProfessionalBadgeH, kProfessionalBadgeW, QStringLiteral("职业"), badgeFm);
        drawBadge(w, kProfessionalBadgeH, levelBottom - kProfessionalBadgeH, [&](const QRect& r) {
            drawIconBadge(p, r, QColor(kColorProfessionalBg), QColor(kColorProfessionalText), BadgeIcon::Briefcase,
                          QStringLiteral("职业"), kProfessionalBadgeBgPath);
        });
    }
    // Expert
    if (expert) {
        const int w = badgeWidthForText(kExpertBadgeBgPath, kExpertBadgeH, kExpertBadgeW, QStringLiteral("大神"), badgeFm);
        drawBadge(w, kExpertBadgeH, levelBottom - kExpertBadgeH, [&](const QRect& r) {
            drawIconBadge(p, r, QColor(kColorExpertBg), QColor(kColorExpertText), BadgeIcon::Crown,
                          QStringLiteral("大神"), kExpertBadgeBgPath);
        });
    }
}

/// \brief 徽章宽度：explicitW > 0 直接返回，否则按图片比例反算，图片不存在返回 0
int CommunityDelegate::badgeWidthFor(const QString& bgImagePath, int height, int explicitW) const {
    if (explicitW > 0) return explicitW;
    if (bgImagePath.isEmpty()) return 0;
    auto& cached = cl_badge_bg_cache_[bgImagePath];
    if (cached.isNull()) cached.load(bgImagePath);
    if (cached.isNull()) return 0;
    return qRound(static_cast<qreal>(cached.width()) * height / cached.height());
}

int CommunityDelegate::badgeWidthForText(const QString& bgPath, int height, int explicitW,
                                          const QString& label, const QFontMetrics& fm) const {
  return (std::max)(badgeWidthFor(bgPath, height, explicitW),
              fm.horizontalAdvance(label) + kBadgeIconSize + 9);
}

/// \brief 带图标徽章 — 有图片时整张图就是徽章（缩放 + 圆角裁切），无图片回退纯色 + 矢量图标 + 文字
void CommunityDelegate::drawIconBadge(QPainter* p, const QRect& r, const QColor& bg, const QColor& fg, BadgeIcon icon,
                                      const QString& text, const QString& bgImagePath) const {
    // 图片模式：整张背景图即徽章（K03 border-image，图标资源由设计提供），不画文字和图标
    if (!bgImagePath.isEmpty()) {
        auto& cached = cl_badge_bg_cache_[bgImagePath];
        if (cached.isNull()) cached.load(bgImagePath);
        if (!cached.isNull()) {
            QPainterPath clip;
            clip.addRoundedRect(QRectF(r), 3, 3);
            p->save();
            p->setClipPath(clip);
            p->drawPixmap(r, cached.scaled(r.size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
            p->restore();
            return;
        }
    }
    // 纯色模式：fallback
    p->setPen(Qt::NoPen);
    p->setBrush(bg);
    p->drawRoundedRect(r, 3, 3);

    const int iconX = r.left() + 3;
    const int iconY = r.top() + (r.height() - kBadgeIconSize) / 2;
    p->setBrush(fg);
    if (icon == BadgeIcon::Mic) {
        // 麦克风：圆头 + 柄
        p->drawEllipse(QPointF(iconX + 4, iconY + 2.5), 2.5, 2.0);
        p->drawRect(QRectF(iconX + 3, iconY + 4.5, 2, 3.5));
    } else if (icon == BadgeIcon::Crown) {  // 三尖皇冠
        QPolygonF crown;
        const qreal x = iconX, y = iconY;
        crown << QPointF(x, y + 7) << QPointF(x, y + 2) << QPointF(x + 2, y + 4) << QPointF(x + 3.5, y + 1)
              << QPointF(x + 5, y + 4) << QPointF(x + 7, y + 2) << QPointF(x + 7, y + 7);
        p->drawPolygon(crown);
    } else if (icon == BadgeIcon::Star) {  // 五角星
        QPolygonF star;
        const qreal cx = iconX + 3.5, cy = iconY + 4;
        for (int j = 0; j < 10; ++j) {
            qreal a = -M_PI_2 + j * (M_PI / 5);
            qreal r = (j % 2 == 0) ? 3.5 : 1.3;
            star << QPointF(cx + cos(a) * r, cy + sin(a) * r);
        }
        p->drawPolygon(star);
    } else {                                                           // BadgeIcon::Briefcase — 公文包
        p->drawRoundedRect(QRectF(iconX + 1, iconY + 2, 6, 5), 1, 1);  // 包体
        p->drawRect(QRectF(iconX + 3, iconY + 1, 2, 1.5));             // 提手
    }

    p->setPen(fg);
    p->drawText(QRect(r.left() + kBadgeIconSize + 5, r.top(), r.width() - kBadgeIconSize - 5, r.height()),
                Qt::AlignLeft | Qt::AlignVCenter, text);
}

/// \brief K03 widget_03 — 方案信息框：圆角深色底，71×71 图标 + 名称 + 描述 + 徽章行
/// 布局对齐 K03 CustomQWidgetPlanInfo（Large_71_71）：icon(12,12,71×71)、
/// name(95,14)、desc(95,38)、badges(95,65)
void CommunityDelegate::paintPlanInfo(QPainter* p, const QStyleOptionViewItem& opt, const QModelIndex& idx,
                                      int zone) const {
    const QRect box = planInfoRect(opt);
    // box bg: K03 rgba(0,0,0,0.45), radius 8
    p->setPen(Qt::NoPen);
    p->setBrush(QColor(0, 0, 0, 115));
    p->drawRoundedRect(box, kPlanBoxRadius, kPlanBoxRadius);

    // category icon: K03 CategoryIcon 映射表 → 资源图片，无匹配则回退文字占位
    const QRect icon = planIconRect(opt);
    const auto tags = idx.data(CommunityModel::TagsRole).toStringList();
    QString iconBase = kDefaultCategoryBase;
    for (const auto& tag : tags) {
        if (categoryIconMap().contains(tag)) {
            iconBase = categoryIconMap().value(tag);
            break;
        }
    }
    const QString iconPath = categoryIconPath(iconBase, /*selected=*/false);
    QPixmap iconPm(iconPath);
    if (!iconPm.isNull()) {
        p->drawPixmap(icon, iconPm.scaled(icon.size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
    } else {
        // 图片不存在 → 文字占位
        p->setBrush(QColor(255, 255, 255, 14));
        p->drawRoundedRect(icon, 6, 6);
        p->setPen(QColor(kColorPlanIconText));
        p->setFont(makeFont(p, kBadgeFontSize));
        const QString fallback = !tags.isEmpty() ? tags.first() : idx.data(CommunityModel::DeviceNameRole).toString();
        p->drawText(icon.adjusted(4, 0, -4, 0), Qt::AlignCenter | Qt::TextWordWrap, fallback);
    }

    // plan name: K03 14px white weight 500, elide to rect width
    const QRect nameR = planNameRect(opt);
    QFont nameFont = makeFont(p, kPlanTitleFontSize);
    nameFont.setWeight(QFont::Medium);
    p->setFont(nameFont);
    const bool hovered = cl_hover_text_enabled_ && (zone == ZonePlanName);  ///< 悬停效果受开关控制（默认关闭）
    p->setPen(QColor(hovered ? kColorAccent : kColorNameText));
    const QString name = idx.data(CommunityModel::PlanNameRole).toString();
    p->drawText(nameR, Qt::AlignLeft | Qt::AlignVCenter,
                DeSheng::elideTextWithDots(name, p->font(), nameR.width()));

    // desc: K03 10px rgba(161,168,179,0.5), 超过 5 字符截断 + "..."
    p->setFont(makeFont(p, kBadgeFontSize));
    p->setPen(QColor(161, 168, 179, 128));
    const QString desc = idx.data(CommunityModel::DescriptionRole).toString();
    const QString descText = desc.length() > 5 ? desc.left(5) + QStringLiteral("...") : desc;
    p->drawText(planDescRect(opt), Qt::AlignLeft | Qt::AlignVCenter, descText);

    paintPlanBadges(p, opt, idx);
}

/// \brief 徽章行 — 设备短名 + 场景标签（K03 TextBadgeContainer，radius 8.5）
void CommunityDelegate::paintPlanBadges(QPainter* p, const QStyleOptionViewItem& opt, const QModelIndex& idx) const {
    const QRect area = planBadgesRect(opt);
    p->setFont(makeFont(p, kBadgeFontSize));
    const QFontMetrics fm(p->font());

    int x = area.left();
    auto drawBadge = [&](const QString& text) {
        const int w = fm.horizontalAdvance(text) + kBadgeHPadding;
        if (x + w > area.right()) return;  // overflow: clip
        const QRect r(x, area.top(), w, kBadgeHeight);
        p->setPen(Qt::NoPen);
        p->setBrush(QColor(255, 255, 255, 20));
        p->drawRoundedRect(r, kBadgeHeight / 2, kBadgeHeight / 2);
        p->setPen(QColor(kColorBadgeText));
        p->drawText(r, Qt::AlignCenter, text);
        x += w + kBadgeSpacing;
    };

    const QString device = idx.data(CommunityModel::DeviceNameRole).toString();
    const auto tags = idx.data(CommunityModel::TagsRole).toStringList();
    if (!device.isEmpty()) drawBadge(device);
    if (!tags.isEmpty()) drawBadge(tags.join(QStringLiteral("+")));
}

/// \brief 懒加载缓存 pixmap / Cached pixmap loader (single source of truth)
QPixmap CommunityDelegate::cachedPixmap(const char* path) const {
    if (!path || !*path) return {};
    auto& c = cl_btn_img_cache_[path];
    if (c.isNull()) c.load(path);
    return c;
}

/// \brief K03 三态图选择：checked > hover > normal（pressed 复用 checked）
QPixmap CommunityDelegate::selectActionPixmap(const char* normal, const char* hover, const char* checked,
                                              bool isChecked, bool isHovered) const {
    if (isChecked) return cachedPixmap(checked);
    if (isHovered) return cachedPixmap(hover);
    return cachedPixmap(normal);
}

/// \brief K03 widget_04 — 6 态图 + 下载进度圆环（K03 QStackedWidget 等价）
void CommunityDelegate::paintActionBar(QPainter* p, const QStyleOptionViewItem& opt, const QModelIndex& idx,
                                       int zone) const {
    (void)zone;
    p->setFont(makeFont(p, kActionCountFontSize));

    const auto rects = visibleActionRects(opt);
    for (int i = 0; i < rects.size(); ++i) {
        const auto& [actionZone, r] = rects[i];  // r = slot rect
        const int iconSz = [&] {
            switch (actionZone) {
                case ZoneLike:     return kLikeIconSize;
                case ZoneDislike:  return kDislikeIconSize;
                case ZoneDownload: return kDownloadIconSize;
                default:           return kShareIconSize;
            }
        }();
        const bool hovered = (actionZone == zone);
        QPixmap pm;
        QString fallback;
        int count = 0;
        QRect iconR(r.left(), r.top(), iconSz, iconSz);

        switch (actionZone) {
            case ZoneLike:
                count = idx.data(CommunityModel::LikeCountRole).toInt();
                fallback = QString::fromUtf8("\xe2\x99\xa5");
                pm = selectActionPixmap(
                    kLikeNormalPath, kLikeHoverPath, kLikeCheckedPath,
                    idx.data(CommunityModel::IsLikedRole).toBool(),
                    hovered || (cl_pressed_zones_.value(idx.row(), ZoneNone) == ZoneLike));
                break;
            case ZoneDislike:
                count = idx.data(CommunityModel::DislikeCountRole).toInt();
                fallback = QString::fromUtf8("\xe2\x86\x98");
                pm = selectActionPixmap(
                    kDislikeNormalPath, kDislikeHoverPath, kDislikeCheckedPath,
                    idx.data(CommunityModel::IsDislikedRole).toBool(),
                    hovered || (cl_pressed_zones_.value(idx.row(), ZoneNone) == ZoneDislike));
                break;
            case ZoneDownload: {
                count = idx.data(CommunityModel::DownloadCountRole).toInt();
                fallback = QString::fromUtf8("\xe2\x86\x93");
                const int uid = idx.data(CommunityModel::UserIdRole).toInt();
                const int prog = cl_download_progress_.value(uid, -1);
                if (prog >= 0 && prog <= 100) {
                    const QRectF arcR = iconR.adjusted(2, 2, -2, -2);
                    p->setPen(QPen(QColor(255, 255, 255, 50), kDownloadRingWidth));
                    p->setBrush(Qt::NoBrush);
                    p->drawArc(arcR, 0, 360 * 16);
                    QPen rp(QColor(kDownloadRingColor), kDownloadRingWidth);
                    rp.setCapStyle(Qt::RoundCap);
                    p->setPen(rp);
                    p->drawArc(arcR, 90 * 16, -(std::max)(1, prog * 360 / 100) * 16);
                    p->setPen(QColor(kColorActionInactive));
                    p->drawText(QRect(iconR.right() + kActionIconTextSpacing, r.top(), kActionCountWidth, r.height()), Qt::AlignLeft | Qt::AlignVCenter,
                                tagCountText(count));
                    continue;
                }
                pm = hovered ? cachedPixmap(kDownloadHoverPath) : cachedPixmap(kDownloadNormalPath);
                break;
            }
            case ZoneShare:
                count = idx.data(CommunityModel::ShareCountRole).toInt();
                fallback = QString::fromUtf8("\xe2\x86\x97");
                pm = hovered ? cachedPixmap(kShareHoverPath) : cachedPixmap(kShareNormalPath);
                break;
            default:
                continue;
        }

        if (!pm.isNull()) {
            p->drawPixmap(iconR, pm.scaled(iconR.size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
        } else {
            p->setPen(QColor(hovered ? kColorAccent : kColorActionInactive));
            p->drawText(iconR, Qt::AlignCenter, fallback);
        }
        p->setPen(QColor(kColorActionInactive));
        p->drawText(QRect(iconR.right() + kActionIconTextSpacing, r.top(), kActionCountWidth, r.height()), Qt::AlignLeft | Qt::AlignVCenter,
                    tagCountText(count));
    }
}

/// \brief K03 widget_02 — 评论区：流式标签 + 右下角展开/收起按钮（无线框背景）
void CommunityDelegate::paintComments(QPainter* p, const QStyleOptionViewItem& opt, const QModelIndex& idx,
                                      int zone) const {
    const QRect cArea = commentsAreaRect(opt);
    const bool expanded = idx.data(CommunityModel::ExpandedRole).toBool();

    // comment tags in flow layout (K03: CustomQWidgetComments)
    paintCommentTags(p, cArea, idx);

    // K03: 无评论标签时不绘制展开按钮
    const auto comments =
        idx.data(CommunityModel::CommentsRole)
            .value<QList<DeSheng::GetPublicConfigurationListResponse::Comment>>();
    if (comments.isEmpty()) return;

    // K03: expand button — 末行标签右侧 40×20, text "展开"/"收起" + arrow 图标
    // 箭头语义与 CustomQWidgetComments 一致：展开→arrow_down，收起→arrow_up
    const QRect btn = expandBtnRect(opt, idx.row());
    const bool buttonHovered = (zone == ZoneExpand);
    const QColor buttonColor = expanded ? QColor(buttonHovered ? kColorExpandExpandedHover : kColorExpandExpanded)
                                        : QColor(buttonHovered ? kColorExpandCollapsedHover : kColorExpandCollapsed);
    p->setPen(buttonColor);
    p->setFont(makeFont(p, kExpandBtnFontSize));
    // 展开/收起 — 产品要求一直保持中文（不参与翻译）
    p->drawText(btn.adjusted(0, 0, -16, 0), Qt::AlignLeft | Qt::AlignVCenter,
                expanded ? QStringLiteral("收起") : QStringLiteral("展开"));

    // 箭头图标 — K03 GeneralIcon（8×4），hover 用 hover 变体
    const QString arrowPath = expanded
        ? (buttonHovered ? QStringLiteral(":/Skin/Images/GeneralIcon/arrow_down_hover.png")
                         : QStringLiteral(":/Skin/Images/GeneralIcon/arrow_down_normal.png"))
        : (buttonHovered ? QStringLiteral(":/Skin/Images/GeneralIcon/arrow_up_hover.png")
                         : QStringLiteral(":/Skin/Images/GeneralIcon/arrow_up_normal.png"));
    QPixmap arrowPm(arrowPath);
    if (!arrowPm.isNull()) {
        const QRect arrowR(btn.right() - 12, btn.top() + (btn.height() - 4) / 2, 8, 4);
        p->drawPixmap(arrowR, arrowPm);
    } else {
        // fallback Unicode：展开→▼，收起→▲
        p->drawText(btn.adjusted(btn.width() - 16, 0, 0, 0), Qt::AlignCenter,
                    expanded ? QString::fromUtf8("\xe2\x96\xbc") : QString::fromUtf8("\xe2\x96\xb2"));
    }
}

/// \brief 统一标签流式布局 — 收起态 ≤kMaxCollapsedTagRows 行，展开态全部行
/// 结果前缀截断（rects[i] 对应 comments[i]），sizeHint 与 paint 共用同一算法
CommunityDelegate::TagLayout CommunityDelegate::layoutCommentTags(
    const QRect& area, const QList<DeSheng::GetPublicConfigurationListResponse::Comment>& comments,
    bool expanded) const {
    TagLayout out;
    if (comments.isEmpty()) return out;

    const int rowH = kTagHeight + kTagVSpacing;
    const int maxRows = expanded ? std::numeric_limits<int>::max() : kMaxCollapsedTagRows;
    const int btnReserve = kExpandBtnW + kTagHSpacing;

    QFont f;
    f.setPixelSize(kTagFontSize);
    QFontMetrics fm(f);

    // measure: text + formatted count + padding
    QVector<int> widths;
    widths.reserve(comments.size());
    for (const auto& cm : comments) {
        widths.append(fm.horizontalAdvance(cm.comment_text + " " + tagCountText(cm.count)) + kTagHPadding);
    }

    int cx = area.left();
    int cy = area.top();
    const int areaEnd = area.left() + area.width();  // exclusive end; QRect::right() is inclusive

    for (int i = 0; i < comments.size(); ++i) {
        int curRow = (cy - area.top()) / rowH;
        if (curRow >= maxRows) break;

        // 末行（收起第 2 行 / 展开最后一行）右侧预留展开按钮空间
        const bool lastRow =
            (!expanded && curRow == kMaxCollapsedTagRows - 1) || (expanded && i == comments.size() - 1);
        const int rowEnd = lastRow ? areaEnd - btnReserve : areaEnd;

        if (cx > area.left() && cx + widths[i] > rowEnd) {
            cx = area.left();
            cy += rowH;
            curRow = (cy - area.top()) / rowH;
            if (curRow >= maxRows) break;
            // 换行后落入收起态末行 → 按预留宽度重新校验
            if (!expanded && curRow == kMaxCollapsedTagRows - 1 &&
                widths[i] > area.width() - btnReserve) break;
        }
        // 单标签比预留后行宽还宽 → 收起态直接截断
        if (!expanded && curRow == kMaxCollapsedTagRows - 1 &&
            widths[i] > area.width() - btnReserve) break;

        out.rects.append(QRect(cx, cy, widths[i], kTagHeight));
        out.ids.append(comments[i].id);
        cx += widths[i] + kTagHSpacing;
    }

    if (!out.rects.isEmpty()) {
        out.rows = (out.rects.last().top() - area.top()) / rowH + 1;
        out.contentBottom = out.rects.last().bottom() + 1;
    }
    return out;
}

/// \brief K03 CustomQWidgetComments — 绘制评论标签（布局统一走 layoutCommentTags）
void CommunityDelegate::paintCommentTags(QPainter* p, const QRect& area, const QModelIndex& idx) const {
    const int row = idx.row();
    const auto* model = qobject_cast<const CommunityModel*>(idx.model());
    if (!model) return;
    const auto opt = model->findById(idx.data(CommunityModel::UserIdRole).toInt());
    if (!opt.has_value() || opt->comments.isEmpty()) {
        // 清空缓存 — 防止 row 复用后命中上一轮的标签/按钮矩形
        cl_tag_rects_.remove(row);
        cl_tag_ids_.remove(row);
        cl_tag_content_bottom_.remove(row);
        return;
    }

    const auto& comments = opt->comments;
    const bool expanded = idx.data(CommunityModel::ExpandedRole).toBool();
    const auto layout = layoutCommentTags(area, comments, expanded);

    // cache for hit-testing in editorEvent + expand button alignment
    cl_tag_rects_[row] = layout.rects;
    cl_tag_ids_[row] = layout.ids;
    cl_tag_content_bottom_[row] = layout.contentBottom;

    // draw — rects 为前缀截断，rects[i] 对应 comments[i]
    p->setFont(makeFont(p, kTagFontSize));
    for (int i = 0; i < layout.rects.size(); ++i) {
        const QRect& r = layout.rects[i];
        const auto& cm = comments[i];
        const bool selected = cm.is_clicked;
        p->setPen(Qt::NoPen);
        p->setBrush(QColor(selected ? kColorAccent : kColorTagNormal));
        p->drawRoundedRect(r, kTagHeight / 2, kTagHeight / 2);
        p->setPen(QColor(selected ? kColorTagSelectedText : kColorTagNormalText));
        p->drawText(r, Qt::AlignCenter, cm.comment_text + " " + tagCountText(cm.count));
    }
}

/// \brief 命中测试评论标签 — 遍历缓存矩形，返回 tag ID，未命中返回 -1
int CommunityDelegate::commentTagHitTest(const QPoint& pos, int row) const {
    const auto& rects = cl_tag_rects_.value(row);
    for (int i = 0; i < rects.size(); ++i) {
        if (rects[i].contains(pos)) {
            const auto& ids = cl_tag_ids_.value(row);
            return (i < ids.size()) ? ids[i] : -1;
        }
    }
    return -1;
}

// ── editorEvent ──

/// \brief 事件处理 — MouseMove 更新 hover zone，MouseRelease 分发到各区域/标签
bool CommunityDelegate::editorEvent(QEvent* event, QAbstractItemModel* model, const QStyleOptionViewItem& option,
                                    const QModelIndex& index) {
    const int row = index.row();
    // ── MouseMove: track hover + pressed zone per row ──
    if (event->type() == QEvent::MouseMove) {
        auto* me = static_cast<QMouseEvent*>(event);
        const auto oldHover = cl_hover_zones_.value(row, ZoneNone);
        const auto cur = hitTest(me->pos(), option);
        if (cur != oldHover) {
            if (cur == ZoneNone)
                cl_hover_zones_.remove(row);
            else
                cl_hover_zones_[row] = cur;
        }
        // 左键按住时追踪 pressed zone（6 态图 pressed 状态）
        if (me->buttons() & Qt::LeftButton)
            cl_pressed_zones_[row] = cur;
        else
            cl_pressed_zones_.remove(row);
        return (cur != oldHover);
    }
    // ── MouseButtonRelease: clear pressed, dispatch click ──
    if (event->type() == QEvent::MouseButtonRelease) {
        cl_pressed_zones_.remove(row);
        auto* me = static_cast<QMouseEvent*>(event);
        const auto zone = hitTest(me->pos(), option);
        const int userId = model->data(index, CommunityModel::UserIdRole).toInt();
        LOG_DEBUG("[Delegate] click row:{} userId:{} zone:{}", index.row(), userId,
                  static_cast<int>(zone));

        // K03: comment tag click — optimistic toggle + signal
        const int tagId = commentTagHitTest(me->pos(), index.row());
        if (tagId > 0) {
            auto* cm = qobject_cast<CommunityModel*>(model);
            if (cm) {
                auto itemOpt = cm->findById(userId);
                if (itemOpt.has_value()) {
                    for (auto& c : itemOpt->comments) {
                        if (c.id == tagId) {
                            c.is_clicked = !c.is_clicked;
                            c.count += c.is_clicked ? 1 : -1;
                            cm->setField(userId, CommunityModel::CommentsRole, QVariant::fromValue(itemOpt->comments));
                            emit commentTagClicked(userId, tagId, c.is_clicked);
                            return true;
                        }
                    }
                }
            }
        }

        // ── Zone dispatch (priority below comment tags) ──
        if (zone == ZoneExpand) {
            // 无评论时展开无意义（按钮未绘制，拦截命中）
            const auto comments =
                model->data(index, CommunityModel::CommentsRole)
                    .value<QList<DeSheng::GetPublicConfigurationListResponse::Comment>>();
            if (!comments.isEmpty()) emit iconClicked(userId);
            return true;
        }
        if (zone == ZoneLike) {
            emit likeClicked(userId, !model->data(index, CommunityModel::IsLikedRole).toBool());
            return true;
        }
        if (zone == ZoneDislike) {
            emit dislikeClicked(userId, !model->data(index, CommunityModel::IsDislikedRole).toBool());
            return true;
        }
        if (zone == ZoneDownload) {
            emit downloadClicked(userId);
            return true;
        }
        if (zone == ZoneShare) {
            emit shareClicked(userId);
            return true;
        }
        if (zone == ZoneMore && cl_more_visible_) {
            // 传更多按钮的全局矩形（视口坐标 → 屏幕坐标），panel 据此锚定菜单位置
            const QRect btnRect = moreRect(option);
            const QPoint btnTopLeft = option.widget
                ? option.widget->mapToGlobal(btnRect.topLeft())
                : me->globalPos() - QPoint(btnRect.width() / 2, btnRect.height() / 2);
            emit moreClicked(userId, QRect(btnTopLeft, btnRect.size()));
            return true;
        }
        if (zone == ZoneAvatar) {
            emit avatarClicked(userId);
            return true;
        }
    }
    return QStyledItemDelegate::editorEvent(event, model, option, index);
}

// ── hitTest ──

/// \brief 区域命中检测 — 按优先级：expand > more > 可见操作按钮 > planName > name > avatar
CommunityDelegate::HoverZone CommunityDelegate::hitTest(const QPoint& pos, const QStyleOptionViewItem& opt) const {
    if (expandBtnRect(opt, opt.index.row()).contains(pos)) return ZoneExpand;
    if (cl_more_visible_ && moreRect(opt).contains(pos)) return ZoneMore;
    for (const auto& [zone, r] : visibleActionRects(opt)) {
        if (r.contains(pos)) return static_cast<HoverZone>(zone);
    }
    if (planNameRect(opt).contains(pos)) return ZonePlanName;
    if (nameRect(opt).contains(pos)) return ZoneName;
    if (avatarRect(opt).contains(pos)) return ZoneAvatar;
    return ZoneNone;
}

// ── Rect methods (K03-aligned: 330×300 card with widget_01/02/03/04 zones) ──

QRect CommunityDelegate::avatarRect(const QStyleOptionViewItem& opt) const {
    // K03: avatar at left=28, top=23, size 40×40
    return {opt.rect.left() + kCardMargin + kAvatarLeftMargin,
            opt.rect.top() + kCardMargin + kAvatarTopMargin + pinnedOffset(opt), kAvatarSize, kAvatarSize};
}

QRect CommunityDelegate::nameRect(const QStyleOptionViewItem& opt) const {
    const int cardLeft = opt.rect.left() + kCardMargin;
    const int cardTop = opt.rect.top() + kCardMargin;
    const int contentW = opt.rect.width() - 2 * kCardMargin;
    // 右边界避让：眼睛可见 → 距眼睛 8px；否则 more 可见 → 距 more 12px；都无 → 常规右边距
    const int rightEdge = [&] {
        if (eyeVisible(opt)) return eyeBtnRect(opt).left() - kNameEyeGap;
        if (cl_more_visible_) return moreRect(opt).left() - kNameMoreGap;
        return cardLeft + contentW - kNameRightMargin;
    }();
    return {cardLeft + kNameLeftMargin, cardTop + kNameTop + pinnedOffset(opt),
            rightEdge - cardLeft - kNameLeftMargin, kNameHeight};
}

QRect CommunityDelegate::moreRect(const QStyleOptionViewItem& opt) const {
    // K03: more button (actionBtn) at far right, 20×20, right margin 24
    return {opt.rect.right() - kCardMargin - kMoreRightMargin - kMoreSize,
            opt.rect.top() + kCardMargin + kAvatarTopMargin + (kAvatarSize - kMoreSize) / 2 + pinnedOffset(opt),
            kMoreSize, kMoreSize};
}

// K03 widget_02: comments area (min 46px, expands to fill extra height)
QRect CommunityDelegate::commentsAreaRect(const QStyleOptionViewItem& opt) const {
    const int offset = pinnedOffset(opt);
    const int y = opt.rect.top() + kCardMargin + kAvatarTopMargin + kTopInfoHeight + kSpacerAfterTopInfo + offset;
    // stretches down to plan info top, minus spacers and plan info height
    const int maxH = opt.rect.height() - 2 * kCardMargin - kAvatarTopMargin - kTopInfoHeight -
                     kSpacerAfterTopInfo - kSpacerAfterComments - kPlanInfoHeight -
                     kSpacerAfterPlanInfo - kActionBarHeight - kBottomSpacing - offset;
    return {opt.rect.left() + kCardMargin + kCommentHMargin, y,
            opt.rect.width() - 2 * kCardMargin - kCommentHMargin * 2,
            (std::max)(kCommentAreaMinHeight, maxH)};
}

QRect CommunityDelegate::expandBtnRect(const QStyleOptionViewItem& opt, int row) const {
    const QRect c = commentsAreaRect(opt);
    // K03: 按钮与最后一行标签同排（底部对齐），而非钉在评论区底部
    const int contentBottom = cl_tag_content_bottom_.value(row, c.bottom());
    return {c.right() - kExpandBtnW, contentBottom - kExpandBtnH, kExpandBtnW, kExpandBtnH};
}

// K03 widget_03: plan info box (rounded dark box, 95px, margins 22 L/R)
QRect CommunityDelegate::planInfoRect(const QStyleOptionViewItem& opt) const {
    const QRect c = commentsAreaRect(opt);
    const int y = c.bottom() + kSpacerAfterComments;
    return {opt.rect.left() + kCardMargin + kPlanInfoLeftMargin, y,
            opt.rect.width() - 2 * kCardMargin - kPlanInfoLeftMargin * 2, kPlanInfoHeight};
}

// K03 CustomQWidgetPlanInfo: icon at box(12,12), 71×71
QRect CommunityDelegate::planIconRect(const QStyleOptionViewItem& opt) const {
    const QRect box = planInfoRect(opt);
    return {box.left() + kPlanIconMargin, box.top() + kPlanIconMargin, kPlanIconSize, kPlanIconSize};
}

// K03: plan name at box(95,14), 14px white, height 20
QRect CommunityDelegate::planNameRect(const QStyleOptionViewItem& opt) const {
    const QRect box = planInfoRect(opt);
    return {box.left() + kPlanTextX, box.top() + kPlanNameY, box.width() - kPlanTextX - kPlanTextRight,
            kPlanNameHeight};
}

// K03: desc at box(95,38), 10px gray, height 14
QRect CommunityDelegate::planDescRect(const QStyleOptionViewItem& opt) const {
    const QRect box = planInfoRect(opt);
    return {box.left() + kPlanTextX, box.top() + kPlanDescY, box.width() - kPlanTextX - kPlanTextRight,
            kPlanDescHeight};
}

// K03: badge container at box(95,65), height 16
QRect CommunityDelegate::planBadgesRect(const QStyleOptionViewItem& opt) const {
    const QRect box = planInfoRect(opt);
    return {box.left() + kPlanTextX, box.top() + kPlanBadgesY, box.width() - kPlanTextX - kPlanTextRight, kBadgeHeight};
}

// K03 widget_04: action bar (bottom, 14px, left/right margin 37)
QRect CommunityDelegate::actionBarRect(const QStyleOptionViewItem& opt) const {
    const QRect box = planInfoRect(opt);
    const int y = box.bottom() + kSpacerAfterPlanInfo;
    return {opt.rect.left() + kCardMargin + kActionBarMargin, y,
            opt.rect.width() - 2 * kCardMargin - kActionBarMargin * 2, kActionBarHeight};
}

/// \brief 四等分 slot 边界 — 图标位置由 paintActionBar 按实际计数动态计算
QVector<QPair<int, QRect>> CommunityDelegate::visibleActionRects(const QStyleOptionViewItem& opt) const {
    static constexpr int zones[] = {ZoneLike, ZoneDislike, ZoneDownload, ZoneShare};
    QVector<QPair<int, QRect>> out;
    const QRect bar = actionBarRect(opt);
    int visibleCount = 0;
    for (int btn = ActionLike; btn <= ActionShare; ++btn) {
        if (actionVisible(static_cast<ActionButton>(btn))) ++visibleCount;
    }
    if (visibleCount == 0) return out;
    static constexpr int iconSizes[] = {kLikeIconSize, kDislikeIconSize, kDownloadIconSize,
                                        kShareIconSize};
    // 每组 = 图标 + 间隔 + 计数（64px），间隙均分，余数放分享前
    int totalGroupsW = 0;
    for (int btn = ActionLike; btn <= ActionShare; ++btn) {
        if (actionVisible(static_cast<ActionButton>(btn)))
            totalGroupsW += iconSizes[btn] + kActionIconTextSpacing + kActionCountWidth;
    }
    const int gapSpace = (std::max)(0, bar.width() - totalGroupsW);
    const int gap = visibleCount > 1 ? gapSpace / (visibleCount - 1) : 0;
    const int extra = visibleCount > 1 ? gapSpace % (visibleCount - 1) : 0;
    int x = bar.left();
    for (int i = 0, btn = ActionLike; btn <= ActionShare; ++btn) {
        if (!actionVisible(static_cast<ActionButton>(btn))) continue;
        const int iw = iconSizes[btn];
        // 每个 group 的整体宽度（图标 + 间隔 + 计数），图标定位在 group 左侧
        const int gw = iw + kActionIconTextSpacing + kActionCountWidth;
        // 余数放在倒数第二个 gap（即分享整体前）
        const int g = gap + ((i == visibleCount - 2) ? extra : 0);
        out.append({zones[btn], QRect(x, bar.top(), iw, bar.height())});
        x += gw + g;
        ++i;
    }
    return out;
}
