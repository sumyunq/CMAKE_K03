#include "EQCurve/EditPanelTip.h"
#include "qaction.h"
#include "qdebug.h"
#include "qgraphicseffect.h"
#include "qscreen.h"
#include "ui_EditPanelTip.h"

double m_previousValidValue = 0.5;
double m_previousValidGain = 0.0;

EditPanelTip::EditPanelTip(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::EditPanelTip)
{
    ui->setupUi(this);
    // setWindowFlags(Qt::ToolTip | Qt::FramelessWindowHint); // 无边框、置顶、不获取焦点
    setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
    setAttribute(Qt::WA_StyledBackground, true);   // 关键：让样式表背景生效
    setAttribute(Qt::WA_ShowWithoutActivating);

    // 让面板接受鼠标事件，以便用户点击编辑框
    setMouseTracking(true);

    // 设置样式表
    ui->pBt_filter->setIcon(QIcon(":/Skin/Images/Headphones/Filter/HighPass-no.png"));
    ui->pBt_filter->setText("High Shelving");

    // 样式表只包含视觉属性
    ui->pBt_filter->setStyleSheet(
        "QToolButton {"
        "   color: #A1A8B3;"
        "   font-family: \"Noto Sans S Chinese\";"
        "   font-weight: 500;"
        "   font-size: 10px;"
        "}"
        );
    //调整间距
    ui->pBt_filter->setIconLeftMargin(7);
    ui->pBt_filter->setTextSpacing(16);

    // 添加阴影
    QGraphicsDropShadowEffect *shadow = new QGraphicsDropShadowEffect(ui->frame);
    shadow->setBlurRadius(10);                 // 模糊半径 20px
    shadow->setXOffset(0);                     // 水平偏移 0
    shadow->setYOffset(0);                     // 垂直偏移 0
    shadow->setColor(QColor(0, 0, 0, 77));    // 黑色半透明 rgba(0,0,0,0.3)
    ui->frame->setGraphicsEffect(shadow);

    //滤波器弹窗
    filterPopup = new FilterPopupWidget(this);
    filterPopup->setModal(false);

    connect(filterPopup,&FilterPopupWidget::SwitchFilter,this,[this](int id){
        switch(id)
        {
        case 0:
            ui->pBt_filter->setIcon(QIcon(":/Skin/Images/Headphones/Filter/LowPass-no.png"));
            ui->pBt_filter->setText("Low Pass");
            emit filterChanged(m_bandIndex,2);
            break;
        case 1:
            ui->pBt_filter->setIcon(QIcon(":/Skin/Images/Headphones/Filter/HighPass-no.png"));
            ui->pBt_filter->setText("High Pass");
            emit filterChanged(m_bandIndex,1);
            break;
        case 2:
            ui->pBt_filter->setIcon(QIcon(":/Skin/Images/Headphones/Filter/NotchFilter-no.png"));
            ui->pBt_filter->setText("Notch Filter");
            emit filterChanged(m_bandIndex,5);
            break;
        case 3:
            ui->pBt_filter->setIcon(QIcon(":/Skin/Images/Headphones/Filter/PeakingEQ-no.png"));
            ui->pBt_filter->setText("Peaking EQ");
            emit filterChanged(m_bandIndex,0);
            break;
        case 4:
            ui->pBt_filter->setIcon(QIcon(":/Skin/Images/Headphones/Filter/LowShelving-no.png"));
            ui->pBt_filter->setText("Low Shelving");
            emit filterChanged(m_bandIndex,4);
            break;
        case 5:
            ui->pBt_filter->setIcon(QIcon(":/Skin/Images/Headphones/Filter/HighShelving-no.png"));
            ui->pBt_filter->setText("High Shelving");
            emit filterChanged(m_bandIndex,3);
            break;
        }

    });



}

EditPanelTip::~EditPanelTip()
{
    delete ui;
}

