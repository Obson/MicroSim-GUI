#ifndef CREATEDOMAINDLG_H
#define CREATEDOMAINDLG_H

#include <QDialog>

namespace Ui {
class CreateDomainDlg;
}

class CreateDomainDlg : public QDialog
{
    Q_OBJECT

public:
    explicit CreateDomainDlg(QWidget *parent = nullptr);
    ~CreateDomainDlg() override;

    QString getDomainName();
    QString getCurrency();
    QString getCurrencyAbbrev();

private slots:
    void on_leDomainName_textChanged(const QString &arg1);

private:
    Ui::CreateDomainDlg *ui;

    void accept() override;
};

#endif // CREATEDOMAINDLG_H
