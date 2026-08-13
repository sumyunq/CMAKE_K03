#include "modules/CommunityModule/ui/ranking/ranking_delegate.h"

#include <QAbstractItemView>
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>

#include "modules/CommunityModule/ui/ranking/ranking_model.h"
#include "modules/Common/elide_text.h"  ///< DeSheng::elideTextWithDots

namespace {
const QRect kRankGeometry(15, 16, 23, 23);        ///< 排名色块
const QRect kAvatarGeometry(52, 8, 39, 39);       ///< 头像
const QRect kPlanGeometry(101, 10, 126, 20);      ///< 方案名
const QRect kUsernameGeometry(101, 33, 100, 14);  ///< 用户名
const QRect kHeatGeometry(316, 19, 30, 17);       ///< 热度
const QRect kLikeGeometry(294, 20, 14, 14);       ///< 点赞/下载按钮

const QColor kColorPlanText(0xD7, 0xE2, 0xE8);       ///< 方案名 #D7E2E8
const QColor kColorPlanHover(0x7F, 0xD4, 0xFF);      ///< 方案名 hover #7FD4FF
const QColor kColorUsername(0x6D, 0x8A, 0xA2);       ///< 用户名 #6D8AA2
const QColor kColorUsernameHover(0x67, 0xAC, 0xCF);  ///< 用户名 hover #67ACCF
const QColor kColorRank1(0xFF, 0x32, 0x32);          ///< 第 1 名 #FF3232
const QColor kColorRank2(0xFF, 0x98, 0x2A);          ///< 第 2 名 #FF982A
const QColor kColorRank3(0xF7, 0xE8, 0x4A);          ///< 第 3 名 #F7E84A
const QColor kColorRank4(0xA1, 0xA8, 0xB3);          ///< 第 4 名起 #A1A8B3
const QColor kColorRowHover(223, 243, 255, 51);      ///< 行 hover rgba(223,243,255,0.2)

const QString kDefaultAvatar(":/Skin/Images/system/system_avatar/system_avatar_2x_01.png");
const QString kAvatarHoverMask(":/Skin/Images/Community/Avatar-ho.png");
const QString kImageBase(":/Skin/Images/Community/");

/// \brief 资源 pixmap 静态缓存（滚动重绘高频，避免每次 paint 重新解码文件）
const QPixmap& cachedPixmap(const QString& path) {
    static QHash<QString, QPixmap> t_cache;
    const auto t_it = t_cache.constFind(path);
    if (t_it != t_cache.constEnd()) return t_it.value();
    t_cache.insert(path, QPixmap(path));
    return t_cache[path];
}
}  // namespace

RankingDelegate::RankingDelegate(QObject* parent) : QStyledItemDelegate(parent) {}

void RankingDelegate::setRankingType(int type) { cl_ranking_type_ = type; }

QFont RankingDelegate::makeFont(int pointSize, int weight) {
    QFont t_font(QStringLiteral("Noto Sans S Chinese"), pointSize);
    t_font.setWeight(QFont::Weight(weight));
    return t_font;
}

QColor RankingDelegate::rankColor(int rank) {
    switch (rank) {
        case 1:
            return kColorRank1;
        case 2:
            return kColorRank2;
        case 3:
            return kColorRank3;
        default:
            return kColorRank4;
    }
}

QString RankingDelegate::formatHeat(int count) {
    if (count < 10000) return QString::number(count);
    const int t_rounded = qRound(count / 10000.0 * 10);
    if (t_rounded % 10 == 0) return QString("%1w").arg(t_rounded / 10);
    return QString("%1.%2w").arg(t_rounded / 10).arg(t_rounded % 10);
}

QRect RankingDelegate::rankRect(const QRect& row) const {
    return kRankGeometry.translated(row.topLeft());
}

QRect RankingDelegate::avatarRect(const QRect& row) const {
    return kAvatarGeometry.translated(row.topLeft());
}

QRect RankingDelegate::planRect(const QRect& row) const {
    return kPlanGeometry.translated(row.topLeft());
}

QRect RankingDelegate::usernameRect(const QRect& row) const {
    return kUsernameGeometry.translated(row.topLeft());
}

