#include "parameterwizard.h"
#include <QFormLayout>
#include <QLabel>
#include <QTextEdit>
#include <QLineEdit>
#include <QComboBox>
#include <QtDebug>
#include <QtGlobal>
#include <QListWidgetItem>

ParameterWizard::ParameterWizard(QWidget *parent) : QWizard(parent)
{
    // WARNING: During testing (at least) we don't want to clutter up the settings
    // with values we can't remove with a settings.clear() call. This can probably
    // be changed once it's all working.
    // settings.setFallbacksEnabled(false);

    // TODO (Important): We seem to have lost the 'Add conditional parameters'
    // button.

    setWindowTitle("MicroSim Parameter Setup Wizard");

    setButtonText(QWizard::CustomButton1, tr("&Add conditional parameters"));
    setOption(QWizard::HaveCustomButton1, false);
    setOption(QWizard::NoCancelButton, false);
    setOption(QWizard::CancelButtonOnLeft, true);

    import_model.clear();

    setMinimumHeight(560);

    props << "Current period"
          << "Govt expenditure excl benefits"
          << "Govt expenditure incl benefits"
          << "Benefits paid"
          << "Government receipts"
          << "Deficit (absolute)"
          << "Deficit as % of GDP (consumption)"
          << "Government sector balance"
          << "Number of businesses"
          << "Number employed"
          << "Number unemployed"
          << "Percent unemployed"
          << "Percent employed"
          << "Percent active"
          << "Number of new hires"
          << "Number of new fires"
          << "Business sector balance"
          << "Wages paid"
          << "Consumption (sales/purchases)"
          << "Bonuses paid"
          << "Pre-tax deductions made"
          << "Income tax"
          << "Sales/purchase tax"
          << "Domestic sector balance";

    rels << "is less than"
         << "is equal to"
         << "is more than"
         << "is not less than"
         << "is not equal to"
         << "is not more than";

    connect(this, &QWizard::customButtonClicked, this, &ParameterWizard::createNewPage);
}

void ParameterWizard::importFrom(QString model_name)
{
    import_model = model_name;
}

void ParameterWizard::setCurrentModel(QString model_name)
{
    qDebug() << "ParameterWizard::setCurrentModel(): model_name =" << model_name;

    // TODO: Build pages from settings for current_model. At the moment we just
    // have a default page unless the user adds more pages. For updating the
    // parameters we will need to create however many pages are needed.

    // First, remove all existing pages
    QList<int> page_ids = pageIds();
    foreach(int pid, page_ids)
    {
        qDebug() << "ParameterWizard::modelChanged(): removing page" << pid;
        removePage(pid);
    }

    // Then add a default page for the new model
    current_model = model_name;
    qDebug() << "ParameterWizard::modelChanged(): adding default page";
    setPage(default_page, new DefaultPage(this));
    qDebug() << "ParameterWizard::modelChanged(): restarting";
    //restart();

    // and then extra pages if needed
}

void ParameterWizard::done(int result)
{
    qDebug() << "ParameterWizard::done() called: result =" << result << ", page id =" << currentId();
    if (result == QDialog::Accepted) {
        currentPage()->validatePage();
    }
    QDialog::done(result);
}

///
/// \brief ParameterWizard::currentIdChanged
/// \param id New wizard page id
///
void ParameterWizard::currentIdChanged(int id)
{
    // WARNING: Check on id here was to prevent crash if they cancelled from the
    // first page, but this no longer seems to be necessary. Check some time...
    setOption(QWizard::HaveCustomButton1, /*id > 0 &&*/ currentPage()->isFinalPage());
}

void ParameterWizard::createNewPage()
{
    ExtraPage *page = new ExtraPage(this);
    addPage(page);
    next();
}

///
/// \brief ParameterWizard::getSpinBox
/// \param min Minimum value the spinbox can hold
/// \param max Maximum value the spinbox can hold
/// \return a pointer to a new QSpinBox with the given min and max values
///
QSpinBox *ParameterWizard::getSpinBox(int min, int max)
{
    QSpinBox *sb = new QSpinBox;
    sb->setMinimum(min);
    sb->setMaximum(max);
    return sb;
}

