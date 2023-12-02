/*
 * domain.cpp
 */

#include "account.h"
#include <math.h>
#include "QtCore/qdebug.h"
#include <QSettings>
#include <QListWidgetItem>
#include <QSettings>
#include <QMessageBox>

#define NUMBER_OF_BANKS 3
#define CLEARING_FREQUENCY 10

/*
 * Statics
 */
QList<Domain*> Domain::domains;                         // static

const QMap<ParamType,QString> Domain::parameterKeys     // static
{
    {ParamType::procurement, "govt-procurement"},
    //{ParamType::emp_rate, "employment-rate"},         // probably redundant
    {ParamType::prop_con, "propensity-to-consume"},
    {ParamType::inc_tax_rate, "income-tax-rate"},
    {ParamType::inc_thresh, "income-threshold"},
    {ParamType::sales_tax_rate, "sales-tax-rate"},
    {ParamType::firm_creation_prob, "firm-creation-prob"},
    {ParamType::dedns, "pre-tax-dedns-rate"},
    {ParamType::unemp_ben_rate, "unempl-benefit-rate"},
    {ParamType::pop, "population"},
    {ParamType::distrib, "reserve-rate"},
    {ParamType::prop_inv, "prop-invest"},
    {ParamType::boe_int, "boe-interest"},
    {ParamType::bus_int, "bus-interest"},
    {ParamType::loan_prob, "loan-prob"},
    {ParamType::recoup, "capex-recoup-periods"},
    {ParamType::std_wage, "standard-wage"},
    {ParamType::gov_size, "government-size"},
};

QMap<QString,Property> Domain::propertyMap;  // static, can't be const as we have to initialise it

void Domain::initialisePropertyMap()                    // static
{
    static bool is_initialised = false;

    if (!is_initialised)
    {
        propertyMap[tr("100 reference line")] = Property::hundred;
        propertyMap[tr("Average business size")] = Property::bus_size;
        propertyMap[tr("Bank loans")] = Property::amount_owed;
        propertyMap[tr("Benefits paid")] = Property::bens_paid;
        propertyMap[tr("Bonuses paid")] = Property::bonuses;
        propertyMap[tr("Businesses balance")] = Property::prod_bal;
        propertyMap[tr("Consumption")] = Property::consumption;
        propertyMap[tr("Deficit (absolute)")] = Property::deficit;
        propertyMap[tr("Deficit as % GDP")] = Property::deficit_pc;
        propertyMap[tr("GINI coefficient")] = Property::gini;
        propertyMap[tr("Government receipts (cumulative)")] = Property::gov_recpts;
        propertyMap[tr("Govt direct support")] = Property::unbudgeted;
        propertyMap[tr("Govt exp excl benefits")] = Property::gov_exp;
        propertyMap[tr("Govt exp incl benefits")] = Property::gov_exp_plus;
        propertyMap[tr("Households balance")] = Property::dom_bal;
        propertyMap[tr("Income tax paid")] = Property::inc_tax;
        propertyMap[tr("National Debt")] = Property::gov_bal;
        propertyMap[tr("Number employed")] = Property::num_emps;
        propertyMap[tr("Number of businesses")] = Property::num_firms;
        propertyMap[tr("Number of govt employees")] = Property::num_gov_emps;
        propertyMap[tr("Number of new hires")] = Property::num_hired;
        propertyMap[tr("Number of new fires")] = Property::num_fired;
        propertyMap[tr("Number unemployed")] = Property::num_unemps;
        propertyMap[tr("Percent active")] = Property::pc_active;
        propertyMap[tr("Percent employed")] = Property::pc_emps;
        propertyMap[tr("Percent unemployed")] = Property::pc_unemps;
        propertyMap[tr("Population size")] = Property::pop_size;
        propertyMap[tr("Pre-tax deductions")] = Property::dedns;
        propertyMap[tr("Procurement expenditure")] = Property::procurement;
        propertyMap[tr("Productivity")] = Property::productivity;
        propertyMap[tr("Productivity (relative)")] = Property::rel_productivity;
        propertyMap[tr("Sales tax paid")] = Property::sales_tax;
        propertyMap[tr("Wages paid")] = Property::wages;
        propertyMap[tr("Zero reference line")] = Property::zero;

        is_initialised = true;
    }
}

