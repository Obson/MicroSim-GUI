/*
 * domain.cpp
 */

#include "account.h"
#include <math.h>
#include "QtCore/qdebug.h"
#include <QSettings>

#define NUMBER_OF_BANKS 3
#define CLEARING_FREQUENCY 10

/*
 * Statics
 */
QList<Domain*> Domain::domains;

const QMap<ParamType,QString> Domain::parameterKeys
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
     * Add this domain to the list of domains
     */
    qDebug() << "Adding domain" << name << "to domains";
    domains.append(this);
    qDebug() << "There are now" << domains.count() << "domains on the list";


    /*
     * Create an arbitrary number of banks
     */
    for (int i = 0; i < NUMBER_OF_BANKS; i++)
    {
        banks.append(new Bank(this));
    }
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
            if (settings.contains(key_string))
            {
                dom->params[p] =  settings.value(key_string).toInt();
            }
            else
            {
                qWarning() << "Parameter" << key_string
                           << "is missing from settings for"
                           << dom->getName();
            }
        }
        settings.endGroup();
    }

    settings.endGroup();    // end Domains group

    qDebug() << domains.count() << "domains created";
    return domains.count();
}

void Domain::drawCharts()
{
    qDebug() << "Domain::drawCharts() called";
    foreach(Domain *dom, domains)
    {
        qDebug() << "About to draw chart for domain" << dom->getName();
        dom->drawChart();
    }
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
    case Property::current_period:
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
    case Property::investment:
    case Property::gdp:
    case Property::profit:
        return val * _scale;
    }
}

Domain::Property Domain::getProperty(int n)
{
    Property p;
    foreach(p, prop_list)
    {
        if (n == static_cast<int>(p))
        {
            return p;
        }
    }

    Q_ASSERT(false);
    return Property::zero;  // prevent compiler warning
}

/*∫
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
        /*
         * This is not currently handled
         */
        Q_ASSERT(false);
        return 0;

    case Property::current_period:
        return getPeriod();

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
        // We are displaying this as a positive debt rather than a negative
        // balance just so we can avoid displaying negative quamtities (with
        // exception of negative deficits) and keep the x-axis at the bottom
        // as there's no simple provision for displaying it at y = 0.
        _gov_bal = -(_gov->getBalance());
        return _gov_bal;

    case Property::num_firms:
        // num_firms is (must be) only used for stats as it doesn't include
        // the government-owned firm. For the actual number (unscaled) use
        // firms.count().
        _num_firms = firms.count() - 1;
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
        _wages = getWagesPaid();                // not cumulative
        return _wages;

    case Property::consumption:
        _consumption = getPurchasesMade();      // not cumulative
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
        if (_num_emps == 0)
        {
            Q_ASSERT(_rel_productivity != 0.0);
            _rel_productivity = 1.0;  // default
        }
        else
        {
           _rel_productivity = (_productivity * _pop_size) / _num_emps;
        }
        return _rel_productivity;

    case Property::unbudgeted:
        return _gov->getUnbudgetedExp();

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
        tot += f->getNumEmployees() * f->getProductivity();
    }
    double res = count == 0 ? 0 : (tot * 100) / _pop_size;
    return res;
}


void Domain::setChartView(QChartView *chartView)
{
    _chartView = chartView;
    chart = chartView->chart();
}

int Domain::magnitude(double y)
{
    int x = (y == 0.0 ? -INT_MAX : (static_cast<int>(log10(abs(y)))));
    qDebug() << "Domain::magnitude(): magnitude of" << y << "is" << x;
    return x;
}


