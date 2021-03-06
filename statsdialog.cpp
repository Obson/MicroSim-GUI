#include <math.h>

#include "statsdialog.h"
#include "ui_statsdialog.h"

StatsDialog::StatsDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::StatsDialog)
{
    ui->setupUi(this);
}

StatsDialog::~StatsDialog()
{
    delete ui;
}

void StatsDialog::setLimits(QString property, double min, double max, double mean)
{
    ui->labProperty->setText(property);
    ui->labMin->setText(QString::number(min));
    ui->labMax->setText(QString::number(max));
    ui->labMean->setText(QString::number(mean));
}