void EditPanelTip::setBandData(int index, double freq, double gain, double q,int FilterType)
{
    m_bandIndex = index;
    ui->lEdit_freq->blockSignals(true);
    ui->lEdit_GVal->blockSignals(true);
    ui->lEdit_QVal->blockSignals(true);


    // 格式化显示
    QString displayText;
    if (freq >= 1000) {
        double kVal = freq / 1000.0;
        if (qFuzzyCompare(kVal, std::floor(kVal)))
            displayText = QString::number(static_cast<int>(kVal)) + "KHz";
        else
            displayText = QString::number(kVal, 'g', 4) + "KHz";//三位有效数字
    } else {
        displayText = QString::number(freq,'f',1) + "Hz";
    }

    //ui->lEdit_freq->setText(QString::number(freq,'f',1));
    ui->lEdit_freq->setText(displayText);

    ui->lEdit_GVal->setText(QString::number(gain,'f',1) + "dB");
    ui->lEdit_QVal->setText(QString::number(q,'f',1));
    ui->lEdit_freq->blockSignals(false);
    ui->lEdit_GVal->blockSignals(false);
    ui->lEdit_QVal->blockSignals(false);

    switch(FilterType)
    {
    case 2:
        ui->pBt_filter->setIcon(QIcon(":/Skin/Images/Headphones/Filter/LowPass-no.png"));
        ui->pBt_filter->setText("Low Pass");
        break;
    case 1:
        ui->pBt_filter->setIcon(QIcon(":/Skin/Images/Headphones/Filter/HighPass-no.png"));
        ui->pBt_filter->setText("High Pass");
        break;
    case 5:
        ui->pBt_filter->setIcon(QIcon(":/Skin/Images/Headphones/Filter/NotchFilter-no.png"));
        ui->pBt_filter->setText("Notch Filter");
        break;
    case 0:
        ui->pBt_filter->setIcon(QIcon(":/Skin/Images/Headphones/Filter/PeakingEQ-no.png"));
        ui->pBt_filter->setText("Peaking EQ");
        break;
    case 4:
        ui->pBt_filter->setIcon(QIcon(":/Skin/Images/Headphones/Filter/LowShelving-no.png"));
        ui->pBt_filter->setText("Low Shelving");
        break;
    case 3:
        ui->pBt_filter->setIcon(QIcon(":/Skin/Images/Headphones/Filter/HighShelving-no.png"));
        ui->pBt_filter->setText("High Shelving");
        break;
    }


}

// void EditPanelTip::on_lEdit_freq_textEdited(const QString &arg1)
// {
//     double value = arg1.toDouble();
//     emit frequencyChanged(m_bandIndex, value);
// }


// void EditPanelTip::on_lEdit_QVal_textEdited(const QString &arg1)
// {
//     double value = arg1.toDouble();
//     emit qChanged(m_bandIndex, value);
// }


// void EditPanelTip::on_lEdit_GVal_textEdited(const QString &arg1)
// {
//     double value = arg1.toDouble();
//     emit gainChanged(m_bandIndex, value);
// }

