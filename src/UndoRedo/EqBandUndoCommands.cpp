#include "UndoRedo/EqBandUndoCommands.h"
#include "EQCurve/EQCurveWidget.h"


// ==================== 增益撤销回退 ====================
EQBandGainCommand::EQBandGainCommand(EQCurveWidget *widget, int index,
                                     double oldGain, double newGain,
                                     QUndoCommand *parent)
    : QUndoCommand(parent), m_widget(widget), m_index(index),
    m_oldGain(oldGain), m_newGain(newGain)
{
    // 设置命令描述，会显示在撤销/重做菜单中
    setText(QStringLiteral("Change gain of band %1").arg(index));
}

void EQBandGainCommand::redo()
{
    //qDebug("进入EQBandGainCommand::redo() \n");
    m_widget->setBandGainInternal(m_index, m_newGain);
}

void EQBandGainCommand::undo()
{
    //qDebug("进入EQBandGainCommand::undo() \n");
    m_widget->setBandGainInternal(m_index, m_oldGain);
}

// ==================== 频率撤销回退 ====================
EQBandFrequencyCommand::EQBandFrequencyCommand(EQCurveWidget *widget, int index,
                                               double oldFreq, double newFreq,
                                               QUndoCommand *parent)
    : QUndoCommand(parent), m_widget(widget), m_index(index),
    m_oldFreq(oldFreq), m_newFreq(newFreq)
{
    setText(QStringLiteral("Change frequency of band %1").arg(index));
}

void EQBandFrequencyCommand::redo()
{
    m_widget->setBandFrequencyInternal(m_index, m_newFreq);
}

void EQBandFrequencyCommand::undo()
{
    m_widget->setBandFrequencyInternal(m_index, m_oldFreq);
}

// ==================== Q 值撤销回退 ====================
EQBandQCommand::EQBandQCommand(EQCurveWidget *widget, int index,
                               double oldQ, double newQ,
                               QUndoCommand *parent)
    : QUndoCommand(parent), m_widget(widget), m_index(index),
    m_oldQ(oldQ), m_newQ(newQ)
{
    setText(QStringLiteral("Change Q of band %1").arg(index));
}

void EQBandQCommand::redo()
{
    m_widget->setBandQInternal(m_index, m_newQ);
}

void EQBandQCommand::undo()
{
    m_widget->setBandQInternal(m_index, m_oldQ);
}

// ==================== 滤波器 值撤销回退 ====================
EQBandFliterCommand::EQBandFliterCommand(EQCurveWidget *widget, int index,
                               int oldFilterType, int newFilterType,
                               QUndoCommand *parent)
    : QUndoCommand(parent), m_widget(widget), m_index(index),
    m_oldFilterType(oldFilterType), m_newFilterType(newFilterType)
{
    setText(QStringLiteral("Change Q of band %1").arg(index));
}

void EQBandFliterCommand::redo()
{
    m_widget->setBandFilterInternal(m_index, m_newFilterType);
}

void EQBandFliterCommand::undo()
{
    m_widget->setBandFilterInternal(m_index, m_oldFilterType);
}
