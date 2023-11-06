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

    if (getDomain(name) == nullptr)
    {
        // A domain with given name is not in list. Create a new domain having
        // the required name and currency, and default parameters, and add it
        // to the end of the list
        dom = new Domain(name, currency, currencyAbbrev);
        domains.append(dom);
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
        const QString &currencyAbbrev,
               bool restore)
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
     * Create an arbitrary number of banks
     */
    for (int i = 0; i < NUMBER_OF_BANKS; i++)
    {
        banks.append(new Bank(this));
    }

    /*
     * Recover the parameter values from settings if required; otherwise set
     * their default values
     */
    if (restore)
    {

    }
    else
    {

    }

    /*
     * Update settings for this domain. We do this even if we have just restored
     * it from settings, because if any parameters were not restored (file was
     * corrupted, new parameters were added, whatever) the changes should be
     * preserved for consistency.
     */

    QSettings settings;

    settings.beginGroup("Domains");

    settings.beginGroup(name);
    settings.setValue("Currency", currency);
    settings.setValue("Abbrev", currencyAbbrev);

    // Set up default parameters
    settings.setValue(parameterKeys[ParamType::procurement],        0);
    settings.setValue(parameterKeys[ParamType::emp_rate],          95);
    settings.setValue(parameterKeys[ParamType::prop_con],          80);
    settings.setValue(parameterKeys[ParamType::inc_tax_rate],      10);
    settings.setValue(parameterKeys[ParamType::inc_thresh],        50);
    settings.setValue(parameterKeys[ParamType::sales_tax_rate],     0);
    settings.setValue(parameterKeys[ParamType::firm_creation_prob], 0);
    settings.setValue(parameterKeys[ParamType::dedns],              0);
    settings.setValue(parameterKeys[ParamType::unemp_ben_rate],    60);

    // This setting is not currently used and should not be confused with the
    // emp_rate property. active_pop was intended to allow a distinction to be
    // made between the population as a whole and the part of it that was
    // actually active. In practice this hasn't been needed as we have
    // assumed that the whole population is active (or equivalently that the
    // nominal population size refers only to the active population).
    // settings.setValue(parameterKeys[ParamType::active_pop],        60);

    settings.setValue(parameterKeys[ParamType::distrib],           50);
    settings.setValue(parameterKeys[ParamType::prop_inv],           2);
    settings.setValue(parameterKeys[ParamType::boe_int],            1);
    settings.setValue(parameterKeys[ParamType::bus_int],            3);
    settings.setValue(parameterKeys[ParamType::loan_prob],          0);
    settings.setValue(parameterKeys[ParamType::recoup],            10);
    settings.endGroup();

    settings.endGroup();

}

/*
 * Restore all domains from settings
 */
int Domain::restoreDomains()
{
    /* ADD CODE HERE */
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
    } else
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


