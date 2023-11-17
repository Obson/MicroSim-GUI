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
    getParameters();

    // Ignore the compiler warning -- I've done it this way for consistency with
    // the SIGNAL template as defined for QObject::connect...
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

void DomainParametersDialog::getParameters()
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

    // See also Domain::restoreDomains...

    /*
     * Copy settings into the dialog fields for the currently selected domain
     */
    foreach (ParamType p, Domain::parameterKeys.keys())
    {
        QString key_string = Domain::parameterKeys.value(p);
        if (settings.contains(key_string))
        {
            dom->params[p] =  settings.value(key_string).toInt();

            /*
             * NB params emp_rate and active_pop have been provisionally
             * discontinued
             */
            switch(p)
            {
            case ParamType::procurement:
                break;

            case ParamType::prop_con:
                break;

            case ParamType::inc_tax_rate:
                break;

            case ParamType::inc_thresh:
                break;

            case ParamType::sales_tax_rate:
                break;

            case ParamType::firm_creation_prob:
                break;

            case ParamType::dedns:
                break;

            case ParamType::unemp_ben_rate:
                break;

            case ParamType::distrib:
                break;

            case ParamType::prop_inv:
                break;

            case ParamType::boe_int:
                break;

            case ParamType::bus_int:
                break;

            case ParamType::loan_prob:
                break;

            case ParamType::recoup:
                break;

            default:
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

