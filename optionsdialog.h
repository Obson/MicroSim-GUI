#ifndef OPTIONSDIALOG_H
#define OPTIONSDIALOG_H

#include <QDialog>

namespace Ui {
class OptionsDialog;
}

class OptionsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit OptionsDialog(QWidget *parent = 0);
    ~OptionsDialog();

    int iterations();
    int startups();
    int startPeriod();
    int std_wage();
    int population();

    void accept();

private:
    Ui::OptionsDialog *ui;
};

#endif // OPTIONSDIALOG_H
