#ifndef NEWBEHAVIOURLDLG_H
#define NEWBEHAVIOURLDLG_H

#include <QDialog>

namespace Ui {
class NewBehaviourDlg;
}

class NewBehaviourlDlg : public QDialog
{
    Q_OBJECT

public:
    explicit NewBehaviourlDlg(QWidget *parent);
    ~NewBehaviourlDlg() override;

    QString getName();
    QString getNotes();
    int     getIters();
    QString importFrom();

    void    setExistingBehaviourNames(QStringList*);

    void    setPreexisting();

private:
    Ui::NewBehaviourDlg *ui;

    QString current_name;                   // when updating
    QString behaviourName;
    QStringList *existingBehaviourNames;
    QString notes;

    bool preexisting = false;

    void accept() override;
};

#endif // NEWBEHAVIOURLDLG_H
