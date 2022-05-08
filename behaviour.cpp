#include "behaviour.h"
#include <QtGlobal>
#include "account.h"
#include <QDebug>
#include <limits>
#include <algorithm>

#include <math.h>

QList<Behaviour*> Behaviour::behaviours;
Behaviour *Behaviour::currentBehaviour = nullptr;

QMap<ParamType,QString> Behaviour::parameter_keys
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
    {ParamType::active_pop, "active-population-rate"},
    {ParamType::distrib, "reserve-rate"},
    {ParamType::prop_inv, "prop-invest"},
    {ParamType::boe_int, "boe-interest"},
    {ParamType::bus_int, "bus-interest"},
    {ParamType::loan_prob, "loan-prob"},
    {ParamType::recoup, "capex-recoup-periods"},
};

int Behaviour::loadAllBehaviours()
{
    qDebug() << "Behaviour::loadAllBehaviours()";

    // Read the behaviour (model) names from settings and create a new
    // behaviour for each one, returning the number of behaviours loaded.
    QSettings settings;
    int count = settings.beginReadArray("Models");
    for (int i = 0; i < count; ++i)
    {
        settings.setArrayIndex(i);
        behaviours.append(new Behaviour(settings.value("name").toString()));
        // NEXT: Currently we rely on the behaviour properties in settings. It
        // would be much better to store them all internally in the behaviour
        // instances. This will then mean we have to change the code where they
        // are retrieved or updated as well.
    }
    settings.endArray();

    qDebug() << "Behaviour::loadAllModels():" << count << "models loaded";
    return count;
}

// This function creates a Behaviour with the given name and default
// parameters (but doesn't set it as current -- perhaps it should).
Behaviour *Behaviour::createBehaviour(QString name)
{
    qDebug() << "Behaviour::createBehaviour(): name =" << name;
    qDebug() << "Behaviour::createBehaviour(): setting default parameters for model" << name;

    // Store default parameters:
    // Note that global parameters (Behaviour-wide) are not listed in ParamType
    // because they have no conditional values and so cannot be looked up in
    // the general parameter retrieval function getParameterVal(). They are
    // therefore dealt with as special cases without the convenience of being
    // listed in parameter_keys[ParamType].
    QSettings settings;
    settings.beginGroup(name);

    // Add all Behaviour-specific parameters with default values here,,,
    settings.beginGroup("/default");
    settings.setValue(parameter_keys[ParamType::procurement],        0);
    settings.setValue(parameter_keys[ParamType::emp_rate],          95);
    settings.setValue(parameter_keys[ParamType::prop_con],          80);
    settings.setValue(parameter_keys[ParamType::inc_tax_rate],      10);
    settings.setValue(parameter_keys[ParamType::inc_thresh],        50);
    settings.setValue(parameter_keys[ParamType::sales_tax_rate],     0);
    settings.setValue(parameter_keys[ParamType::firm_creation_prob], 0);
    settings.setValue(parameter_keys[ParamType::dedns],              0);
    settings.setValue(parameter_keys[ParamType::unemp_ben_rate],    60);

    // This setting is not currently used and should not be confused with the
    // emp_rate property. active_pop was intended to allow a distinction to be
    // made between the population as a whole and the part of it that was
    // actually active. In practice this hasn't been needed as we have
    // assumed that the whole population is active (or equivalently that the
    // nominal population size refers only to the active population).
    settings.setValue(parameter_keys[ParamType::active_pop],        60);

    settings.setValue(parameter_keys[ParamType::distrib],           50);
    settings.setValue(parameter_keys[ParamType::prop_inv],           2);
    settings.setValue(parameter_keys[ParamType::boe_int],            1);
    settings.setValue(parameter_keys[ParamType::bus_int],            3);
    settings.setValue(parameter_keys[ParamType::loan_prob],          0);
    settings.setValue(parameter_keys[ParamType::recoup],            10);

    settings.endGroup();
    settings.endGroup();

    // Create a Behaviour using the default parameters
    qDebug() << "Behaviour::createBehaviour(): creating Behaviour"
             << name;
    Behaviour *behaviour = new Behaviour(name);

    // Add it to the global (static) list of models
    qDebug() << "Behaviour::createBehaviour(): adding Behaviour"
             << name
             << "to list";

    behaviours.append(behaviour);

    // Return a pointer to the new Behaviour
    return behaviour;
}

Behaviour *Behaviour::getBehaviour(QString name)
{
    qDebug() << "Behaviour::getBehaviour("
             << name
             << "):"
             << "checking"
             << behaviours.count()
             << "behaviours";

    for (int i = 0; i < behaviours.count(); i++)
    {
        if (behaviours[i]->name() == name)
        {
            qDebug() << "Behaviour::getBehaviour(): found behaviour with name"
                     << name;
            currentBehaviour = behaviours[i];
            return currentBehaviour;
        }
    }

    qDebug() << "Behaviour::getBehaviour(): cannot find behaviour with name"
             << name;
    return nullptr;         // no model found with that name
}

