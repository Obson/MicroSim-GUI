#include "parameterwizard.h"
#include <QFormLayout>
#include <QLabel>
#include <QTextEdit>
#include <QLineEdit>
#include <QComboBox>
#include <QtDebug>
#include <QtGlobal>
#include <QListWidgetItem>
#include <QPixmap>

ParameterWizard::ParameterWizard(QWidget *parent) : QWizard(parent)
{
    setWindowTitle("Behaviour Definition");
    setPixmap(QWizard::BackgroundPixmap, QPixmap(":/background3.png"));
    setButtonText(QWizard::CustomButton1, tr("&Add conditionals"));
    setOption(QWizard::HaveCustomButton1, true);
    setOption(QWizard::NoCancelButton, false);
    setOption(QWizard::CancelButtonOnLeft, true);

    //setOption(QWizard::HaveFinishButtonOnEarlyPages, true);

    importDomain.clear();

    setMinimumHeight(660);

    rels << "is less than"
         << "is equal to"
         << "is more than"
         << "is not less than"
         << "is not equal to"
         << "is not more than";

    connect(this, &QWizard::customButtonClicked, this, &ParameterWizard::createNewPage);
}

void ParameterWizard::setProperties(QMap<QString, Property> map)
{
    // NOTE: This includes the preudo-series 'Zero reference line' and
    // 'Hundred reference line'. These serve no useful purpose here and
    // should be removed. Care needed if translating to make sure the
    // same translation is used here as in MainWindow. Any other redundant
    // series (names) should also be removed here.

    propertyMap = map;

    propertyNames.append(map.keys());
    propertyNames.removeOne(tr("Zero reference line"));
    propertyNames.removeOne(tr("100 reference line"));

}

void ParameterWizard::importFrom(QString domainName)
{
    importDomain = domainName;
}

void ParameterWizard::setCurrentDomain(QString domainName)
{
    qDebug() << "ParameterWizard::setCurrentBehaviour(): model_name =" << domainName;

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
    currentBehaviour = domainName;
    qDebug() << "ParameterWizard::modelChanged(): adding default page";
    setPage(default_page, new DefaultPage(this));
    qDebug() << "ParameterWizard::modelChanged(): restarting";

    // and then extra pages if needed
    QSettings settings;
    int num_pages = settings.value(domainName + "/pages", 1).toInt();
    qDebug() << "ParameterWizard::setCurrentModel():  existing pages =" << num_pages;
    for (int i = 0; i < num_pages - 1; i++) {
        ExtraPage *page = createNewPage();
        page->readSettings(currentBehaviour);
    }
}

void ParameterWizard::done(int result)
{
    qDebug() << "ParameterWizard::done() called: result =" << result << ", page id =" << currentId();
    if (result == QDialog::Accepted) {
        currentPage()->validatePage();
        QSettings settings;
        settings.setValue(currentBehaviour + "/pages", this->pageIds().count());
    }
    QDialog::done(result);
}

int ParameterWizard::nextId() const
{
    // This is a 'read-once' value
    return nextPageNumber == -1 ? QWizard::nextId() : nextPageNumber;
}

void ParameterWizard::currentIdChanged(int id)
{
    qDebug() << "ParameterWizard::currentIdChanged():  id =" << id << ",  isFinalPage() returns" << currentPage()->isFinalPage();
    setOption(QWizard::HaveCustomButton1, currentPage()->isFinalPage());
}

