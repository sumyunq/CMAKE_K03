#ifndef NEWRADIOBTNTEXT_H
#define NEWRADIOBTNTEXT_H

#include <QRadioButton>
#include <QStylePainter>
#include <QStyleOptionButton>
#include <QTextOption>

//当前预设方框，QRadioButton指示器图标内部左下角显示文字, 当radio文本过长时，换行显示
class NewRadioBtnText : public QRadioButton
{
    Q_OBJECT
public:
    explicit NewRadioBtnText(QWidget *parent = nullptr);
    void setIndicatorText(const QString &text,const QString &label);
    QString getIndicatorText();

    void setThemeAndPanelTransparency(int idx,int PValue);
    void updateStyleSheet();
protected:
    void paintEvent(QPaintEvent *event) override;

private:
    QString indicatorText;
    QString m_baseName;
    int m_Themeidx;
    double m_PanelTransparency;

signals:
    void SetITextSignal(const QString &text,const QString &label);
};

#endif // NEWRADIOBTNTEXT_H