Behaviour::Behaviour(QString behaviourName)
{
    qDebug() << "Behaviour::Behaviour(" << behaviourName << ")";

    _name = behaviourName;

    // _notes and _iterations must be retrieved from settings. Possibly best not
    // to do this until they are needed as this constructor is called while
    // traversing settings and looking up different settings seems likely to
    // disrupt the process. Haven't checked this.
    //_notes = notes;
    //_iterations = iterations;

    // Set up a default set of parameters and copy them to settings
    // Same applies here...
    // Params *params = new Params();

    // TODO: Disassociate Model (now called 'behaviour' in the GUI) from a
    // specific Government. Instead, behaviours should defined independently
    // and associated with Domains as required. Any given behaviour can be
    // associated with any number of Domains. This will complicate deletions of
    // behaviours as they cannot be deleted if they are assigned.

    // Create the Government. This will automatically set up a single firm
    // representing nationalised industries, civil service, and any other
    // government owned business. Note that the Government itself (created
    // here) is not a business and taxation is not a payment for goods or
    // services. To access the Government-owned firm, use
    // Government::gov_firm().
    qDebug() << "Behaviour::Behaviour(" << behaviourName << "): creating Government";

    // TODO: *** Government and central Bank now belong to Domain, but leave
    // them here as well until new setup is in place. ***
    _gov = new Government(this);
    _bank = new Bank(this);

    // -------------------------------------------------------------------------
    // To add a new property it must be added to the enum class Property in the
    // header file, and to the prop_list below (in the same position). Ordering
    // is significant because it will be assumed in the getProperty() function.
    // -------------------------------------------------------------------------

    prop_list << Property::current_period
              << Property::pop_size
              << Property::gov_exp
              << Property::bens_paid
              << Property::gov_exp_plus
              << Property::gov_recpts
              << Property::deficit
              << Property::gov_bal
              << Property::num_firms
              << Property::num_emps
              << Property::pc_emps
              << Property::num_unemps
              << Property::pc_unemps
              << Property::num_gov_emps
              << Property::pc_active
              << Property::num_hired
              << Property::num_fired
              << Property::prod_bal
              << Property::wages
              << Property::consumption
              << Property::deficit_pc
              << Property::bonuses
              << Property::dedns
              << Property::inc_tax
              << Property::sales_tax
              << Property::dom_bal
              << Property::amount_owed
              << Property::bus_size
              << Property::hundred
              << Property::zero
              << Property::procurement
              << Property::productivity
              << Property::rel_productivity
              << Property::unbudgeted
              << Property::investment
              << Property::gdp
              << Property::profit
              << Property::num_properties;      // dummy (keep at end)

    // Set up an empty series for each property
    qDebug() << "Behaviour::Behaviour():" << behaviourName << "setting up results series";

    for (int i = 0; i < static_cast<int>(Property::num_properties); i++)
    {
        series[prop_list[i]] = new QLineSeries;
    }

    qDebug() << "Behaviour::Behaviour():" << behaviourName << "reading default parameters";
    readParameters();
}

double Behaviour::scale(Property p)
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

double Behaviour::gini()
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
            qDebug() << "Behaviour::gini(): negative diff (" << diff << ") at interval" << i;
        }
        a += diff;                          // area A
    }

    qDebug() << "Behaviour::gini():  cum_tot =" << cum_tot << ",  pop =" << pop << ",  a =" << a << ",  a_tot =" << a_tot;
    _gini = a / a_tot;
    return _gini;
}

double Behaviour::getGini()
{
    return _gini;
}

double Behaviour::getProductivity()
{
    return productivity();
}

double Behaviour::productivity()
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

