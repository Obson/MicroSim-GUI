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
    _gov_firm = model()->createFirm();
}

Government::Government(Model *model) : Account(model)
{
    // This must be the first Firm created, as this ensures that government
    // money is always spent at the start of the period.
    _gov_firm = model->createFirm();
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

int Government::getTopup(Account *requester, int amount)
{
    if (requester->isGovernmentSupported())
    {
        balance -= amount;
        return amount;
    }
    else
    {
        return 0;
    }
}

Firm *Government::gov_firm()
{
    return _gov_firm;
}

void Government::trigger(int period)
{
    // Government grants (incl support of gov-owned businesses)
    int amt = model()->getGovExpRate();
    _gov_firm->grant(amt);
    balance -= amt;
    exp += amt;

    // Pay benefits to all unemployed workers
    ben += model()->payWorkers((model()->getStdWage() * model()->getUBR()) / 100,
                           0,                   // no max amount
                           this,                // source
                           Model::for_benefits, // reason
                           period
                           );

    balance -= ben;
}

//
// This function overrides Account::transferTo to allow a negative balance.
//
void Government::transferTo(Account *recipient, int amount, Account *creditor)
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