/*
 * This function creates a new domain having the given name and currency, and
 * default parameters, and returns a pointer to it. If the domain already exists
 * (i.e. is already on the list) it returns a nullptr and doesn't create a new
 * domain.
 */
Domain *Domain::createDomain(const QString &name)
{
    Domain *dom = nullptr;

    if (getDomain(name) == nullptr)
    {
        // A domain with given name is not in list. Create a new domain having
        // the required name and currency, and default parameters, and add it
        // to the end of the list
        dom = new Domain(name);
    }

    // Return a pointer to the domain or nullptr if it already exists
    return dom;
}

/*
 * Return a pointer to the the domain having the given name, or a nullptr if it
 * is not listed.
 */
Domain *Domain::getDomain(const QString &name)
{
    for (int i = 0; i < domains.count(); i++)
    {
        Domain *dom = Domain::domains.at(i);
        if (dom->getName() == name) {
            return dom;
        }
    }
    return nullptr;
}


void Domain::reset()
{
    qDebug() << "Initialising domain" << getName();
    last_period = -1;

    int pop = getParameterVal(ParamType::pop) * 100; // for internal use. For
                                                     // display, divide this by
                                                     // 100 to get the result
                                                     // in millions, or
                                                     // multiply by 10,000 for
                                                     // result in units

    /*
     * Remove old instances from the list of workers
     */
    for (int i = 0 ; i < workers.count(); i++)
    {
        delete(workers[i]);
    }

    workers.clear();

    for (int i = 0 ; i < pop; i++)
    {
        workers.append(new Worker(this));
    }

    /*
     * Remove old instances from list of firms (this will include _gov)
     */
    for (int i = 0 ; i < firms.count() ; i++)
    {
        delete(firms[i]);
    }

    firms.clear();

    /*
     * Create a government with the required number of employees
     */
    if (_gov != nullptr)
    {
        delete (_gov);
    }
    _gov = new Government(this, (pop * static_cast<int>(getGovSize()) / 100));

    /*
     * Add firms
     */

    QSettings settings;
    int n = settings.value("start-ups", 10).toInt();
    for (int i = 0; i < n; i++)
    {
        firms.append(new Firm(this));
    }

    /*
     * Reomove old Bank instances and re-create
     */
    for (int i = 0; i < banks.count(); i++)
    {
        delete(banks[i]);
    }

    banks.clear();

    for (int i = 0; i < NUMBER_OF_BANKS; i++)
    {
        banks.append(new Bank(this));
    }


    foreach(Firm *firm, firms)
    {
        firm->init();
    }
}


/*
 * This constructor is private and is only called via createDomain, which
 * handles all associated admin.
 */
