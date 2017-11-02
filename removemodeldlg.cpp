#include "removemodeldlg.h"
#include "ui_removemodeldlg.h"

#include <QSettings>
#include <QMessageBox>

RemoveModelDlg::RemoveModelDlg(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::RemoveModelDlg)
{
    ui->setupUi(this);

    // Populate list with names of models
    QSettings settings;

    // Get current model (if any) so we can exclude it from the list
    QString current = settings.value("current_model", "").toString();

    // Read the model names from settings
    int count = settings.beginReadArray("Models");
    if (count > 0)
    {
        for (int i = 0; i < count; ++i)
        {
            settings.setArrayIndex(i);
            QString name = settings.value("name").toString();
            if (name != current) {
                ui->listWidget->addItem(name);
            }
        }
    }
    settings.endArray();

}

RemoveModelDlg::~RemoveModelDlg()
{
    delete ui;
}

void RemoveModelDlg::on_listWidget_itemSelectionChanged()
{
    ui->btnRemove->setEnabled(ui->listWidget->count() > 0);
}

void RemoveModelDlg::on_btnRemove_clicked()
{
    QString name = ui->listWidget->currentItem()->text();
    QMessageBox msgBox(this);
    msgBox.setWindowModality(Qt::WindowModal);
    msgBox.setText("Removing model '" + name + "'.");
    msgBox.setInformativeText("Are you sure you want to remove this model?");
    msgBox.setIcon(QMessageBox::Question);
    msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    msgBox.setDefaultButton(QMessageBox::No);
    if (msgBox.exec() == QMessageBox::Yes) {
        remove(name);
    }
}

void RemoveModelDlg::remove(QString model)
{
    QSettings settings;

    // Get current model so we can reinstate it
    QString current = settings.value("current_model", "").toString();

    // Rewrite the list of models as an 'array' but without the deleted model
    settings.remove("Models");
    settings.beginWriteArray("Models");
    ui->listWidget->takeItem(ui->listWidget->currentRow()); // remove select item from list
    int n = 0;
    for (int i = 0; i < ui->listWidget->count(); i++) {
        QString name = ui->listWidget->item(i)->text();
        if (name != model) {
            settings.setArrayIndex(n++);
            settings.setValue("name", name);
        }
    }
    settings.setArrayIndex(n);
    settings.setValue("name", current); // reinstate current
    settings.endArray();

    settings.remove("Models/" + model);
    settings.remove(model);
}

void RemoveModelDlg::on_btnOK_clicked()
{
    accept();
}
