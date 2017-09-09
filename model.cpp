#include "model.h"
#include <QtGlobal>
#include "account.h"
#include <QDebug>

QList<Model*> Model::models;
Model *Model::current = nullptr;

QMap<ParamType,QString> Model::parameter_keys
{
    {ParamType::emp_rate, "employment-rate"},
    {ParamType::prop_con, "propensity-to-consume"},
    {ParamType::inc_tax_rate, "income-tax-rate"},
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
};

int Model::loadAllModels()
{
    qDebug() << "Model::loadAllModels()";

    // Read the model names from settings and create a new model for each one,
    // returning the number of models loaded.
    QSettings settings;
    int count = settings.beginReadArray("Models");
    for (int i = 0; i < count; ++i)
    {
        settings.setArrayIndex(i);
        Model *m = new Model(settings.value("name").toString());
        models.append(m);       // should this be a QMap so we can quickly find
                                // a model by name?
    }
    settings.endArray();

    qDebug() << "Model::loadAllModels():" << count << "models loaded";
    return count;
}

// This function creates a model with the given arguments but doesn't set it
// as current. (Perhaps it should?)
Model *Model::createModel(QString name)
{
    qDebug() << "Model::createModel(): name =" << name;

    qDebug() << "Model::createModel(): setting default parameters for model" << name;
    // Store default parameters
    QSettings settings;

    // Note that global parameters (model-wide) are not listed in ParamType
    // because they have no conditional values and so cannot be looked up in
    // the general parameter retrieval function getParameterVal(). They are
    // therefore dealt with as special cases without the convenience of being
    // listed in parameter_keys[ParamType].
    settings.beginGroup(name);

    // Add any model-specific parameters with default values here,,,

    settings.beginGroup("/default");
    settings.setValue(parameter_keys[ParamType::emp_rate],          95);
    settings.setValue(parameter_keys[ParamType::prop_con],          80);
    settings.setValue(parameter_keys[ParamType::inc_tax_rate],      10);
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

    settings.setValue(parameter_keys[ParamType::distrib],          100);
    settings.setValue(parameter_keys[ParamType::prop_inv],           2);
    settings.setValue(parameter_keys[ParamType::boe_int],            1);
    settings.setValue(parameter_keys[ParamType::bus_int],            3);
    settings.setValue(parameter_keys[ParamType::loan_prob],          0);

    settings.endGroup();
    settings.endGroup();

    // Create a model using the default parameters
    qDebug() << "Model::createModel(): creating model" << name;
    Model *model = new Model(name);

    // Add it to the global (static) list of models
    qDebug() << "Model::createModel(): adding model" << name << "to list";
    models.append(model);

    // Return a pointer to the new model
    return model;
}

Model *Model::model(QString name)
{
    for (int i = 0; i < models.count(); i++)
    {
        if (models[i]->name() == name)
        {
            qDebug() << "Model::model(): found model with name" << name;
            current = models[i];
            return current;
        }
    }

    qDebug() << "Model::model(): cannot find model with name" << name;
    return nullptr;         // no model found with that name
}

Model::Model(QString model_name)
{
    qDebug() << "Model::Model(): name =" << model_name;

    _name = model_name;

    // _notes and _iterations must be retrieved from settings. Possibly best not
    // to do this until they are needed as this constructor is called while
    // traversing settings and looking up different settings seems likely to
    // disrupt the process. Haven't checked this.
    //_notes = notes;
    //_iterations = iterations;

    // Set up a default set of parameters and copy them to settings
    // Same applies here...
    // Params *params = new Params();

    // Create the Government. This will automatically set up a single firm
    // representing nationalised industries, civil service, and any other
    // government owned business. Note that the Government itself (created
    // here) is not a business and taxation is not a payment for goods or
    // services. To access the Government-owned firm, use
    // Government::gov_firm().
    qDebug() << "Model::Model():" << model_name << "creating Government";
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
              << Property::num_properties;      // dummy

    // Set up an empty series for each property
    qDebug() << "Model::Model():" << model_name << "setting up results series";

    for (int i = 0; i < static_cast<int>(Property::num_properties); i++)
    {
        series[prop_list[i]] = new QLineSeries;
    }

    qDebug() << "Model::Model():" << model_name << "reading default parameters";
    readDefaultParameters();

    // TODO: Add conditional parameters
}

