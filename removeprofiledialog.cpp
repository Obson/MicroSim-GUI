#include <QSettings>

#include "removeprofiledialog.h"
#include "ui_removeprofiledialog.h"

RemoveProfileDialog::RemoveProfileDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::RemoveProfileDialog)
{
    ui->setupUi(this);
    load();
}

void RemoveProfileDialog::load()
{
    QSettings settings;
    QString prof = settings.value("current-profile", "").toString();
    settings.beginGroup("Profiles");
    QStringList list = settings.childGroups();
    settings.endGroup();
    ui->listWidget->clear();
    foreach(QString s, list) {
        if (s != prof) {
            ui->listWidget->addItem(s);
        }
    }
    ui->btnRemove->setEnabled(false);
}

RemoveProfileDialog::~RemoveProfileDialog()
{
    delete ui;
}

void RemoveProfileDialog::on_btnRemove_clicked()
{
    QString s = ui->listWidget->currentItem()->text();
    QSettings settings;
    settings.remove("Profiles/" + s);
    load();
}

void RemoveProfileDialog::on_btnCancel_clicked()
{
    accept();
}

void RemoveProfileDialog::on_listWidget_itemSelectionChanged()
{
    ui->btnRemove->setEnabled(true);
}
