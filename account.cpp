#include "account.h"
#include <QDebug>

int Account::_id = 0;

int Account::nextId() {
    return _id++;
}

Account::Account(Model *model)
{
    _model = model;
    id = nextId();
    //balance = 0;
    //owed_to_bank = 0;
}

Model *Account::model()
{
    return _model;
}

int Account::getBalance()
{
    return balance;
}

int Account::getAmountOwed()
{
    return owed_to_bank;
}

// To be overridden by classes that could be government supported (i.e. Firm)
bool Account::isGovernmentSupported()
{
    return false;
}

bool Account::isGovernment()
{
    return false;
}

// Use transferTo() in preference to credit() as credit() doesn't upate our
// balance
bool Account::transferSafely(Account *recipient, int amount, Account *creditor)
{
    if (amount > balance)
    {
        // TODO: This needs to go into a log somewhere
        qDebug() << "Account::transferTo(): done (insufficient funds)";
        return false;
    }
    else
    {
        recipient->credit(amount, creditor);
        balance -= amount;
        return true;
    }
}

void Account::credit(int amount, Account *creditor, bool force)
{
    balance += amount;
}

void Account::loan(int amount, int rate, Account *creditor)
{
    balance += amount;
    if (creditor == model()->bank())
    {
        owed_to_bank += amount;

        // TODO: Note that if interest rate changes on subsequent loans, the
        // new rate will replace the old rate for the whole amount. This is not
        // very realistic and should be changed eventually.
        interest_rate = rate;
    }
    else
    {
        qCritical() << "Only bank loans allowed at present";
    }
}

int Account::getId() const
{
    return id;
}