void Domain::readParameters()
{
    // Get parameters for this Domain from settings.

    QSettings settings;

//    qDebug() << "Domain::readParameters(): file is" << settings.fileName();

    // TODO: Global settings (these should be read by, and stored in MainWindow)

//    _iterations = settings.value("iterations", 100).toInt();
//    _startups = settings.value("startups", 10).toInt();
//    _first_period = settings.value("start-period", 1).toInt();
//    _scale = settings.value("nominal-population", 1000).toDouble() / 1000;
//    _std_wage = settings.value("unit-wage", 100).toInt();
//    _population = 1000;

    // Select parameters for this Domain
    settings.beginGroup("Domain");
    settings.beginGroup(_name);

    /*
     * Before reading the default parameters we must get the currency and
     * currency abbreviation for the domain. These don't count as default
     * parameters as they can't be overridden by conditional parameters.
     */

    //
    // ************* TO BE ADDED HERE *******************
    //

    /*
     * We allow default and conditional parameters. Every domain will have a
     * defaukt set, which will be applied in the absence of any specified
     * conditions.
     */
    settings.beginGroup(_name + "/default");

    // For the default parameter set we only care about the val component of
    // each pair (and that's all that will have been stored for default
    // parameters). For conditional parameters we will have to read in both
    // elements of the pair.

    Params *p = new Params;     // default parameter set

    p->procurement.val        = settings.value(parameterKeys[ParamType::procurement], 0).toInt();
    p->emp_rate.val           = settings.value(parameterKeys[ParamType::emp_rate], 95).toInt();
    p->prop_con.val           = settings.value(parameterKeys[ParamType::prop_con], 80).toInt();
    p->inc_tax_rate.val       = settings.value(parameterKeys[ParamType::inc_tax_rate], 10).toInt();
    p->inc_thresh.val         = settings.value(parameterKeys[ParamType::inc_thresh], 50).toInt();
    p->sales_tax_rate.val     = settings.value(parameterKeys[ParamType::sales_tax_rate], 0).toInt();
    p->firm_creation_prob.val = settings.value(parameterKeys[ParamType::firm_creation_prob], 0).toInt();
    p->dedns.val              = settings.value(parameterKeys[ParamType::dedns], 0).toInt();
    p->unemp_ben_rate.val     = settings.value(parameterKeys[ParamType::unemp_ben_rate], 100).toInt();
    p->active_pop.val         = settings.value(parameterKeys[ParamType::active_pop], 60).toInt();
    p->distrib.val            = settings.value(parameterKeys[ParamType::distrib], 90).toInt();
    p->prop_inv.val           = settings.value(parameterKeys[ParamType::prop_inv], 20).toInt();
    p->boe_int.val            = settings.value(parameterKeys[ParamType::boe_int], 2).toInt();
    p->bus_int.val            = settings.value(parameterKeys[ParamType::bus_int], 3).toInt();
    p->loan_prob.val          = settings.value(parameterKeys[ParamType::loan_prob], 4).toInt();
    p->recoup.val             = settings.value(parameterKeys[ParamType::recoup], 10).toInt();

    settings.endGroup();        // end defaults

    parameterSets.clear();     // TODO: it would be better to do this after appending
    parameterSets.append(p);   // append the defaults we just read

    // How many conditional parameter sets...
    numParameterSets = settings.value("pages", 1).toInt();

    qDebug() << "Domain::readParameters():  Domain"
             << _name
             << "has"
             << numParameterSets
             << "pages";

    for (int page = 1; page < numParameterSets; page++)
    {
        p = new Params;         // conditional parameter set

        // Start conditional settings
        settings.beginGroup("condition-" + QString::number(page));

        // Conditions are relations between properties and integer values
        p->condition.property = getProperty(settings.value("property").toInt());

        // relations are encoded as integers
        int rel = settings.value("rel").toInt();
        switch (rel)
        {
        case 0:
            p->condition.opr = Opr::eq;
            break;
        case 1:
            p->condition.opr = Opr::neq;
            break;
        case 2:
            p->condition.opr = Opr::lt;
            break;
        case 3:
            p->condition.opr = Opr::gt;
            break;
        case 4:
            p->condition.opr = Opr::leq;
            break;
        case 5:
            p->condition.opr = Opr::geq;
            break;
        default:
            p->condition.opr = Opr::invalid_op;
            break;
        }

        // Set the value
        p->condition.val = settings.value("value").toInt();


        // Get the parameter values to be applied if the codition is met. Where
        // is_set is false the value will be ignored and the existing value --
        // either default or from a previous condition -- used instead
        QString attrib;

        attrib = parameterKeys[ParamType::procurement];
        p->procurement.is_set     = settings.value(attrib + "/isset").toBool();
        p->procurement.val        = settings.value(attrib + "/value").toInt();

        attrib = parameterKeys[ParamType::emp_rate];
        p->emp_rate.is_set        = settings.value(attrib + "/isset").toBool();
        p->emp_rate.val           = settings.value(attrib + "/value").toInt();

        attrib = parameterKeys[ParamType::prop_con];
        p->prop_con.is_set        = settings.value(attrib + "/isset").toBool();
        p->prop_con.val           = settings.value(attrib + "/value").toInt();

        attrib = parameterKeys[ParamType::inc_tax_rate];
        p->inc_tax_rate.is_set    = settings.value(attrib + "/isset").toBool();
        p->inc_tax_rate.val       = settings.value(attrib + "/value").toInt();

        attrib = parameterKeys[ParamType::inc_thresh];
        p->inc_thresh.is_set      = settings.value(attrib + "/isset").toBool();
        p->inc_thresh.val         = settings.value(attrib + "/value").toInt();

        attrib = parameterKeys[ParamType::sales_tax_rate];
        p->sales_tax_rate.is_set  = settings.value(attrib + "/isset").toBool();
        p->sales_tax_rate.val     = settings.value(attrib + "/value").toInt();

        attrib = parameterKeys[ParamType::firm_creation_prob];
        p->firm_creation_prob.is_set = settings.value(attrib + "/isset").toBool();
        p->firm_creation_prob.val = settings.value(attrib + "/value").toInt();

        attrib = parameterKeys[ParamType::dedns];
        p->dedns.is_set           = settings.value(attrib + "/isset").toBool();
        p->dedns.val              = settings.value(attrib + "/value").toInt();

        attrib = parameterKeys[ParamType::unemp_ben_rate];
        p->unemp_ben_rate.is_set  = settings.value(attrib + "/isset").toBool();
        p->unemp_ben_rate.val     = settings.value(attrib + "/value").toInt();

        attrib = parameterKeys[ParamType::active_pop];
        p->active_pop.is_set      = settings.value(attrib + "/isset").toBool();
        p->active_pop.val         = settings.value(attrib + "/value").toInt();

        attrib = parameterKeys[ParamType::distrib];
        p->distrib.is_set         = settings.value(attrib + "/isset").toBool();
        p->distrib.val            = settings.value(attrib + "/value").toInt();

        attrib = parameterKeys[ParamType::prop_inv];
        p->prop_inv.is_set        = settings.value(attrib + "/isset").toBool();
        p->prop_inv.val           = settings.value(attrib + "/value").toInt();

        attrib = parameterKeys[ParamType::boe_int];
        p->boe_int.is_set         = settings.value(attrib + "/isset").toBool();
        p->boe_int.val            = settings.value(attrib + "/value").toInt();

        attrib = parameterKeys[ParamType::bus_int];
        p->bus_int.is_set         = settings.value(attrib + "/isset").toBool();
        p->bus_int.val            = settings.value(attrib + "/value").toInt();

        attrib = parameterKeys[ParamType::loan_prob];
        p->loan_prob.is_set       = settings.value(attrib + "/isset").toBool();
        p->loan_prob.val          = settings.value(attrib + "/value").toInt();

        attrib = parameterKeys[ParamType::recoup];
        p->recoup.is_set          = settings.value(attrib + "/isset").toBool();
        p->recoup.val             = settings.value(attrib + "/value").toInt();

        // End settings for this condition
        settings.endGroup();

        parameterSets.append(p);
    }

    settings.endGroup();        // end Domains group

    qDebug() << "Domain::readParameters(): completed OK";
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
                     : ((type == ParamType::active_pop) ? parameterSets[i]->active_pop
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
}

bool Domain::isParamSet(ParamType t, int n)
{
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
         : ((t == ParamType::active_pop) ? parameterSets[n]->active_pop.is_set
         : ((t == ParamType::distrib) ? parameterSets[n]->distrib.is_set
         : ((t == ParamType::prop_inv) ? parameterSets[n]->prop_inv.is_set
         : ((t == ParamType::boe_int) ? parameterSets[n]->boe_int.is_set
         : ((t == ParamType::bus_int) ? parameterSets[n]->bus_int.is_set
         : ((t == ParamType::loan_prob) ? parameterSets[n]->loan_prob.is_set
         : ((t == ParamType::inc_thresh) ? parameterSets[n]->inc_thresh.is_set
         : ((t == ParamType::recoup) ? parameterSets[n]->recoup.is_set
         : parameterSets[n]->invalid.is_set
         )))))))))))))));
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

/**
 * Discontinued, but the formula might be useful some time
int Domain::getGovExpRate(int target_pop)
{
    // TODO: Consider re-instating this...

    // We calculate govt. expenditure using the formula:
    // population * target employment rate * average wage * tax rate.
    // Tax rate here is the 'effective tax rate' taking into account all taxes.
    // If we assume only income tax is applied then this is the income tax
    // rate. Otherwise it's quite complicated -- we should probably look into
    // this later.
    int target_emp = (target_pop > 0 ? target_pop : (population() * getTargetEmpRate()) / 100);
    int basic_wage = target_emp * getStdWage();
    return (basic_wage * getIncTaxRate()) / 100;
}
*/

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





int Domain::getActivePop()
{
    return getParameterVal(ParamType::active_pop);
}

