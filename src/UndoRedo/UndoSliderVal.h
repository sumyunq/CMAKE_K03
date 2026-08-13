#ifndef UNDOSLIDERVAL_H
#define UNDOSLIDERVAL_H

#include <QUndoCommand>
#include <QSlider>
#include <QLineEdit>

class UndoSliderVal : public QUndoCommand
{
public:
    UndoSliderVal(QSlider *slider, QLineEdit *lab, const int &oldVal, const int &newVal,int IsFloat);

    void undo() override;
    void redo() override;

private:
    QSlider *m_slider;
    QLineEdit *m_lab;
    int m_oldVal;
    int m_newVal;
    int m_IsFloat;

};

#endif // UNDOSLIDERVAL_H