void Behaviour::readParameters()
{
    // Get parameters from settings.

    QSettings settings;

    qDebug() << "Behaviour::readParameters(): file is" << settings.fileName();

    // Global settings
    _iterations = settings.value("iterations", 100).toInt();
    _startups = settings.value("startups", 10).toInt();
    _first_period = settings.value("start-period", 1).toInt();
    _scale = settings.value("nominal-population", 1000).toDouble() / 1000;
    _std_wage = settings.value("unit-wage", 100).toInt();
    _population = 1000;

    // Model-specific settings
    settings.beginGroup(_name);

    _notes = settings.value("notes").toString();

    // Default model-specific settings
    settings.beginGroup("default");

    // For the default parameter set we only care about the val component of
    // each pair (and that's all that will have been stored for default
    // parameters). For conditional parameters we will have to read in both
    // elements of the pair.

    Params *p = new Params;     // default parameter set

    p->procurement.val        = settings.value(parameter_keys[ParamType::procurement]).toInt();
    p->emp_rate.val           = settings.value(parameter_keys[ParamType::emp_rate]).toInt();
    p->prop_con.val           = settings.value(parameter_keys[ParamType::prop_con]).toInt();
    p->inc_tax_rate.val       = settings.value(parameter_keys[ParamType::inc_tax_rate]).toInt();
    p->inc_thresh.val         = settings.value(parameter_keys[ParamType::inc_thresh]).toInt();
    p->sales_tax_rate.val     = settings.value(parameter_keys[ParamType::sales_tax_rate]).toInt();
    p->firm_creation_prob.val = settings.value(parameter_keys[ParamType::firm_creation_prob]).toInt();
    p->dedns.val              = settings.value(parameter_keys[ParamType::dedns]).toInt();
    p->unemp_ben_rate.val     = settings.value(parameter_keys[ParamType::unemp_ben_rate]).toInt();
    p->active_pop.val         = settings.value(parameter_keys[ParamType::active_pop]).toInt();
    p->distrib.val            = settings.value(parameter_keys[ParamType::distrib]).toInt();
    p->prop_inv.val           = settings.value(parameter_keys[ParamType::prop_inv]).toInt();
    p->boe_int.val            = settings.value(parameter_keys[ParamType::boe_int]).toInt();
    p->bus_int.val            = settings.value(parameter_keys[ParamType::bus_int]).toInt();
    p->loan_prob.val          = settings.value(parameter_keys[ParamType::loan_prob]).toInt();
    p->recoup.val             = settings.value(parameter_keys[ParamType::recoup]).toInt();

    settings.endGroup();

    // Conditional parameter sets must be appended, but the default set must
    // always be the first, so clear the list
    parameter_sets.clear();
    parameter_sets.append(p);

    num_parameter_sets = settings.value("pages", 1).toInt();
    qDebug() << "Behaviour::readParameters():  model" << _name << "has" << num_parameter_sets << "pages";
    for (int page = 1; page < num_parameter_sets; page++)
    {
        p = new Params;         // conditional parameter set

        settings.beginGroup("condition-" + QString::number(page));

        p->condition.property = getProperty(settings.value("property").toInt());

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

        p->condition.val = settings.value("value").toInt();

        QString attrib;

        attrib = parameter_keys[ParamType::procurement];
        p->procurement.is_set     = settings.value(attrib + "/isset").toBool();
        p->procurement.val        = settings.value(attrib + "/value").toInt();

        attrib = parameter_keys[ParamType::emp_rate];
        p->emp_rate.is_set        = settings.value(attrib + "/isset").toBool();
        p->emp_rate.val           = settings.value(attrib + "/value").toInt();

        attrib = parameter_keys[ParamType::prop_con];
        p->prop_con.is_set        = settings.value(attrib + "/isset").toBool();
        p->prop_con.val           = settings.value(attrib + "/value").toInt();

        attrib = parameter_keys[ParamType::inc_tax_rate];
        p->inc_tax_rate.is_set    = settings.value(attrib + "/isset").toBool();
        p->inc_tax_rate.val       = settings.value(attrib + "/value").toInt();

        attrib = parameter_keys[ParamType::inc_thresh];
        p->inc_thresh.is_set      = settings.value(attrib + "/isset").toBool();
        p->inc_thresh.val         = settings.value(attrib + "/value").toInt();

        attrib = parameter_keys[ParamType::sales_tax_rate];
        p->sales_tax_rate.is_set  = settings.value(attrib + "/isset").toBool();
        p->sales_tax_rate.val     = settings.value(attrib + "/value").toInt();

        attrib = parameter_keys[ParamType::firm_creation_prob];
        p->firm_creation_prob.is_set = settings.value(attrib + "/isset").toBool();
        p->firm_creation_prob.val = settings.value(attrib + "/value").toInt();

        attrib = parameter_keys[ParamType::dedns];
        p->dedns.is_set           = settings.value(attrib + "/isset").toBool();
        p->dedns.val              = settings.value(attrib + "/value").toInt();

        attrib = parameter_keys[ParamType::unemp_ben_rate];
        p->unemp_ben_rate.is_set  = settings.value(attrib + "/isset").toBool();
        p->unemp_ben_rate.val     = settings.value(attrib + "/value").toInt();

        attrib = parameter_keys[ParamType::active_pop];
        p->active_pop.is_set      = settings.value(attrib + "/isset").toBool();
        p->active_pop.val         = settings.value(attrib + "/value").toInt();

        attrib = parameter_keys[ParamType::distrib];
        p->distrib.is_set         = settings.value(attrib + "/isset").toBool();
        p->distrib.val            = settings.value(attrib + "/value").toInt();

        attrib = parameter_keys[ParamType::prop_inv];
        p->prop_inv.is_set        = settings.value(attrib + "/isset").toBool();
        p->prop_inv.val           = settings.value(attrib + "/value").toInt();

        attrib = parameter_keys[ParamType::boe_int];
        p->boe_int.is_set         = settings.value(attrib + "/isset").toBool();
        p->boe_int.val            = settings.value(attrib + "/value").toInt();

        attrib = parameter_keys[ParamType::bus_int];
        p->bus_int.is_set         = settings.value(attrib + "/isset").toBool();
        p->bus_int.val            = settings.value(attrib + "/value").toInt();

        attrib = parameter_keys[ParamType::loan_prob];
        p->loan_prob.is_set       = settings.value(attrib + "/isset").toBool();
        p->loan_prob.val          = settings.value(attrib + "/value").toInt();

        attrib = parameter_keys[ParamType::recoup];
        p->recoup.is_set          = settings.value(attrib + "/isset").toBool();
        p->recoup.val             = settings.value(attrib + "/value").toInt();

        settings.endGroup();

        parameter_sets.append(p);
    }


    qDebug() << "Behaviour::readParameters(): completed OK";
}

