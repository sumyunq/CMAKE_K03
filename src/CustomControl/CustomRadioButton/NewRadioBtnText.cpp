#include "CustomControl/CustomRadioButton/NewRadioBtnText.h"
#include "modules/Common/elide_text.h"  ///< DeSheng::elideTextWithDots


NewRadioBtnText::NewRadioBtnText(QWidget *parent)
    : QRadioButton(parent), indicatorText("")
{
    m_baseName = "";
    m_Themeidx = 0;//主题
    m_PanelTransparency = 0.2;//透明度

}

void NewRadioBtnText::setIndicatorText(const QString &text,const QString &label)
{
    // 1. 定义标签到图片基础名的映射（不区分大小写，统一用小写）
    static const QHash<QString, QString> nameMap = {
        { "游戏",      "game" },
        { "电影",      "movie" },
        { "音乐",      "music" },
        { "三角洲行动", "delta" },
        { "pubg",     "pubg" },      // 英文原样
        { "csgo",     "csgo" },
        { "无畏契约",   "valorant" },
        { "暗区突围",   "AB" },
        { "apex",     "apex" },
        { "穿越火线",   "CF" }
    };
    m_baseName = nameMap.value(label.toLower(), "default"); // 找不到时用 "default"

    updateStyleSheet();

    indicatorText = text;
    update(); // 触发重绘
    emit SetITextSignal(text,label);//发给其他界面的当前方案
}
void NewRadioBtnText::setThemeAndPanelTransparency(int idx,int PValue)
{
    double PanelTransparency = PValue / 100.0;   //面板透明度(默认值是0.2)
    if ((m_PanelTransparency != PanelTransparency) || (m_Themeidx != idx)) {
        m_PanelTransparency = PanelTransparency;
        m_Themeidx = idx;
        updateStyleSheet();
    }
}
void NewRadioBtnText::updateStyleSheet()
{
    QString suffix;
    switch (m_Themeidx)
    {
    case 0: suffix = ""/*"_darkBlue"*/; break;//深蓝色（还未修改主题图片）
    case 1: suffix = "_white";  break;//白色
    case 2: suffix = "_black";  break;//黑色
    default: suffix = "";      break;
    }
    int r, g, b;
    QString textColor;
    switch (m_Themeidx) {
    case 0:
        r = 81; g = 96; b = 122;
        textColor = "#A1A8B3";
        break;// 深蓝色
    case 1:
        r = 81; g = 96; b = 122;
        textColor = "#A1A8B3";
        break;// 白色
    case 2:
        r = 81; g = 96; b = 122;
        textColor = "#A1A8B3";
        break; // 黑色
    default:
        r = 81; g = 96; b = 122;
        textColor = "#A1A8B3";
        break;
    }

    QString colorStr = QString("rgba(%1, %2, %3, %4)")
                           .arg(r).arg(g).arg(b).arg(m_PanelTransparency);

    QString style = QString(
                        "QRadioButton {"
                        "  text-align: left;"
                        "  background-color: %2;"
                        "  color: %3;"
                        "  border-radius: 10px;"
                        "  font-family: \"Noto Sans S Chinese\";"
                        "   font-width:500;"
                        "  font-size: 14px;"
                        "  spacing: -5px;"
                        "}"
                        "QRadioButton:checked {"
                        "}"
                        "QRadioButton::indicator {"
                        "  height: 58px;"
                        "  width: 58px;"
                        "  border-image: url(:/Skin/Images/Headphones/CurrentPlanRadio/c-%1-no%4.png);"
                        "  margin-right: 17px;"
                        "  margin-left: 25px;"
                        "}"
                        ).arg(m_baseName).arg(colorStr).arg(textColor).arg(suffix);

    setStyleSheet(style);
}

QString NewRadioBtnText::getIndicatorText()
{
    return indicatorText;
}

void NewRadioBtnText::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

   /* QStylePainter painter(this);
    QStyleOptionButton option;
    initStyleOption(&option);
    // 绘制默认的 QRadioButton（包括背景和文本标签）
    painter.drawControl(QStyle::CE_RadioButton, option);

    // 获取指示器矩形区域
    QRect indicatorRect = style()->subElementRect(
        QStyle::SE_RadioButtonIndicator,
        &option,
        this
        );

    // 绘制指示器内部文字（左下角）
    if (!indicatorText.isEmpty()) {
        painter.save(); // 保存当前画笔状态

        // 设置文本风格
        QFont font = painter.font();
        font.setPointSize(8); // 设置较小的字体
        painter.setFont(font);
        painter.setPen(palette().buttonText().color());

        // 计算文本位置（左下角），使用QTextOption设置对齐
        QTextOption textOption;
        textOption.setAlignment(Qt::AlignLeft | Qt::AlignBottom);

        // 调整文本位置（添加边距）
        QRect textRect = indicatorRect.adjusted(30, 0, -4, -4);

        // 绘制文本
        painter.drawText(textRect, indicatorText, textOption);

        painter.restore(); // 恢复画笔状态
    }*/

    QStylePainter painter(this);
    QStyleOptionButton option;
    initStyleOption(&option);
    // 绘制默认的 QRadioButton（包括背景和文本标签）
    painter.drawControl(QStyle::CE_RadioButton, option);

    // 获取指示器矩形区域
    QRect indicatorRect = style()->subElementRect(
        QStyle::SE_RadioButtonIndicator,
        &option,
        this
        );
    if (!indicatorText.isEmpty()) {
        painter.save();

        // 设置文本风格
        //QFont font = painter.font();
        //font.setPointSize(8);
        QFont font("Noto Sans S Chinese");
        font.setPixelSize(10); // 明确的像素大小

        painter.setFont(font);
        painter.setPen(palette().buttonText().color());

        // 计算文本位置
        //QRect textRect = indicatorRect.adjusted(38, 0, -4, -4);
        QRect textRect = indicatorRect.adjusted(30, 0, 0, -7);

        // 关键修改：添加文本省略
        QFontMetrics fm(font);
        QString elidedText = DeSheng::elideTextWithDots(
            indicatorText,               // 原始文本
            font,                        // 与 QFontMetrics fm 同源
            textRect.width()/6*5-4            // 使用当前矩形宽度
            );

        // 绘制文本（使用原有位置和对齐方式）
        QTextOption textOption;
        textOption.setAlignment(Qt::AlignLeft | Qt::AlignBottom);
        painter.drawText(textRect, elidedText, textOption); // 使用省略后的文本

        setToolTip(elidedText == indicatorText ? "" : indicatorText);
        setProperty("fullText", indicatorText);

        painter.restore();
    }
}