Domain::Domain(const QString &name)
{
    qDebug() << "Domain::Domain(" << name << ")";

    /*
     * Set the domain's parameters. Note that this is driven by the
     * parameters we expect (i.e. that are listed in parameterKeys),
     * Parameters don't have default values because we refer to them
     * indirectly (we could change this but it would be a hassle) so
     * if a key is missing from settings the parameter will not be set.
     */

    QSettings settings;
    settings.beginGroup("Domains");

    QString group = settings.group();

    QStringList keys = settings.childGroups();

    bool is_default;

    if (keys.contains(name))
    {
        settings.beginGroup(name);
        is_default = false;
    }
    else
    {
        /*
         * The default settings are stored in the group [Default]
         */
        settings.endGroup();                // leave the Domains group
        settings.beginGroup("Default");     // start the Default group
        is_default = true;
    }

    _name = name;
    _currency = settings.value("Currency", "Units").toString();
    _abbrev = settings.value("Abbrev", "CU").toString();

    foreach (ParamType p, parameterKeys.keys())
    {
        QString key_string = parameterKeys.value(p);
        if (settings.contains(key_string))
        {
            params[p] =  settings.value(key_string).toInt();
        }
        else
        {
            QMessageBox msgBox;
            QString msgText;

            msgText = "Parameter \"" + key_string
                    + "\" is missing from settings for "
                    + name;

            msgBox.setText(msgText);
            msgBox.exec();
        }
    }
    if (!is_default)
    {
        settings.endGroup();    // <name>
    }
    settings.endGroup();        // Domains or Default

    /*
     * We separate out initialisation so we can reset everything before
     * (re)drawing the charts.
     */
    _gov = nullptr; // so we don't try to delete it before it's created
    reset();        // set initial conditiions

    /*
     * Add this domain to the list of domains
     */
    domains.append(this);
}

/*
 * Restore all domains from settings
 */
int Domain::restoreDomains(QStringList &domainNameList)
{
    foreach (QString name, domainNameList)
    {
        createDomain(name);
    }
    qDebug() << domains.count() << "domains created";
    return domains.count();
}

void Domain::drawCharts(QListWidget *propertyList)
{
    qDebug() << "Domain::drawCharts() called. propertyList contains"
             << propertyList->count() << "properties";

    /*
     * Draw an unpopulated chart for each domain, creating the necessary
     * series in a QMap but not yet attaching them to the chart
     */
    foreach(Domain *dom, domains)
    {
        qDebug() << "Initialising domain" << dom->getName();
        dom->reset();
        dom->drawChart(propertyList);
    }

    /*
     * Iterate for the required number of periods, populating the series
     */
    QSettings settings;
    int iterations = settings.value("iterations", 100).toInt();
    int start_period = settings.value("start-period").toInt();

    for (int period = 0; period <= iterations + start_period; period++)
    {
        foreach(Domain *dom, domains)
        {
            dom->iterate(period, period < start_period);
        }
    }

    /*
     * Now add the populated series to each of the charts...
     */
    foreach(Domain *dom, domains)
    {
        dom->addSeriesToChart();
    }

}


void Domain::addSeriesToChart()
{
    auto it = series.begin();
    while (it != series.end())
    {
        chart->addSeries(it.value());
        ++it;
    }
    chart->createDefaultAxes();

    /*
     * And add some stats to the status bar
     */
    //double gini = calculateGini(); // function not safe
    double prod = getProductivity();

    // We will need to re-instate these labels on the status bars
//    inequalityLabel->setText(tr("Inequality: ") + QString::number(round(gini * 100)) + "%");
//    productivityLabel->setText(tr("Productivity: ") + QString::number(round(prod + 0.5)) + "%");

    // emit drawingCompleted();
}

Firm *Domain::createFirm(bool state_supported)
{
    Firm *firm = new Firm(this, state_supported);
    if (state_supported)
    {
        // Come back to this later
//        QSettings settings;
//        hireSome(firm, getStdWage(), 0, settings.value("government-employees").toInt());
    }
    firms.append(firm);
    return firm;
}

/*
 * Returns any firm except the government (which is not in the list of firms)
 * and the firm indicated by the exclude argument. If there are none nullptr is
 * returned.
 */
Firm *Domain::selectRandomFirm(Firm *exclude)
{
    Firm *res = nullptr;
    int n = firms.size();

    if (n == 0)
    {
        res = nullptr;
    }
    else
    {
        /*
         * Warning: this could go on forever if there's only one firm and it's
         * excluded. This shouldn't be possible normally.
         */
        while ( (res = firms[ (qrand() % (firms.size() - 1)) ]) == exclude );
    }

    return res;
}

/*
 * This function returns a pointer to one of the banks belonging to the domain,
 * chosen at random.
 */