ExtraPage *ParameterWizard::createNewPage()
{
    ExtraPage *page = new ExtraPage(this);
    int pageNum = addPage(page);
    qDebug() << "ParameterWizard::createNewPage():  new page id =" << pageNum;
    page->setPageNumber(pageNum);
    page->setFinalPage(true);
    nextPageNumber = pageNum;
    next();
    nextPageNumber = -1;
    return page;
}

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

    int width = 48; // change as required

    leCurrencyName = new QLineEdit();
    leCurrencyName->setFixedWidth(width);

    leCurrencyAbbrev = new QLineEdit();
    leCurrencyAbbrev->setFixedWidth(width);

    le_dir_exp_rate = new QLineEdit();
    le_dir_exp_rate->setFixedWidth(width);

    le_thresh = new QLineEdit();
    le_thresh->setFixedWidth(width);

    sb_emp_rate = wiz->getSpinBox(0, 100);
    sb_prop_con = wiz->getSpinBox(0, 100);
    sb_dedns = wiz->getSpinBox(0, 100);
    sb_inc_tax = wiz->getSpinBox(0, 100);
    sb_sales_tax = wiz->getSpinBox(0, 100);
    sb_bcr = wiz->getSpinBox(0, 100);
    sb_recoup = wiz->getSpinBox(1, 100);
    sb_distrib = wiz->getSpinBox(0, 100);
    sb_prop_inv = wiz->getSpinBox(0, 100);
    sb_ubr = wiz->getSpinBox(0, 100);

    sb_boe_loan_int = wiz->getSpinBox(0,99);
    sb_bus_loan_int = wiz->getSpinBox(0,99);

    xb_locked = new QCheckBox("  Locked");

    cb_loan_prob = new QComboBox;
    cb_loan_prob->addItem(tr("Never"));
    cb_loan_prob->addItem(tr("Rarely"));
    cb_loan_prob->addItem(tr("Sometimes"));
    cb_loan_prob->addItem(tr("Usually"));
    cb_loan_prob->addItem(tr("Always"));

    QFormLayout *layout = new QFormLayout;

    layout->addRow(new QLabel(tr("Currency name")));
    layout->addRow(new QLabel(tr("Currency abbreviation")));

    layout->addRow(new QLabel(tr("<b>Government</b>")));

    layout->addRow(new QLabel(tr("Currency name")));
    layout->addRow(new QLabel(tr("Currency abbreviation")));

    layout->addRow(new QLabel(tr("Periodic procurement expenditure")), le_dir_exp_rate);
    layout->addRow(new QLabel(tr("Unemployment benefit (%)")), sb_ubr);

    layout->addRow(new QLabel(tr("<b>Workers</b>")));
    layout->addRow(new QLabel(tr("Propensity to consume (%)")), sb_prop_con);
    layout->addRow(new QLabel(tr("Income tax (%)")), sb_inc_tax);
    layout->addRow(new QLabel(tr("Income threshold")), le_thresh);

    layout->addRow(new QLabel(tr("<b>Businesses</b>")));
    layout->addRow(new QLabel(tr("Pre-tax deductions (%)")), sb_dedns);
    layout->addRow(new QLabel(tr("Sales tax (%)")), sb_sales_tax);
    layout->addRow(new QLabel(tr("Profit distribution rate (%)")), sb_distrib);
    layout->addRow(new QLabel(tr("Investment rate (%)")), sb_prop_inv);
    layout->addRow(new QLabel(tr("Borrow if needed to pay wages")), cb_loan_prob);
    layout->addRow(new QLabel(tr("Business creation rate (%)")), sb_bcr);
    layout->addRow(new QLabel(tr("Time (periods) to recoup capex")), sb_recoup);

    layout->addRow(new QLabel(tr("<b>Banks</b>")));
    layout->addRow(new QLabel(tr("Central Bank lending rate (%)")), sb_boe_loan_int);
    layout->addRow(new QLabel(tr("Retail Banks lending rate (%)")), sb_bus_loan_int);

    layout->addRow(new QLabel(tr("<b> </b>")));
    layout->addRow(new QLabel(tr("<b>Prevent changes</b>")));
    layout->addRow(xb_locked);

    layout->setVerticalSpacing(4);

    setLayout(layout);
}

