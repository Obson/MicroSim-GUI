#ifndef MODEL_H
#define MODEL_H

#include <iostream>
#include <map>
#include <QObject>
#include <QVector>
#include <QtCharts/QLineSeries>
#include <QSettings>
#include <QMap>

class Account;
class Government;
class Worker;
class Firm;
class Bank;

QT_CHARTS_USE_NAMESPACE

// Only potentially conditional parameters are listed here
enum class ParamType {
    procurement,
    emp_rate,
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
};

class Model : public QObject
{
    Q_OBJECT

public:

    static Model *createModel(QString name);

    static int loadAllModels();

    // The list of all models. When a new model is created it is automatically
    // added to this list.
    static QList<Model*> models;

    // Get the model having the given name, returning a pointer to it, and set
    // it as current. Requires loadAllModels() to have been called (which loads
    // all the models into QList<Model*> models).
    static Model *model(QString name);

    static Model *current;

    QString name();

    void run(bool randomised = false);
    void restart();

    int getIters();         // number of iterations (periods)
    int getStartPeriod();

    Government *gov();
    Bank *bank();

    int period();

    int min_value(int);
    int max_value(int);
    int total(int);

    Firm *createFirm(bool state_supported = false);
    Firm *selectRandomFirm(Firm *exclude = nullptr);

    enum class Status {any, employed, unemployed};

    struct HireData {
        int num;
        int wages_due;
    } hire_data;

    double hireSome(Firm *employer, double wage, int period, int number_to_hire);

    Worker *hire(Firm *employer, double wage, int period);

    void fire(Worker *w, int period);

    enum Reason {
        for_benefits,
        for_bonus
    };

    int getWageBill(Firm *employer, bool include_dedns = false);

    int payWages(Firm *payer, int period);

    bool randomCheck(int chances, int in);

    double payWorkers(double amount, Account *source, Reason reason);

    // ------------------------------------------------------------------------
    // Properties
    // ------------------------------------------------------------------------

    enum class Property
    {
        current_period,
        pop_size,           // although constant, we treat this as a property
                            // as it's sometimes useful to show it on a graph
        gov_exp,
        bens_paid,
        gov_exp_plus,
        gov_recpts,
        deficit,
        deficit_pc,
        gov_bal,
        num_firms,
        num_emps,
        pc_emps,
        num_unemps,
        pc_unemps,
        pc_active,
        num_gov_emps,
        num_hired,
        num_fired,
        prod_bal,
        wages,
        consumption,
        bonuses,
        dedns,
        inc_tax,
        sales_tax,
        dom_bal,
        amount_owed,
        bus_size,
        hundred,
        zero,
        procurement,
        productivity,
        rel_productivity,
        unbudgeted,
        gov_bal_pc,
        num_properties
    };

    Property getProperty(int);          // return the property with the given value
    QList<Property> prop_list;          // to simplify iterations

    // This gives us a set of pointers to line series, ordered by the Properties
    // they are associated with. E.g., when iterating the series for bens_paid
    // will be encountered before the series for gov_recpts. Order is
    // significant as it allows us to store values and use them in later
    // composite properties (e,g, deficit).
    QMap<Property, QLineSeries*> series;
    QMap<Property, bool> scalable;

    int _num_properties = static_cast<int>(Property::num_properties);

    int min_val[static_cast<int>(Property::num_properties)];
    int max_val[static_cast<int>(Property::num_properties)];
    int sum[static_cast<int>(Property::num_properties)];

    // Retrieve the current (periodic) value associated with a given Property
    double getPropertyVal(Property p);

    // These are the functions that actually interrogate the components of the
    // model to evaluate its propertis
    int getNumEmployedBy(Firm *firm);
    int getNumEmployed();
    int getNumUnemployed();                 // were employed but now unemployed
    int getNumJustFired();
    int getNumHired();
    int getNumFired();

    double getProdBal();
    double getWagesPaid();
    double getPurchasesMade();
    double getSalesReceipts();
    double getBonusesPaid();
    double getDednsPaid();
    double getIncTaxPaid();
    double getSalesTaxPaid();
    double getWorkersBal(Model::Status status);
    double getAmountOwed();
    double getProcurementExpenditure();

    int population();

    // ------------------------------------------------------------------------
    // Parameters
    // ------------------------------------------------------------------------

    int getActivePop();             // proportion of population that is economically active

    double getProcurement();        //direct government expenditure
    double getTargetEmpRate();      // target rate of employment (%)
    double getStdWage();            // standard wage (currency units per employee per period)
    double getPropCon();            // propensity to consume (%)
    double getIncTaxRate();         // income tax rate (%)
    double getIncomeThreshold();    // 100% of threshold is spent regardless of prop con
    double getSalesTaxRate();       // sales tax rate (%)
    double getPreTaxDedns();        // pre-tax deductions (%)
    double getCapexRecoupTime();    // number of periods to recoup capex
    double getFCP();                // firm creaton probability (%)
    double getUBR();                // unemployment benefit rate (% of std wage)
    double getPropInv();            // propensity to invest
    double getDistributionRate();   // funds kept in reserve for next period (%)

    double getBoeRate();            // BoE lending rate
    double getBusRate();            // retail lending rate

    double getLoanProb();

    double getGini();
    double getProductivity();

    static int getId();

    int num_hired;
    int num_fired;
    int num_just_fired;

private:

    QVector<Firm*> firms;
    QVector<Worker*> workers;

    // We only need one bank
    // QVector<Bank*> banks;

    int _period;
    Government *_gov;
    Bank *_bank;

    double _gini;

    // Constants

    QString _name;
    QString _notes;

    int _iterations;
    int _population;
    int _startups;
    int _first_period;

    double _scale;

    double _std_wage;

    // See getPropertyValue
    int     _num_firms, _num_emps, _num_unemps, _num_gov_emps, _num_hired,
            _num_fired, _pop_size;

    double  _exp, _bens, _rcpts, _gov_bal, _prod_bal, _wages, _consumption,
            _bonuses, _dedns, _inc_tax, _sales_tax, _dom_bal, _loan_prob,
            _amount_owed, _deficit, _pc_active, _bus_size, _proc_exp,
            _productivity, _rel_productivity;

protected:

    Model(QString model_name);
    double scale(Property p);

    double gini();
    double productivity();

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

    struct Params
    {
        Condition condition;
        Pair procurement;
        Pair iters;
        Pair count;
        Pair emp_rate;
        Pair prop_con;
        Pair inc_tax_rate;
        Pair inc_thresh;
        Pair sales_tax_rate;
        Pair firm_creation_prob;
        Pair dedns;
        Pair unemp_ben_rate;
        Pair active_pop;
        Pair distrib;
        Pair prop_inv;
        Pair boe_int;
        Pair bus_int;
        Pair loan_prob;
        Pair recoup;
        Pair invalid;           // Just a marker -- value immaterial
    };

    bool applies(Condition);
    bool compare(int lhs, int rhs, Opr op);

    QVector<Params*> parameter_sets;

    int num_parameter_sets;

    static QMap<ParamType,QString> parameter_keys;

    Params *default_parameters;

    int getParameterVal(ParamType type);
    bool isParamSet(ParamType t, int n);

    void readParameters();

};

#endif // MODEL_H
