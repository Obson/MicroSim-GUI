#ifndef ACCOUNT_H
#define ACCOUNT_H

#include "QtCharts/qchartview.h"
#include <QObject>
#include <iostream>
#include <list>
#include <QList>
#include <QListWidgetItem>
#include <QDebug>

class Statistics;
class Firm;
class Government;

#include <map>
#include <QVector>
#include <QtCharts/QLineSeries>
#include <QLineSeries>
#include <QSettings>
#include <QMap>

QT_CHARTS_USE_NAMESPACE


/*
 *  Parameters should not be confused with properties.
 */

enum class ParamType {
    procurement,
    emp_rate,       // unused (?) but leave in place for ordering
    prop_con,
    inc_tax_rate,
    inc_thresh,
    sales_tax_rate,
    firm_creation_prob,
    recoup,
    dedns,
    unemp_ben_rate,
    active_pop,
    distrib,
    prop_inv,
    boe_int,
    bus_int,
    loan_prob,
    std_wage,
};

enum class Reason {
    for_benefits,
    for_bonus
};

static enum class Property
{
    hundred,
    bus_size,
    amount_owed,
    bens_paid,
    bonuses,
    prod_bal,
    consumption,
    deficit,
    deficit_pc,
    gov_recpts,
    unbudgeted,
    gov_exp,
    gov_exp_plus,
    dom_bal,
    inc_tax,
    gov_bal,
    num_emps,
    num_firms,
    num_gov_emps,
    num_hired,
    num_fired,
    num_unemps,
    pc_active,
    pc_emps,
    pc_unemps,
    pop_size,           // actually constant
    dedns,
    procurement,
    productivity,
    rel_productivity,
    sales_tax,
    wages,
    zero,
    num_properties


// These seem to have gone missing...
//
//    investment,
//    gdp,
//    profit,

} properties;


class Bank;
class Worker;
class Firm;
class Government;

/******************************************************************************
 *
 * Domain is the main driver class and coordinates all Account activities
 *
 ******************************************************************************/

class Domain : public QObject
{
    Q_OBJECT


    friend class Firm;
    friend class Government;

public:

    static const QMap<ParamType,QString> parameterKeys;

    /*
     * propertyMap has to be initialised explicitly (see initialisePropertyMap)
     * and so cannot be const
     */
    static QMap<QString,Property> propertyMap;

    /*
     * Initialising a static QMap is either clunky or obscure. This is the
     * clunky but effective method. It can be called before the Domain
     * constructor.
     */
    static void initialisePropertyMap();

    /*
     * Restore all listed domains from Settings, storing their pointers in
     * domains amd returning the number of domains restored. Must be public so
     * it can be called from mainwindow
     */
    static int restoreDomains(QStringList &domainNameList);

    /*
     * Create a new domain under the given name, with the given currency and
     * currency abbreviation, and with default values for the rest of its
     * parameters.Must be public as the user wiil have to create domains
     * initially.
     */
    static Domain *createDomain(const QString &name,
                                const QString &currency,
                                const QString &currencyAbbrev);

    /*
     * List of all domains. When a new domain is created or restored it is
     * automatically added to this list.
     */
    static QList<Domain*> domains;

    /*
     * Draw all charts
     */
    static void drawCharts(QListWidget *propertyList);


    /*********************************************************
     *                                                       *
     *    Most of the rest of this can probably be private   *
     *                                                       *
     *********************************************************/

    /*
     * Return a pointer to the domain having the given name, If no such domain
     * exists the returned pointer has the value nullptr.
     */
    static Domain *getDomain(const QString &name);
    //static void editParameters(QListWidget *propertyList);

    void initialise();

    Firm *createFirm(bool state_supported = false);
    Firm *selectRandomFirm(Firm *exclude = nullptr);

    Government *government();

    const QString &getName() const;

    /*
     * Get the current period (iteration)
     */
    //int getPeriod();

    int getPopulation();
    int getActivePop();             // target size of economically active population [active_pop]
    int getNumEmployed();
    int getNumEmployedBy(Firm *firm);
    int getNumUnemployed();


    /*
     * Access functions for domain-specific parameters ([ParamType]). There
     * isn't a one-one correspondence between these functions and the
     * parameters. THIS NEEDS SORTING OUT!
     */