void DefaultPage::readSettings(QString model)
{
    // Read the default parameters from settings. We use 'sensible' default
    // defaults so they will have a workable model to start with
    QSettings settings;
    le_dir_exp_rate->setText(settings.value(model + "/default/govt-procurement", 0).toString());
    le_thresh->setText(settings.value(model + "/default/income-threshold", 50).toString());
    sb_emp_rate->setValue(settings.value(model + "/default/employment-rate", 95).toInt());
    sb_prop_con->setValue(settings.value(model + "/default/propensity-to-consume", 80).toInt());
    sb_dedns->setValue(settings.value(model + "/default/pre-tax-dedns-rate", 0).toInt());
    sb_inc_tax->setValue(settings.value(model + "/default/income-tax-rate", 10).toInt());
    sb_sales_tax->setValue(settings.value(model + "/default/sales-tax-rate", 0).toInt());
    sb_bcr->setValue(settings.value(model + "/default/firm-creation-prob", 0).toInt());
    sb_recoup->setValue(settings.value(model + "/default/capex-recoup-periods", 10).toInt());
    sb_distrib->setValue(settings.value(model + "/default/reserve-rate", 50).toInt());
    sb_prop_inv->setValue(settings.value(model + "/default/prop-invest", 2).toInt());
    sb_ubr->setValue(settings.value(model + "/default/unempl-benefit-rate", 60).toInt());
    sb_boe_loan_int->setValue(settings.value(model + "/default/boe-interest", 1).toInt());
    sb_bus_loan_int->setValue(settings.value(model + "/default/bus-interest", 3).toInt());
    cb_loan_prob->setCurrentIndex(settings.value(model + "/default/loan-prob", 0).toInt());
    xb_locked->setChecked(settings.value(model + "/locked", false).toBool());

    connect(xb_locked, &QCheckBox::toggled, this, &DefaultPage::toggleLock);
    toggleLock();
}

void DefaultPage::toggleLock()
{
    qDebug() << "DefaultPage::toggleLock(): called";

    bool unlocked = !xb_locked->isChecked();

    le_dir_exp_rate->setEnabled(unlocked);
    le_thresh->setEnabled(unlocked);

    sb_emp_rate->setEnabled(unlocked);
    sb_prop_con->setEnabled(unlocked);
    sb_dedns->setEnabled(unlocked);
    sb_inc_tax->setEnabled(unlocked);
    sb_sales_tax->setEnabled(unlocked);
    sb_bcr->setEnabled(unlocked);
    sb_recoup->setEnabled(unlocked);
    sb_distrib->setEnabled(unlocked);
    sb_prop_inv->setEnabled(unlocked);
    sb_ubr->setEnabled(unlocked);
    sb_boe_loan_int->setEnabled(unlocked);
    sb_bus_loan_int->setEnabled(unlocked);

    cb_loan_prob->setEnabled(unlocked);
}

// This function should not be called unless wiz->current_model has been set.
void DefaultPage::initializePage()
{
    QString model = wiz->currentBehaviour;
    qDebug() << "DefaultPage::initialize(): currentBehaviourl =" << model;
    setTitle(tr("Default Profile For ") + model);
    readSettings(wiz->importDomain.isEmpty() ? model : wiz->importDomain);
}

bool DefaultPage::validatePage()
{
    qDebug() << "DefaultPage::validatePage()";

    QString model = wiz->currentBehaviour;

    // TODO: We don't currently do any actual validation here.
    // We just save the default settings assuming they're OK.

    QSettings settings;
    settings.setValue(model + "/default/govt-procurement", le_dir_exp_rate->text().toInt());
    settings.setValue(model + "/default/employment-rate", sb_emp_rate->value());
    settings.setValue(model + "/default/propensity-to-consume", sb_prop_con->value());
    settings.setValue(model + "/default/income-threshold", le_thresh->text().toInt());
    settings.setValue(model + "/default/pre-tax-dedns-rate", sb_dedns->value());
    settings.setValue(model + "/default/income-tax-rate", sb_inc_tax->value());
    settings.setValue(model + "/default/sales-tax-rate", sb_sales_tax->value());
    settings.setValue(model + "/default/firm-creation-prob", sb_bcr->value());
    settings.setValue(model + "/default/capex-recoup-periods", sb_recoup->value());

    settings.setValue(model + "/default/reserve-rate", sb_distrib->value());
    settings.setValue(model + "/default/prop-invest", sb_prop_inv->value());
    settings.setValue(model + "/default/unempl-benefit-rate", sb_ubr->value());

    settings.setValue(model + "/default/boe-interest", sb_boe_loan_int->value());
    settings.setValue(model + "/default/bus-interest", sb_bus_loan_int->value());

    settings.setValue(model + "/default/loan-prob", cb_loan_prob->currentIndex());

    settings.setValue(model + "/locked", xb_locked->isChecked());

    return true;
}

