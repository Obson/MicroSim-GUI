#include "newmodeldlg.h"
#include "ui_newmodeldlg.h"
#include <QIntValidator>
#include <QDialogButtonBox>
#include <QPushButton>
#include <QMessageBox>
#include <QSettings>
#include <QDebug>

NewModelDlg::NewModelDlg(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::NewModelDlg)
{
    ui->setupUi(this);

    QSettings settings;
    int count = settings.beginReadArray("Models");
    if (count > 0)
    {
        for (int i = 0; i < count; ++i)
        {
            settings.setArrayIndex(i);
            ui->comboBox->addItem(settings.value("name").toString());
        }
    }
    settings.endArray();

    ui->comboBox->setCurrentIndex(0);
}

NewModelDlg::~NewModelDlg()
{
    delete ui;
}

QString NewModelDlg::getName()
{
    return model_name;
}

QString NewModelDlg::getNotes()
{
    return notes;
}

QString NewModelDlg::importFrom()
{
    if (ui->comboBox->currentIndex() == 0)
    {
        return "";
    }
    else
    {
        return ui->comboBox->currentText();
    }
}

void NewModelDlg::setPreexisting()
{
    preexisting = true;
    ui->leName->setReadOnly(true);

    // Load values from settings
    QSettings settings;
    current_name = settings.value("current_model").toString();
    ui->leName->setText(current_name);
    ui->teNotes->setText(settings.value(current_name + "/notes").toString());
    ui->comboBox->setEnabled(false);
    setWindowTitle("Edit Model Description");
}

void NewModelDlg::accept()
{
    model_name = ui->leName->text().simplified();
    notes      = ui->teNotes->toPlainText().simplified();

    QSettings settings;

    if (preexisting)
    {
        // Can only change notes -- name is fixed
        // Write the notes to settings
        settings.setValue(model_name + "/notes", notes);

        // Close the dialog
        QDialog::accept();
        return;
    }
    else
    {
        QString upper = model_name.toUpper();
        if (model_name.size() > 0 && upper != "GENERAL" && upper != "STATE")
        {
            // Check that the model name isn't a duplicate
            QStringList models;
            int size = settings.beginReadArray("Models");

            qDebug() << "NewModelDlg::accept():" << size << "models found";

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

            // Rewrite the settings array with the new name, if any, appended. If
            // we have changed the name the old name will be omitted as we have
            // removed it from the models list
            settings.beginWriteArray("Models");
            for (int i = 0; i < models.size(); ++i)
            {
                 settings.setArrayIndex(i);
                 settings.setValue("name", models[i]);
            }
            settings.endArray();

            // Write the notes to settings
            settings.setValue(model_name + "/notes", notes);

            // Close the dialog
            QDialog::accept();
            return;
        }
        else
        {
            QMessageBox msgBox(this);
            msgBox.setWindowModality(Qt::WindowModal);
            msgBox.setIcon(QMessageBox::Warning);
            msgBox.setWindowTitle(tr("Error"));
            msgBox.setText(tr("You have not entered a valid model name!"));
            // TODO: remove the reserved words (general and state) from model names by prefixing them with 'u-' or something
            msgBox.setDetailedText(tr("Model names must be unique and must contain "
                                      "at least one non-space character. 'General' "
                                      "and 'state' are not allowed as model names."));
            msgBox.exec();
            return;
        }
    }
}