Behaviour::Property Behaviour::getProperty(int n)
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

QString Behaviour::name()
{
    return _name;
}

int Behaviour::getIters()
{
    return _iterations;
}

int Behaviour::getStartPeriod()
{
    return _first_period;
}

void Behaviour::restart()
{
    readParameters();

    // Clear all series
    for (int i = 0; i < static_cast<int>(Property::num_properties); i++)
    {
        Property prop = prop_list[i];
        series[prop]->clear();
    }

    int num_workers = workers.count();
    int num_firms = firms.count();

    // Delete all workers and clear the list
    for (int i = 0; i < num_workers; i++)
    {
        delete workers[i];
    }
    workers.clear();

    // Delete all firms and clear the list
    for (int i = 0; i < num_firms; i++)
    {
        delete firms[i];
    }
    firms.clear();
    firms.reserve(100);

    // Reset the Government, which will re-create the gov firm as the first
    // firm in the list
    gov()->reset();

    // Don't scale the number of startups internally, but must be scaled in
    // stats reporting
    for (int i = 0; i < _startups; i++)
    {
        createFirm();
    }
}

// -----------------------------------------
// TODO: *** Remove all this function to MainWindow (?)
// Behaviour::run() is the main driver function
//
// TODO: Investigate using different cycles for salaries and payments
// -----------------------------------------