////////////////////////////////////////////////////////////////////////////////
/// \brief ExtraPage::ExtraPage
/// \param w
///

ExtraPage::ExtraPage(ParameterWizard *w)
{
    wiz = w;

    qDebug() << "ExtraPage::ExtraPage():  called";

    cb_property = new QComboBox;
    cb_property->addItems(wiz->propertyNames);

    cb_rel = new QComboBox;
    cb_rel->addItems(wiz->rels);
    le_value = new QLineEdit;

    QLabel *hdg_1 = new QLabel;
    hdg_1->setText("<b>Condition:</b>");

    QVBoxLayout *top_layout = new QVBoxLayout;
    top_layout->addWidget(hdg_1);
    top_layout->addWidget(cb_property);
    top_layout->addWidget(cb_rel);
    top_layout->addWidget(le_value);

    top_layout->setSpacing(3);

    cbx_dir_exp_rate = createCheckBox();
    cbx_thresh = createCheckBox();
    cbx_prop_con = createCheckBox();
    cbx_dedns = createCheckBox();
    cbx_inc_tax = createCheckBox();
    cbx_sales_tax = createCheckBox();
    cbx_bcr = createCheckBox();
    cbx_recoup = createCheckBox();
    cbx_distrib = createCheckBox();
    cbx_prop_inv = createCheckBox();
    cbx_ubr = createCheckBox();
    cbx_boe_loan_int = createCheckBox();
    cbx_bus_loan_int = createCheckBox();
    cbx_loan_prob = createCheckBox();

    le_dir_exp_rate = new QLineEdit();
    le_dir_exp_rate->setFixedWidth(48);

    le_thresh = new QLineEdit();
    le_thresh->setFixedWidth(48);

    sb_prop_con = wiz->getSpinBox(0, 100);
    sb_dedns = wiz->getSpinBox(0, 100);
    sb_inc_tax = wiz->getSpinBox(0, 100);
    sb_sales_tax = wiz->getSpinBox(0, 100);
    sb_bcr = wiz->getSpinBox(0, 100);
    sb_recoup = wiz->getSpinBox(0, 100);
    sb_distrib = wiz->getSpinBox(0, 100);
    sb_prop_inv = wiz->getSpinBox(0, 100);
    sb_ubr = wiz->getSpinBox(0, 100);

    sb_boe_loan_int = wiz->getSpinBox(0,99);
    sb_bus_loan_int = wiz->getSpinBox(0,99);

    cb_loan_prob = new QComboBox;
    cb_loan_prob->addItem(tr("Never"));
    cb_loan_prob->addItem(tr("Rarely"));
    cb_loan_prob->addItem(tr("Sometimes"));
    cb_loan_prob->addItem(tr("Usually"));
    cb_loan_prob->addItem(tr("Always"));

    QGridLayout *bottom_layout = new QGridLayout;

    bottom_layout->addWidget(new QLabel(tr("<b>Government</b>")), 0, 0, 1, 3);

    bottom_layout->addWidget(cbx_dir_exp_rate, 1, 0);
    bottom_layout->addWidget(new QLabel(tr("Periodic procurement expenditure")), 1, 1);
    bottom_layout->addWidget(le_dir_exp_rate, 1, 2);

    bottom_layout->addWidget(cbx_ubr, 2, 0);
    bottom_layout->addWidget(new QLabel(tr("Unemployment benefit (%)")), 2, 1);
    bottom_layout->addWidget(sb_ubr, 2, 2);

    bottom_layout->addWidget(new QLabel(tr("<b>Workers</b>")), 3, 0, 1, 3);

    bottom_layout->addWidget(cbx_prop_con, 4, 0);
    bottom_layout->addWidget(new QLabel(tr("Propensity to consume (%)")), 4, 1);
    bottom_layout->addWidget(sb_prop_con, 4, 2);

    bottom_layout->addWidget(cbx_inc_tax, 5, 0);
    bottom_layout->addWidget(new QLabel(tr("Income tax (%)")), 5, 1);
    bottom_layout->addWidget(sb_inc_tax, 5, 2);

    bottom_layout->addWidget(cbx_thresh, 6, 0);
    bottom_layout->addWidget(new QLabel(tr("Income threshold")), 6, 1);
    bottom_layout->addWidget(le_thresh, 6, 2);

    bottom_layout->addWidget(new QLabel(tr("<b>Businesses</b>")), 7, 0, 1, 3);

    bottom_layout->addWidget(cbx_dedns, 8, 0);
    bottom_layout->addWidget(new QLabel(tr("Pre-tax deductions (%)")), 8, 1);
    bottom_layout->addWidget(sb_dedns, 8, 2);

    bottom_layout->addWidget(cbx_sales_tax, 9, 0);
    bottom_layout->addWidget(new QLabel(tr("Sales tax (%)")), 9, 1);
    bottom_layout->addWidget(sb_sales_tax, 9, 2);

    bottom_layout->addWidget(cbx_distrib, 10, 0);
    bottom_layout->addWidget(new QLabel(tr("Profit distribution rate (%)")), 10, 1);
    bottom_layout->addWidget(sb_distrib, 10, 2);

    bottom_layout->addWidget(cbx_prop_inv, 11, 0);
    bottom_layout->addWidget(new QLabel(tr("Investment rate (%)")), 11, 1);
    bottom_layout->addWidget(sb_prop_inv, 11, 2);

    bottom_layout->addWidget(cbx_loan_prob, 12, 0);
    bottom_layout->addWidget(new QLabel(tr("Borrow if needed to pay wages")), 12, 1);
    bottom_layout->addWidget(cb_loan_prob, 12, 2);

    bottom_layout->addWidget(cbx_bcr, 13, 0);
    bottom_layout->addWidget(new QLabel(tr("Business creation rate (%)")), 13, 1);
    bottom_layout->addWidget(sb_bcr, 13, 2);

    bottom_layout->addWidget(cbx_recoup, 14, 0);
    bottom_layout->addWidget(new QLabel(tr("Time (periods) to recoup capex")), 14, 1);
    bottom_layout->addWidget(sb_recoup, 14, 2);

    bottom_layout->addWidget(new QLabel(tr("<b>Banks</b>")), 15, 0, 1, 3);

    bottom_layout->addWidget(cbx_boe_loan_int, 16, 0);
    bottom_layout->addWidget(new QLabel(tr("Central Bank lending rate (%)")), 16, 1);
    bottom_layout->addWidget(sb_boe_loan_int, 16,2);

    bottom_layout->addWidget(cbx_bus_loan_int, 17, 0);
    bottom_layout->addWidget(new QLabel(tr("Retail lending rate (%)")), 17, 1);
    bottom_layout->addWidget(sb_bus_loan_int, 17, 2);

    bottom_layout->setVerticalSpacing(3);

    QGridLayout *main_layout = new QGridLayout;
    main_layout->addLayout(top_layout, 0, 0, 2, 2);
    main_layout->addLayout(bottom_layout, 2, 0, 12, 2);
    setLayout(main_layout);
}

