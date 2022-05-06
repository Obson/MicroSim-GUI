#ifndef NEWBEHAVIOURLDLG_H
#define NEWBEHAVIOURLDLG_H

#include <QDialog>

namespace Ui {
class NewModelDlg;
}

class NewBehaviourlDlg : public QDialog
{
    Q_OBJECT

public:
    explicit NewBehaviourlDlg(QWidget *parent = nullptr);
    ~NewBehaviourlDlg() override;

    QString getName();
    QString getNotes();
    int     getIters();
    QString importFrom();

    void    setPreexisting();

private:
    Ui::NewModelDlg *ui;

    QString current_name;                   // when updating
    QString behaviourName;
    QString notes;

    bool preexisting = false;

    void accept() override;
};

#endif // NEWBEHAVIOURLDLG_H