int Model::scale(Property p)
{
    int val = getPropertyVal(p);

    switch(p)
    {
    case Property::current_period:
    case Property::pc_emps:
    case Property::pc_unemps:
    case Property::pc_active:
    case Property::deficit_pc:
    case Property::hundred:
    case Property::zero:
    case Property::num_properties:
        return val;

    case Property::pop_size:
    case Property::gov_exp:
    case Property::bens_paid:
    case Property::gov_exp_plus:
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
        return val * _scale;
    }
}

void Model::readDefaultParameters()
{
    // Get parameters from settings. Currently we don't do anything with the
    // notes but this will probably change.
    QSettings settings;

    qDebug() << "Model::readDefaultParameters(): file is" << settings.fileName();

    // Global settings
    _iterations = settings.value("iterations", 100).toInt();
    _startups = settings.value("startups", 10).toInt();
    _first_period = settings.value("start-period", 1).toInt();
    _scale = settings.value("nominal-population", 1000).toInt() / 1000;
    _std_wage = settings.value("unit-wage", 500).toInt();
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

    p->emp_rate.val           = settings.value(parameter_keys[ParamType::emp_rate]).toInt();
    p->prop_con.val           = settings.value(parameter_keys[ParamType::prop_con]).toInt();
    p->inc_tax_rate.val       = settings.value(parameter_keys[ParamType::inc_tax_rate]).toInt();
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

    settings.endGroup();
    settings.endGroup();

    qDebug() << "p->inc_tax_rate.val =" << p->inc_tax_rate.val;
    qDebug() << "Model::readDefaultParameters(): p->dedns.val =" << p->dedns.val << ", key =" << parameter_keys[ParamType::dedns];

    // Conditional parameter sets must be appended, but the default set must
    // always be the first, so clear the list
    parameter_sets.clear();
    parameter_sets.append(p);

    // TODO: Read the conditions from settings (for the time being we will only
    // use default settings, which will always have index zero, so there's no
    // need to bother with this yet).
    // settings.beginReadArray(_name + "/conditions");
    // ...
    // settings.endArray();

    qDebug() << "Model::readDefaultParameters(): completed OK";
}

QString Model::name()
{
    return _name;
}

