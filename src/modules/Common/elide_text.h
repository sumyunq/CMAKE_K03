#ifndef ELIDE_TEXT_H
#define ELIDE_TEXT_H

#include <QFont>
#include <QFontMetrics>
#include <QString>

namespace DeSheng {

/// \brief 省略号处理：超宽文本尾部截断 + ASCII "..."（U+2026 省略号在多数中文字体中垂直居中，
/// 改用基线对齐的三个点）/ elide text with baseline-aligned dots
inline QString elideTextWithDots(const QString& text, const QFont& font, int maxWidth) {
  const QFontMetrics t_fm(font);
  if (t_fm.horizontalAdvance(text) <= maxWidth) return text;
  const QString t_dots = QStringLiteral("...");
  const int t_dots_w = t_fm.horizontalAdvance(t_dots);
  QString t_out = text;
  while (t_fm.horizontalAdvance(t_out) + t_dots_w > maxWidth) t_out.chop(1);
  return t_out + t_dots;
}

}  // namespace DeSheng

#endif  // ELIDE_TEXT_H