void EditPanelTip::on_lEdit_freq_editingFinished()
{
    QLineEdit *input = ui->lEdit_freq;
    QString text = input->text().trimmed();

    // 移除末尾的 "Hz"（不区分大小写）
    if (text.endsWith("Hz", Qt::CaseInsensitive))
        text.chop(2);

    // 1. 解析原始输入（支持 "K" 后缀）
    bool hasK = text.endsWith('K', Qt::CaseInsensitive);
    QString numStr = hasK ? text.left(text.size() - 1) : text;
    bool ok;
    double value = numStr.toDouble(&ok);
    if (!ok) {
        // 解析失败，重置为默认值 20.0 Hz
        input->blockSignals(true);
        input->setText("20.0");
        input->blockSignals(false);
        emit frequencyChanged(m_bandIndex, 20.0);
        return;
    }

    double freqHz = hasK ? value * 1000.0 : value;
    freqHz = qBound(20.0, freqHz, 20000.0);

    // 2. 根据四位有效数字规则四舍五入
    double roundedFreq;
    if (freqHz >= 1000.0) {
        // 转换为 kHz，保留四位有效数字
        double kHz = freqHz / 1000.0;
        // 计算有效数字位数：对于 kHz 值，四位有效数字意味着小数点后需要保留的位数
        // 例如：11.67K -> 四位有效数字，即小数点后两位
        // 更通用的方法：使用 log10 确定数量级，然后四舍五入到指定位数
        int digits = 4; // 四位有效数字
        double magnitude = std::pow(10.0, std::floor(std::log10(kHz)) - digits + 1);
        double roundedKHz = std::round(kHz / magnitude) * magnitude;
        // 四舍五入到整数 Hz
        roundedFreq = std::round(roundedKHz * 1000.0);
        // 确保范围
        roundedFreq = qBound(20.0, roundedFreq, 20000.0);
    } else {
        // 低于 1000 Hz，保留四位有效数字（例如 20.00, 123.4）
        int digits = 4;
        double magnitude = std::pow(10.0, std::floor(std::log10(freqHz)) - digits + 1);
        roundedFreq = std::round(freqHz / magnitude) * magnitude;
        roundedFreq = qBound(20.0, roundedFreq, 20000.0);
    }

    // 3. 格式化显示（与四舍五入后的值保持一致）
    QString displayText;
    if (roundedFreq >= 1000.0) {
        double kVal = roundedFreq / 1000.0;
        // 使用 'g', 4 自动处理四位有效数字，并去除尾随零
        displayText = QString::number(kVal, 'g', 4) + "K";
        // 但 'g' 可能产生 "11.67K"，若需要固定小数点后两位可改为 'f',2
        // 根据您的需求，保持与之前一致使用 'g',4
    } else {
        displayText = QString::number(roundedFreq, 'g', 4);
    }

    // 4. 更新显示（避免循环触发）
    if (input->text() != displayText) {
        input->blockSignals(true);
        input->setText(displayText);
        input->blockSignals(false);
    }

    // 5. 发射信号
    emit frequencyChanged(m_bandIndex, roundedFreq);
}
/*void EditPanelTip::on_lEdit_freq_editingFinished()
{
    QString arg1 = ui->lEdit_freq->text();
    double value = arg1.toDouble();
    emit frequencyChanged(m_bandIndex, value);
}*/

void EditPanelTip::on_lEdit_QVal_editingFinished()
{
    QString arg1 = ui->lEdit_QVal->text();
    bool ok = false;
    double value = arg1.toDouble(&ok);

    if (!ok) {
        // 输入不是有效数字，可以恢复为上一次有效值或默认值
        // 这里简单恢复为当前显示值（可能已被清空），可根据需求调整
        ui->lEdit_QVal->setText(QString::number(m_previousValidValue, 'f', 1));
        return;
    }

    // 限制在 0.1 ~ 10 之间
    value = qBound(0.1, value, 10.0);

    // 更新编辑框显示为限制后的值（保证用户看到的是合规的值）
    ui->lEdit_QVal->setText(QString::number(value, 'f', 1));

    // 记录当前有效值，以便非法输入时恢复
    m_previousValidValue = value;

    emit qChanged(m_bandIndex, value);
}

void EditPanelTip::on_lEdit_GVal_editingFinished()
{
    QString arg1 = ui->lEdit_GVal->text();
    // 移除末尾的 "db"（不区分大小写）
    if (arg1.endsWith("db", Qt::CaseInsensitive))
        arg1.chop(2);

    bool ok = false;
    double value = arg1.toDouble(&ok);

    if (!ok) {
        // 输入不是有效数字，恢复为上一次有效值
        ui->lEdit_GVal->setText(QString::number(m_previousValidGain, 'f', 1) + "db");
        return;
    }

    // 限制在 -12.0 ~ 12.0 之间
    value = qBound(-12.0, value, 12.0);

    // 更新编辑框显示（带 "db" 后缀，保持统一风格）
    ui->lEdit_GVal->setText(QString::number(value, 'f', 1) + "db");

    // 记录当前有效值
    m_previousValidGain = value;

    emit gainChanged(m_bandIndex, value);
}

