#include <QDebug>

#include "createdomaindlg.h"
#include "QtCore/qsettings.h"
#include "QtWidgets/qpushbutton.h"
//#include "behaviour.h"
#include "account.h"
#include "ui_createdomaindlg.h"

CreateDomainDlg::CreateDomainDlg(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CreateDomainDlg)
{
    ui->setupUi(this);
}

CreateDomainDlg::~CreateDomainDlg()
{
    delete ui;
}

QString CreateDomainDlg::getDomainName()
{
    return ui->leDomainName->text().simplified();
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