void Domain::drawChart()
{
    qDebug() << "Domain::drawChart(...) called";

    /*
     * NEXT
     *
     * MainWindow looks after the charts as berfore, but has to create one for
     * each Domain (so wghen domains have allbeen created, emit a signal to
     * let MainWindow know it can create the charts). Althogh mainwindow looks
     * after the charts, Domain builds the series it uses.
     */


    // We are going to remove the chart altogether and replace it with a new
    // one to make sure we get a clean slate. However if we don't remove the
    // objects owned by the old chart the program eventually crashes. So far,
    // the following lines seem to fix that problem. This may all be overkill,
    // but I haven't found an alternative way of keeping the axes up to data.

    QList<QAbstractSeries*> current_series = chart->series();

    for (int i = 0; i < current_series.count(); i++)
    {
        chart->removeSeries(current_series[i]);
    }

    if (chart->axisX() != nullptr)
    {
        chart->removeAxis(chart->axisX());
    }

    if (chart->axisY() != nullptr)
    {
        chart->removeAxis(chart->axisY());
    }

    // Remove the existing chart and replace it with a new one.
    // delete chart;
    // createChart();
    chart->legend()->setAlignment(Qt::AlignTop);



//    if (rerun)
//    {
//        _currentBehaviour->run(randomised);
//        statsDialog->hide();
//        if (property_selected)
//        {
//            updateStatsDialog(propertyList->currentItem());
//        }
//    }

    chart->legend()->show();
    chart->setTitle("<h2 style=\"text-align:center;\">"
                    + getName()
                    + "</h2>");

    QLineSeries *anySeries = nullptr;

    int y_max = -INT_MAX, y_min = INT_MAX;
    qDebug() << "MainWindow::drawChart(): resetting range y_min = " << y_min << "y_max" << y_max << "***";

    for (int i = 0; i < int(Property::num_properties); i++)
    {
//
//        QListWidgetItem *item;
//        item = propertyList->item(i);
//        bool selected = item->checkState();
//        if (selected)
//        {
//            QString series_name = item->text();
//            Behaviour::Property prop = propertyMap[series_name];
//            QLineSeries *ser = _currentBehaviour->series[prop];
//            ser->setName(series_name);
//            chart->addSeries(ser);

//            anySeries = ser;

//            // Set the line colour and type for this series
//            switch(prop)
//            {
//            case Domain::Property::zero:
//            case Domain::Property::hundred:
//                ser->setColor(Qt::black);
//                ser->setPen(QPen(Qt::DotLine));
//                break;
//            default:
//                if (propertyColours.contains(prop))
//                {
//                    ser->setColor(propertyColours[prop]);
//                }
//                else
//                {
//                    QColor colour = nextColour(n++);
//                    propertyColours[prop] = colour;
//                    ser->setColor(colour);
//                }
//                break;
//            }


//            // Set values for y axis range

//            // TODO: prop is an enum but max_value() and min_value() expect
//            // ints, so we have to do a static cast. Perhaps this should really
//            // be done in the functions themselves.

//            /*
//             * NB This will all go into Domain and _currentBehaviour will be redundant
//             */
//            int ix = static_cast<int>(prop);

//            y_max = std::max(y_max, _currentBehaviour->max_value(ix));
//            y_min = std::min(y_min,_currentBehaviour->min_value(ix));
//            qDebug() << "MainWindow::drawChart(): series name" << series_name
//                     << "series max" << _currentBehaviour->max_value(ix)
//                     << "y_max" << y_max
//                     << "series min" << _currentBehaviour->min_value(ix)
//                     << "y_min" << y_min;
//        }

    }

//    int scale = magnitude(std::max(std::abs(y_max), std::abs(y_min)));

//    qDebug() << "MainWindow::drawChart(): min" << y_min << "max" << y_max << "scale" << scale;

//    // Format the axis numbers to whole integers. This needs a series to have
//    // been selected, so avoid otherwise
//    if (anySeries != nullptr)
//    {
//        chart->createDefaultAxes();
//        QValueAxis *x_axis = static_cast<QValueAxis*>(chart->axisX(anySeries));
//        x_axis->setLabelFormat("%d");
//        QValueAxis *y_axis = static_cast<QValueAxis*>(chart->axisY(anySeries));
//        y_axis->setLabelFormat("%d");

//        int temp;
//        if (y_max > 0 && y_min >= 0) {
//            // Both positive: range from zero to y_max rounded up to power of 10
//            temp = std::pow(10, (scale + 1));
//            y_max = (temp >= y_max * 2 ? (temp >= y_max * 4 ? temp / 4 : temp / 2) : temp);
//            y_min = 0;
//        } else if (y_min < 0 && y_max <= 0) {
//            // Both negative: range y_min rounded down to power of 10, to zero
//            y_max = 0;
//            temp = -std::pow(10, (scale + 1));
//            y_min = (temp <= y_min * 2 ? (temp <= y_min * 4 ? temp / 4 : temp / 2) : temp);
//        } else {
//            // TODO: It would be nicer if the intervals were equally reflected
//            // about the x-axis but this isn't really very important
//            temp = std::pow(10, (scale + 1));
//            y_max = (temp >= y_max * 2 ? (temp >= y_max * 4 ? temp / 4: temp / 2) : temp);
//            temp = -std::pow(10, (scale + 1));
//            y_min = (temp <= y_min * 2 ? (temp <= y_min * 4 ? temp / 4 : temp / 2) : temp);
//        }

//        qDebug() << "MainWindow::drawChart(): Setting range from" << y_min << "to" << y_max;
//        y_axis->setRange(y_min, y_max);
//    }

//    double gini = _currentBehaviour->getGini();
//    double prod = _currentBehaviour->getProductivity();

//    inequalityLabel->setText(tr("Inequality: ") + QString::number(round(gini * 100)) + "%");
//    productivityLabel->setText(tr("Productivity: ") + QString::number(round(prod + 0.5)) + "%");

//    // emit drawingCompleted();

}

