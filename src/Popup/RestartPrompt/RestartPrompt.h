#ifndef RESTARTPROMPT_H
#define RESTARTPROMPT_H

#include <QDialog>

namespace Ui {
class RestartPrompt;
}

class RestartPrompt : public QDialog
{
    Q_OBJECT

public:
    explicit RestartPrompt(QWidget *parent = nullptr);
    ~RestartPrompt();

private:
    Ui::RestartPrompt *ui;
};

#endif // RESTARTPROMPT_H