QRect RankingDelegate::heatRect(const QRect& row) const {
    return kHeatGeometry.translated(row.topLeft());
}

QRect RankingDelegate::likeRect(const QRect& row) const {
    return kLikeGeometry.translated(row.topLeft());
}

QPoint RankingDelegate::cursorInItem(const QStyleOptionViewItem& option) const {
    if (const auto* t_view = qobject_cast<const QAbstractItemView*>(option.widget))
        return t_view->viewport()->mapFromGlobal(QCursor::pos());
    return QPoint();
}

QString RankingDelegate::likeIconPath(int rank, bool liked, bool hovered) const {
    if (cl_ranking_type_ == 0) {  // 点赞榜
        if (rank <= 3) {
            if (liked)
                return kImageBase +
                       QString("Like-No%1-ch-%2.png").arg(rank).arg(hovered ? "ho" : "no");
            return kImageBase +
                   QString("Like-No%1-dis-%2.png").arg(rank).arg(hovered ? "ho" : "no");
        }
        if (liked) return kImageBase + QString("Like-ch-%1.png").arg(hovered ? "ho" : "no");
        return kImageBase + QString("Like-dis-%1.png").arg(hovered ? "ho" : "no");
    }
    // 下载榜（DL 系列无 checked 态，忠实原样式）
    if (rank <= 3)
        return kImageBase + QString("DL-No%1-%2.png").arg(rank).arg(hovered ? "ho" : "no");
    return kImageBase + QString("DL-%1.png").arg(hovered ? "ho" : "no");
}

QSize RankingDelegate::sizeHint(const QStyleOptionViewItem&, const QModelIndex&) const {
    return {kRowWidth, kRowHeight};
}