DefaultPage::DefaultPage(ParameterWizard *w)
{
    wiz = w;

    sb_emp_rate = wiz->getSpinBox(0, 100);
    sb_prop_con = wiz->getSpinBox(0, 100);
    sb_dedns = wiz->getSpinBox(0, 100);
    sb_inc_tax = wiz->getSpinBox(0, 100);
    sb_sales_tax = wiz->getSpinBox(0, 100);
    sb_bcr = wiz->getSpinBox(0, 100);
    sb_distrib = wiz->getSpinBox(0, 100);
    sb_prop_inv = wiz->getSpinBox(1, 100);
    sb_ubr = wiz->getSpinBox(0, 100);

    sb_boe_loan_int = wiz->getSpinBox(0,99);
    sb_bus_loan_int = wiz->getSpinBox(0,99);

    cb_loan_prob = new QComboBox;
    cb_loan_prob->addItem(tr("Never"));
    cb_loan_prob->addItem(tr("Rarely"));
    cb_loan_prob->addItem(tr("Sometimes"));
    cb_loan_prob->addItem(tr("Usually"));
    cb_loan_prob->addItem(tr("Always"));

    QFormLayout *layout = new QFormLayout;
    layout->addRow(new QLabel(tr("Target employment rate (%)")), sb_emp_rate);
    layout->addRow(new QLabel(tr("Propensity to consume (%)")), sb_prop_con);
    layout->addRow(new QLabel(tr("Pre-tax deductions (%)")), sb_dedns);
    layout->addRow(new QLabel(tr("Income tax (%)")), sb_inc_tax);
    layout->addRow(new QLabel(tr("Sales tax (%)")), sb_sales_tax);
    layout->addRow(new QLabel(tr("Business creation rate (%)")), sb_bcr);
    layout->addRow(new QLabel(tr("Profit distribution rate (%)")), sb_distrib);
    layout->addRow(new QLabel(tr("Investment rate (%)")), sb_prop_inv);
    layout->addRow(new QLabel(tr("Unemployment benefit (%)")), sb_ubr);
    layout->addRow(new QLabel(tr("BOE lending rate (%)")), sb_boe_loan_int);
    layout->addRow(new QLabel(tr("Retail lending rate (%)")), sb_bus_loan_int);
    layout->addRow(new QLabel(tr("Borrow if needed to pay wages")), cb_loan_prob);
    setLayout(layout);
}

void DefaultPage::readSettings(QString model)
{
    // Read the default parameters from settings.We use 'sensible' default
    // defaults so they will have a workable model to start with
    QSettings settings;
    sb_emp_rate->setValue(settings.value(model + "/default/employment-rate", 95).toInt());
    sb_prop_con->setValue(settings.value(model + "/default/propensity-to-consume", 80).toInt());
    sb_dedns->setValue(settings.value(model + "/default/pre-tax-dedns-rate", 0).toInt());
    sb_inc_tax->setValue(settings.value(model + "/default/income-tax-rate", 10).toInt());
    sb_sales_tax->setValue(settings.value(model + "/default/sales-tax-rate", 0).toInt());
    sb_bcr->setValue(settings.value(model + "/default/firm-creation-prob", 0).toInt());
    sb_distrib->setValue(settings.value(model + "/default/reserve-rate", 100).toInt());
    sb_prop_inv->setValue(settings.value(model + "/default/prop-invest", 2).toInt());
    sb_ubr->setValue(settings.value(model + "/default/unempl-benefit-rate", 60).toInt());
    sb_boe_loan_int->setValue(settings.value(model + "/default/boe-interest", 1).toInt());
    sb_bus_loan_int->setValue(settings.value(model + "/default/bus-interest", 3).toInt());
    cb_loan_prob->setCurrentIndex(settings.value(model + "/default/loan-prob", 0).toInt());
}