    double getStdWage();            // standard wage (CUs per employee per period) [emp_rate?]
    double getPropCon();            // propensity to consume (%) [prop_con]
    double getProcurement();        // direct government expenditure
    double getTargetEmpRate();      // target rate of employment (%)
    double getDistributionRate();   // funds kept in reserve for next period (%)
    double getIncomeThreshold();    // 100% of threshold is always spent [inc_thresh]
    double getIncTaxRate();         // income tax rate (%) [inc_tax_rate]
    double getSalesTaxRate();       // sales tax rate (%) [sales_tax_rate]
    double getPreTaxDedns();        // pre-tax deductions (%)
    double getFCP();                // firm creaton probability (%) [firm_creation_prob]
    double getUBR();                // unemployment benefit rate (% of std wage) [unemp_ben_rate]
    double getPropInv();            // propensity to invest [prop_inv]

    double getLoanProb();           // Probability of successful loan application
    double getBoeRate();            // BoE lending rate
    double getBusRate();            // retail lending rate
    double getCapexRecoupTime();    // number of periods to recoup capex


    /*
     * This is the main driver function
     */
    void iterate(int period, bool silent = 0);

    Property getProperty(QString propertyName);

    // QList<Property> prop_list;          // to simplify iterations

    double scale(Property p);

    enum class Opr
    {
        invalid_op = -1,
        eq,
        neq,
        lt,
        gt,
        leq,
        geq
    };

    struct Condition
    {
        // If property is set to zero the condition is deemed to
        // apply regardless of the values of opr and val. This will be the
        // case for the default parameter set but would be invalid otherwise.
        Property property = Property::zero;
        Opr opr;
        int val;                // possibly extend to allow expressions later
    };

    struct Pair
    {
        bool is_set = false;
        int val;
    };

    /*
     * The previous version had default and conditional parameters (known as
     * ParameterSets. This has been temporarily suspended so now we just have
     * a single set of parameters, which will have to be defined as a QMap
     * with the ParamType as key. The value is always (I think) an integer.
     */
    QMap<ParamType,int> params;

    enum class Reason {
        for_benefits,
        for_bonus
    };

    bool applies(Condition);
    bool compare(int lhs, int rhs, Opr op);
    int getParameterVal(ParamType type);

    /*
     * Read the parameters for this domain from settings
     */
    void readParameters();

    /*
     * Set the chartview
     */
    void setChartView(QChartView *chartView);

    /*
     * Return the gini coefficient based on the wages of all the workers.
     */
    double getGini();

    /*
     * Internally we assume a population of 1000 workers but for presentation we
     * can scale this up (or down) to any number. 1000 is in effect the
     * 'resolution' or 'granularity' of the system. If granularity is other
     * than 1.000 other properties will have to be scaled for presentation as
     * well. This map determines whether any given property is to be scaled or
     * not.
     */
    QMap<Property, bool> scalable;

    /*
     * Variables to update on each iteration for use in producing stats at the
     * end of the run (epilogue)
     */
    double min_val[static_cast<int>(Property::num_properties)];
    double max_val[static_cast<int>(Property::num_properties)];
    double sum[static_cast<int>(Property::num_properties)];

    /*
     * Retrieve the current (periodic) value associated with a given Property
     */
    double getPropertyVal(Property p);

    /*
     * Choose a bank. Every worker and every firm will have a bank that will
     * be chosen at random, with the exception of clearing banks -- which must
     * choose the government as their bank, and the government itself -- which
     * is its own bank.
     */
    Bank *selectRandomBank();

    // These are the functions that actually interrogate the components of the
    // model to evaluate its properties
    int getNumHired();
    int getNumFired();

    double getProdBal();
    double getWagesPaid();
    double getInvestment();
    double getPurchasesMade();
    double getSalesReceipts();
    double getBonusesPaid();
    double getDednsPaid();
    double getIncTaxPaid();
    double getSalesTaxPaid();
    double getWorkersBal();
    double getAmountOwed();
    double getProcurementExpenditure();
    double getProductivity();

private:

    int last_period = -1;

    QString _name;
    QString _currency;
    QString _abbrev;

    QString _notes; // This isn't used at present

    int _population;

    // TODO: Sort out which properties are cumulative and which are reset on
    // each iteration

