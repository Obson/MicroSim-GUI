#include "newmodeldlg.h"
#include "ui_newmodeldlg.h"
#include <QIntValidator>
#include <QDialogButtonBox>
#include <QPushButton>
#include <QMessageBox>
#include <QSettings>

NewModelDlg::NewModelDlg(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::NewModelDlg)
{
    ui->setupUi(this);
}

NewModelDlg::~NewModelDlg()
{
    delete ui;

    QValidator *validIters = new QIntValidator(3, 10000, this);
    ui->leIterations->setValidator(validIters);

    ui->buttonBox->button((QDialogButtonBox::Ok))->setEnabled(false);
}

QString NewModelDlg::getName()
{
    return model_name;
}

QString NewModelDlg::getNotes()
{
    return notes;
}

int NewModelDlg::getIters()
{
    return iterations;
}

void NewModelDlg::accept()
{
    bool ok;

    model_name = ui->leName->text().simplified();
    iterations = ui->leIterations->text().trimmed().toInt(&ok);
    notes      = ui->teNotes->toPlainText().simplified();

    if (model_name.size() > 0 && ok && iterations > 2 && iterations < 1000)
    {
        // Check that the model name isn't a duplicate

        QSettings settings;
        QStringList models;
        int size = settings.beginReadArray("models");

        for (int i = 0; i < size; ++i)
        {
            settings.setArrayIndex(i);
            QString used_name = settings.value("name").toString();

            if (used_name == model_name)
            {
                // duplicate name entered
                QMessageBox msgBox;
                msgBox.setText(tr("Please enter a valid model name"));
                msgBox.setInformativeText(tr("This name has already been used!"));
                msgBox.setIcon(QMessageBox::Warning);
                msgBox.setWindowTitle(tr("Error"));
                msgBox.exec();
                return;
            }
            models.append(used_name);
        }
        settings.endArray();

        // Append the new name to the list of models
        models.append(model_name);

        // Rewrite the array with the new name appended
        settings.beginWriteArray("Models");
        for (int i = 0; i < models.size(); ++i)
        {
             settings.setArrayIndex(i);
             settings.setValue("name", models[i]);
        }
        settings.endArray();

        // Write the notes to settings
        settings.setValue(model_name + "/notes", notes);

        // Write number of iterations to settings
        settings.setValue(model_name + "/iterations", iterations);

        // Close the dialog
        QDialog::accept();
        return;
    }
    else
    {
        QMessageBox msgBox;
        msgBox.setIcon(QMessageBox::Warning);
        msgBox.setWindowTitle(tr("Error"));

        if (!ok || iterations < 3 || iterations > 999)
        {
            msgBox.setText(tr("Please enter a valid number of iterations"));
            msgBox.setInformativeText(tr("The number of iterations must be greater "
                                         "than 2 and less than 1000"));
        }
        else
        {
            msgBox.setText(tr("Please enter a valid model name"));
            msgBox.setInformativeText(tr("Model names must be unique and must "
                                         "contain at least one non-space character"));
        }
        msgBox.exec();
        return;
    }
}
