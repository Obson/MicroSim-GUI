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

    void    setPreexisting();

private:
    Ui::NewModelDlg *ui;

    QString current_name;                   // when updating
    QString model_name;
    QString notes;

    bool preexisting = false;

    void accept();
};

#endif // NEWMODELDLG_H
