#include <QDebug>

#include "createdomaindlg.h"
#include "QtCore/qsettings.h"
#include "QtWidgets/qpushbutton.h"
#include "behaviour.h"
#include "domain.h"
#include "ui_createdomaindlg.h"

CreateDomainDlg::CreateDomainDlg(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CreateDomainDlg)
{
    ui->setupUi(this);
    ui->cbBehaviour->insertItem(0, "Default");

    int ixCurrent = 0;

    /*
     * This all needs rewriting
     */

//    QSettings settings;
//    int count = settings.beginReadArray(
//                "Behaviours"    // historical (Behaviour used to be Model)
//                );
//    if (count > 0)
//    {
//        for (int i = 0; i < count; ++i)
//        {
//            settings.setArrayIndex(i);
//            QString name = settings.value("name").toString();
//            if (name == Behaviour::currentBehaviour->name()) {
//                ixCurrent = i;
//            }
//            ui->cbBehaviour->addItem(settings.value("name").toString());
//        }
//    }
//    settings.endArray();

//    ui->cbBehaviour->setCurrentIndex(ixCurrent);
//    ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
}

CreateDomainDlg::~CreateDomainDlg()
{
    delete ui;
}

QString CreateDomainDlg::getDomainName()
{
    return ui->leDomainName->text().simplified();
}

QString CreateDomainDlg::getBehaviourName()
{
    return ui->cbBehaviour->currentText().simplified();
}

QString CreateDomainDlg::getCurrency()
{
    return ui->leCurrencyName->text().simplified();
}

QString CreateDomainDlg::getCurrencyAbbrev()
{
    return ui->leAbbrev->text().simplified();
}

void CreateDomainDlg::on_leDomainName_textChanged(const QString &arg1)
{
    // qDebug() << "CreateDomainDlg::on_leDomainName_textChanged called";
    QPushButton *okButton = ui->buttonBox->button(QDialogButtonBox::Ok);
    if (arg1.isEmpty()) {
        okButton->setEnabled(false);
    } else {
        // TODO: Check that domain name hasn't already been used
        okButton->setEnabled(true);
    }
}

void CreateDomainDlg::accept()
{
    // QString domainName = ui->leDomainName->text().simplified();
    // qDebug() << "CreateDomainDlg::accept(): creating domain" << domainName;
    // Domain *newDomain = new Domain(domainName, nullptr);
    QDialog::accept();
}

