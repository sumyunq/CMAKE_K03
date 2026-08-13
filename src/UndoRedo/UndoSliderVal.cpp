#include "UndoRedo/UndoSliderVal.h"

UndoSliderVal::UndoSliderVal(QSlider *slider, QLineEdit *lab, const int &oldVal, const int &newVal,int IsFloat)
{
    this->m_slider = slider;
    this->m_lab = lab;
    this->m_oldVal = oldVal;
    this->m_newVal = newVal;
    this->m_IsFloat = IsFloat;
}
void UndoSliderVal::undo()
{
    //qDebug("撤销操作:%d->%d",m_newVal,m_oldVal);

    m_slider->blockSignals(true);
    m_slider->setValue(m_oldVal);
    m_slider->blockSignals(false);

    if(m_IsFloat == 1)//1:代表变化的eq滑条
    {
        if(m_lab)
        {
            double val = m_oldVal/10.0;//注：若不带.0，则计算的val，没有小数
            m_lab->setText(QString::number(val,'f',1));//+"db"
        }

    }else if(m_IsFloat == 0)//0:代表变化的低音、空间、增益
    {
        if(m_lab)
        {
            double val = m_oldVal;
            m_lab->setText(QString::number(val));//+"db"
        }

    }else if(m_IsFloat == 2)//2:代表变化的EQ增益的QLinEdit
    {
        if(m_lab)
        {
            double val = m_oldVal/10.0;
            m_lab->setText(QString::number(val,'f',1));//+"db"
        }

    }

    m_slider->setFocus();
}
void UndoSliderVal::redo()
{
    //qDebug("重做操作:%d->%d",m_oldVal,m_newVal);
    m_slider->blockSignals(true);
    m_slider->setValue(m_newVal);
    m_slider->blockSignals(false);

    if(m_IsFloat == 1)
    {
        if(m_lab)
        {
            double val = m_newVal/10.0;
            m_lab->setText(QString::number(val,'f',1));//+"db"
        }

    }else if(m_IsFloat == 0)
    {
        if(m_lab)
        {
            double val = m_newVal;
            m_lab->setText(QString::number(val));//+"db"
        }

    }else if(m_IsFloat == 2)
    {
        if(m_lab)
        {
            double val = m_newVal/10.0;
            m_lab->setText(QString::number(val,'f',1));//+"db"
        }

    }

    m_slider->setFocus();
}
