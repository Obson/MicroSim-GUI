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
    setWindowTitle("MicroSim Parameter Setup");
    setPixmap(QWizard::BackgroundPixmap, QPixmap(":/background.png"));

    setButtonText(QWizard::CustomButton1, tr("&Add conditionals"));
    setOption(QWizard::HaveCustomButton1, true);
    setOption(QWizard::NoCancelButton, false);
    setOption(QWizard::CancelButtonOnLeft, true);
    //setOption(QWizard::HaveFinishButtonOnEarlyPages, true);

    import_model.clear();

    setMinimumHeight(660);

    rels << "is less than"
         << "is equal to"
         << "is more than"
         << "is not less than"
         << "is not equal to"
         << "is not more than";

    connect(this, &QWizard::customButtonClicked, this, &ParameterWizard::createNewPage);
}

void ParameterWizard::setProperties(QMap<QString,Model::Property> map)
{
    // NOTE: This includes the preudo-series 'Zero reference line' and
    // 'Hundred reference line'. These serve no useful purpose here and
    // should be removed. Care needed if translating to make sure the
    // same translation is used here as in MainWindow. Any other redundant
    // series (names) should also be removed here.
    property_map = map;

    prop_names.append(map.keys());
    prop_names.removeOne(tr("Zero reference line"));
    prop_names.removeOne(tr("100 reference line"));

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

    // and then extra pages if needed
    QSettings settings;
    int num_pages = settings.value(model_name + "/pages", 1).toInt();
    qDebug() << "ParameterWizard::setCurrentModel():  existing pages =" << num_pages;
    for (int i = 0; i < num_pages - 1; i++) {
        ExtraPage *page = createNewPage();
        page->readSettings(current_model);
    }
}

void ParameterWizard::done(int result)
{
    qDebug() << "ParameterWizard::done() called: result =" << result << ", page id =" << currentId();
    if (result == QDialog::Accepted) {
        currentPage()->validatePage();
        QSettings settings;
        settings.setValue(current_model + "/pages", this->pageIds().count());
    }
    QDialog::done(result);
}

int ParameterWizard::nextId() const
{
    // This is a 'read-once' value
    return next_id == -1 ? QWizard::nextId() : next_id;
}

void ParameterWizard::currentIdChanged(int id)
{
    qDebug() << "ParameterWizard::currentIdChanged():  id =" << id << ",  isFinalPage() returns" << currentPage()->isFinalPage();
    setOption(QWizard::HaveCustomButton1, currentPage()->isFinalPage());
}

ExtraPage *ParameterWizard::createNewPage()
{
    ExtraPage *page = new ExtraPage(this);
    int id = addPage(page);
    qDebug() << "ParameterWizard::createNewPage():  new page id =" << id;
    page->setPageNumber(id);
    page->setFinalPage(true);
    next_id = id;
    next();
    next_id = -1;
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

    le_dir_exp_rate = new QLineEdit();
    le_dir_exp_rate->setFixedWidth(48);

    le_thresh = new QLineEdit();
    le_thresh->setFixedWidth(48);

    sb_emp_rate = wiz->getSpinBox(0, 100);
    sb_prop_con = wiz->getSpinBox(0, 100);
    sb_dedns = wiz->getSpinBox(0, 100);
    sb_inc_tax = wiz->getSpinBox(0, 100);
    sb_sales_tax = wiz->getSpinBox(0, 100);
    sb_bcr = wiz->getSpinBox(0, 100);
    sb_recoup = wiz->getSpinBox(1, 100);
    sb_distrib = wiz->getSpinBox(0, 100);
    sb_prop_inv = wiz->getSpinBox(1, 100);
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

    layout->addRow(new QLabel(tr("<b>Government</b>")));
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
    layout->addRow(new QLabel(tr("BOE lending rate (%)")), sb_boe_loan_int);
    layout->addRow(new QLabel(tr("Retail lending rate (%)")), sb_bus_loan_int);

    layout->addRow(xb_locked);

    layout->setVerticalSpacing(3);

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
    sb_distrib->setValue(settings.value(model + "/default/reserve-rate", 100).toInt());
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
    QString model = wiz->current_model;
    qDebug() << "DefaultPage::initialize(): current_model =" << model;
    setTitle(tr("Default Parameters For ") + model);
    readSettings(wiz->import_model.isEmpty() ? model : wiz->import_model);
}