    int _startups;
    int _num_hired;     // per iteration
    int _num_fired;     // per itertion
    int _num_firms;
    int _num_emps;
    int _num_unemps;
    int _num_gov_emps;
    int _pop_size;

    double _scale;
    // double _std_wage;
    double _gini;

    /*
     * Dynamic properties
     */
    double  _exp, _bens, _rcpts, _gov_bal, _prod_bal, _wages, _consumption,
            _bonuses, _dedns, _inc_tax, _sales_tax, _dom_bal, _loan_prob,
            _amount_owed, _deficit, _pc_active, _bus_size, _proc_exp,
            _productivity, _rel_productivity, _investment, _gdp, _profit;


    /*
     * This constructor creates a bare-bones domain having the required name,
     * currency and currency abbreviation. If the restore argument has the
     * value TRUE it will then (attempt to) populate the parameters from
     * Settings; otherwise it will simply assign them sensible default values.
     * Default values will also be assigned to any parameters that cannot be
     * restored.
     */
    Domain(const QString &name, const QString &currency,
           const QString &currencyAbbrev);

    QChartView *_chartView;
    QChart *chart;

    /*
     * This gives us a set of pointers to line series, ordered by the Properties
     * they are associated with. E.g., when iterating, the series for bens_paid
     * will be encountered before the series for gov_recpts. Order is
     * significant as it allows us to store values and use them in later
     * composite properties (e,g, deficit).
     */

    // NOTE: I have changed the order of the properties to alphabetical by
    // plain-text name -- possibly not a good idea. The order in which the
    // properties are calculated will need to be reviewed to make sure it still
    // works.
    QMap<Property, QLineSeries*> series;

    /*
     * drawChart just sets up the chart with a title and a set of empty series.
     * It doesn't run the model.
     */
    void drawChart(QListWidget *propertyList);

    void addSeriesToChart();

    //static void run();

    /*
     * Each domain contains a small number of 'clearing banks'. Every worker
     * and firm will have one such bank assigned and payments between them will
     * result in a 'clearing' operation after a given number (TBD) of clock
     * ticks (periods). There should be mechanisms for varying the clearing
     * frequency and the number of banks. This may make no difference at all,
     * but it would be nice to check! Note that the Government class is derived
     * from the Bank class but its only clients are the clearing banks. Unlike
     * clearing banks it never has to borrow funds as it can simply create them
     * as required.
     */
    QList<Bank*> banks;

    /*
     * List of firms. The number of firms at the start will be determined by
     * the user. Subsequently firms will be 'created' and can also go out of
     * existence (i.e. be deleted) if the become insolvent.
     */
    QVector<Firm*> firms;

    /*
     * List of all workers, whether employed or not. For now we will assume the
     * number of workers, unlike firms, will remain unchanged for the duration
     */
    QVector<Worker*> workers;

    /*
     * List of unemployed workers. When fired a worker is removed from the
     * firms employee list and added to this list instead. Firms can
     * recruit employees from this list. Workers are removed from this list
     * when they are employed, but must not be deleted
     */
    QList<Worker*> unemployedWorkers;

    /*
     * When a worker is hired s/he is removed from the unemployedWorkers list
     * and employer is updated. Note that individual firms keep their own lists
     * of employees.
     */
    void hire(Worker *w, Firm *f);

    /*
     * When a worker is fired s/he is added to the unemployedWorkers list and
     * employer is set to nullptr.
     */
    void fire(Worker *w);

     /*
     * The government associated with this domain. It might have been possible
     * to conflate the two classes (Government and Domain) but it seems clearer
     * to keep them separate
     */
    Government *_gov;

signals:

    void domainsRestored();

};


/******************************************************************************
 * Account is the base class for every active entity in the system and provides
 * the basic overrideable functionality.
 ******************************************************************************/

class Account : public QObject
{
    /*
     * For the avoidance of doubt: according to Qt docs and several entries on
     * StackOverflow, the Q_OBJECT macro should be used in every class that
     * derives *directly or indirectly* from QObject. This means that not only
     * Account but also all its subclasses must include Q_OBJECT.
     */

    Q_OBJECT

    friend class Domain;

public:

    Account(Domain *domain);

    /*
     * init() may be called at the start of each period
     */
    virtual void init() {
        last_triggered = -1;
    }