void Model::restart()
{
    readDefaultParameters();

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

void Model::run()
{
    qDebug() << "Model::run(): _name =" << _name;

    restart();

    // Seed the pseudo-random number generator. We need reproducibility so we
    // always seed with the same number. This makes inter-model comparisons
    // more valid
    qsrand(42);

    for (int _period = 1; _period <= _iterations + _first_period; _period++)
    {
        // -------------------------------------------
        // Initialise objects ready for next iteration
        // -------------------------------------------

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

        // Triggering government will transfer grants and benefits to firms
        // and workers before they are triggered
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

        // -------------------------------------------
        // Stats
        // -------------------------------------------

        // Append the values from this iteration to the series
        if (_period >= _first_period)
        {
            for (int i = 0; i < _num_properties/*static_cast<int>(Property::num_properties)*/; i++)
            {
                Property prop = prop_list[i];
                int val = scale(prop);
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

                    if (val < min_val[j])
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

    qDebug() << "Model::run(): _name =" << _name << "done";
}

int Model::period()
{
    return _period;
}

int Model::min_value(int ix)
{
    return min_val[ix];
}

int Model::max_value(int ix)
{
    return max_val[ix];
}

int Model::total(int ix)
{
    return sum[ix];
}

Government *Model::gov()
{
    return _gov;
}

Bank *Model::bank()
{
    return _bank;
}

Firm *Model::createFirm(bool state_supported)
{
    qDebug() << "Model::createFirm(): creating new firm";
    Firm *firm = new Firm(this, state_supported);
    if (state_supported)
    {
        QSettings settings;
        hireSome(firm, getStdWage(), 0, settings.value("government-employees").toInt());
    }
    firms.append(firm);
    return firm;
}

Firm *Model::selectRandomFirm()
{
    // Any firm apart from the government firm. This is a bit of a trick. We
    // make sure that the main (and currently, only) government-supported firm
    // is created first so it will always be firms[0]. This enables us to
    // select a random firm but excluding the government firm from the
    // selection.
    return firms[(qrand() % (firms.size() - 1)) + 1];
}

int Model::getWageBill(Firm *employer, bool include_dedns)
{
    int tot = 0;

    for (int i = 0; i < workers.count(); i++)
    {
        Worker *w = workers[i];
        if (w->employer == employer)
        {
            tot += w->agreedWage();
        }
    }

    if (include_dedns)
    {
        QSettings settings;
        tot = (tot * (100 + settings.value("pre-tax-dedns-rate", 0).toInt())) / 100;
    }

    return tot;
}

int Model::payWages(Firm *payer, int period)
{
    int amt_paid = 0;
    num_just_fired = 0;

    QSettings settings;

    int dedns_rate = settings.value("pre-tax-dedns-rate", 0).toInt();
    int num_workers = workers.count();

    for (int i = 0; i < num_workers; i++)
    {
        Worker *w = workers[i];
        if (w->isEmployedBy(payer))
        {
            int wage_due = w->agreedWage();
            int dedns = (wage_due * dedns_rate) / 100;
            int funds_available = payer->getBalance();

            bool ok_to_pay = false;

            if (funds_available - amt_paid < wage_due + dedns)
            {
                int shortfall = wage_due + dedns - funds_available + amt_paid;
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

int Model::payWorkers(int amount, int max_tot, Account *source, Reason reason)
{
    int amt_paid = 0;
    int num_workers = workers.count();

    for (int i = 0; i < num_workers; i++)
    {
        switch (reason)
        {
        case for_benefits:

            // qDebug() << "Model::payWorkers(): paying benefits";

            if (!workers[i]->isEmployed())
            {
                // qDebug() << "Model::payWorkers(): paying benefits (worker is unemployed)";
                workers[i]->credit(amount, source);
                amt_paid += amount;
            }
            else
            {
                // qDebug() << "Model::payWorkers(): no action (worker is employed)";
            }
            break;

        case for_bonus:

            // Note that when paying bonuses we do not check that
            // sufficient funds are available -- it's up to the caller to
            // ensure that the amount is correct. Any overpayment will
            // simply create a negative balance in the caller's account.

            // qDebug() << "Model::payWorkers(): paying benefits";

            if (workers[i]->getEmployer() == source)
            {
                // qDebug() << "Model::payWorkers(): worker is employee of payer";

                workers[i]->credit(amount, source);
                amt_paid += amount;
            }
            else
            {
                // qDebug() << "Model::payWorkers(): no action (worker is not employee of payer)";
            }
            break;
        }
    }

    // qDebug() << "Model::payWorkers(): returning amt_paid" << amt_paid;
    return amt_paid;
}

// ----------------------------------------------------------------------------
// These functions retrieve model properties
// ----------------------------------------------------------------------------

// IMPORTANT: Values must be retrieved in ascending order of Property because
// we save results on the way through so they can be used to deliver composite
// results more efficiently. If we ever need the composite results
// independently they should be calculated from scratch.
int Model::getPropertyVal(Property p)
{
    switch(p)
    {
    case Property::current_period:
        return period();

    case Property::pop_size:
        _pop_size = population();
        return _pop_size;

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
        // the government-owned firm. For the actually number (unscaled) use
        // firms.count().
        _num_firms = firms.count() - 1;
        return _num_firms;

    case Property::num_emps:
        _num_emps = getNumEmployed();
        return _num_emps;

    case Property::pc_emps:
        return (_pop_size > 0 ? (_num_emps * 100) / _pop_size : 0);

    case Property::num_unemps:
        _num_unemps = getNumUnemployed();
        return _num_unemps;

    case Property::pc_unemps:
        return (_pop_size > 0 ? (_num_unemps * 100) / _pop_size : 0);

    case Property::num_gov_emps:
        _num_gov_emps = getNumEmployedBy(gov()->gov_firm());
        return _num_gov_emps;

    case Property::pc_active:
        // We are assuming granularity is 1000
        _pc_active = (_num_emps + _num_unemps) / 10;
        return _pc_active;

    case Property::num_hired:
        _num_hired = getNumHired();
        return _num_hired;

    case Property::num_fired:
        _num_fired = getNumFired();
        return _num_fired;

    case Property::prod_bal:
        _prod_bal = getProdBal();
        return _prod_bal;

    case Property::wages:
        _wages = getWagesPaid();
        return _wages;

    case Property::consumption:
        _consumption = getPurchasesMade();
        return _consumption;

    case Property::deficit_pc:
        return _consumption == 0 ? 0 : (_deficit * 100) / _consumption;

    case Property::bonuses:
        _bonuses = getBonusesPaid();
        return _bonuses;

    case Property::dedns:
        _dedns = getDednsPaid();
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
        _amount_owed = getAmountOwed();
        return _amount_owed;

    case Property::bus_size:
        // We take the government firm into account here because _num_emps
        // doesn't make a distinction
        _bus_size = _num_emps  / (_num_firms + 1);
        return _bus_size;

    case Property::hundred:
        return 100;

    case Property::zero:
        return 0;

    case Property::num_properties:
        Q_ASSERT(false);
        return 0;                       // prevent compiler warning
    }
}

int Model::population()
{
    return _population;
}

int Model::getNumEmployed()
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

int Model::getNumEmployedBy(Firm *firm)
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

int Model::getNumUnemployed()
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

Worker *Model::hire(Firm *employer, int wage, int period)
{
    // Calculate friction
    int pop = population();
    int avail = pop - workers.count();      // number potentially available
    int access = (avail * 1000) / pop;      // accessibility as permillage
    int prob = employer->isGovernmentSupported() ? 0 : qrand() % 1000;
    if (prob >= access)
    {
        return nullptr;
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
        // changes in future we make sure you can't employ more people that
        // there actually are. This constraint could conceivably be varied if
        // we want to allow differential labour values by treating one worker
        // as equivalent to several workers.
        if (workers.count() < population())     // (unscaled)
        {
            w = new Worker(this);
            workers.push_back(w);
        }
        else
        {
            qDebug() << "Model::hire(): 100% employed, cannot hire any more!";
            return nullptr;
        }
    }

    w->employer = employer;
    w->period_hired = period;
    w->agreed_wage = wage;

    num_hired += 1;

    return w;
}

int Model::hireSome(Firm *employer, int wage, int period, int number_to_hire)
{
    int count;
    int wages_due;
    for (count = 0, wages_due = 0; count < number_to_hire; count++)
    {
        Worker *w = hire(employer, wage, period);
        if (w == nullptr)
        {
            break;
        }
        else
        {
            wages_due += w->agreedWage();
        }
    }

    return wages_due;
}

void Model::fire(Worker *w, int period)
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

int Model::getNumJustFired()
{
    return num_just_fired;
}

int Model::getNumHired()
{
    return num_hired;
}

int Model::getNumFired()
{
    return num_fired;
}

int Model::getProdBal()
{
    int bal = 0;
    for (int i = 0; i < firms.count(); i++)
    {
        bal += firms[i]->getBalance();
    }

    return bal;
}

int Model::getWagesPaid()
{
    int tot = 0;
    for (int i = 0; i < firms.count(); i++)
    {
        tot += firms[i]->getWagesPaid();
    }

    return tot;
}

// Tis should be the sames as getSalesReceipts(), which would be quicker to
// evaluate, but we use two different functions in case they should ever differ
// in future. This is the one we use for consumption as it looks at it from the
// consumers' end.
int Model::getPurchasesMade()
{
    int tot = 0;
    for (int i = 0; i < workers.count(); i++)
    {
        tot += workers[i]->getPurchasesMade();
    }

    return tot;
}

int Model::getSalesReceipts()
{
    int tot = 0;
    for (int i = 0; i < firms.count(); i++)
    {
        tot += firms[i]->getSalesReceipts();
    }

    return tot;
}

int Model::getBonusesPaid()
{
    int tot = 0;
    for (int i = 0; i < firms.count(); i++)
    {
        tot += firms[i]->getBonusesPaid();
    }

    return tot;
}

int Model::getDednsPaid()
{
    int tot = 0;
    for (int i = 0; i < firms.count(); i++)
    {
        tot += firms[i]->getDednsPaid();
    }

    return tot;
}

int Model::getIncTaxPaid()
{
    int tot = 0;
    for (int i = 0; i < workers.count(); i++)
    {
        tot += workers[i]->getIncTaxPaid();
    }

    return tot;
}

int Model::getSalesTaxPaid()
{
    int tot = 0;
    for (int i = 0; i < firms.count(); i++)
    {
        tot += firms[i]->getSalesTaxPaid();
    }

    return tot;
}

int Model::getWorkersBal(Model::Status status)
{
    int tot = 0;
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
int Model::getAmountOwed()
{
    int tot = 0;
    for (int i = 0; i < firms.count(); i++)
    {
        tot += firms[i]->getAmountOwed();
    }
    return tot;
}

// ----------------------------------------------------------------------------
// These functions retrieve model parameters
// ----------------------------------------------------------------------------

int Model::getParameterVal(ParamType type)
{
    // TODO: Currently we only check the default parameter set. This needs
    // extending to check conditional parameter sets (not yet implemented).

    // qDebug() << "Model::getParameterVal()";

    int i = 0;      // only default parameter set at present

    Pair p = (type == ParamType::emp_rate) ? parameter_sets[i]->emp_rate
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
             : parameter_sets[i]->invalid
             ))))))))))));


    // qDebug() << "Model::getParameterVal(): returning" << p.val;
    return p.val;
}