QCheckBox *ExtraPage::createCheckBox()
{
    QCheckBox *cbx = new QCheckBox;
    connect(cbx, &QCheckBox::stateChanged, this, &ExtraPage::showAssocCtrl);
    return cbx;
}

void ExtraPage::showAssocCtrl(int)
{
    // Placeholder function in case we want to (e.g.) disable controls whose
    // associated checkbox in unchecked...
}

void ExtraPage::setPageNumber(int page_num)
{
    qDebug() << "ExtraPage::setPageNumber():  page_num =" << page_num;
    pnum = page_num;
    setTitle(tr("Conditional Parameters - Page ") + QString::number(page_num));
    readSettings(wiz->importDomain.isEmpty() ? wiz->currentBehaviour : wiz->importDomain);
}

// Attempts to read setting from the settings for the current page. if it
// cannot be found, read the default setting instead
QString ExtraPage::readCondSetting(QString behaviourName, QString key)
{
    QSettings settings;
    QString base1 = behaviourName + "/condition-" + QString::number(pnum) + "/" + key + "/value";
    QString base2 = behaviourName + "/default/" + key;
    QString res = settings.value(base1, settings.value(base2 + key)).toString();
    return res;
}

QString ExtraPage::getPropertyName(int prop)
{
    // Convert the int to a property
    Property p = static_cast<Property>(prop);

    // and look it up in the property map
    QMapIterator<QString,Property> it(wiz->propertyMap);
    while (it.hasNext())
    {
        it.next();
        if (it.value() == p) {
            return it.key();
        }
    }
    Q_ASSERT(false);
    return QString(""); // prevent compiler warning
}