Bank *Domain::selectRandomBank()
{
    return banks[(qrand() % (banks.size() - 1))];   // *** CHECK THIS! ***
}

/*
 * This function takes the name of the property as listed in the dock widget
 * and returns the atual property (i.e. the enum)
 */
Property Domain::getProperty(QString propertyName)
{
    QMap<QString,Property>::const_iterator it = propertyMap.find(propertyName);

    Q_ASSERT(it != propertyMap.end());

    return it.value();
}

/*
 * Properties are domain values that are liable to change at each iteration.
 * This function returns the current value of each property. Note that is is
 * important for the order of properties inside the switch to ne maintained
 * as some properties are dependent of properties defined earlier.
 */
double Domain::getPropertyVal(Property p)
{
    switch(p)
    {
    case Property::num_gov_emps:
        return _gov->getNumEmployees();

    case Property::pop_size:
        _pop_size = getPopulation();
        return double(_pop_size);

    case Property::gov_exp:
        _exp = _gov->getExpenditure();
        return _exp;

    case Property::bens_paid:
        _bens = _gov->getBenefitsPaid();
        return _bens;

    case Property::gov_exp_plus:
        return _exp + _bens;

    case Property::gov_recpts:
        _rcpts = _gov->getReceipts();
        return _rcpts;

    case Property::deficit:
        _deficit = _exp + _bens - _rcpts;
        return _deficit;

    case Property::gov_bal:
        /*
         * This will always be negative as the government is never in receipt
         * of its own currency. It amounts to the sum of all government
         * expenditures in the currency of account. It will always (numerically)
         * exceed the National Debt as it ignores tax receipts -- conventionally
         * taken as offsetting expenditure. To find the 'deficit' (see above)
         * you have to add tax receipts.
         */
        _gov_bal = _gov->getBalance();
        return _gov_bal;

    case Property::num_firms:
        _num_firms = firms.count();
        return double(_num_firms);

    case Property::num_emps:
        _num_emps = getNumEmployed();
        return double(_num_emps);

    case Property::pc_emps:
        return (_pop_size > 0 ? double(_num_emps * 100) / _pop_size : 0.0);

    case Property::num_unemps:
        _num_unemps = getNumUnemployed();
        return double(_num_unemps);

    case Property::pc_unemps:
        return (_pop_size > 0 ? double(_num_unemps * 100) / _pop_size : 0);

    case Property::pc_active:
        _pc_active = double(_num_emps + _num_unemps) / 10;  // assuming granularity 1000
        return _pc_active;

    case Property::num_hired:
        return double(_num_hired);

    case Property::num_fired:
        return double(_num_fired);

    case Property::prod_bal:
        _amount_owed = getAmountOwed();
        _prod_bal = getProdBal();               // ignoring loans
        return _prod_bal - _amount_owed;        // but show minus loans
                                                // see http://bilbo.economicoutlook.net/blog/?p=32396
                                                // where domestic sector is taken
                                                // to include banks

    case Property::wages:
        _wages = getWagesPaid();                // not cumulative -- consider adding cumulative amount
        return _wages;

    case Property::consumption:
        _consumption = getPurchasesMade();      // not cumulative -- consider adding cumulative amount
        return _consumption;

    case Property::deficit_pc:
        return abs(_consumption) < 1.0 ? 0.0 : (_deficit * 100) / _consumption;

    case Property::gini:
        return calculateGini();

    case Property::bonuses:
        _bonuses = getBonusesPaid();
        return _bonuses;

    case Property::dedns:
        return _dedns;

    case Property::inc_tax:
        _inc_tax = getIncTaxPaid();
        return _inc_tax;

    case Property::sales_tax:
        _sales_tax = getSalesTaxPaid();
        return _sales_tax;

    case Property::dom_bal:
        _dom_bal = getWorkersBal();
        return _dom_bal;

    case Property::amount_owed:
        //_amount_owed = getAmountOwed();
        return _amount_owed;

    case Property::bus_size:
        _num_firms = 1;                             // government is a firm
        _num_emps = _gov->employees.count();
        foreach(Firm *f, firms)
        {
            ++_num_firms;
            _num_emps += f->employees.count();
        }
        _bus_size = _num_emps  / _num_firms;
        return _bus_size;

    case Property::hundred:
        return 100.0;

    case Property::zero:
        return 0.0;

    case Property::procurement:
        _proc_exp = getProcurementExpenditure();    // government purchases
        return _proc_exp;

    case Property::productivity:
        _productivity = getProductivity();
        return _productivity;

    case Property::rel_productivity:
      if (_num_emps == 0) {
        Q_ASSERT(_rel_productivity != 0.0);
        _rel_productivity = 1.0; // default
      } else {
        _rel_productivity = (_productivity * _pop_size) / _num_emps;
      }
      return _rel_productivity;

    case Property::unbudgeted:
        return _gov->getUnbudgetedExp();


    /*
     * The following properties may need reinstating
     *
     *

#if 0
    case Property::investment:
        _investment = getInvestment();
        return _investment;

    case Property::gdp:
        //_gdp = _consumption + _investment + _exp + _bens;
        _gdp = _consumption - _investment;  // https://en.wikipedia.org/wiki/Gross_domestic_product
        return _gdp;

    case Property::profit:
        // ***** I don't think we should be subtracting income tax here!
        _profit = _gdp - _wages - _inc_tax - _sales_tax;
        return _profit;
#endif
    */

    case Property::num_properties:
        Q_ASSERT(false);
        return 0;                       // prevent compiler warning
    }
}