//根据主题设置样式
void EditPanelTip::setTheme_EditPanelTip(int idx)
{
    QString textColor,colorStr;

    switch (idx)
    {
    case 0: {colorStr = "#324A68"; break;}   // 深蓝色
    case 1: {colorStr = "#324A68"; break;}   // 白色
    case 2: {colorStr = "#324A68"; break;}   // 黑色
    default: {colorStr = "#324A68"; break;}
    }
    //整体背景
    this->setStyleSheet(QString("border-radius: 8px;background-color: %1;").arg(colorStr));

    //文本
    switch (idx)
    {
    case 0: {textColor = "#A1A8B3"; break;}   // 深蓝色
    case 1: {textColor = "#A1A8B3"; break;}   // 白色
    case 2: {textColor = "#A1A8B3"; break;}   // 黑色
    default: {textColor = "#A1A8B3"; break;}
    }

    ui->label->setStyleSheet(
        QString("font-family: \"Noto Sans S Chinese\"; "
                "font-weight: 500;"
                "font-size: 12px;"
                "color: %1;"
                "background:transparent;")
            .arg(textColor)
        );
    ui->label_2->setStyleSheet(
        QString("font-family: \"Noto Sans S Chinese\"; "
                "font-weight: 500;"
                "font-size: 12px;"
                "color: %1;"
                "background:transparent;")
            .arg(textColor)
        );
    ui->label_3->setStyleSheet(
        QString("font-family: \"Noto Sans S Chinese\"; "
                "font-weight: 500;"
                "font-size: 12px;"
                "color: %1;"
                "background:transparent;")
            .arg(textColor)
        );

    switch (idx)
    {
    case 0:// 深蓝色
    {
        textColor = "#FFFFFF";
        colorStr = "rgba(0, 0, 0, 0.2)";
        break;
    }
    case 1:// 白色
    {
        textColor = "#FFFFFF";
        colorStr = "rgba(0, 0, 0, 0.2)";
        break;
    }
    case 2:// 黑色
    {
        textColor = "#FFFFFF";
        colorStr = "rgba(0, 0, 0, 0.2)";
        break;
    }
    default:
    {
        textColor = "#FFFFFF";
        colorStr = "rgba(0, 0, 0, 0.2)";
        break;
    }
    }

    ui->lEdit_freq->setStyleSheet(
        QString("background-color: %2;"
                "border-radius: 2px;"
                "color: %1;"
                "font-family: \"Noto Sans S Chinese\"; "
                "font-weight: 500;"
                "font-size: 10px;")
            .arg(textColor).arg(colorStr)
        );
}
//点击滤波器选择
void EditPanelTip::on_pBt_filter_clicked()
{
    if(filterPopup->isVisible())
    {
        filterPopup->close();
        return;
    }

    filterPopup->SetCheckedBtn(ui->pBt_filter->text());

    // 2. 获取父窗口（EditPanelTip）的全局矩形
    QPoint parentTopLeft = this->mapToGlobal(QPoint(0, 0));
    QRect parentRect(parentTopLeft, this->size());

    // 3. 获取 EditPanelTip 的顶层窗口（主窗口）的全局矩形
    QWidget *topLevel = this->window();          // 最上层窗口
    QPoint topLeft = topLevel->mapToGlobal(QPoint(0, 0));
    QRect windowRect(topLeft, topLevel->size()); // 主窗口的全局矩形

    // 4. 获取弹出窗口的大小（假设已固定）
    QSize popupSize = filterPopup->sizeHint();
    if (popupSize.isEmpty())
        popupSize = QSize(138, 202);

    // 5. 计算水平位置：优先显示在父窗口右侧，若超出顶层窗口则左侧
    int x;
    int offset = -10;
    int rightX = parentRect.right() + offset;
    int leftX = parentRect.left() - popupSize.width() - offset;

    if (rightX + popupSize.width() <= windowRect.right()) {
        x = rightX;   // 右侧放得下
    } else if (leftX >= windowRect.left()) {
        x = leftX;    // 左侧放得下
    } else {
        // 两侧都放不下，则强制在主窗口右侧边界内
        x = windowRect.right() - popupSize.width();
    }

    // 6. 垂直位置：与父窗口顶部对齐，防止超出主窗口上下边界
    int y = parentRect.top();
    if (y + popupSize.height() > windowRect.bottom()) {
        y = windowRect.bottom() - popupSize.height();
    }
    if (y < windowRect.top()) {
        y = windowRect.top();
    }
    qDebug("EditPanelTip x:%d,y:%d\n",x,y);
    //定位并显示(根据主窗口的范围，显示在左侧或者右侧，距离4px)
    filterPopup->move(x, y);
    filterPopup->show();
}
//重写隐藏事件（m_filterPopup同步隐藏）
void EditPanelTip::hideEvent(QHideEvent *event)
{
    if (filterPopup) {
        filterPopup->hide();   // 隐藏弹窗，但不销毁
    }
    QWidget::hideEvent(event);
}
