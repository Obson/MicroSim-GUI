#include "domainparametersdialog.h"
#include "QtCore/qobjectdefs.h"
#include "ui_domainparametersdialog.h"

#include "account.h"

DomainParametersDialog::DomainParametersDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DomainParametersDialog)
{
    ui->setupUi(this);

    QSettings settings;
    QString current_domain = settings.value("current-domain").toString();

    int ix = -1;

    /*
     * Populate the combobox, noting which index corresponds to the
     * current-domain value is settings
     */
    foreach(Domain *dom, Domain::domains)
    {
        QString domainName = dom->getName();
        ui->cbDomainName->addItem(domainName);
        if (domainName == current_domain) {
            ix = ui->cbDomainName->count() - 1;
        }
    }

    /*
     * There must be at least one domain in the list -- don't allow this
     * dialog to be called otherwise
     */
    Q_ASSERT (ix != -1);

    /*
     * Set the combobox to the current domain and prevent it from being
     * edited
     */
    ui->cbDomainName->setCurrentIndex(ix);
    ui->cbDomainName->setEditable(false);

    /*
     * Get the settings for the current domain and populate the rest of the
     * parameters. This must happen whenever the comboboc value is changed
     */
    readParameters();

    // Ignore the compiler warning -- I've done it this way for consistency with
    // the SIGNAL macro as defined for QObject::connect...
    connect(ui->cbDomainName, SIGNAL(currentIndexChanged(const QString&)),
            this, SLOT(getParameters()));

    /***
     * This is from the documentation for Signals and Slots
     *
    connect(sender, SIGNAL(destroyed(QObject*)), this, SLOT(objectDestroyed(Qbject*)));
    connect(sender, SIGNAL(destroyed(QObject*)), this, SLOT(objectDestroyed()));
    connect(sender, SIGNAL(destroyed()), this, SLOT(objectDestroyed()));
    *
    */
}

DomainParametersDialog::~DomainParametersDialog()
{
    delete ui;
}

void DomainParametersDialog::setDomain(QString name)
{

}

void DomainParametersDialog::readParameters()
{
    qDebug() << "DomainParametersDialog::getParameters()";

    /*
     * Get the new domain from the combobox and populate the parameters from
     * the settings for that domain. (See also Domain::restoreDomains())
     */
    QString name = ui->cbDomainName->currentText();
    Domain *dom = Domain::getDomain(name);
    qDebug() << "Gettng parameters for domain:" << name;

    QSettings settings;
    settings.beginGroup("Domains");
    settings.beginGroup(name);

    /*
     * Copy settings into the dialog fields for the currently selected domain
     */
    foreach (ParamType p, Domain::parameterKeys.keys())
    {
        QString key_string = Domain::parameterKeys.value(p);
        if (settings.contains(key_string))
        {
            int val =  settings.value(key_string).toInt();
            dom->params[p] = val;

            /*
             * NB params emp_rate and active_pop have been provisionally
             * discontinued
             */

            switch(p)
            {
            case ParamType::procurement:
                ui->sbGovProcurement->setValue(val);
                break;

            case ParamType::prop_con:
                ui->sbPropConsInc->setValue(val);
                break;

            case ParamType::inc_tax_rate:
                ui->sbIncTax->setValue(val);
                break;

            case ParamType::inc_thresh:
                ui->sbTaxThresh->setValue(val);
                break;

            case ParamType::sales_tax_rate:
                ui->sbSalesTax->setValue(val);
                break;

            case ParamType::firm_creation_prob:
                ui->sbStartupProb->setValue(val);
                break;

            case ParamType::dedns:
                ui->sbDeductions->setValue(val);
                break;

            case ParamType::unemp_ben_rate:
                ui->sbBens->setValue(val);
                break;

            case ParamType::distrib:
                ui->sbPropInvest->setValue(val);
                break;

            case ParamType::prop_inv:
                ui->sbPropInvest->setValue(val);
                break;

            case ParamType::boe_int:
                ui->sbCentralBankInterest->setValue(val);
                break;

            case ParamType::bus_int:
                ui->sbClearingBankInterest->setValue(val);
                break;

            case ParamType::loan_prob:
                ui->sbLoanProb->setValue(val);
                break;

            case ParamType::recoup:
                ui->sbRecoup->setValue(val);
                break;

            default:

                // TO DO: Missing parameters
                // sbBasicInc, sbImportPref, sbropConsSav are not handled, no
                // paramType for standard wage (sbStdWage)


                // NB: We should maintain separate retail interest rates for
                // saving and borrowing. At the moment we only have one, which
                // is implicitly a borrowing rate

                Q_ASSERT(false);
            }
        }
        else
        {
            qWarning() << "Parameter" << key_string
                       << "is missing from settings for"
                       << dom->getName();
        }
    }

    settings.endGroup();
    settings.endGroup();

}


QString DomainParametersDialog::getDomain()
{
    return ui->cbDomainName->currentText();
}

int DomainParametersDialog::getProcurement()
{
    return ui->sbGovProcurement->value();
}

int DomainParametersDialog::getPropConsumeInc()
{
    return ui->sbPropConsInc->value();
}

int DomainParametersDialog::getIncTaxRate()
{
    return ui->sbIncTax->value();
}

int DomainParametersDialog::getIncTaxThresh()
{
    return ui->sbTaxThresh->value();
}

int DomainParametersDialog::getSalesTaxRate()
{
    return ui->sbSalesTax->value();
}

int DomainParametersDialog::getStartupProb()
{
    return ui->sbStartupProb->value();
}

int DomainParametersDialog::getDedns()
{
    return ui->sbDeductions->value();
}

int DomainParametersDialog::getUnempBen()
{
    return ui->sbBens->value();
}

int DomainParametersDialog::getPropInvest()
{
    return ui->sbPropInvest->value();
}

int DomainParametersDialog::getDistrib()
{
    Q_ASSERT(false);
    return 0;
}

int DomainParametersDialog::getCBInterest()
{
    return ui->sbCentralBankInterest->value();
}

int DomainParametersDialog::getClearingBankInterest()
{
    return ui->sbClearingBankInterest->value();
}

int DomainParametersDialog::getLoanProb()
{
    return ui->sbLoanProb->value();
}

int DomainParametersDialog::getRecoupPeriods()
{
    return ui->sbRecoup->value();
}