    virtual bool isBank() { return _isBank; }
    virtual bool isGovernment() { return _isGovernment; }

    virtual double getBalance();
    virtual double getAmountOwed();

    /*
     * This function is declared as virtual to allow derived classes to add
     * functionality, e.g. diagnostics
     */
    virtual void credit(double amount, Account *creditor = nullptr,
                        bool force = false);

    virtual void loan(double amount, double rate, Account *creditor);

    /*
     * Every derived class must provide a trigger function, which will be
     * called once per period.
     */
    virtual void trigger(int period) = 0;

    int getId() const;

    static int nextId();

    virtual bool isGovernmentSupported();

    void breakpoint();

protected:

    /*
     * Every acount must belong to a 'domain', which contains the government
     * that issues its currency. Different domains will use different currencies
     * and imports and exports will involve an exchange rate. However, rather
     * than maintaining rates between every pair of currencies we will simply
     * maintain rates relative to a notional underlying currency (NUC). This
     * makes some assumptions about the way the money markets work that may not
     * be true for certain transactions where the actors may have made special
     * arrangements.
     */
    Domain *_domain;

    /*
     * The bank at which this account is held. If this account IS a bank the
     * account is held at the central bank/government. If this is the central
     * bank (i.e. is owned by the government) this entry is a nullptr as it
     * does not itself have a bank. Clearing banks (i.e. banks that are not the
     * central bank) must maintain a separate 'account' called 'reserves' that
     * is used for clearing only. Reserves are HPM and can only be paid into
     * (or from) other banks reserve accounts or the central bank. The central
     * bank does not have to maintain a separate reserve account because it
     * only deals with HPM. (Payments to its employees are made via the
     * emloyees' clearing banks.)
     *
     * It follows that Government is a (and must be derived from) Bank, i.e.
     * the central bank, and does not have (or need) an account at a clearing
     * bank. This is rather confusing but is also Quite Neat.
     */
    Bank *_bank;

    bool _isBank = false;
    bool _isGovernment = false;

    double balance = 0;
    double owed_to_bank = 0;
    double interest_rate = 0;

    int last_triggered = -1;

    virtual bool transferSafely(Account *recipient, double amount,
                                Account *creditor);

private:

    int id;
    static int _id;

signals:

public slots:

};


/******************************************************************************
 * Workers may receive wages, pay taxes, make purchases, take out and repay
 * loans and may be employed or unemployed.
 *****************************************************************************/

class Worker: public Account
{
    Q_OBJECT

    friend class Domain;
    friend class Government;
    friend class Firm;

private:

    int wage;
    Firm *employer;

    // Receipts and payments for current period only. Must be set to zero on
    // each trigger (unless re-triggered in same period) and accumulated
    // throughout period. 'Get' methods must be used to retrieve these values
    // at the end of each period, for statistics.
    double purchases;
    double benefits;
    double wages;
    double inc_tax;

    int period_hired;
    int period_fired;

    double agreed_wage;
    double average_wages;

protected:

    Government *gov;

    void init() override;

    void setEmployer(Firm*);
    void setPeriodHired(int period);

public:

    Worker(Domain *domain);

    Firm *getEmployer();

    bool isEmployed();
    bool isEmployedBy(Account *emp);
    bool isNewHire(int period);

    // Overrides
    void credit(double amount, Account *creditor = nullptr,
                bool force = false) override;
    void trigger(int period) override;

    void epilogue(int period);

    void setAgreedWage(double wage);

    double getWagesReceived();
    double getAverageWages();
    double getBenefitsReceived();
    double getPurchasesMade();
    double getIncTaxPaid();

    double agreedWage();
};

/*
 * Firms may employ workers, pay wages to workers it employs, make sales, take
 * out and repay loans, make investments (purchases that have the effect of
 * increasing productivity).
 */

class Firm: public Account
{
    Q_OBJECT

    friend class Domain;
    friend class Government;

private:

    double wages_paid = 0;
    double bonuses_paid = 0;
    double sales_tax_paid = 0;
    double sales_receipts = 0;
    double investment = 0;

    int num_hired = 0;
    int num_fired = 0;
    int num_just_fired = 0;

    bool _state_supported = false;

    double productivity = 1;
    double _dedns = 0;

protected:

