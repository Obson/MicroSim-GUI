#ifndef STATSDIALOG_H
#define STATSDIALOG_H

#include <QDialog>

namespace Ui {
class StatsDialog;
}

class StatsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit StatsDialog(QWidget *parent = 0);
    ~StatsDialog();

    void setLimits(QString property, double min, double max, double mean);

    // TODO: Add SD later...

private:
    Ui::StatsDialog *ui;
};

#endif // STATSDIALOG_H