// This function should not be called unless wiz->current_model has been set.
void DefaultPage::initializePage()
{
    QString model = wiz->current_model;
    qDebug() << "DefaultPage::initialize(): current_model =" << model;
    setTitle(tr("Default Parameters For ") + model);
    readSettings(wiz->import_model.isEmpty() ? model : wiz->import_model);
}

bool DefaultPage::validatePage()
{
    qDebug() << "DefaultPage::validatePage()";

    QString model = wiz->current_model;

    QSettings settings;
    settings.setValue(model + "/default/employment-rate", sb_emp_rate->value());
    settings.setValue(model + "/default/propensity-to-consume", sb_prop_con->value());
    settings.setValue(model + "/default/pre-tax-dedns-rate", sb_dedns->value());
    settings.setValue(model + "/default/income-tax-rate", sb_inc_tax->value());
    settings.setValue(model + "/default/sales-tax-rate", sb_sales_tax->value());
    settings.setValue(model + "/default/firm-creation-prob", sb_bcr->value());
    settings.setValue(model + "/default/reserve-rate", sb_distrib->value());
    settings.setValue(model + "/default/prop-invest", sb_prop_inv->value());
    settings.setValue(model + "/default/unempl-benefit-rate", sb_ubr->value());

    settings.setValue(model + "/default/boe-interest", sb_boe_loan_int->value());
    settings.setValue(model + "/default/bus-interest", sb_bus_loan_int->value());

    settings.setValue(model + "/default/loan-prob", cb_loan_prob->currentIndex());

    return true;
}

ExtraPage::ExtraPage(ParameterWizard *w)
{
    wiz = w;

    setTitle(tr("Conditional Parameters"));

    QComboBox *comboProperty = new QComboBox;
    comboProperty->addItems(wiz->props);

    QComboBox *comboRel = new QComboBox;
    comboRel->addItems(wiz->rels);

    QLineEdit *leValue = new QLineEdit;

    QLabel *hdg_1 = new QLabel;
    hdg_1->setText("Condition:");

    QLabel *hdg_2 = new QLabel;
    hdg_2->setText("Parameters:");

    QVBoxLayout *top_layout = new QVBoxLayout;
    top_layout->addWidget(hdg_1);
    top_layout->addWidget(comboProperty);
    top_layout->addWidget(comboRel);
    top_layout->addWidget(leValue);
    top_layout->addWidget(hdg_2);

    QFormLayout *bottom_layout = new QFormLayout;
    bottom_layout->addRow(new QLabel(tr("Standard wage")), new QLineEdit);
    bottom_layout->addRow(new QLabel(tr("Target employment rate (%)")), wiz->getSpinBox(0, 100));
    bottom_layout->addRow(new QLabel(tr("Propensity to consume (%)")), wiz->getSpinBox(0, 100));
    bottom_layout->addRow(new QLabel(tr("Pre-tax deductions (%)")), wiz->getSpinBox(0, 100));
    bottom_layout->addRow(new QLabel(tr("Income tax (%)")), wiz->getSpinBox(0, 100));
    bottom_layout->addRow(new QLabel(tr("Sales tax (%)")), wiz->getSpinBox(0, 100));
    bottom_layout->addRow(new QLabel(tr("Business creation rate (%)")), wiz->getSpinBox(0, 100));
    bottom_layout->addRow(new QLabel(tr("Reserve rate (%)")), wiz->getSpinBox(0, 100));
    bottom_layout->addRow(new QLabel(tr("Investment rate (%)")), wiz->getSpinBox(0, 100));
    bottom_layout->addRow(new QLabel(tr("Unemployment benefit (%)")), wiz->getSpinBox(0, 100));

    QGridLayout *main_layout = new QGridLayout;
    main_layout->addLayout(top_layout, 0, 0, 2, 2);
    main_layout->addLayout(bottom_layout, 2, 0, 11, 2);
    setLayout(main_layout);

    QString page_id = QString::number(wiz->currentId());

    qDebug() << "ExtraPage::ExtraPage(): page_id =" << page_id;
}


