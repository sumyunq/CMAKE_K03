#include "UndoRedo/UndoLEditVal.h"

UndoLEditVal::UndoLEditVal(QLineEdit *lineEdit, const QString &oldText, const QString &newText)
{
    this->m_lineEdit = lineEdit;
    this->m_oldText = oldText;
    this->m_newText = newText;
}

void UndoLEditVal::undo()
{
    m_lineEdit->blockSignals(true);
    m_lineEdit->setText(m_oldText);
    m_lineEdit->blockSignals(false);

    // m_lineEdit->setFocus();
}

void UndoLEditVal::redo()
{
    m_lineEdit->blockSignals(true);
    m_lineEdit->setText(m_newText);
    m_lineEdit->blockSignals(false);

    // m_lineEdit->setFocus();
}
