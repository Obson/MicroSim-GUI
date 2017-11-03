#ifndef REMOVEPROFILEDIALOG_H
#define REMOVEPROFILEDIALOG_H

#include <QDialog>

namespace Ui {
class RemoveProfileDialog;
}

class RemoveProfileDialog : public QDialog
{
    Q_OBJECT

public:
    explicit RemoveProfileDialog(QWidget *parent = 0);
    ~RemoveProfileDialog();

private slots:
    void on_btnRemove_clicked();
    void on_btnCancel_clicked();
    void on_listWidget_itemSelectionChanged();

private:
    Ui::RemoveProfileDialog *ui;
    void load();
};

#endif // REMOVEPROFILEDIALOG_H
