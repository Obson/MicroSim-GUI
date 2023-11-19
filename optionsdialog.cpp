#include "optionsdialog.h"
#include "ui_optionsdialog.h"
#include <QSettings>
#include <QString>
#include <QMessageBox>

OptionsDialog::OptionsDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::OptionsDialog)
{
    ui->setupUi(this);

    QSettings settings;
    bool ok = false;
    int iterations = settings.value("iterations", 100).toInt(&ok);
    if (!ok) {
        iterations = 100;
    }
    int first = settings.value("start-period", 0).toInt(&ok);
    if (!ok) {
        first = 0;
    }

    ui->lineEdit->setText(QString::number(iterations));
    ui->lineEdit_5->setText(QString::number(first));
}

OptionsDialog::~OptionsDialog()
{
    delete ui;
}

int OptionsDialog::iterations()
{
    return ui->lineEdit->text().toInt();
}

int OptionsDialog::startPeriod()
{
    return ui->lineEdit_5->text().toInt();
}

void OptionsDialog::accept()
{
    int iterations = ui->lineEdit->text().toInt();
    int first = ui->lineEdit_5->text().toInt();

    QSettings settings;

    settings.setValue("iterations", iterations);
    settings.setValue("start-period", first);

    QDialog::accept();
}
