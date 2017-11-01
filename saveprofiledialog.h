#ifndef SAVEPROFILEDIALOG_H
#define SAVEPROFILEDIALOG_H

#include <QDialog>
#include <QPushButton>

namespace Ui {
class SaveProfileDialog;
}

class SaveProfileDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SaveProfileDialog(QWidget *parent = 0);
    ~SaveProfileDialog();

    QString profileName();

private:
    Ui::SaveProfileDialog *ui;
};

#endif // SAVEPROFILEDIALOG_H
