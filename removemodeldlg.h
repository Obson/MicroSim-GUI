#ifndef REMOVEMODELDLG_H
#define REMOVEMODELDLG_H

#include <QDialog>

namespace Ui {
class RemoveModelDlg;
}

class RemoveModelDlg : public QDialog
{
    Q_OBJECT

public:
    explicit RemoveModelDlg(QWidget *parent = 0);
    ~RemoveModelDlg();

private slots:
    void on_listWidget_itemSelectionChanged();

    void on_btnRemove_clicked();

    void on_btnOK_clicked();

private:
    Ui::RemoveModelDlg *ui;

    void remove(QString model);
};

#endif // REMOVEMODELDLG_H
