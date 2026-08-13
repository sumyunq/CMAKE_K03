#ifndef UNDOLEDITVAL_H
#define UNDOLEDITVAL_H

#include <QUndoCommand>
#include <QLineEdit>

class UndoLEditVal : public QUndoCommand
{
public:
    UndoLEditVal(QLineEdit *lineEdit, const QString &oldText, const QString &newText);

    void undo() override;
    void redo() override;

private:
    QLineEdit *m_lineEdit;
    QString m_oldText;
    QString m_newText;

};

#endif // UNDOLEDITVAL_H