/*
 * Procurement expenditure is government spending on purchases
 */
double Domain::getProcurementExpenditure()
{
    return _gov->getProcExp();
}

double Domain::getProductivity()
{
    double tot = 0.0;
    int count = firms.count();
    for (int i = 0; i < count; i++)
    {
        Firm *f = firms[i];
        tot += double(f->getNumEmployees()) * f->getProductivity();
    }
    double res = count == 0 ? 0 : (tot * 100) / _pop_size;
    return res;
}


void Domain::setChartView(QChartView *chartView)
{
    _chartView = chartView;
    chart = chartView->chart();
}

/*
 * drawChart() simply sets up a chart but doesn't populate it.
 * See drawCharts...
 */
void Domain::drawChart(QListWidget *propertyList)
{
    qDebug() << "Domain::drawChart(...) called";

    /*
     * Note that the parameters may have been changed since the last time
     * drawChart was called. However they should have been updated in
     * params[ParamType] and should be retrieved from there rather than using
     * specific instance-global values. (E.g. params[ParamType::std_wage
     * rather than _std_wage, which has been discontinued).
     */

    chart->removeAllSeries();   // built-in chart series
    series.clear();             // our global copy, used to hold generated data points

    chart->legend()->setAlignment(Qt::AlignTop);
    chart->legend()->show();

    chart->setTitle("<h2 style=\"text-align:center;\">" + getName() + "</h2>");

    Q_ASSERT(propertyList->count() > 0);

    for (int i = 0; i < int(Property::num_properties); i++)
    {
        /*
         * We need to construct a line series for each selected property. We
         * will let Qt look after values on axes
         */

        QListWidgetItem *item;
        item = propertyList->item(i);
        bool selected = item->checkState();

        if (selected)
        {
            QString series_name = item->text();
            QLineSeries *ser = new QLineSeries();

            ser->setName(series_name);

            /*
             * This just inserts the series into our list of series. It doesn't
             * add it to the chart
             */
            series.insert(static_cast<Property>(i), ser);
        }
    }


#if 0
    double gini = getGini();
    double prod = getProductivity();
    inequalityLabel->setText(tr("Inequality: ") + QString::number(round(gini * 100)) + "%");
    productivityLabel->setText(tr("Productivity: ") + QString::number(round(prod + 0.5)) + "%");

    emit drawingCompleted();
#endif
}