void RankingDelegate::paint(QPainter* p, const QStyleOptionViewItem& opt,
                            const QModelIndex& idx) const {
    p->save();
    p->setRenderHint(QPainter::Antialiasing);  // 透明背景上抗锯齿（否则圆角/圆形边缘呈锯齿颗粒，与社区委托一致）

    const QRect t_row = opt.rect;
    const int t_rank = idx.data(static_cast<int>(RankingModel::RankingRole::RankRole)).toInt();
    const bool t_hovered = (opt.state & QStyle::State_MouseOver);

    // 背景：前三名渐变底图，其余透明 + hover 高亮
    if (t_rank <= 3) {
        p->drawPixmap(t_row, cachedPixmap(kImageBase + QString("No%1-bk.png").arg(t_rank)));
    } else if (t_hovered) {
        p->setPen(Qt::NoPen);
        p->setBrush(kColorRowHover);
        p->drawRoundedRect(t_row, 4, 4);
    }

    // 排名色块 + 数字
    const QRect t_rank_r = rankRect(t_row);
    p->setPen(Qt::NoPen);
    p->setBrush(rankColor(t_rank));
    p->drawRoundedRect(t_rank_r, 4, 4);
    p->setPen(Qt::white);
    p->setFont(makeFont(14, QFont::Medium));
    p->drawText(t_rank_r, Qt::AlignCenter, QString::number(t_rank));

    // 头像（圆形裁剪；无头像用默认图；hover 遮罩）
    const QRect t_avatar_r = avatarRect(t_row);
    const QPixmap t_avatar =
        idx.data(static_cast<int>(RankingModel::RankingRole::AvatarRole)).value<QPixmap>();
    const QPixmap& t_source = t_avatar.isNull() ? cachedPixmap(kDefaultAvatar) : t_avatar;
    p->save();
    QPainterPath t_path;
    t_path.addEllipse(t_avatar_r);
    p->setClipPath(t_path);
    p->drawPixmap(t_avatar_r, t_source.scaled(t_avatar_r.size(), Qt::KeepAspectRatioByExpanding,
                                              Qt::SmoothTransformation));
    p->restore();
    const QPoint t_cursor = cursorInItem(opt);
    if (t_hovered && avatarRect(t_row).contains(t_cursor))
        p->drawPixmap(t_avatar_r, cachedPixmap(kAvatarHoverMask));

    // 方案名（14px #D7E2E8，hover #7FD4FF，超宽省略）
    const QString t_plan =
        idx.data(static_cast<int>(RankingModel::RankingRole::PlanNameRole)).toString();
    const bool t_plan_hovered = t_hovered && planRect(t_row).contains(t_cursor);
    const QRect t_plan_r = planRect(t_row);
    p->setFont(makeFont(14, QFont::Medium));
    const QFontMetrics t_plan_fm(p->font());
    p->setPen(t_plan_hovered ? kColorPlanHover : kColorPlanText);
    p->drawText(t_plan_r, Qt::AlignLeft | Qt::AlignVCenter,
                DeSheng::elideTextWithDots(t_plan, p->font(), t_plan_r.width()));

    // 用户名（10px #6D8AA2，hover #67ACCF）
    const QString t_user =
        idx.data(static_cast<int>(RankingModel::RankingRole::UserNameRole)).toString();
    const bool t_user_hovered = t_hovered && usernameRect(t_row).contains(t_cursor);
    const QRect t_user_r = usernameRect(t_row);
    p->setFont(makeFont(10, QFont::Medium));
    const QFontMetrics t_user_fm(p->font());
    p->setPen(t_user_hovered ? kColorUsernameHover : kColorUsername);
    p->drawText(t_user_r, Qt::AlignLeft | Qt::AlignVCenter,
                DeSheng::elideTextWithDots(t_user, p->font(), t_user_r.width()));

    // 热度（12px，色随排名）
    const int t_heat = idx.data(static_cast<int>(RankingModel::RankingRole::HeatCountRole)).toInt();
    p->setFont(makeFont(12, QFont::Medium));
    p->setPen(rankColor(t_rank));
    p->drawText(heatRect(t_row), Qt::AlignLeft | Qt::AlignVCenter, formatHeat(t_heat));

    // 点赞/下载按钮（下载中：半透明置灰 + 无 hover 变体）
    const bool t_liked =
        idx.data(static_cast<int>(RankingModel::RankingRole::IsLikedRole)).toBool();
    const bool t_downloading =
        idx.data(static_cast<int>(RankingModel::RankingRole::DownloadingRole)).toBool();
    const int t_progress =
        idx.data(static_cast<int>(RankingModel::RankingRole::DownloadProgressRole)).toInt();
    const bool t_like_hovered = t_hovered && !t_downloading && likeRect(t_row).contains(t_cursor);
    const QRect t_like_r = likeRect(t_row);
    if (t_downloading) {
        // 下载中：排名色圆环进度（替代图标，与社区圆环一致）
        const QColor t_rank_color = rankColor(t_rank);
        p->setPen(QPen(QColor(t_rank_color.red(), t_rank_color.green(), t_rank_color.blue(), 80), 2));
        p->setBrush(Qt::NoBrush);
        p->drawEllipse(t_like_r.adjusted(1, 1, -2, -2));
        const int t_span = qBound(0, t_progress, 100) * 360 / 100;
        p->setPen(QPen(t_rank_color, 2));
        p->drawArc(t_like_r.adjusted(1, 1, -2, -2), 90 * 16, -t_span * 16);  // 顶部起顺时针
    } else {
        const QString t_icon = likeIconPath(t_rank, t_liked, t_like_hovered);
        if (!t_icon.isEmpty())
            p->drawPixmap(t_like_r, cachedPixmap(t_icon));
    }

    p->restore();
}

bool RankingDelegate::editorEvent(QEvent* event, QAbstractItemModel* model,
                                  const QStyleOptionViewItem& option, const QModelIndex& index) {
    if (event->type() != QEvent::MouseButtonRelease)
        return QStyledItemDelegate::editorEvent(event, model, option, index);
    auto* t_mouse = static_cast<QMouseEvent*>(event);
    if (t_mouse->button() != Qt::LeftButton)
        return QStyledItemDelegate::editorEvent(event, model, option, index);

    const QPoint t_pos = t_mouse->pos();
    if (likeRect(option.rect).contains(t_pos)) {
        emit buttonClicked(index);
        return true;
    }
    if (planRect(option.rect).contains(t_pos)) {
        emit planNameClicked(index, t_pos);
        return true;
    }
    return QStyledItemDelegate::editorEvent(event, model, option, index);
}