void Domain::run()
{
    qDebug() << "Domain::run()";

//    restart();

//    // ***
//    // Seed the pseudo-random number generator.
//    // We need reproducibility so we always seed with the same number.
//    // This makes inter-model comparisons more valid.
//    // ***

//    if (!randomised) {
//        qDebug() << "Behaviour::run(): using fixed seed (42)";
//        qsrand(42);
//    }

//    for (_period = 0; _period <= _iterations + _first_period; _period++)
//    {
//        /*
//         *  Signal domains to go on to the next period. _period==0 should
//         *   trigger initialisation
//         */
//        emit(clockTick(_period));

//        // -------------------------------------------
//        // Initialise objects ready for next iteration
//        // -------------------------------------------

//        _dedns = 0;         // deductions are tracked by the model object and are
//                            // accumulated within but not across periods

//        // _gov shuld be a member of Domain
//        _gov->init();

//        int num_workers = workers.count();
//        int num_firms = firms.count();

//        for (int i = 0; i < num_firms; i++) {
//            firms[i]->init();
//        }

//        for (int i = 0; i < num_workers; i++) {
//            workers[i]->init();
//        }

//        // Reset counters

//        num_hired = 0;
//        num_fired = 0;
//        num_just_fired = 0;

//        // -------------------------------------------
//        // Trigger objects
//        // -------------------------------------------

//        // Triggering government will direct payments to firms and benefits to
//        //  workers before they are triggered
//        _gov->trigger(_period);

//        // Triggering firms will pay deductions to government and wages to
//        // workers. Firms will also fire any workers they can't afford to pay.
//        // Workers receiving payment will pay income tax to the government
//        for (int i = 0; i < num_firms; i++) {
//            firms[i]->trigger(_period);
//        }

//        // Trigger workers to make purchases
//        for (int i = 0; i < num_workers; i++) {
//            workers[i]->trigger(_period);
//        }

//        // -------------------------------------------
//        // Post-trigger (epilogue) phase
//        // -------------------------------------------

//        // Post-trigger for firms so they can pay tax on sales just made, pay
//        // bonuses, and hire more employees (investment)
//        for (int i = 0, c = firms.count(); i < c; i++) {
//            firms[i]->epilogue(_period);
//        }

//        // Same for workers so they can keep rolling averages up to date
//        for (int i = 0, c = workers.count(); i < c; i++) {
//            workers[i]->epilogue(_period);
//        }

//        // -------------------------------------------
//        // Stats
//        // -------------------------------------------

//        // Append the values from this iteration to the series
//        if (_period >= _first_period)
//        {
//            for (int i = 0; i < _num_properties/*static_cast<int>(Property::num_properties)*/; i++)
//            {
//                Property prop = prop_list[i];
//                double val = scale(prop);
//                series[prop]->append(_period, val);

//                int j = static_cast<int> (prop);

//                if (_period == _first_period)
//                {
//                    max_val[j] = val;
//                    min_val[j] = val;
//                    sum[j] = val;
//                }
//                else
//                {
//                    if (val > max_val[j])
//                    {
//                        max_val[j] = val;
//                    }
//                    else if (val < min_val[j])
//                    {
//                        min_val[j] = val;
//                    }

//                    sum[j] += val;
//                }
//            }
//        }

//        // -------------------------------------------
//        // Exogenous changes
//        // -------------------------------------------

//        // Create a new firm, possibly
//        if (qrand() % 100 < getFCP()) {
//            createFirm();
//        }
//    }


//    qDebug() << "Behaviour::run(): _name =" << _name << "  gini =" << gini();
}


/*
 * This returns the Gini coefficient for the population based on each worker's
 * average wage
 */
double Domain::gini()
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