/*
 * This functiion now calculates the Gini coefficient as well as the standard
 * deviation of average wages and the range r, defined so that about 99.7% of
 * the population have average wages within ±r percent of the mean. We should
 * probably store these variables as properties of the domain and the function
 * should return void.
 */
double Domain::calculateGini()
{
    const int pop = workers.count();

    /*
     * RMS variables
     */
    long int total = 0;
    long int mean = 0;
    double rms = 0.0;

    long int a = 0;

    long int n[pop];

    for (int i = 0; i < pop; i++)
    {
        Worker *w = workers[i];
        n[i] = static_cast<int>(w->getAverageWages());        // extend as required

        total += n[i];  // for RMS

        //qDebug() << "n[" << i << "] =" << n[i];
        Q_ASSERT(true);
    }

    mean = total / pop;

    std::sort(n, n + pop);                  // ascending order

    int i;

    /*
     * Calculate RMS
     */
    for (i = 0; i < pop; i++)
    {
        rms += (n[i - 1] - mean) ^2;
    }
    rms = sqrt(rms / pop);

    for (i = 1; i < pop; i++)
    {
        n[i] += n[i - 1];                    // make values cumulative
    }

    double inequality = mean > 0 ? ((rms * 3) / mean) : 0;

    long int cum_tot = n[pop - 1];

    /*
     * We don't really need cum_tot as well as total, but it's a good idea to
     * keep it as a check during testing.
     */
    Q_ASSERT(cum_tot == total);

    long int a_tot = static_cast<long int>((cum_tot * pop) / 2);        // area A+B

    if (a_tot < 0)
    {
        qDebug() << "i =" << i << ", cum_tot =" << cum_tot << ", a_tot =" << a_tot;
    }

    for (int i = 0; i < pop; i++)
    {
        long int diff = ((cum_tot * (i + 1)) / pop) - n[i];
        if (diff < 0) {
            // diff = -diff;
            qDebug() << "Domain::gini(): negative diff (" << diff << ") at interval" << i;
        }
        a += diff;                          // area A
    }

    //qDebug() << "Domain::gini():  cum_tot =" << cum_tot << ",  pop =" << pop << ",  a =" << a << ",  a_tot =" << a_tot;
    _gini = (round(double(a * 100) / double(a_tot)))/100;

    if (_gini > 100 || _gini < 0)
    {
        qDebug() << "a =" << a << ", a_tot =" << a_tot << "gini =" << _gini;
        Q_ASSERT(false);
    }

    qDebug() << "a =" << a << ", a_tot =" << a_tot << "gini =" << _gini << "RMS =" << rms << "range ±" << (inequality * 100) << "% of mean";

    return _gini;
}


// NEXT: IN PROGRESS...