bool ExtraPage::isChecked(QString behaviourName, QString attrib)
{
    QSettings settings;
    return settings.value(behaviourName + "/condition-" + QString::number(pnum) + "/" + attrib + "/isset", false).toBool();
}

void ExtraPage::readSettings(QString behaviourName)
{
    qDebug() << "ExtraPage::readSettings():  from model =" << behaviourName;

    QSettings settings;
    QString base = behaviourName + "/condition-" + QString::number(pnum) + "/";

    cb_property->setCurrentText(getPropertyName(settings.value(base + "property", 0).toInt()));
    cb_rel->setCurrentIndex(settings.value(base + "rel", 3).toInt());
    le_value->setText(settings.value(base + "value", 0).toString());

    QString attrib;

    attrib = "govt-procurement";
    cbx_dir_exp_rate->setChecked(isChecked(behaviourName, attrib));
    le_dir_exp_rate->setText(readCondSetting(behaviourName, attrib));

    attrib = "income-threshold";
    cbx_thresh->setChecked(isChecked(behaviourName, attrib));
    le_thresh->setText(readCondSetting(behaviourName, attrib));

    attrib = "propensity-to-consume";
    cbx_prop_con->setChecked(isChecked(behaviourName, attrib));
    sb_prop_con->setValue(readCondSetting(behaviourName, attrib).toInt());

    attrib = "pre-tax-dedns-rate";
    cbx_dedns->setChecked(isChecked(behaviourName, attrib));
    sb_dedns->setValue(readCondSetting(behaviourName, attrib).toInt());

    attrib = "income-tax-rate";
    cbx_inc_tax->setChecked(isChecked(behaviourName, attrib));
    sb_inc_tax->setValue(readCondSetting(behaviourName, attrib).toInt());

    attrib = "sales-tax-rate";
    cbx_sales_tax->setChecked(isChecked(behaviourName, attrib));
    sb_sales_tax->setValue(readCondSetting(behaviourName, attrib).toInt());

    attrib = "firm-creation-prob";
    cbx_bcr->setChecked(isChecked(behaviourName, attrib));
    sb_bcr->setValue(readCondSetting(behaviourName, attrib).toInt());

    attrib = "capex-recoup-periods";
    cbx_recoup->setChecked(isChecked(behaviourName, attrib));
    sb_recoup->setValue(readCondSetting(behaviourName, attrib).toInt());

    attrib = "reserve-rate";
    cbx_distrib->setChecked(isChecked(behaviourName, attrib));
    sb_distrib->setValue(readCondSetting(behaviourName, attrib).toInt());

    attrib = "prop-invest";
    cbx_prop_inv->setChecked(isChecked(behaviourName, attrib));
    sb_prop_inv->setValue(readCondSetting(behaviourName, attrib).toInt());

    attrib = "unempl-benefit-rate";
    cbx_ubr->setChecked(isChecked(behaviourName, attrib));
    sb_ubr->setValue(readCondSetting(behaviourName, attrib).toInt());

    attrib = "boe-interest";
    cbx_boe_loan_int->setChecked(isChecked(behaviourName, attrib));
    sb_boe_loan_int->setValue(readCondSetting(behaviourName, attrib).toInt());

    attrib = "bus-interest";
    cbx_bus_loan_int->setChecked(isChecked(behaviourName, attrib));
    sb_bus_loan_int->setValue(readCondSetting(behaviourName, attrib).toInt());

    attrib = "loan-prob";
    cbx_loan_prob->setChecked(isChecked(behaviourName, attrib));
    cb_loan_prob->setCurrentIndex(readCondSetting(behaviourName, attrib).toInt());
}