void Behaviour::run(bool randomised)
{
    qDebug() << "Behaviour::run(): randomised =" << randomised << "  _name =" << _name;

    restart();

    // ***
    // Seed the pseudo-random number generator.
    // We need reproducibility so we always seed with the same number.
    // This makes inter-model comparisons more valid.
    // ***

    if (!randomised) {
        qDebug() << "Behaviour::run(): using fixed seed (42)";
        qsrand(42);
    }

    for (_period = 0; _period <= _iterations + _first_period; _period++)
    {
        // -------------------------------------------
        // Initialise objects ready for next iteration
        // -------------------------------------------

        _dedns = 0;         // deductions are tracked by the model object and are
                            // accumulated within but not across periods

        _gov->init();

        int num_workers = workers.count();
        int num_firms = firms.count();

        for (int i = 0; i < num_firms; i++) {
            firms[i]->init();
        }

        for (int i = 0; i < num_workers; i++) {
            workers[i]->init();
        }

        // Reset counters

        num_hired = 0;
        num_fired = 0;
        num_just_fired = 0;

        // -------------------------------------------
        // Trigger objects
        // -------------------------------------------

        // Triggering government will direct payments to firms and benefits to
        //  workers before they are triggered
        _gov->trigger(_period);

        // Triggering firms will pay deductions to government and wages to
        // workers. Firms will also fire any workers they can't afford to pay.
        // Workers receiving payment will pay income tax to the government
        for (int i = 0; i < num_firms; i++) {
            firms[i]->trigger(_period);
        }

        // Trigger workers to make purchases
        for (int i = 0; i < num_workers; i++) {
            workers[i]->trigger(_period);
        }

        // -------------------------------------------
        // Post-trigger (epilogue) phase
        // -------------------------------------------

        // Post-trigger for firms so they can pay tax on sales just made, pay
        // bonuses, and hire more employees (investment)
        for (int i = 0, c = firms.count(); i < c; i++) {
            firms[i]->epilogue(_period);
        }

        // Same for workers so they can keep rolling averages up to date
        for (int i = 0, c = workers.count(); i < c; i++) {
            workers[i]->epilogue(_period);
        }

        // -------------------------------------------
        // Stats
        // -------------------------------------------

        // Append the values from this iteration to the series
        if (_period >= _first_period)
        {
            for (int i = 0; i < _num_properties/*static_cast<int>(Property::num_properties)*/; i++)
            {
                Property prop = prop_list[i];
                double val = scale(prop);
                series[prop]->append(_period, val);

                int j = static_cast<int> (prop);

                if (_period == _first_period)
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
        }

        // -------------------------------------------
        // Exogenous changes
        // -------------------------------------------

        // Create a new firm, possibly
        if (qrand() % 100 < getFCP()) {
            createFirm();
        }
    }

    qDebug() << "Behaviour::run(): _name =" << _name << "  gini =" << gini();
}


int Behaviour::period()
{
    return _period;
}

int Behaviour::min_value(int ix)
{
    qDebug() << "Behaviour::min_value(" << ix << ") returning" << min_val[ix];
    return min_val[ix];
}

int Behaviour::max_value(int ix)
{
    qDebug() << "Behaviour::max_value(" << ix << ") returning" << max_val[ix];
    return max_val[ix];
}

int Behaviour::total(int ix)
{
    return sum[ix];
}

Government *Behaviour::gov()
{
    return _gov;
}

Bank *Behaviour::bank()
{
    return _bank;
}

Firm *Behaviour::createFirm(bool state_supported)
{
    Firm *firm = new Firm(this, state_supported);
    if (state_supported)
    {
        QSettings settings;
        hireSome(firm, getStdWage(), 0, settings.value("government-employees").toInt());
    }
    firms.append(firm);
    return firm;
}

// Returns a random non-government firm or nullptr if there are none
Firm *Behaviour::selectRandomFirm(Firm *exclude)
{
    // ***
    // Any firm apart from the government firm. This is a bit of a trick. We
    // make sure that the main (and currently, only) government-supported firm
    // is created first so it will always be firms[0]. This enables us to
    // select a random firm but excluding the government firm from the
    // selection. Special acion is required if there are fewer than 2 firms.
    // ***

    Firm *res;

    int n = firms.size();
    if (n < 3) {
        res = nullptr;
    } else if (n == 3) {
        return (exclude == firms[1] ? firms[2] : firms[1]);
    } else {
        while ((res = firms[(qrand() % (firms.size() - 1)) + 1]) == exclude);
    }
    return res;
}

int Behaviour::getWageBill(Firm *employer, bool include_dedns)
{
    int tot = 0;

    for (int i = 0; i < workers.count(); i++)
    {
        Worker *w = workers[i];
        if (w->employer == employer)
        {
            tot += w->agreed_wage;
        }
    }

    if (include_dedns)
    {
        tot *= (1.0 + getPreTaxDedns());
    }

    return tot;
}

int Behaviour::payWages(Firm *payer, int period)
{
    double amt_paid = 0;
    num_just_fired = 0;

    QSettings settings;

    double dedns_rate = getPreTaxDedns();
    int num_workers = workers.count();

    for (int i = 0; i < num_workers; i++)
    {
        Worker *w = workers[i];
        if (w->isEmployedBy(payer))
        {
            int wage_due = w->agreed_wage;
            double dedns = dedns_rate * wage_due;
            double funds_available = payer->getBalance();

            bool ok_to_pay = false;

            if (funds_available - amt_paid < wage_due + dedns)
            {
                double shortfall = wage_due + dedns - funds_available + amt_paid;
                if (payer->isGovernmentSupported())
                {
                    // Get additional funds from government
                    gov()->debit(payer, shortfall);

                    // Transfer the funds to the payer
                    payer->credit(shortfall, gov());

                    ok_to_pay = true;
                }
                else
                {
                    // Possibly request a loan, depending on policy.
                    // This assumes there are getLoanProb() returns an integer
                    // from 0 (= never) to 4 (= always)
                    if (qrand() % 4 < getLoanProb())
                    {
                        // Apply a bank loan to cover the shortfall
                        _bank->lend(shortfall, getBusRate(), payer);
                        ok_to_pay = true;
                    }
                }
            }
            else
            {
                ok_to_pay = true;
            }

            if (ok_to_pay)
            {
                // Pay the full amount of wages to worker
                workers[i]->credit(wage_due, payer);

                // Pay the deductions straight back to the government.
                gov()->credit(dedns, payer);

                amt_paid += wage_due + dedns;
                _dedns += dedns;
            }
            else
            {
                if (payer->isGovernmentSupported())
                {
                    // This should never happen as govt-suptd payer can
                    // always get required funds from government
                    Q_ASSERT(false);
                }
                else
                {
                    // Not able to pay this worker so fire instead
                    fire(w, period);
                }
            }
        }
    }
    return amt_paid;                // so caller can update balance
}

double Behaviour::payWorkers(double amount, Account *source, Reason reason)
{
    double amt_paid = 0;
    int num_workers = workers.count();

    for (int i = 0; i < num_workers; i++)
    {
        switch (reason)
        {
        case for_benefits:

            if (!workers[i]->isEmployed())
            {
                workers[i]->credit(amount, source);
                amt_paid += amount;
            }
            break;

        case for_bonus:

            // Note that when paying bonuses we do not check that
            // sufficient funds are available -- it's up to the caller to
            // ensure that the amount is correct. Any overpayment will
            // simply create a negative balance in the caller's account.

            if (workers[i]->getEmployer() == source)
            {
                workers[i]->credit(amount, source);
                amt_paid += amount;
            }
            break;
        }
    }

    return amt_paid;
}

// ----------------------------------------------------------------------------
// These functions retrieve model properties
// ----------------------------------------------------------------------------

// IMPORTANT: Values must be retrieved in ascending order of Property because
// we save results on the way through so they can be used to deliver composite
// results more efficiently. If we ever need the composite results
// independently they should be calculated from scratch.
//    Note also that this function returns a double, regardless of whether the
// parameter is integral or not. Stored values (generally marked by having '_'
// as the first character) are held in the most appropriate form, and access
// functions (e.g. period(), getWagesPaid(), etc) also return the correct type.

double Behaviour::getPropertyVal(Property p)
{
    switch(p)
    {
    case Property::current_period:
        return period();

    case Property::pop_size:
        _pop_size = population();
        return double(_pop_size);

    case Property::gov_exp:
        _exp = gov()->getExpenditure();
        return _exp;

    case Property::bens_paid:
        _bens = gov()->getBenefitsPaid();
        return _bens;

    case Property::gov_exp_plus:
        return _exp + _bens;

    case Property::gov_recpts:
        _rcpts = gov()->getReceipts();
        return _rcpts;

    case Property::deficit:
        _deficit = _exp + _bens - _rcpts;
        return _deficit;

    case Property::gov_bal:
        // We are displaying this as a positive debt rather than a negative
        // balance just so we can avoid displaying negative quamtities (with
        // exception of negative deficits) and keep the x-axis at the bottom
        // as there's no simple provision for displaying it at y = 0.
        _gov_bal = -(gov()->getBalance());
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

    case Property::num_gov_emps:
        _num_gov_emps = getNumEmployedBy(gov()->gov_firm());
        return double(_num_gov_emps);

    case Property::pc_active:
        _pc_active = double(_num_emps + _num_unemps) / 10;  // assuming granularity 1000
        return _pc_active;

    case Property::num_hired:
        _num_hired = getNumHired();
        return double(_num_hired);

    case Property::num_fired:
        _num_fired = getNumFired();
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
        return _consumption == 0 ? 0.0 : (_deficit * 100) / _consumption;

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
        _dom_bal = getWorkersBal(Status::any);
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
        _proc_exp = getProcurementExpenditure();
        return _proc_exp;

    case Property::productivity:
        _productivity = productivity();
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
        return gov()->getUnbudgetedExp();

    case Property::investment:
        _investment = getInvestment();
        return _investment;

    case Property::gdp:
        //_gdp = _consumption + _investment + _exp + _bens;
        _gdp = _consumption - _investment;  // https://en.wikipedia.org/wiki/Gross_domestic_product
        return _gdp;

    case Property::profit:
        _profit = _gdp - _wages - _inc_tax - _sales_tax;
        return _profit;

    case Property::num_properties:
        Q_ASSERT(false);
        return 0;                       // prevent compiler warning
    }
}

int Behaviour::population()
{
    return _population;
}

int Behaviour::getNumEmployed()
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

int Behaviour::getNumEmployedBy(Firm *firm)
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

int Behaviour::getNumUnemployed()
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

double Behaviour::getProcurementExpenditure()
{
    return gov()->getProcExp();
}

Worker *Behaviour::hire(Firm *employer, double wage, int period)
{
    // Calculate friction
    if (period > 0)
    {
        int pop = population();
        Q_ASSERT(pop != 0);

        int avail = pop - getNumEmployed();     // number potentially available
        int access = (avail * 1000) / pop;      // accessibility per thousand
        int prob = employer->isGovernmentSupported() ? 0 : qrand() % 1000;
        if (prob >= access)
        {
            return nullptr;
        }
    }

    Worker *w = nullptr;
    for (int i = 0; i < workers.count(); i++)
    {
        if (workers[i]->getEmployer() == nullptr)       // i.e. unemployed
        {
            w = workers[i];
        }
    }

    if (w == nullptr)
    {
        // We've already checked availability, but in case the mechanism
        // changes in future we make sure you can't employ more people than
        // there actually are. This constraint could conceivably be varied if
        // we want to allow differential labour values by treating one worker
        // as equivalent to several workers.
        if (workers.count() < population() || period == 0)     // (unscaled)
        {
            w = new Worker(this);
            workers.push_back(w);
        }
        else
        {
            qDebug() << "Behaviour::hire(): 100% employed, cannot hire any more!";
            return nullptr;
        }
    }

    w->employer = employer;
    w->period_hired = period;
    w->agreed_wage = wage;

    num_hired += 1;

    return w;
}

double Behaviour::hireSome(Firm *employer, double wage, int period, int number_to_hire)
{
    int count;
    double wages_due;
    for (count = 0, wages_due = 0; count < number_to_hire; count++)
    {
        Worker *w = hire(employer, wage, period);
        if (w == nullptr)
        {
            break;
        }
        else
        {
            wages_due += w->agreed_wage;
        }
    }

    return wages_due;
}

void Behaviour::fire(Worker *w, int period)
{
    w->employer = nullptr;
    w->period_fired = period;
    num_fired++;
}

// getNumJustFired() gets the number of workers that were fired the last time
// payWorkers() was called with reason == wages in the current period. It's
// provided because the value returned is the amount paid, but the caller may
// also want to know how many workers were fired. For the total number of
// workers fired this period use getNumFired() instead.

int Behaviour::getNumJustFired()
{
    return num_just_fired;
}

int Behaviour::getNumHired()
{
    return num_hired;
}

int Behaviour::getNumFired()
{
    return num_fired;
}

double Behaviour::getProdBal()
{
    double bal = 0;
    for (int i = 0; i < firms.count(); i++)
    {
        bal += firms[i]->getBalance();
    }

    return bal;
}

double Behaviour::getWagesPaid()
{
    double tot = 0.0;
    for (int i = 0; i < firms.count(); i++)
    {
        tot += firms[i]->getWagesPaid();
    }

    return tot;
}

double Behaviour::getInvestment()
{
    double tot = 0.0;
    for (int i = 0; i < firms.count(); i++)
    {
        tot += firms[i]->getInvestment();
    }

    return tot;
}

// This should be the same as getSalesReceipts(), which would be quicker to
// evaluate, but we use two different functions in case they should ever differ
// in future. This is the one we use for consumption as it looks at it from the
// consumers' end.
double Behaviour::getPurchasesMade()
{
    double tot = 0;
    for (int i = 0; i < workers.count(); i++)
    {
        tot += workers[i]->getPurchasesMade();
    }

    return tot;
}

double Behaviour::getSalesReceipts()
{
    double tot = 0;
    for (int i = 0; i < firms.count(); i++)
    {
        tot += firms[i]->getSalesReceipts();
    }

    return tot;
}

double Behaviour::getBonusesPaid()
{
    double tot = 0;
    for (int i = 0; i < firms.count(); i++)
    {
        tot += firms[i]->getBonusesPaid();
    }

    return tot;
}

double Behaviour::getDednsPaid()
{
    return _dedns;
}

double Behaviour::getIncTaxPaid()
{
    double tot = 0;
    for (int i = 0; i < workers.count(); i++)
    {
        tot += workers[i]->getIncTaxPaid();
    }

    return tot;
}

double Behaviour::getSalesTaxPaid()
{
    double tot = 0;
    for (int i = 0; i < firms.count(); i++)
    {
        tot += firms[i]->getSalesTaxPaid();
    }

    return tot;
}

double Behaviour::getWorkersBal(Behaviour::Status status)
{
    double tot = 0.0;
    for (int i = 0; i < workers.count(); i++)
    {
        switch (status)
        {
            case Status::any:
                tot += workers[i]->getBalance();
                break;

            case Status::employed:
                if (workers[i]->isEmployed()) {
                    tot += workers[i]->getBalance();
                }
                break;

            case Status::unemployed:
                if (!workers[i]->isEmployed()) {
                    tot += workers[i]->getBalance();
                }
                break;
        }
    }

    return tot;
}

// TODO: At present only businesses can get loans, but this should be extended
// to workers in due course. We also need to allow banks to get loans from the
// central bank -- i.e. from the government.
double Behaviour::getAmountOwed()
{
    double tot = 0;
    for (int i = 0; i < firms.count(); i++)
    {
        tot += firms[i]->getAmountOwed();
    }
    return tot;
}

// ----------------------------------------------------------------------------
// These functions retrieve model parameters
// ----------------------------------------------------------------------------

int Behaviour::getParameterVal(ParamType type)
{
    Pair p;
    for (int i = 0; i < num_parameter_sets; i++)
    {
        if (i == 0 || applies(parameter_sets[i]->condition))
        {
            // TODO: p doesn't have to be a Pair as we now only use the val component
            if (isParamSet(type, i))
            {
                p =    (type == ParamType::procurement) ? parameter_sets[i]->procurement
                     : ((type == ParamType::emp_rate) ? parameter_sets[i]->emp_rate
                     : ((type == ParamType::prop_con) ? parameter_sets[i]->prop_con
                     : ((type == ParamType::inc_tax_rate) ? parameter_sets[i]->inc_tax_rate
                     : ((type == ParamType::sales_tax_rate) ? parameter_sets[i]->sales_tax_rate
                     : ((type == ParamType::firm_creation_prob) ? parameter_sets[i]->firm_creation_prob
                     : ((type == ParamType::dedns) ? parameter_sets[i]->dedns
                     : ((type == ParamType::unemp_ben_rate) ? parameter_sets[i]->unemp_ben_rate
                     : ((type == ParamType::active_pop) ? parameter_sets[i]->active_pop
                     : ((type == ParamType::distrib) ? parameter_sets[i]->distrib
                     : ((type == ParamType::prop_inv) ? parameter_sets[i]->prop_inv
                     : ((type == ParamType::boe_int) ? parameter_sets[i]->boe_int
                     : ((type == ParamType::bus_int) ? parameter_sets[i]->bus_int
                     : ((type == ParamType::loan_prob) ? parameter_sets[i]->loan_prob
                     : ((type == ParamType::inc_thresh) ? parameter_sets[i]->inc_thresh
                     : ((type == ParamType::recoup) ? parameter_sets[i]->recoup
                     : parameter_sets[i]->invalid
                     )))))))))))))));
            }
        }
    }

    return p.val;
}

bool Behaviour::isParamSet(ParamType t, int n)
{
    if (n == 0) {
        return true;
    }

    return  (t == ParamType::procurement) ? parameter_sets[n]->procurement.is_set
         : ((t == ParamType::emp_rate) ? parameter_sets[n]->emp_rate.is_set
         : ((t == ParamType::prop_con) ? parameter_sets[n]->prop_con.is_set
         : ((t == ParamType::inc_tax_rate) ? parameter_sets[n]->inc_tax_rate.is_set
         : ((t == ParamType::sales_tax_rate) ? parameter_sets[n]->sales_tax_rate.is_set
         : ((t == ParamType::firm_creation_prob) ? parameter_sets[n]->firm_creation_prob.is_set
         : ((t == ParamType::dedns) ? parameter_sets[n]->dedns.is_set
         : ((t == ParamType::unemp_ben_rate) ? parameter_sets[n]->unemp_ben_rate.is_set
         : ((t == ParamType::active_pop) ? parameter_sets[n]->active_pop.is_set
         : ((t == ParamType::distrib) ? parameter_sets[n]->distrib.is_set
         : ((t == ParamType::prop_inv) ? parameter_sets[n]->prop_inv.is_set
         : ((t == ParamType::boe_int) ? parameter_sets[n]->boe_int.is_set
         : ((t == ParamType::bus_int) ? parameter_sets[n]->bus_int.is_set
         : ((t == ParamType::loan_prob) ? parameter_sets[n]->loan_prob.is_set
         : ((t == ParamType::inc_thresh) ? parameter_sets[n]->inc_thresh.is_set
         : ((t == ParamType::recoup) ? parameter_sets[n]->recoup.is_set
         : parameter_sets[n]->invalid.is_set
         )))))))))))))));
}

double Behaviour::getProcurement()
{
    return double(getParameterVal(ParamType::procurement));
}

double Behaviour::getTargetEmpRate()
{
    return double(getParameterVal(ParamType::emp_rate)) / 100;
}

double Behaviour::getStdWage()
{
    return _std_wage;
}

double Behaviour::getPropCon()
{
    return double(getParameterVal(ParamType::prop_con)) / 100;
}

double Behaviour::getIncomeThreshold()
{
    return double(getParameterVal(ParamType::inc_thresh));
}

double Behaviour::getIncTaxRate()
{
    return double(getParameterVal(ParamType::inc_tax_rate)) / 100;
}

double Behaviour::getSalesTaxRate()
{
    return double(getParameterVal(ParamType::sales_tax_rate)) / 100;
}

double Behaviour::getPreTaxDedns()
{
    return double(getParameterVal(ParamType::dedns)) / 100;
}

double Behaviour::getCapexRecoupTime()
{
    return double(getParameterVal(ParamType::recoup));
}

double Behaviour::getFCP()
{
    return double(getParameterVal(ParamType::firm_creation_prob)) / 100;
}

double Behaviour::getUBR()
{
    return double(getParameterVal(ParamType::unemp_ben_rate)) / 100;
}

double Behaviour::getDistributionRate()
{
    return double(getParameterVal(ParamType::distrib)) / 100;
}

double Behaviour::getPropInv()
{
    return double(getParameterVal(ParamType::prop_inv)) / 100;
}

double Behaviour::getBoeRate()
{
    return double(getParameterVal(ParamType::boe_int)) / 100;
}

double Behaviour::getBusRate()
{
    return double(getParameterVal(ParamType::bus_int)) / 100;
}

double Behaviour::getLoanProb()
{
    return double(getParameterVal(ParamType::loan_prob)) / 100;
}

/* Discontinued, but the formula might be useful some time
int Behaviour::getGovExpRate(int target_pop)
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

int Behaviour::getActivePop()
{
    return getParameterVal(ParamType::active_pop);
}

// ----------------------------------------------------------------------------
// Condition processing
// ----------------------------------------------------------------------------

bool Behaviour::applies(Condition condition)
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

bool Behaviour::compare(int lhs, int rhs, Opr opr)
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


