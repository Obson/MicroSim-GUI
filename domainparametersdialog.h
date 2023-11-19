#ifndef DOMAINPARAMETERSDIALOG_H
#define DOMAINPARAMETERSDIALOG_H

#include <QDialog>

namespace Ui {
class DomainParametersDialog;
}

class DomainParametersDialog : public QDialog
{
    Q_OBJECT

public:
    explicit DomainParametersDialog(QWidget *parent = nullptr);
    ~DomainParametersDialog() override;

    void setDomain(QString name);

    QString getDomain();

    int getProcurement();
    int getPropConsumeInc();
    int getIncTaxRate();
    int getIncTaxThresh();
    int getSalesTaxRate();
    int getStartupProb();
    int getDedns();
    int getUnempBen();
    int getPropInvest();
    int getDistrib();
    int getCBInterest();
    int getClearingBankInterest();
    int getLoanProb();
    int getRecoupPeriods();
    int getStdWage();

private:
    Ui::DomainParametersDialog *ui;


public slots:

    void readParameters();
};

#endif // DOMAINPARAMETERSDIALOG_H
