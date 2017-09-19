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
    sales_tax_rate,
    firm_creation_prob,
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

    void run();
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

    int hireSome(Firm *employer, int wage, int period, int number_to_hire);

    Worker *hire(Firm *employer, int wage, int period);

    void fire(Worker *w, int period);

    enum Reason {
        for_benefits,
        for_bonus
    };

    int getWageBill(Firm *employer, bool include_dedns = false);

    int payWages(Firm *payer, int period);

    bool randomCheck(int chances, int in);

    int payWorkers(int amount, int max_tot, Account *source, Reason reason);

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
        num_properties
    };

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
    int getPropertyVal(Property p);

    // These are the functions that actually interrogate the components of the
    // model to evaluate its propertis
    int getNumEmployedBy(Firm *firm);
    int getNumEmployed();
    int getNumUnemployed();                 // were employed but now unemployed
    int getNumJustFired();
    int getNumHired();
    int getNumFired();
    int getProdBal();
    int getWagesPaid();
    int getPurchasesMade();
    int getSalesReceipts();
    int getBonusesPaid();
    int getDednsPaid();
    int getIncTaxPaid();
    int getSalesTaxPaid();
    int getWorkersBal(Model::Status status);
    int population();
    int getAmountOwed();
    int getProcurementExpenditure();

    // ------------------------------------------------------------------------
    // Parameters
    // ------------------------------------------------------------------------

    int getActivePop();     // proportion of population that is economically active
    int getProcurement();   //direct government expenditure
    int getTargetEmpRate(); // target rate of employment (%)
    int getStdWage();       // standard wage (currency units per employee per period)
    int getPropCon();       // propensity to consume (%)
    int getIncTaxRate();    // income tax rate (%)
    int getSalesTaxRate();  // sales tax rate (%)
    int getPreTaxDedns();   // pre-tax deductions (%)
    int getFCP();           // firm creaton probability (%)
    int getUBR();           // unemployment benefit rate (% of std wage)
    int getPropInv();       // propensity to invest
    int getDistributionRate();       // funds kept in reserve for next period (%)

    int getBoeRate();       // BoE lending rate
    int getBusRate();       // retail lending rate

    int getLoanProb();

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
    int _scale;
    int _std_wage;

    // See getPropertyValue
    int _exp, _bens, _rcpts, _gov_bal, _num_firms, _num_emps, _num_unemps,
    _num_gov_emps, _num_hired, _num_fired, _prod_bal, _wages, _consumption,
    _bonuses, _dedns, _inc_tax, _sales_tax, _dom_bal, _deficit, _pop_size,
    _loan_prob, _amount_owed, _bus_size, _pc_active, _proc_exp;

    double _productivity;

protected:

    Model(QString model_name);
    int scale(Property p);

    double gini();
    double productivity();

    enum class Opr
    {
        invalid_op,
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
        Pair procurement;
        Pair iters;
        Pair count;
        Pair emp_rate;
        Pair prop_con;
        Pair inc_tax_rate;
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
        Pair invalid;           // Just a marker -- value immaterial
    };

    bool applies(Condition *);
    bool compare(int lhs, int rhs, Opr op);

    // Conditional parameters
    QVector<Condition*> conditions;
    QVector<Params*> parameter_sets;

    static QMap<ParamType,QString> parameter_keys;

    // Default parameters
    Params *default_parameters;

    //int loadDefaultParameter(QSettings &settings, ParamType p);
    int getParameterVal(ParamType type);

    void readDefaultParameters();

};

#endif // MODEL_H