void Domain::iterate(int duration)
{
    for (period = 0; period < duration; period++)
    {
        if (period == 0)
        {
            // ...
        }

        // deductions are accumulated within but not across periods
        _dedns = 0;

        _gov->init();

        // Reset counters

        _num_hired = 0;
        _num_fired = 0;

        // -------------------------------------------
        // Trigger objects
        // -------------------------------------------

        // Triggering government will direct payments to firms and benefits to
        //  workers before they are triggered
        _gov->trigger(period);

        // Triggering firms will pay deductions to government and wages to
        // workers. Firms will also fire any workers they can't afford to pay.
        // Workers receiving payment will pay income tax to the government
        for (int i = 0; i < firms.count(); i++)
        {
            firms[i]->init();
            firms[i]->trigger(period);
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
            Property prop = prop_list[i];
            double val = scale(prop);
            series[prop]->append(period, val);

            int j = static_cast<int> (prop);

            if (i == 0)
            {
                max_val[j] = val;
                min_val[j] = val;
                sum[j] = val;
            }
            else
            {
                if (val > max_val[j])
                {
                    max_val[j] = val;
                }
                else if (val < min_val[j])
                {
                    min_val[j] = val;
                }

                sum[j] += val;
            }
        }

        // -------------------------------------------
        // Exogenous changes
        // -------------------------------------------

        // Create a new firm, possibly
        if (qrand() % 100 < getFCP())
        {
            createFirm();
        }


        qDebug() << "Domain::iterate(): _name =" << _name << "  gini =" << gini();
    }

}

int Domain::getPeriod()
{
    return period;
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

int Domain::getParameterVal(ParamType type)
{
    // REWRITE
    /*
    Pair p;
    for (int i = 0; i < numParameterSets; i++)
    {
        if (i == 0 || applies(parameterSets[i]->condition))
        {
            // TODO: p doesn't have to be a Pair as we now only use the val component
            if (isParamSet(type, i))
            {
                p =    (type == ParamType::procurement) ? parameterSets[i]->procurement
                     : ((type == ParamType::emp_rate) ? parameterSets[i]->emp_rate
                     : ((type == ParamType::prop_con) ? parameterSets[i]->prop_con
                     : ((type == ParamType::inc_tax_rate) ? parameterSets[i]->inc_tax_rate
                     : ((type == ParamType::sales_tax_rate) ? parameterSets[i]->sales_tax_rate
                     : ((type == ParamType::firm_creation_prob) ? parameterSets[i]->firm_creation_prob
                     : ((type == ParamType::dedns) ? parameterSets[i]->dedns
                     : ((type == ParamType::unemp_ben_rate) ? parameterSets[i]->unemp_ben_rate
                     : ((type == ParamType::distrib) ? parameterSets[i]->distrib
                     : ((type == ParamType::prop_inv) ? parameterSets[i]->prop_inv
                     : ((type == ParamType::boe_int) ? parameterSets[i]->boe_int
                     : ((type == ParamType::bus_int) ? parameterSets[i]->bus_int
                     : ((type == ParamType::loan_prob) ? parameterSets[i]->loan_prob
                     : ((type == ParamType::inc_thresh) ? parameterSets[i]->inc_thresh
                     : ((type == ParamType::recoup) ? parameterSets[i]->recoup
                     : parameterSets[i]->invalid
                     )))))))))))))));
            }
        }
    }

    return p.val;
    */
}

bool Domain::isParamSet(ParamType t, int n)
{
    // REWRITE
    /*
    if (n == 0) {
        return true;
    }

    return  (t == ParamType::procurement) ? parameterSets[n]->procurement.is_set
         : ((t == ParamType::emp_rate) ? parameterSets[n]->emp_rate.is_set
         : ((t == ParamType::prop_con) ? parameterSets[n]->prop_con.is_set
         : ((t == ParamType::inc_tax_rate) ? parameterSets[n]->inc_tax_rate.is_set
         : ((t == ParamType::sales_tax_rate) ? parameterSets[n]->sales_tax_rate.is_set
         : ((t == ParamType::firm_creation_prob) ? parameterSets[n]->firm_creation_prob.is_set
         : ((t == ParamType::dedns) ? parameterSets[n]->dedns.is_set
         : ((t == ParamType::unemp_ben_rate) ? parameterSets[n]->unemp_ben_rate.is_set
         : ((t == ParamType::distrib) ? parameterSets[n]->distrib.is_set
         : ((t == ParamType::prop_inv) ? parameterSets[n]->prop_inv.is_set
         : ((t == ParamType::boe_int) ? parameterSets[n]->boe_int.is_set
         : ((t == ParamType::bus_int) ? parameterSets[n]->bus_int.is_set
         : ((t == ParamType::loan_prob) ? parameterSets[n]->loan_prob.is_set
         : ((t == ParamType::inc_thresh) ? parameterSets[n]->inc_thresh.is_set
         : ((t == ParamType::recoup) ? parameterSets[n]->recoup.is_set
         : parameterSets[n]->invalid.is_set
         )))))))))))))));
    */
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

double Domain::getDistributionRate()
{
    return double(getParameterVal(ParamType::distrib)) / 100;
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
