#include <QSettings>

#include "saveprofiledialog.h"
#include "ui_saveprofiledialog.h"

SaveProfileDialog::SaveProfileDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SaveProfileDialog)
{
    ui->setupUi(this);

    QSettings settings;
    ui->lineEdit->setText(settings.value("current-profile", "Untitled").toString());
}

SaveProfileDialog::~SaveProfileDialog()
{
    delete ui;
}

QString SaveProfileDialog::profileName()
{
    return ui->lineEdit->text();
}
