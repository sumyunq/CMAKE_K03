#include "UndoRedo/UndoSpaceSize.h"
#include <QAbstractButton>
UndoSpaceSize::UndoSpaceSize(QButtonGroup* group, int oldSize, int newSize)
    : m_group(group), m_oldSize(oldSize), m_newSize(newSize)
{
}

void UndoSpaceSize::undo()
{
    if (m_group) {
        QAbstractButton* btn = m_group->button(m_oldSize);
        if (btn) btn->setChecked(true);
    }
}

void UndoSpaceSize::redo()
{
    if (m_group) {
        QAbstractButton* btn = m_group->button(m_newSize);
        if (btn) btn->setChecked(true);
    }
}
