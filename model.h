#ifndef MODEL_H
#define MODEL_H

//#include <stdio.h>
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

enum class ParamType {
    pop_size,
    iters,
    emp_rate,
    std_wage,
    prop_con,
    inc_tax_rate,
    sales_tax_rate,
    firm_creation_prob,
    dedns,
    unemp_ben_rate,
    active_pop,
    reserve,
    prop_inv,
    boe_int,
    bus_int,
    loan_prob
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

    Government *gov();
    // Bank *bank();

    int period();

    Firm *createFirm();
    Firm *selectRandomFirm();

    enum class Status {any, employed, unemployed};

    int hireSome(Firm *employer, int period, int number_to_hire);

    Worker *hire(Firm *employer, int period);

    void fire(Worker *w, int period);

    enum Reason {
        for_wages,
        for_benefits,
        for_bonus
    };

    // The value of the period argument is immaterial unless the reason
    // argument is for_wages
    int payWorkers(int amount, int max_tot, Account *source,
                   Reason reason = for_wages, int period = 0);

    // ------------------------------------------------------------------------
    // Properties
    // ------------------------------------------------------------------------

    enum class Property
    {
        current_period,
        pop_size,
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
        zero,
        num_properties
    };

    QList<Property> prop_list;          // to simplify iterations

    // This gives us a set of pointers to line series, ordered by the Properties
    // they are associated with. E.g., when iterating the series for bens_paid
    // will be encountered before the series for gov_recpts. Order is
    // significant as it allows us to store values and use them in later
    // composite properties (e,g, deficit).
    QMap<Property, QLineSeries*> series;

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

    // ------------------------------------------------------------------------
    // Parameters
    // ------------------------------------------------------------------------

    int getIters();         // number of iterations (periods)
    int getPopSize();       // max available population size
    int getActivePop();     // proportion of population that is economically active
    int getGovExpRate();    // government expenditure (currency units per period)
    int getTargetEmpRate(); // target rate of employment (%)
    int getStdWage();       // standard wage (currency units per employee per period)
    int getPropCon();       // propensity to consume (%)
    int getIncTaxRate();    // income tax rate (%)
    int getSalesTaxRate();  // sales tax rate (%)
    int getPreTaxDedns();   // pre-tax deductions (%)
    int getFCP();           // firm creaton probability (%)
    int getUBR();           // unemployment benefit rate (% of std wage)
    int getPropInv();       // propensity to invest
    int getReserve();       // funds kept in reserve for next period (%)

    int getBoeRate();       // BoE lending rate
    int getBusRate();       // retail lending rate

    int getLoanProb();

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

    // Constants

    QString _name;
    QString _notes;

    int _iterations;
    int _population;
    int _gov_exp_rate = -1;

    // See getPropertyValue
    int _exp, _bens, _rcpts, _gov_bal, _num_firms, _num_emps, _num_unemps,
    _num_gov_emps, _num_hired, _num_fired, _prod_bal, _wages, _consumption,
    _bonuses, _dedns, _inc_tax, _sales_tax, _dom_bal, _deficit, _pop_size,
    _loan_prob;

protected:

    Model(QString model_name);

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
        Pair pop_size;
        Pair iters;
        Pair count;
        Pair emp_rate;
        Pair std_wage;
        Pair prop_con;
        Pair inc_tax_rate;
        Pair sales_tax_rate;
        Pair firm_creation_prob;
        Pair dedns;
        Pair unemp_ben_rate;
        Pair active_pop;
        Pair reserve;
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
