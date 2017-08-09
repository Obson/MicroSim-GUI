#ifndef NEWMODELDLG_H
#define NEWMODELDLG_H

#include <QDialog>

namespace Ui {
class NewModelDlg;
}

class NewModelDlg : public QDialog
{
    Q_OBJECT

public:
    explicit NewModelDlg(QWidget *parent = 0);
    ~NewModelDlg();

    QString getName();
    QString getNotes();
    int     getIters();

private:
    Ui::NewModelDlg *ui;

    QString model_name;
    QString notes;

    void accept();
};

#endif // NEWMODELDLG_H
