#ifndef DOMAIN_H
#define DOMAIN_H

//#include "mainwindow.h"
//#include "behaviour.h"

// #include "account.h"


#include <iostream>
#include <map>
#include <QObject>
#include <QVector>
#include <QtCharts/QLineSeries>
#include <QSettings>
#include <QMap>

// NEXT: This class should contain the main driver code to trigger its
// components in response to a clock maintained by MainWindow. Also, allow
// clock tick frequency to be set as a global option so we can put time
// durations on the x-axes of the charts. Suggested default: one tick per week.

QT_CHARTS_USE_NAMESPACE

/*
class Account;
class Government;
class Worker;
class Firm;
class Bank;
*/

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

const static QMap<ParamType,QString> parameterKeys
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

class Domain : public QObject
{
    Q_OBJECT

public:

    static Domain *createDomain(const QString &name, const QString &currency, const QString &currencyAbbrev);

    // List of all domains. When a new domain is created it is automatically
    // added to this list.
    static QList<Domain*> domains;

    // Get the domain having the given name, returning a pointer to it, and
    // set it as current. Requires loadAllDomains() to have been called
    // (which loads all the Domains into QList<Domain*> domains).
    static Domain *getDomain(const QString &name);
    static Domain *currentDomain;

    /*
    inline static Domain *getDefaultDomain() {
        return defaultDomain;
    }
    */

    void initialise();

    Firm *createFirm(bool state_supported = false);
    Firm *selectRandomFirm(Firm *exclude = nullptr);

    Bank *getCentral_bank() const;
    const QString &getName() const;
    int getPopulation();

    int getPopSize();       // max available population size
    // int getActivePop();     // target size of economically active population
    // int getGovExpRate();    // government expenditure (currency units per period)

    int getStdWage();       // standard wage (currency units per employee per period)
    int getPropCon();       // propensity to consume (%)
    int getIncTaxRate();    // income tax rate (%)
    int getSalesTaxRate();  // sales tax rate (%)
    int getFCP();           // firm creaton probability (%)
    int getUBR();           // unemployment benefit rate (% of std wage)
    int getPropInv();       // propensity to invest
    int getReserve();       // funds kept in reserve for next period (%)


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
        investment,
        gdp,
        profit,
        num_properties
    };

    Property getProperty(int);          // return the property with the given value
    QList<Property> prop_list;          // to simplify iterations
    double scale(Property p);
    double gini();

    // This gives us a set of pointers to line series, ordered by the Properties
    // they are associated with. E.g., when iterating, the series for bens_paid
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


private:

    // A domain should create its own (anonymous) behaviour, including currency
    // and currency abbreviation
    Domain(const QString &name, const QString &currency, const QString &currencyAbbrev);

    QString _name;
    QString _currency;
    QString _abbrev;

    // This class is invoked (directly or indirectly) from MainWindow so it
    // cannot itself the MainWindow object
    // MainWindow *_mainwindow;

    int _population;

    // One of these is redundant!
    Bank *_centralBank;
    Bank *_bank;

    Government *_gov;

    QVector<Firm*> firms;
    QVector<Worker*> workers;


    double _gini;

    // Constants

    QString _notes; // This isn't used at present

    int _startups;

    double _scale;
    double _std_wage;

    // See getPropertyValue
    int     _num_firms, _num_emps, _num_unemps, _num_gov_emps, _num_hired,
            _num_fired, _pop_size;

    double  _exp, _bens, _rcpts, _gov_bal, _prod_bal, _wages, _consumption,
            _bonuses, _dedns, _inc_tax, _sales_tax, _dom_bal, _loan_prob,
            _amount_owed, _deficit, _pc_active, _bus_size, _proc_exp,
            _productivity, _rel_productivity, _investment, _gdp, _profit;


public slots:

    void iterate(int _period, bool visible = false);

signals:


};



#endif // DOMAIN_H
