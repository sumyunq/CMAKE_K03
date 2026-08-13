// UndoSpaceSize.h
#include <QUndoCommand>
#include <QButtonGroup>

class UndoSpaceSize : public QUndoCommand
{
public:
    UndoSpaceSize(QButtonGroup* group, int oldSize, int newSize);
    void undo() override;
    void redo() override;

private:
    QButtonGroup* m_group;
    int m_oldSize;
    int m_newSize;
};