bool ExtraPage::validatePage()
{
    qDebug() << "ExtraPage::validatePage()";

    QString behaviourName = wiz->currentBehaviour;
    QString key = behaviourName + "/condition-" + QString::number(pnum) + "/";

    // TODO: We don't currently do any actual validation here.
    // We just save the settings assuming they're OK.

    QSettings settings;
    bool ok;

    // We need to store an actual property (Property enum) here, but we have
    // to convert it to an int first
    QString selected_text = cb_property->currentText();
    Property prop = wiz->propertyMap.value(selected_text);
    settings.setValue(key + "property", static_cast<int>(prop));

    settings.setValue(key + "rel", cb_rel->currentIndex());

    // TODO: force input value to be numeric and remove the OK check
    int val = le_value->text().toInt(&ok);
    if (!ok) {
        val = 0;
    }
    settings.setValue(key + "value", val);

    QString attrib;

    attrib = "govt-procurement/";
    settings.setValue(key + attrib + "isset", cbx_dir_exp_rate->isChecked());
    settings.setValue(key + attrib + "value", le_dir_exp_rate->text().toInt());

    attrib = "income-threshold/";
    settings.setValue(key + attrib + "isset", cbx_thresh->isChecked());
    settings.setValue(key + attrib + "value", le_thresh->text().toInt());

    attrib = "propensity-to-consume/";
    settings.setValue(key + attrib + "isset", cbx_prop_con->isChecked());
    settings.setValue(key + attrib + "value", sb_prop_con->value());

    attrib = "pre-tax-dedns-rate/";
    settings.setValue(key + attrib + "isset", cbx_dedns->isChecked());
    settings.setValue(key + attrib + "value", sb_dedns->value());

    attrib = "income-tax-rate/";
    settings.setValue(key + attrib + "isset", cbx_inc_tax->isChecked());
    settings.setValue(key + attrib + "value", sb_inc_tax->value());

    attrib = "sales-tax-rate/";
    settings.setValue(key + attrib + "isset", cbx_sales_tax->isChecked());
    settings.setValue(key + attrib + "value", sb_sales_tax->value());

    attrib = "firm-creation-prob/";
    settings.setValue(key + attrib + "isset", cbx_bcr->isChecked());
    settings.setValue(key + attrib + "value", sb_bcr->value());

    attrib = "capex-recoup-periods/";
    settings.setValue(key + attrib + "isset", cbx_recoup->isChecked());
    settings.setValue(key + attrib + "value", sb_recoup->value());

    attrib = "reserve-rate/";
    settings.setValue(key + attrib + "isset", cbx_distrib->isChecked());
    settings.setValue(key + attrib + "value", sb_distrib->value());

    attrib = "prop-invest/";
    settings.setValue(key + attrib + "isset", cbx_prop_inv->isChecked());
    settings.setValue(key + attrib + "value", sb_prop_inv->value());

    attrib = "unempl-benefit-rate/";
    settings.setValue(key + attrib + "isset", cbx_ubr->isChecked());
    settings.setValue(key + attrib + "value", sb_ubr->value());

    attrib = "boe-interest/";
    settings.setValue(key + attrib + "isset", cbx_boe_loan_int->isChecked());
    settings.setValue(key + attrib + "value", sb_boe_loan_int->value());

    attrib = "bus-interest/";
    settings.setValue(key + attrib + "isset", cbx_bus_loan_int->isChecked());
    settings.setValue(key + attrib + "value", sb_bus_loan_int->value());

    attrib = "loan-prob/";
    settings.setValue(key + attrib + "isset", cbx_loan_prob->isChecked());
    settings.setValue(key + attrib + "value", cb_loan_prob->currentIndex());

    return true;

}


