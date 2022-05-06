#ifndef ACCOUNT_H
#define ACCOUNT_H

#include "behaviour.h"

#include <QObject>
#include <iostream>
#include <list>
#include <QList>

// Account is the base class for every active entity in the system and provides
// the basic overrideable functionality.

class Statistics;
class Firm;
class Government;
class Domain;

class Account : public QObject
{
    // For the avoidance of doubt: according to Qt docs and several entries on
    // StackOverflow, the Q_OBJECT macro should be used in every class that
    // derives *directly or indirectly* from QObject. This means that not only
    // Account but also all its subclasses must include Q_OBJECT.

    Q_OBJECT

public:

    // NOTE: *** Account is no longer owned by a Behaviour but vice versa ***
    Account(Behaviour *behaviour);

    Behaviour *behaviour();             // returns model that 'owns' this account

    virtual double getBalance();
    virtual double getAmountOwed();

    // This function is declared as virtual to allow derived class to add
    // functionality, e.g. diagnostics
    virtual void credit(double amount, Account *creditor = nullptr, bool force = false);

    virtual void loan(double amount, double rate, Account *creditor);

    // Every derived class must provide a trigger function, which will be
    // called once per period.
    virtual void trigger(int period) = 0;

    int getId() const;

    static int nextId();

    virtual bool isGovernmentSupported();
    virtual bool isGovernment();

    void breakpoint();

protected:

    Behaviour *_behaviour;

    // We can't have an instance of Government as a member of Account because
    // Government is derived from Account
    // Government *gov;

    double balance = 0;
    double owed_to_bank = 0;
    double interest_rate = 0;

    int last_triggered = -1;

    // If this account is a bank, the bank loan refers to its account at the
    // central bank. The central bank, in turn, has an account at the treasury,
    // but this will not be modelled at present -- banks will simply draw on
    // funds from the government, and will repay them at an interest rate that
    // is (normally) lower than that charged to its customers. The difference
    // will be distributed to workers designated as staff and shareholders of
    // the bank.

    bool transferSafely(Account *recipient, double amount, Account *creditor);

private:

    int id;
    static int _id;

    // FOREIGN TRADE
    // -------------
    // Added 2 May 2022
    //
    // Every acount must belong to a 'domain', which is in effect the
    // government that issues its currency. A government's domain is itself if
    // it is sovereign. Different domains will use different currencies and
    // imports and exports will involve an exchange rate. However, rather than
    // maintaining rates between every pair of currencies we will simply
    // maintain rates relative to a notional underlying currency (NUC). This
    // makes some assumptions about the way the money markets work that may not
    // be true for certain transactions where the actors may have made special
    // arrangements.

    // NEXT: Fix Model so it isn't limited to one Government instance
    Domain *domain;

signals:

public slots:

};

class Worker: public Account
{
    Q_OBJECT

    friend class Government;
    friend class Firm;
    friend class Behaviour;

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

    void init();

    void setEmployer(Firm*);
    void setPeriodHired(int period);

public:

    Worker(Behaviour *behaviour);

    Firm *getEmployer();

    bool isEmployed();
    bool isEmployedBy(Account *emp);
    bool isNewHire(int period);

    // Overrides
    void credit(double amount, Account *creditor = nullptr, bool force = false) override;
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

class Firm: public Account
{
    Q_OBJECT

    friend class Government;
    friend class Behaviour;

private:

    double wages_paid = 0;
    double bonuses_paid = 0;
    double sales_tax_paid = 0;
    double sales_receipts = 0;
    double investment = 0;

    int num_hired = 0;
    int num_fired = 0;

    bool _state_supported = false;

    double productivity = 1;

protected:

    Government *gov;

    void init();

public:

    // RATIONALE
    // ---------
    // We consider a year to be made up of 50 periods and a typical yearly wage
    // to be 25000 -- i.e. 500 per period. Initially we will assume everyone is
    // paid the same amount, but we will need to allow different schemes, such
    // as random with even distribution, random with normal distribution, and
    // random with lognormal distribution. The latter seems to give a fairly
    // convincing 'long-tail' distribution.
    //
    // A distribution based on recorded peercentile figures for the UK in 2015
    // approximates to the polynomial:
    //
    // 0.00001229(x^6) + 0.0033(x^5) + 0.3373(x^4) + 16.066(x^3) + 359
    //
    // where 0 < x < 100 is the percentile. It should be possible to work out a
    // distribution function that would produce this result. The table itself
    // can be found at
    //
    // https://www.gov.uk/government/statistics/percentile-points-from-1-to-99-for-total-income-before-and-after-tax
    //
    // and it might be more straightforward to base wages on that. An
    // alternative approach would be to start off with equal (or evenly
    // distributed) wages and see what develops over time.

    Firm(Behaviour *behaviour, bool state_supported = false);

    bool isGovernmentSupported() override;
    void trigger(int period) override;
    void credit(double amount, Account *creditor = nullptr, bool force = false) override;

    void epilogue(int period);

    double getWagesPaid();
    double getBonusesPaid();
    double getSalesTaxPaid();
    double getSalesReceipts();
    double getInvestment();

    size_t getNumEmployees();

    int getNumHired();
    int getNumFired();

    double getProductivity();
};

// There is just one instance of the Government class in each domain
class Government: public Account
{
    Q_OBJECT

    friend class Behaviour;

private:

    Firm *_gov_firm;     // (see constructor for assignment to firms)

    double exp, unbudgeted, rec, ben, proc;

protected:

    // This method overrides the method in the base (Account) class, which
    // prohibits transfers that would leave a negative balance. This
    // restriction doesn't apply to the government, which creates money
    // precisely by creating transfers that leave a negative balance.
    bool transferSafely(Account *recipient, double amount, Account*, bool procurement = false);

    // We override the base method here just so we can extract the balance for
    // statistics.
    void credit(double amount, Account *creditor = nullptr, bool force = false);

    void init();
    void reset();

public:

    Government(Behaviour *model);

    bool isGovernment();

    Firm *gov_firm();

    void trigger(int period);

    double getExpenditure();   // Gov expenditure in current period (excl benefits)
    double getUnbudgetedExp(); // Gov expenditure on demand from gov_firm
    double getBenefitsPaid();  // Benefits paid this period
    double getReceipts();      // Gov receipts (taxes and dedns) in current period
    double getProcExp();       // Procurement expenditure

    double debit(Account *requester, double amount);

    size_t getNumEmployees();  // Number of government employees
};


class Bank: public Account
{
    Q_OBJECT

    friend class Behaviour;

public:
    Bank(Behaviour *behaviour);

    void lend(double amount, double rate, Account *recipient);
    void trigger(int period);
};


#endif // ACCOUNT_H