    QList<Worker*> employees;

    void init() override;

public:

    /*
     * RATIONALE
     * ---------
     * We consider a year to be made up of 50 periods and a typical yearly wage
     * to be 25000 -- i.e. 500 per period. Initially we will assume everyone is
     * paid the same amount, but we will need to allow different schemes, such
     * as random with even distribution, random with normal distribution, and
     * random with lognormal distribution. The latter seems to give a fairly
     * convincing 'long-tail' distribution.
     *
     * A distribution based on recorded peercentile figures for the UK in 2015
     * approximates to the polynomial:
     *
     * 0.00001229(x^6) + 0.0033(x^5) + 0.3373(x^4) + 16.066(x^3) + 359
     *
     * where 0 < x < 100 is the percentile. It should be possible to work out a
     * distribution function that would produce this result. The table itself
     * can be found at
     *
     * https://www.gov.uk/government/statistics/percentile-points-from-1-to-99-for-total-income-before-and-after-tax
     *
     * and it might be more straightforward to base wages on that. An
     * alternative approach would be to start off with equal (or evenly
     * distributed) wages and see what develops over time.
     */

    Firm(Domain *domain, bool state_supported = false);

    bool isGovernmentSupported() override;
    void trigger(int period) override;
    void credit(double amount, Account *creditor = nullptr, bool force = false) override;

    Worker *hire(double wage);
    double hireSome(double wage, int number_to_hire);

    void epilogue();

    double payWages();

    double getWagesPaid();
    double getBonusesPaid();
    double getSalesTaxPaid();
    double getSalesReceipts();
    double getInvestment();

    size_t getNumEmployees();

    /*
     * Fire the given worker
     */
    void fire(Worker *w);

    /*
     * Faster version for when the index into list of workers is known
     */
    void fire(int ix);

    int getNumHired();
    int getNumFired();

    double getProductivity();
};

/*
 * A bank may make a loan without having any 'reserves'. However, if it is
 * called upon to make a payment on behalf of a client to a client of another
 * bank it must make that payment in reserves. As long as there is only one
 * bank (in a domain) this cannot arise. For this reason a (more) realistic
 * model is one in which there are several banks.
 */

class Bank: public Firm
{
    Q_OBJECT

    friend class Domain;

public:
    Bank(Domain *domain);

    /*
     * This method overrides the method in the base (Account) class, which
     * prohibits transfers that would leave a negative balance. This
     * restriction doesn't apply to the government, which creates money
     * precisely by creating transfers that leave a negative balance.
     */
    bool transferSafely(Account *recipient, double amount, Account*) override;

    bool isBank() override { return true; }
    void trigger(int period) override;

    void lend(double amount, double rate, Account *recipient);

private:

    /*
     * List of accounts held at this bank
     */
    QList<Account*> accounts;

    double reserves;    // HPM, only used for cleaing

};


/*
 * There is just one instance of the Government class in each domain. Note
 * that Government is derived from Bank since central banking is one of its
 * principal roles.
 */

class Government: public Bank
{
    Q_OBJECT

    friend class Domain;
    friend class Firm;

private:

    /*
     * We no longer distinguish a 'government firm' as Government is now
     * derived from Firm and so has all the necessary functionality. However,
     * some function overrides may be necessary.
     */
    // Firm *_gov_firm;     // (see constructor for assignment to firms)

    double exp, unbudgeted, rec, ben, proc;

protected:

    /*
     * We override the base method here so we can extract the balance for
     * statistics. I'm not sure what this means -- investigate!
     */

    void credit(
            double amount, Account *creditor = nullptr, bool force = false
            ) override;

    void reset();

public:

    Government(Domain *domain);

    bool isGovernment() override { return true; }
    bool transferSafely(Account *recipient, double amount, Account*) override;
    void trigger(int period) override;

    double payWorkers(double amount, Account *source, Reason reason);

    double getExpenditure();   // Gov expenditure in current period (excl benefits)
    double getUnbudgetedExp(); // Gov expenditure on demand from gov_firm
    double getBenefitsPaid();  // Benefits paid this period
    double getReceipts();      // Gov receipts (taxes and dedns) in current period
    double getProcExp();       // Procurement expenditure

    double debit(Account *requester, double amount);

};

#endif // ACCOUNT_H
