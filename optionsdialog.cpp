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
    int first = settings.value("start-period", 1).toInt(&ok);
    if (!ok) {
        first = 1;
    }
    int startups = settings.value("startups", 10).toInt(&ok);
    if (!ok) {
        startups = 10;
    }
    int nom_pop = settings.value("nominal-population", 1000000).toInt(&ok);
    if (!ok) {
        nom_pop = 10000000;
    }
    int wage = settings.value("unit-wage", 1000000).toInt(&ok);
    ui->lineEdit->setText(QString::number(iterations));
    ui->lineEdit_2->setText(QString::number(startups));
    ui->lineEdit_3->setText(QString::number(nom_pop));
    ui->lineEdit_4->setText(QString::number(wage));
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

int OptionsDialog::startups()
{
    return ui->lineEdit_2->text().toInt();
}

int OptionsDialog::startPeriod()
{
    return ui->lineEdit_5->text().toInt();
}

int OptionsDialog::std_wage()
{
    return ui->lineEdit_4->text().toInt();
}

int OptionsDialog::population()
{
    return ui->lineEdit_3->text().toInt();
}

void OptionsDialog::accept()
{
    bool ok1 = false;
    bool ok2 = false;
    bool ok3 = false;
    bool ok4 = false;
    bool ok5 = false;
    int iterations, first, startups, nom_pop, unit_wage;

    QMessageBox msgBox(this);
    msgBox.setWindowModality(Qt::WindowModal);

    iterations = ui->lineEdit->text().toInt(&ok1);
    if (ok1 && iterations >= 10 && iterations <= 1000)
    {
        first = ui->lineEdit_5->text().toInt(&ok5);
        if (ok5 && first >= 0 && first + iterations <= 1000)
        {
            startups = ui->lineEdit_2->text().toInt(&ok2);
            if (ok2 && startups >= 0 && startups <= 100)
            {
                nom_pop = ui->lineEdit_3->text().toInt(&ok3);
                if (ok3 && nom_pop >= 1000 && nom_pop <= 100000000)
                {
                    unit_wage = ui->lineEdit_4->text().toInt(&ok4);
                    if (ok4 && unit_wage > 100 && unit_wage < 1000) {
                        QSettings settings;
                        settings.setValue("iterations", iterations);
                        settings.setValue("start-period", first);
                        settings.setValue("startups", startups);
                        settings.setValue("nominal-population", nom_pop);
                        settings.setValue("unit-wage", unit_wage);
                        QDialog::accept();
                        return;
                    }
                    else
                    {
                        msgBox.setText(tr("You have not entered a valid standard unit wage!"));
                        msgBox.setDetailedText(tr("The standard unit wage must be in the range 100 to 1000. 500 is suggested."));
                    }
                }
                else
                {
                    msgBox.setText(tr("You have not entered a valid nominal population!"));
                    msgBox.setDetailedText(tr("The nominal population must be in "
                                              "the range 1000 to 100,000,000, and "
                                              "will be rounded down to the nearest "
                                              "lower 1000."));
                }
            }
            else
            {
                msgBox.setText(tr("You have not entered a valid number of startups!"));
                msgBox.setDetailedText(tr("You can specify from 0 to 100 startups per 1000 economically active population."));
            }
        }
        else
        {
            msgBox.setText(tr("You have not entered a start period!"));
            msgBox.setDetailedText(tr("As you have entered ")
                                   + QString::number(iterations)
                                   + tr(" as the number of iterations, you can "
                                        "specify any number from 1 to ")
                                   + QString::number(1000 - iterations)
                                   + tr(" as the start period."));
        }
    }
    else
    {
        msgBox.setText(tr("You have not entered a valid number of iterations!"));
        msgBox.setDetailedText(tr("The number of iterations must be in the range 10 to 1000. 100 is recommended."));
    }

    msgBox.setIcon(QMessageBox::Warning);
    msgBox.setWindowTitle(tr("Error"));
    msgBox.exec();
 }
