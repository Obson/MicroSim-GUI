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

private:
    Ui::DomainParametersDialog *ui;


public slots:

    void getParameters();
};

#endif // DOMAINPARAMETERSDIALOG_H
