#include "account.h"
#include <QDebug>

void Government::init()
{
    exp = 0;
    unbudgeted = 0;
    rec = 0;
    ben = 0;
    proc = 0;
    last_triggered = -1;
}

void Government::reset()
{
    init();
    balance = 0;
    _gov_firm = behaviour()->createFirm(true);
}

Government::Government(Behaviour *model) : Account(model)
{
    // The 'true' argument tells the model that this is a (the) government-
    // supported firm and that it should have a preset (user-defined) number
    //of employees.
    _gov_firm = model->createFirm(true);
}

size_t Government::getNumEmployees()
{
    return _gov_firm->getNumEmployees();
}

double Government::getExpenditure()
{
    return exp;
}

double Government::getUnbudgetedExp()
{
    return unbudgeted;
}

double Government::getBenefitsPaid()
{
    return ben;
}

double Government::getReceipts()
{
    return rec;
}

double Government::getProcExp()
{
    return proc;
}

double Government::debit(Account *requester, double amount)
{
    // If this is called by a non-govt-supported firm the program will abort
    Q_ASSERT(requester->isGovernmentSupported());

    balance -= amount;
    exp += amount;          // include direct unbudgeted support as expenditure.
    unbudgeted += amount;

    return amount;
}

bool Government::isGovernment()
{
    return true;
}

Firm *Government::gov_firm()
{
    return _gov_firm;
}

void Government::trigger(int period)
{
    // ***
    // This code originally allowed for government-supported industries to
    // receive a grant that was designed to cover the expected number of
    // employees. This has now been superseded and government-supported
    // industries are now allocated a user-defined number of employees and can
    // claim the necessary funds to pay them as and when required. However, it
    // makes sense to retain the grant mechanism in case it is needed in
    // future. Change and uncomment as required...
    // ***

    /* Government grants (incl support of gov-owned businesses)
    QSettings settings;
    int amt = settings.value("government-employees").toInt() * model()->getStdWage();
    _gov_firm->grant(amt);
    balance -= amt;
    exp += amt;
    */

    if (period > last_triggered)    // to prevent double counting
    {
        last_triggered = period;

        // Direct purchases (procurement), adjusts balance automatically
        double amt = behaviour()->getProcurement();
        transferSafely(behaviour()->selectRandomFirm(), amt, this, true);
        proc += amt;
        exp += amt;     // include in expenditure not as a separate item as
                        // less confusing

        // Benefits payments to all unemployed workers (doesn't adjust balance,
        // so we must do this on return)
        ben += behaviour()->payWorkers(behaviour()->getStdWage() * behaviour()->getUBR(),
                               this,                // source
                               Behaviour::for_benefits  // reason
                               );
        balance -= ben;
    }
}

//
// This function is an alternative to Account::transferTo allowing a negative balance.
//
bool Government::transferSafely(Account *recipient, double amount, Account *, bool procurement)
{
    // ***
    // We adopt the convention that receipts from the government are not
    // taxable. This is probably a rather murky area, given that the
    // mechanisms by which government injects money into the economy are
    // unclear and seem to involve financing banks to make more loans.
    // In the case of the NHS, nationalised industries, the civil service
    // and the 'armed forces' the mechanism is probably more direct.
    // Anyway, to go into this would be a distraction so we'll simply
    // treat it as untaxable payment for services.
    // ***

    if (recipient != nullptr) {
        recipient->credit(amount, this, procurement);
        balance -= amount;
    }

    // The government can always transfer any amount
    return true;
}

// All credits to the Government are (at present) regarded as tax. This
// means that while they offset the deficit we need to keep a separate
// record as well. However we don't distinguish between income tax, sales
// tax, and 'pre-tax deductions'. These are all accounted for elsewhere.
// Obviously, the government doesn't pay tax.
void Government::credit(double amount, Account*, bool)
{
    Account::credit(amount);
    rec += amount;
}



