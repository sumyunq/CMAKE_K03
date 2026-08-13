#ifndef EQBANDUNDOCOMMANDS_H
#define EQBANDUNDOCOMMANDS_H

#include <QUndoCommand>

class EQCurveWidget; // 前向声明，避免循环依赖

// ---------- 增益修改命令 ----------
class EQBandGainCommand : public QUndoCommand
{
public:
    EQBandGainCommand(EQCurveWidget *widget, int index,
                      double oldGain, double newGain,
                      QUndoCommand *parent = nullptr);

    void undo() override;
    void redo() override;

private:
    EQCurveWidget *m_widget;
    int m_index;
    double m_oldGain;
    double m_newGain;
};

// ---------- 频率修改命令 ----------
class EQBandFrequencyCommand : public QUndoCommand
{
public:
    EQBandFrequencyCommand(EQCurveWidget *widget, int index,
                           double oldFreq, double newFreq,
                           QUndoCommand *parent = nullptr);

    void undo() override;
    void redo() override;

private:
    EQCurveWidget *m_widget;
    int m_index;
    double m_oldFreq;
    double m_newFreq;
};

// ---------- Q 值修改命令 ----------
class EQBandQCommand : public QUndoCommand
{
public:
    EQBandQCommand(EQCurveWidget *widget, int index,
                   double oldQ, double newQ,
                   QUndoCommand *parent = nullptr);

    void undo() override;
    void redo() override;

private:
    EQCurveWidget *m_widget;
    int m_index;
    double m_oldQ;
    double m_newQ;
    int m_oldFilterType;
    int m_newFilterType;
};

// ---------- 滤波器 值修改命令 ----------
class EQBandFliterCommand : public QUndoCommand
{
public:
    EQBandFliterCommand(EQCurveWidget *widget, int index,
                        int oldFilterType, int newFilterType,
                        QUndoCommand *parent = nullptr);
    void undo() override;
    void redo() override;

private:
    EQCurveWidget *m_widget;
    int m_index;
    int m_oldFilterType;
    int m_newFilterType;
};
#endif // EQBANDUNDOCOMMANDS_H
