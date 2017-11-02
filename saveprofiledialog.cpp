#include <QSettings>

#include "saveprofiledialog.h"
#include "ui_saveprofiledialog.h"

// This dialog doesn't actually update the current profile -- it just selects
// the required name (which may or may not be the name of an existing profile)
// and returns it via profileMame().
SaveProfileDialog::SaveProfileDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SaveProfileDialog)
{
    ui->setupUi(this);

    QSettings settings;
    settings.beginGroup("Profiles");
    ui->comboBox->addItems(settings.childGroups());
    settings.endGroup();

    // Select the item corresponding to the current profile
    QString prof = settings.value("current-profile", "").toString();
    ui->comboBox->setCurrentText(prof.isEmpty() ? "Untitled" : prof);
}

SaveProfileDialog::~SaveProfileDialog()
{
    delete ui;
}

QString SaveProfileDialog::profileName()
{
    return ui->comboBox->currentText();
}
