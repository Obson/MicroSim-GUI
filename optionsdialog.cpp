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
    int startups = settings.value("startups", 10).toInt(&ok);
    if (!ok) {
        startups = 10;
    }
    int nom_pop = settings.value("nominal-population", 1000000).toInt(&ok);
    if (!ok) {
        nom_pop = 10000000;
    }
    ui->lineEdit->setText(QString::number(iterations));
    ui->lineEdit_2->setText(QString::number(startups));
    ui->lineEdit_3->setText(QString::number(nom_pop));
}

OptionsDialog::~OptionsDialog()
{
    delete ui;
}

// TODO: Add validity checking

int OptionsDialog::iterations()
{
    return ui->lineEdit->text().toInt();
}

int OptionsDialog::startups()
{
    return ui->lineEdit_2->text().toInt();
}

void OptionsDialog::accept()
{
    bool ok1 = false;
    bool ok2 = false;
    bool ok3 = false;
    int iterations, startups, nom_pop;

    QMessageBox msgBox(this);

    iterations = ui->lineEdit->text().toInt(&ok1);
    if (ok1 && iterations >= 100 && iterations <= 1000)
    {
        startups = ui->lineEdit_2->text().toInt(&ok2);
        if (ok2 && startups >= 0 && startups <= 100)
        {
            nom_pop = ui->lineEdit_3->text().toInt(&ok3);
            if (ok3 && nom_pop >= 1000 && nom_pop <= 100000000)
            {
                QSettings settings;
                settings.setValue("iterations", iterations);
                settings.setValue("startups", startups);
                settings.setValue("nominal-population", nom_pop);
                QDialog::accept();
                return;
            }
            else
            {
                msgBox.setText(tr("Please enter a valid nominal population"));
                msgBox.setInformativeText(tr("A population of from 1000 to 100,000,000 is suggested"));
            }
        }
        else
        {
            msgBox.setText(tr("Please enter a valid number of startups"));
            msgBox.setInformativeText(tr("From 0 to 100 startups per 1000 recommended"));
        }
    }
    else
    {
        msgBox.setText(tr("Please enter a number of iterations from 10 to 1000"));
        msgBox.setInformativeText(tr("100 iterations recommended"));
    }

    msgBox.setIcon(QMessageBox::Warning);
    msgBox.setWindowTitle(tr("Error"));
    msgBox.exec();
 }