void Domain::iterate(int period, bool silent)
{
    Q_ASSERT(period > last_period);

    // -------------------------------------------
    // Initialisation phase
    // -------------------------------------------

    last_period = period;

    if (period == 0)
    {
        /*
         * Reset government
         */
        _gov->reset();

        // TODO: The government will also be initialised as a firm, so it would
        // be better to remove the reset() function and overload init() instead

        /*
         * Initialise firms
         */
        for (int i = 0; i < firms.count(); i++)
        {
            firms[i]->init();
        }

        /*
         * Initialise workers
         */
        for (int i = 0; i < workers.count(); i++)
        {
            workers[i]->init();
        }
    }

    /*
     * Reset counters
     */
    _num_hired = 0;
    _num_fired = 0;
    _dedns = 0;             // TODO: CHECK THIS

    // -------------------------------------------
    // Trigger phase
    // -------------------------------------------

    /*
     * Triggering government will direct payments to firms and benefits to
     * workers before they are triggered
     */
    _gov->trigger(period);

    // Triggered firms will pay deductions to government and wages to
    // workers. Firms will also fire any workers they can't afford to pay.
    // Workers receiving payment will pay income tax to the government
    for (int i = 0; i < firms.count(); i++)
    {
        firms[i]->trigger(period);
    }

    // Trigger workers to make purchases
    for (int i = 0; i < workers.count(); i++)
    {
        workers[i]->trigger(period);
    }


    // -------------------------------------------
    // Post-trigger (epilogue) phase
    // -------------------------------------------

    // Post-trigger for firms so they can pay tax on sales just made, pay
    // bonuses, and hire more employees (investment)
    for (int i = 0, c = firms.count(); i < c; i++)
    {
        firms[i]->epilogue();
    }

    // Same for workers so they can keep rolling averages up to date
    for (int i = 0, c = workers.count(); i < c; i++)
    {
        workers[i]->epilogue(period);
    }


    // -------------------------------------------
    // Stats
    // -------------------------------------------

    /*
     * Append the values from this iteration to the series
     */
    for (auto it = series.begin(); it != series.end(); ++it)
    {
        Property p = it.key();
        QLineSeries *s = series[p];
        double value = getPropertyVal(p);

        if (!silent)
        {
            s->append(period, value);

            /*
             * TODO: Record the maximum, minimum and average values of the
             * property (non-silent entries only)
             */

            // ...
        }
    }


    // -------------------------------------------
    // Exogenous changes
    // -------------------------------------------

    // Create a new firm, possibly
    if (qrand() % 100 < getFCP())
    {
        qDebug() << "Creating new firm";
        createFirm();
    }

    qDebug() << "Domain::iterate(): _name =" << _name << "  gini =" << calculateGini();

}

Government *Domain::government()
{
    return _gov;
}

const QString &Domain::getName() const
{
    return _name;
}

/*
 * This refers to the property <population>, not the starting parameter
 */
int Domain::getPopulation()
{
    return _population * 100;
}


int Domain::getNumEmployed()
{
    int n = 0;
    for (int i = 0; i < firms.count(); i++)
    {
        n += firms[i]->employees.count();
    }

    return n;
}

int Domain::getNumEmployedBy(Firm *firm)
{
    int n = 0;
    for (int i = 0; i < workers.count(); i++)
    {
        if (workers[i]->isEmployedBy(firm))
        {
            n++;
        }
    }

    return n;
}

int Domain::getNumUnemployed()
{
    int n = 0;
    for (int i = 0; i < workers.count(); i++)
    {
        if (!workers[i]->isEmployed())
        {
            n++;
        }
    }

    return n;
}

double Domain::getPurchasesMade()
{
    double tot = 0;
    for (int i = 0; i < workers.count(); i++)
    {
        tot += workers[i]->getPurchasesMade();
    }

    return tot;
}

double Domain::getSalesReceipts()
{
    double tot = 0;
    for (int i = 0; i < firms.count(); i++)
    {
        tot += firms[i]->getSalesReceipts();
    }

    return tot;
}

double Domain::getBonusesPaid()
{
    double tot = 0;
    for (int i = 0; i < firms.count(); i++)
    {
        tot += firms[i]->getBonusesPaid();
    }

    return tot;
}

double Domain::getDednsPaid()
{
    return _dedns;
}

double Domain::getIncTaxPaid()
{
    double tot = 0;
    for (int i = 0; i < workers.count(); i++)
    {
        tot += workers[i]->getIncTaxPaid();
    }

    return tot;
}

double Domain::getSalesTaxPaid()
{
    double tot = 0;
    for (int i = 0; i < firms.count(); i++)
    {
        tot += firms[i]->getSalesTaxPaid();
    }

    return tot;
}

double Domain::getWorkersBal()
{
    double tot = 0.0;
    for (int i = 0; i < workers.count(); i++)
    {
        tot += workers[i]->getBalance();
    }

    return tot;
}

// TODO: At present only businesses can get loans, but this should be extended
// to workers in due course. We also need to allow banks to get loans from the
// central bank -- i.e. from the government.
double Domain::getAmountOwed()
{
    double tot = 0;
    for (int i = 0; i < firms.count(); i++)
    {
        tot += firms[i]->getAmountOwed();
    }
    return tot;
}

