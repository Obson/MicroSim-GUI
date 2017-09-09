#include "account.h"

void Government::init()
{
    exp = 0;
    rec = 0;
    ben = 0;
}

void Government::reset()
{
    init();
    balance = 0;
    _gov_firm = model()->createFirm(true);
}

Government::Government(Model *model) : Account(model)
{
    // This must be the first Firm created, as this ensures that government
    // money is always spent at the start of the period.
    _gov_firm = model->createFirm(true);
}

// TO DO NEXT. OR AT LEAST VERY SOON
// ---------------------------------
// This method creates and registers a new independent firm with, at present,
// no visible means of support. To fix this we will need to provide a means
// of accessing government funds. Conventionally this is achieved via banks,
// so what we probably need to do is implement the Bank class and set the
// new firm up with a 'line of credit'. I think this means that the firm can
// keep borrowing more money as long as it keeps up the repayments on the
// existing loan and pays the interest due. I suspect this is inherently
// unstable and is only sustainable as long as the firm continues to grow.
// Eventually the market becomes saturated and the firm dies -- or possibly
// it just sucks in business from other less successful firms in the same
// field, and they die.

size_t Government::getNumEmployees()
{
    return _gov_firm->getNumEmployees();
}

int Government::getExpenditure()
{
    return exp;
}

int Government::getBenefitsPaid()
{
    return ben;
}

int Government::getReceipts()
{
    return rec;
}

int Government::debit(Account *requester, int amount)
{
    // If this is called by a non-govt-supported firm the program will abort
    Q_ASSERT(requester->isGovernmentSupported());

    balance -= amount;
    exp += amount;          // include direct unbudgeted support as expenditure.
                            // TODO: Keep account of this separately
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
    // Government grants (incl support of gov-owned businesses)

    // This should probably be divided between support of govt industries
    // (gov_firm) and direct govt purchasing. Gap-filling by gov_firm (if
    // necessary) should be counted as 'unbudgeted govt expenditure'
    QSettings settings;

    // Direct government support
    /*
    int amt = settings.value("government-employees").toInt() * model()->getStdWage();
    _gov_firm->grant(amt);
    balance -= amt;
    exp += amt;
    */

    // TODO: currently gov_firm recruits dynamically according to availability
    // of funds (and other criteria). Should start off with designated number
    // of employees and request funds as equired. This may mean we can dispense
    // with the 'grant' mechanism,although it's probably worth keeping in place
    // in case it's needed for oter purposes.

    // Pay benefits to all unemployed workers

    ben += model()->payWorkers((model()->getStdWage() * model()->getUBR()) / 100,
                           0,                   // no max amount
                           this,                // source
                           Model::for_benefits // reason
                           );

    balance -= ben;
}

//
// This function overrides Account::transferTo to allow a negative balance.
//
bool Government::transferSafely(Account *recipient, int amount, Account *)
{
    // We adopt the convention that receipts from the government are not
    // taxable. This is probably a rather murky area, given that the
    // mechanisms by which government injects money into the economy are
    // unclear and seem to involve financing banks to make more loans.
    // In the case of the NHS, nationalised industries, the civil service
    // and the 'armed forces' the mechanism is probably more direct.
    // Anyway, to go into this would be a distraction so we'll simply
    // treat it as untaxable payment for services.
    recipient->credit(amount, this);
    balance -= amount;

    // The government can always transfer any amount
    return true;
}

// All credits to the Government are (at present) regarded as tax. This
// means that while they offset the deficit we need to keep a separate
// record as well. However we don't distinguish between income tax, sales
// tax, and 'pre-tax deductions'. These are all accounted for elsewhere.
// Obviously, the government doesn't pay tax.
void Government::credit(int amount, Account *creditor)
{
    Account::credit(amount);
    rec += amount;
}