bool DefaultPage::validatePage()
{
    qDebug() << "DefaultPage::validatePage()";

    QString model = wiz->current_model;

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

ExtraPage::ExtraPage(ParameterWizard *w)
{
    wiz = w;

    qDebug() << "ExtraPage::ExtraPage():  called";

    cb_property = new QComboBox;
    cb_property->addItems(wiz->prop_names);

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

    le_dir_exp_rate = new QLineEdit();
    le_dir_exp_rate->setFixedWidth(48);

    le_thresh = new QLineEdit();
    le_thresh->setFixedWidth(48);

    sb_emp_rate = wiz->getSpinBox(0, 100);
    sb_prop_con = wiz->getSpinBox(0, 100);
    sb_dedns = wiz->getSpinBox(0, 100);
    sb_inc_tax = wiz->getSpinBox(0, 100);
    sb_sales_tax = wiz->getSpinBox(0, 100);
    sb_bcr = wiz->getSpinBox(0, 100);
    sb_recoup = wiz->getSpinBox(1, 100);
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

    QFormLayout *bottom_layout = new QFormLayout;
    bottom_layout->addRow(new QLabel(tr("<b>Government</b>")));
    bottom_layout->addRow(new QLabel(tr("Periodic procurement expenditure")), le_dir_exp_rate);
    bottom_layout->addRow(new QLabel(tr("Unemployment benefit (%)")), sb_ubr);

    bottom_layout->addRow(new QLabel(tr("<b>Workers</b>")));
    bottom_layout->addRow(new QLabel(tr("Propensity to consume (%)")), sb_prop_con);
    bottom_layout->addRow(new QLabel(tr("Income tax (%)")), sb_inc_tax);
    bottom_layout->addRow(new QLabel(tr("Income threshold")), le_thresh);

    bottom_layout->addRow(new QLabel(tr("<b>Businesses</b>")));
    bottom_layout->addRow(new QLabel(tr("Pre-tax deductions (%)")), sb_dedns);
    bottom_layout->addRow(new QLabel(tr("Sales tax (%)")), sb_sales_tax);
    bottom_layout->addRow(new QLabel(tr("Profit distribution rate (%)")), sb_distrib);
    bottom_layout->addRow(new QLabel(tr("Investment rate (%)")), sb_prop_inv);
    bottom_layout->addRow(new QLabel(tr("Borrow if needed to pay wages")), cb_loan_prob);
    bottom_layout->addRow(new QLabel(tr("Business creation rate (%)")), sb_bcr);
    bottom_layout->addRow(new QLabel(tr("Time (periods) to recoup capex")), sb_recoup);

    bottom_layout->addRow(new QLabel(tr("<b>Banks</b>")));
    bottom_layout->addRow(new QLabel(tr("BOE lending rate (%)")), sb_boe_loan_int);
    bottom_layout->addRow(new QLabel(tr("Retail lending rate (%)")), sb_bus_loan_int);

    bottom_layout->setVerticalSpacing(3);

    QGridLayout *main_layout = new QGridLayout;
    main_layout->addLayout(top_layout, 0, 0, 2, 2);
    main_layout->addLayout(bottom_layout, 2, 0, 12, 2);
    setLayout(main_layout);
}

void ExtraPage::setPageNumber(int page_num)
{
    qDebug() << "ExtraPage::setPageNumber():  page_num =" << page_num;
    pnum = page_num;
    setTitle(tr("Conditional Parameters - Page ") + QString::number(page_num));


    readSettings(wiz->import_model.isEmpty() ? wiz->current_model : wiz->import_model);
}

// Attempts to read setting from the settings for the current page. if it
// cannot be found, read the default setting instead
QString ExtraPage::readCondSetting(QString model, QString key)
{
    QSettings settings;
    QString base1 = model + "/condition-" + QString::number(pnum) + "/";
    QString base2 = model + "/default/";
    return (settings.value(base1 + key, settings.value(base2 + key)).toString());
}

QString ExtraPage::getPropertyName(int prop)
{
    // Convert the int to a property
    Model::Property p = static_cast<Model::Property>(prop);

    // and look it up in the property map
    QMapIterator<QString, Model::Property> it(wiz->property_map);
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

void ExtraPage::readSettings(QString model)
{
    qDebug() << "ExtraPage::readSettings():  from model =" << model;

    QSettings settings;
    QString base = model + "/condition-" + QString::number(pnum) +"/";

    cb_property->setCurrentText(getPropertyName(settings.value(base + "property", 0).toInt()));
    cb_rel->setCurrentIndex(settings.value(base + "rel", 3).toInt());
    le_value->setText(settings.value(base + "value", 0).toString());

    le_dir_exp_rate->setText(readCondSetting(model, "govt-procurement"));
    le_thresh->setText(readCondSetting(model, "income-threshold"));
    sb_emp_rate->setValue(readCondSetting(model, "employment-rate").toInt());
    sb_prop_con->setValue(readCondSetting(model, "propensity-to-consume").toInt());
    sb_dedns->setValue(readCondSetting(model, "pre-tax-dedns-rate").toInt());
    sb_inc_tax->setValue(readCondSetting(model, "income-tax-rate").toInt());
    sb_sales_tax->setValue(readCondSetting(model, "sales-tax-rate").toInt());
    sb_bcr->setValue(readCondSetting(model, "firm-creation-prob").toInt());
    sb_recoup->setValue(readCondSetting(model, "capex-recoup-periods").toInt());
    sb_distrib->setValue(readCondSetting(model, "reserve-rate").toInt());
    sb_prop_inv->setValue(readCondSetting(model, "prop-invest").toInt());
    sb_ubr->setValue(readCondSetting(model, "unempl-benefit-rate").toInt());
    sb_boe_loan_int->setValue(readCondSetting(model, "boe-interest").toInt());
    sb_bus_loan_int->setValue(readCondSetting(model, "bus-interest").toInt());
    cb_loan_prob->setCurrentIndex(readCondSetting(model, "loan-prob").toInt());
}

bool ExtraPage::validatePage()
{
    qDebug() << "ExtraPage::validatePage()";

    QString model = wiz->current_model;
    QString key = model + "/condition-" + QString::number(pnum) + "/";

    // TODO: We don't currently do any actual validation here.
    // We just save the default settings assuming they're OK.

    QSettings settings;
    bool ok;

    // We need to store an actual property (Model::Property enum) here
    QString selected_text = cb_property->currentText();
    Model::Property prop = wiz->property_map.value(selected_text);
    settings.setValue(key + "property", static_cast<int>(prop));

    settings.setValue(key + "rel", cb_rel->currentIndex());

    // TODO: We really ought to do something if !ok, but for now we'll
    // just treat it as zero. If we force it to be numeric (integral)
    // we won't need to check it.
    int val = le_value->text().toInt(&ok);
    if (!ok) {
        val = 0;
    }
    settings.setValue(key + "value", val);

    settings.setValue(key + "govt-procurement", le_dir_exp_rate->text().toInt());
    settings.setValue(key + "employment-rate", sb_emp_rate->value());
    settings.setValue(key + "propensity-to-consume", sb_prop_con->value());
    settings.setValue(key + "income-threshold", le_thresh->text().toInt());
    settings.setValue(key + "pre-tax-dedns-rate", sb_dedns->value());
    settings.setValue(key + "income-tax-rate", sb_inc_tax->value());
    settings.setValue(key + "sales-tax-rate", sb_sales_tax->value());
    settings.setValue(key + "firm-creation-prob", sb_bcr->value());
    settings.setValue(key + "capex-recoup-periods", sb_recoup->value());

    settings.setValue(key + "reserve-rate", sb_distrib->value());
    settings.setValue(key + "prop-invest", sb_prop_inv->value());
    settings.setValue(key + "unempl-benefit-rate", sb_ubr->value());

    settings.setValue(key + "boe-interest", sb_boe_loan_int->value());
    settings.setValue(key + "bus-interest", sb_bus_loan_int->value());

    settings.setValue(key + "loan-prob", cb_loan_prob->currentIndex());

    return true;

}