int Domain::getNumHired()
{
    return _num_hired;
}

int Domain::getNumFired()
{
    return _num_fired;
}

double Domain::getProdBal()
{
    double bal = 0;
    for (int i = 0; i < firms.count(); i++)
    {
        bal += firms[i]->getBalance();
    }

    return bal;
}

double Domain::getWagesPaid()
{
    double tot = 0.0;
    for (int i = 0; i < firms.count(); i++)
    {
        tot += firms[i]->getWagesPaid();
    }

    return tot;
}

double Domain::getInvestment()
{
    double tot = 0.0;
    for (int i = 0; i < firms.count(); i++)
    {
        tot += firms[i]->getInvestment();
    }

    return tot;
}

int Domain::getParameterVal(ParamType type)
{
    return params[type];
}

/*
 * Domain constants
 */

int Domain::getProcurement()
{
    return getParameterVal(ParamType::procurement);
}

/*
 * This doesn't seem to be used for anything
 *
double Domain::getTargetEmpRate()
{
    return getParameterVal(ParamType::emp_rate)) / 100;
}
*/

int Domain::getStdWage()
{
    return getParameterVal(ParamType::std_wage);
}

int Domain::getPropCon()
{
    return getParameterVal(ParamType::prop_con);
}

int Domain::getIncomeThreshold()
{
    return getParameterVal(ParamType::inc_thresh);
}

int Domain::getIncTaxRate()
{
    return getParameterVal(ParamType::inc_tax_rate);
}

int Domain::getSalesTaxRate()
{
    return getParameterVal(ParamType::sales_tax_rate);
}

int Domain::getPreTaxDedns()
{
    return getParameterVal(ParamType::dedns);
}

int Domain::getCapexRecoupTime()
{
    return getParameterVal(ParamType::recoup);
}

int Domain::getFCP()
{
    return getParameterVal(ParamType::firm_creation_prob);
}

int Domain::getUBR()
{
    return getParameterVal(ParamType::unemp_ben_rate);
}

/*
 * Distribution rate is a percentage
 */
int Domain::getDistributionRate()
{
    return params[ParamType::distrib];
}

int Domain::getPropInv()
{
    return getParameterVal(ParamType::prop_inv);
}

int Domain::getBoeRate()
{
    return getParameterVal(ParamType::boe_int);
}

int Domain::getBusRate()
{
    return getParameterVal(ParamType::bus_int);
}

int Domain::getLoanProb()
{
    return getParameterVal(ParamType::loan_prob);
}

/*
 * This returns the relative government size as a percentage
 */
int Domain::getGovSize()
{
    return getParameterVal(ParamType::gov_size);
}

#if 0
/*
 * Condition processing
 */
bool Domain::applies(Condition condition)
{
    // Property::zero is always zero and can be used as a marker for the end of
    // the enum (Better to use num_properties). In a condition we also use it
    // to indicate that the associated parameters are to be applied
    // unconditionally (unless overridden by a subsequent condition). In
    // practice we currently access defaults (unconditionals) separately from
    // conditionals so we don't actually have to eveluate the condition.
    if (condition.property == Property::zero)
    {
        return true;
    }
    else
    {
        int lhs = getPropertyVal(condition.property);
        Opr opr = condition.opr;
        int rhs = condition.val;
        return compare(lhs, rhs, opr);
    }
}

bool Domain::compare(int lhs, int rhs, Opr opr)
{
    switch (opr) {
        case Opr::eq:
            return lhs == rhs;
        case Opr::neq:
            return lhs != rhs;
        case Opr::lt:
            return lhs < rhs;
        case Opr::gt:
            return lhs > rhs;
        case Opr::leq:
            return lhs <= rhs;
        case Opr::geq:
            return lhs >= rhs;
        default:
            return false;
    }
}

#endif
