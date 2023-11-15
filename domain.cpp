/*
 * domain.cpp
 */

#include "account.h"
#include <math.h>
#include "QtCore/qdebug.h"
#include <QSettings>
#include <QListWidgetItem>

#define NUMBER_OF_BANKS 3
#define CLEARING_FREQUENCY 10

/*
 * Statics
 */
QList<Domain*> Domain::domains;                         // static

const QMap<ParamType,QString> Domain::parameterKeys     // static
{
    {ParamType::procurement, "govt-procurement"},
    {ParamType::emp_rate, "employment-rate"},
    {ParamType::prop_con, "propensity-to-consume"},
    {ParamType::inc_tax_rate, "income-tax-rate"},
    {ParamType::inc_thresh, "income-threshold"},
    {ParamType::sales_tax_rate, "sales-tax-rate"},
    {ParamType::firm_creation_prob, "firm-creation-prob"},
    {ParamType::dedns, "pre-tax-dedns-rate"},
    {ParamType::unemp_ben_rate, "unempl-benefit-rate"},
    {ParamType::distrib, "reserve-rate"},
    {ParamType::prop_inv, "prop-invest"},
    {ParamType::boe_int, "boe-interest"},
    {ParamType::bus_int, "bus-interest"},
    {ParamType::loan_prob, "loan-prob"},
    {ParamType::recoup, "capex-recoup-periods"},
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
        propertyMap[tr("Government receipts")] = Property::gov_recpts;
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
Domain *Domain::createDomain(
        const QString &name,
        const QString &currency,
        const QString &currencyAbbrev)
{
    Domain *dom = nullptr;

    qDebug() << "Domain::createDomain("
             << name << "," << currency << "," << currencyAbbrev
             << ")";

    if (getDomain(name) == nullptr)
    {
        // A domain with given name is not in list. Create a new domain having
        // the required name and currency, and default parameters, and add it
        // to the end of the list
        dom = new Domain(name, currency, currencyAbbrev);
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


void Domain::initialise()
{
    qDebug() << "Initialising domain" << getName();
    last_period = -1;
    foreach(Firm *firm, firms)
    {
        firm->init();
    }

    /*
    foreach(Worker *w, workers)
    {
        w->init();
    }

    workers.clear();
    */
}

/*
 * This constructor is private and is only called via createDomain, which
 * handles all associated admin.
 */
Domain::Domain(const QString &name,
        const QString &currency,
        const QString &currencyAbbrev)
{
    qDebug() << "Domain::Domain("
             << name
             << ","
             << currency
             << ","
             << currencyAbbrev
             << ")";

    /*
     * Store the domain's name, currency and currency abbreviation
     */
    _name = name;
    _currency = currency;
    _abbrev = currencyAbbrev;

    /*
     * Create a government
     */
    _gov = new Government(this);

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
     * Add this domain to the list of domains
     */
    domains.append(this);

    /*
     * Create an arbitrary number of banks
     */
    for (int i = 0; i < NUMBER_OF_BANKS; i++)
    {
        banks.append(new Bank(this));
    }

    initialise();

    qDebug() << "Domain" << getName() << "has" << firms.count()
             << "firms and" << banks.count() << "banks";

}

/*
 * Restore all domains from settings
 */
int Domain::restoreDomains(QStringList &domainNameList)
{
    qDebug() << "Domain::restoreDomains(); There are currently" << domains.count() << "domains";

    QSettings settings;

    settings.beginGroup("Domains");

    foreach (QString name, domainNameList)
    {
        qDebug() << "restoring domain" << name;
        settings.beginGroup(name);

        QString currency = settings.value("Currency", "Units").toString();
        QString abbrev = settings.value("Abbrev", "CU").toString();

        Domain *dom = createDomain(name, currency, abbrev);
        if (dom == nullptr)
        {
            qDebug() << "could not create domain" << name;
        }

        /*
         * Note that this is driven by the parameters we expect (i.e. that are
         * listed in parameterKeys), If a key is missing from settings we leave
         * the default value that was set up in the constructor intact.
         */
        foreach (ParamType p, parameterKeys.keys())
        {
            QString key_string = parameterKeys.value(p);
            qDebug() << "reading parameter" << key_string << "from settings";
            if (settings.contains(key_string))
            {
                if (key_string == "reserve-rate")
                    qDebug() << "setting reserve-rate (distribution) to" << settings.value(key_string).toInt();
                dom->params[p] =  settings.value(key_string).toInt();
            }
            else
            {
                qWarning() << "Parameter" << key_string
                           << "is missing from settings for"
                           << dom->getName();
            }
        }
        /*
         * TODO: (Ignore compiler error message)
         * Error msg from compiler can be ignored. Domain *domain is  added to
         * the domains list by the constructor, ensuring that a domain cannot
         * be created without adding it to the list. However, for tidyness,
         * when the program closes and the domains list is destroyed, we should
         * destroy the listed domains explicitly.
         */
        settings.endGroup();
    }

    settings.endGroup();    // end Domains group

    qDebug() << domains.count() << "domains created";
    return domains.count();
}

void Domain::drawCharts(QListWidget *propertyList)
{
    qDebug() << "Domain::drawCharts() called. propertyList contains"
             << propertyList->count() << "properties";

    /*
     * Draw an unpopulated chart for each domain, with series attached
     */
    foreach(Domain *dom, domains)
    {
        qDebug() << "About to draw chart for domain" << dom->getName();
        dom->initialise();
        dom->drawChart(propertyList);
    }

    /*
     * Iterate for the required number of periods, populating the series
     */
    QSettings settings;
    int iterations = settings.value("iterations", 100).toInt();
    for (int period = 0; period <= iterations; period++)
    {
        /*
         * Iterate each domain
         */
        foreach(Domain *dom, domains)
        {
            qDebug() << "About to iterate domain" << dom->getName() << ", period" << period;
            dom->iterate(period);
        }
    }

    /*
     * Now we need to add the populated series to the charts...
     */
    foreach(Domain *dom, domains)
    {
        // NB we have the series in QMap *series somewhere. Make
        // sure it's global
        dom->addSeriesToChart();
    }

}

// ---------- End of statics -----------

void Domain::addSeriesToChart()
{
    auto it = series.begin();
    while (it != series.end())
    {
        qDebug() << "Adding series to chart for" << getName();
        chart->addSeries(it.value());
        ++it;
    }
    chart->createDefaultAxes();

    /*
     * And add some stats to the status bar
     */
    double gini = getGini();
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

    if (n < 2)
    {
        res = nullptr;
    }
    else
    {
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
 * If the property is scalable this function returns the scaled value; otherwise
 * it returns the value.
 */
double Domain::scale(Property p)
{
    double val = getPropertyVal(p);

    switch(p)
    {
    case Property::pc_emps:
    case Property::pc_unemps:
    case Property::pc_active:
    case Property::deficit_pc:
    case Property::hundred:
    case Property::zero:
    case Property::productivity:
    case Property::rel_productivity:
    case Property::num_properties:
        return val;

    case Property::pop_size:
    case Property::gov_exp:
    case Property::bens_paid:
    case Property::gov_exp_plus:
    case Property::unbudgeted:
    case Property::gov_recpts:
    case Property::deficit:
    case Property::gov_bal:
    case Property::num_firms:
    case Property::num_emps:
    case Property::num_unemps:
    case Property::num_gov_emps:
    case Property::num_hired:
    case Property::num_fired:
    case Property::prod_bal:
    case Property::wages:
    case Property::consumption:
    case Property::bonuses:
    case Property::dedns:
    case Property::inc_tax:
    case Property::sales_tax:
    case Property::dom_bal:
    case Property::amount_owed:
    case Property::bus_size:
    case Property::procurement:

    // These are missing from Property -- may need re-instating
    //
    // case Property::investment:
    // case Property::gdp:
    // case Property::profit:

        return val * _scale;
    }
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
        // We take the government firm into account here because _num_emps
        // doesn't make a distinction
        _bus_size = double(_num_emps)  / (_num_firms + 1);
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
     */

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
 * drawChart() simply sets up a chart but doesn't populate it. See drawCharts...
 */
void Domain::drawChart(QListWidget *propertyList)
{
    qDebug() << "Domain::drawChart(...) called";

    /*
     * Since we may be redrawing the chart we must clear existing series (and
     * axes?)
     */

    chart->removeAllSeries();   // built-in chart series
    series.clear();             // our global copy, used to hold generated data points

    chart->legend()->setAlignment(Qt::AlignTop);
    chart->legend()->show();

    chart->setTitle("<h2 style=\"text-align:center;\">" + getName() + "</h2>");

    qDebug() << "Creating line series";

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
 * This returns the Gini coefficient for the population based on each worker's
 * average wage
 */
double Domain::getGini()
{
    int pop = workers.count();

    double a = 0.0;

    int n[pop];         //

    for (int i = 0; i < pop; i++)
    {
        Worker *w = workers[i];
        n[i] = w->getAverageWages();        // extend as required
    }

    std::sort(n, n + pop);                  // ascending order

    for (int i = 1; i < pop; i++)
    {
       n[i] += n[i - 1];                    // make values cumulative
    }

    double cum_tot = n[pop - 1];

    double a_tot = (cum_tot * pop) / 2.0;   // area A+B

    for (int i = 0; i < pop; i++)
    {
        double diff = ((cum_tot * (i + 1)) / pop) - n[i];
        if (diff < 0) {
            diff = -diff;
            qDebug() << "Domain::gini(): negative diff (" << diff << ") at interval" << i;
        }
        a += diff;                          // area A
    }

    qDebug() << "Domain::gini():  cum_tot =" << cum_tot << ",  pop =" << pop << ",  a =" << a << ",  a_tot =" << a_tot;
    _gini = a / a_tot;
    return _gini;
}


// NEXT: IN PROGRESS...

void Domain::iterate(int period)
{
    qDebug() << "Starting iteration for period" << period
             << "for domain" << getName();

    Q_ASSERT(period > last_period);

    last_period = period;

    if (period == 0)
    {
        /*
         * Do any initialisation here
         */
        _gov->reset();
        qDebug() << "Government reset";
    }

    // deductions are accumulated within but not across periods
    _dedns = 0;

    // Reset counters

    _num_hired = 0;
    _num_fired = 0;

    // -------------------------------------------
    // Trigger phase
    // -------------------------------------------

    // Triggering government will direct payments to firms and benefits to
    //  workers before they are triggered
    _gov->trigger(period);

    // Triggered firms will pay deductions to government and wages to
    // workers. Firms will also fire any workers they can't afford to pay.
    // Workers receiving payment will pay income tax to the government
    for (int i = 0; i < firms.count(); i++)
    {
        firms[i]->init();
        firms[i]->trigger(period);  // FIX: _gov is a firm, so this will trigger it again
        // if trigger is virtual (I don't think it is) or if _gov has been added to the
        // list of firms.
    }

    // Trigger workers to make purchases
    for (int i = 0; i < workers.count(); i++)
    {
        workers[i]->init();
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

    // Append the values from this iteration to the series
    for (int i = 0; i < static_cast<int>(Property::num_properties); i++)
    {
        qDebug() << "Updating chart for" << getName() << ", property" << i;

        /*
         * For active properties (only) we must get the current value and
         * append it to the appropriate series against the current period
         */

        for (auto it = series.begin(); it != series.end(); ++it)
        {
            Property p = it.key();
            QLineSeries *s = series[p];
            qDebug() << "Calling getPropertyValue" << static_cast<int>(p);
            double value = getPropertyVal(p);
            qDebug() << "getPropertyValue() returns" << value;
            s->append(period, value);


            qDebug() << "Appending (" << period << ","
                     << static_cast<int>(value) << "to series"
                     << static_cast<int>(p);
        }



        return; // TEMPORARY, FOR TESTING




        Property prop = static_cast<Property>(i);   // convert i to a Property
        double val = scale(prop);
        series[prop]->append(period, val);
        qDebug() << "property value is" << val;

        if (i == 0)
        {
            max_val[i] = val;
            min_val[i] = val;
            sum[i] = val;
        }
        else
        {
            if (val > max_val[i])
            {
                max_val[i] = val;
            }
            else if (val < min_val[i])
            {
                min_val[i] = val;
            }

            sum[i] += val;
        }
    }


    //return; // TEMPORARY, FOR TESTING

    // -------------------------------------------
    // Exogenous changes
    // -------------------------------------------

    // Create a new firm, possibly
    if (qrand() % 100 < getFCP())
    {
        createFirm();
    }


    qDebug() << "Domain::iterate(): _name =" << _name << "  gini =" << getGini();

}

Government *Domain::government()
{
    return _gov;
}

const QString &Domain::getName() const
{
    return _name;
}

int Domain::getPopulation()
{
    return _population;
}


int Domain::getNumEmployed()
{
    int n = 0;
    for (int i = 0; i < workers.count(); i++)
    {
        if (workers[i]->isEmployed())
        {
            n++;
        }
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

// TODO Replace getParameterVal(type) with params[type] throughout
int Domain::getParameterVal(ParamType type)
{
    return params[type];
}

/*
 * Domain constants
 */

double Domain::getProcurement()
{
    return double(getParameterVal(ParamType::procurement));
}

double Domain::getTargetEmpRate()
{
    return double(getParameterVal(ParamType::emp_rate)) / 100;
}

double Domain::getStdWage()
{
    return _std_wage;
}

double Domain::getPropCon()
{
    return double(getParameterVal(ParamType::prop_con)) / 100;
}

double Domain::getIncomeThreshold()
{
    return double(getParameterVal(ParamType::inc_thresh));
}

double Domain::getIncTaxRate()
{
    return double(getParameterVal(ParamType::inc_tax_rate)) / 100;
}

double Domain::getSalesTaxRate()
{
    return double(getParameterVal(ParamType::sales_tax_rate)) / 100;
}

double Domain::getPreTaxDedns()
{
    return double(getParameterVal(ParamType::dedns)) / 100;
}

double Domain::getCapexRecoupTime()
{
    return double(getParameterVal(ParamType::recoup));
}

double Domain::getFCP()
{
    return double(getParameterVal(ParamType::firm_creation_prob)) / 100;
}

double Domain::getUBR()
{
    return double(getParameterVal(ParamType::unemp_ben_rate)) / 100;
}

/*
 * Distribution rate is a percentage
 */
double Domain::getDistributionRate()
{
    return params[ParamType::distrib];
}

double Domain::getPropInv()
{
    return double(getParameterVal(ParamType::prop_inv)) / 100;
}

double Domain::getBoeRate()
{
    return double(getParameterVal(ParamType::boe_int)) / 100;
}

double Domain::getBusRate()
{
    return double(getParameterVal(ParamType::bus_int)) / 100;
}

double Domain::getLoanProb()
{
    return double(getParameterVal(ParamType::loan_prob)) / 100;
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
