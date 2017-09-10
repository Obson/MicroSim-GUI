#ifndef ACCOUNT_H
#define ACCOUNT_H

#include <QObject>
#include <model.h>
#include <iostream>
#include <list>

// Account is the base class for every active entity in the system and provides
// the basic overrideable functionality.

class Statistics;
class Firm;
class Government;

class Model;

class Account : public QObject
{
    // For the avoidance of doubt: according to Qt docs and several entries on
    // StackOverflow, the Q_OBJECT macro should be used in every class that
    // derives *directly or indirectly* from QObject. This means that not only
    // Account but also all its subclasses must include Q_OBJECT.

    Q_OBJECT

public:
    Account(Model *model);

    Model *model();             // returns model that 'owns' this account

    virtual int getBalance();
    virtual int getAmountOwed();

    // This function is declared as virtual to allow derived class to add
    // functionality, e.g. diagnostics
    virtual void credit(int amount, Account *creditor = nullptr, bool force = false);

    virtual void loan(int amount, int rate, Account *creditor);

    // Every derived class must provide a trigger function, which will be
    // called once per period.
    virtual void trigger(int period) = 0;

    int getId() const;

    static int nextId();

    virtual bool isGovernmentSupported();
    virtual bool isGovernment();

    void breakpoint();

protected:

    Model *_model;

    // We can't have an instance of Government as a member of Account because
    // Government is derived from Account
    // Government *gov;

    int balance = 0;
    int owed_to_bank = 0;
    int interest_rate = 0;
    int last_triggered = -1;

    // If this account is a bank, the bank loan refers to its account at the
    // central bank. The central bank, in turn, has an account at the treasury,
    // but this will not be modelled at present -- banks will simply draw on
    // funds from the government, and will repay them at an interest rate that
    // is (normally) lower than that charged to its customers. The difference
    // will be distributed to workers designated as staff and shareholders of
    // the bank.

    bool transferSafely(Account *recipient, int amount, Account *creditor);

private:

    int id;
    static int _id;

signals:

public slots:

};

class Worker: public Account
{
    Q_OBJECT

    friend class Government;
    friend class Firm;
    friend class Model;

private:

    int wage;
    Firm *employer;

    // Receipts and payments for current period only. Must be set to zero on
    // each trigger (unless re-triggered in same period) and accumulated
    // throughout period. 'Get' methods must be used to retrieve these values
    // at the end of each period, for statistics.
    int purchases;
    int benefits;
    int wages;
    int inc_tax;
    int period_hired;
    int period_fired;

    int agreed_wage;

protected:

    Government *gov;

    void init();

    void setEmployer(Firm*);
    void setPeriodHired(int period);

public:

    Worker(Model *model);
    //Worker(int wage, Firm *emp, int period, QObject *parent = nullptr);

    Firm *getEmployer();
    bool isEmployed();
    bool isEmployedBy(Account *emp);
    bool isNewHire(int period);

    // Overrides
    void credit(int amount, Account *creditor = nullptr, bool force = false);
    void trigger(int period);

    void setAgreedWage(int wage);

    int getWagesReceived();
    int getBenefitsReceived();
    int getPurchasesMade();
    int getIncTaxPaid();

    int agreedWage();
};

class Firm: public Account
{
    Q_OBJECT

    friend class Government;
    friend class Model;

private:

    int amount_granted = 0;
    int wages_paid = 0;
    int bonuses_paid = 0;
    int sales_tax_paid = 0;
    int sales_receipts = 0;
    int dedns_paid = 0;
    int num_hired = 0;
    int num_fired = 0;

    //int wage_bill = 0;              // current cost of wages and deductions

    bool _state_supported = false;

protected:

    Government *gov;

    // Only government can make a grant, so this should be protected
    // and the Government class made a friend class
    void grant(int amount);

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

    Firm(Model *model, bool state_supported = false);

    bool isGovernmentSupported();

    void trigger(int period);
    void epilogue(int period);

    // Overrides base mmethod to give additional functionality
    void credit(int amount, Account *creditor = nullptr, bool force = false);

    int getAmountGranted();
    int getWagesPaid();
    int getBonusesPaid();
    int getSalesTaxPaid();
    int getSalesReceipts();
    int getDednsPaid();

    size_t getNumEmployees();

    int getNumHired();
    int getNumFired();
};

// There is just one instance of the Government class in each model, as we are
// currently assuming a closed economy wth a single currency. Foreign trade
// would require firms that were registered to a different Government -- This
// should be added later.

class Government: public Account
{
    Q_OBJECT

    friend class Model;

private:

    Firm *_gov_firm;     // (see constructor for assignment to firms)

    int exp, rec, ben, proc;

protected:

    // This method overrides the method in the base (Account) class, which
    // prohibits transfers that would leave a negative balance. This
    // restriction doesn't apply to the government, which creates money
    // precisely by creating transfers that leave a negative balance.
    bool transferSafely(Account *recipient, int amount, Account*, bool procurement = false);

    // We override the base method here just so we can extract the balance for
    // statistics.
    void credit(int amount, Account *creditor = nullptr, bool force = false);

    void init();
    void reset();

public:

    Government(Model *model);

    bool isGovernment();

    Firm *gov_firm();

    void trigger(int period);

    int getExpenditure();   // Gov expenditure in current period (excl benefits)
    int getBenefitsPaid();  // Benefits paid this period
    int getReceipts();      // Gov receipts (taxes and dedns) in current period

    int debit(Account *requester, int amount);

    size_t getNumEmployees();  // Number of government employees
};


class Bank: public Account
{
    Q_OBJECT

    friend class Model;

public:
    Bank(Model *model);

    void lend(int amount, int rate, Account *recipient);
    void trigger(int period);
};


#endif // ACCOUNT_H