int Model::getTargetEmpRate()
{
    return getParameterVal(ParamType::emp_rate);
}

int Model::getStdWage()
{
    return _std_wage;
}

int Model::getPropCon()
{
    return getParameterVal(ParamType::prop_con);
}

int Model::getIncTaxRate()
{
    int res = getParameterVal(ParamType::inc_tax_rate);
    //qDebug() << "Model::getIncTaxRate() returning" << res;
    return res;
}

int Model::getSalesTaxRate()
{
    return getParameterVal(ParamType::sales_tax_rate);
}

int Model::getPreTaxDedns()
{
    return getParameterVal(ParamType::dedns);
}

int Model::getFCP()
{
    return getParameterVal(ParamType::firm_creation_prob);
}

int Model:: getUBR()
{
    return getParameterVal(ParamType::unemp_ben_rate);
}

int Model::getDistributionRate()
{
    return getParameterVal(ParamType::distrib);
}

int Model::getPropInv()
{
    return getParameterVal(ParamType::prop_inv);
}

int Model::getBoeRate()
{
    return getParameterVal(ParamType::boe_int);
}

int Model::getBusRate()
{
    return getParameterVal(ParamType::bus_int);
}

int Model::getLoanProb()
{
    return getParameterVal(ParamType::loan_prob);
}

int Model::getGovExpRate(int target_pop)
{
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

int Model::getActivePop()
{
    return getParameterVal(ParamType::active_pop);
}

// ----------------------------------------------------------------------------
// Condition processing
// ----------------------------------------------------------------------------

bool Model::applies(Model::Condition *condition)
{
    // Property::zero is always zero and can be used as a marker for the end of
    // the enum (Better to use num_properties). In a condition we also use it
    // to indicate that the associated
    // parameters are to be applied unconditionally (unless overridden by a
    // subsequent condition). In practice we currently access defaults
    // (unconditionals) separately from conditionals so we don't actually have
    // to eveluate the condition.
    if (condition->property == Property::zero) {
        return true;
    } else {
        int lhs = getPropertyVal(condition->property);
        Opr opr = condition->opr;
        int rhs = condition->val;
        return compare(lhs, rhs, opr);
    }
}

bool Model::compare(int lhs, int rhs, Opr opr)
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


