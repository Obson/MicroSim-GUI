
#include "account.h"
#include <QDebug>

int Account::_id = 0;

int Account::nextId() {
    return _id++;
}

Account::Account(Domain *domain)
{
    _domain = domain;
    id = nextId();
    balance = 0;
    owed_to_bank = 0;
}

double Account::getBalance()
{
    return balance;
}

double Account::getAmountOwed()
{
    return owed_to_bank;
}

// To be overridden by classes that could be government supported (i.e. Firm)
bool Account::isGovernmentSupported()
{
    return false;
}

// Use transferSafely() in preference to credit() as credit() doesn't upate our
// balance. recipient will be nullptr if trying to transfer to a non-existent
// startup.
bool Account::transferSafely(Account *recipient, double amount, Account *creditor)
{
    if (amount > balance || recipient == nullptr)
    {
        // TODO: This needs to go into a log somewhere
        qDebug() << "Account::transferSafely(): done (insufficient funds or no recipient)";
        // Q_ASSERT(false);
        return false;
    }
    else
    {
        // qDebug() << "crediting" << amount;
        recipient->credit(amount, creditor);
        balance -= amount;
        return true;
    }
}

void Account::credit(double amount, Account*, bool)
{
    balance += amount;
}

void Account::loan(double amount, double rate, Account *creditor)
{
    balance += amount;
    if (creditor->isBank())
    {
        // TODO: Note that if interest rate changes on subsequent loans, the
        // new rate will replace the old rate for the whole amount. This is not
        // very realistic and should be changed eventually.
        owed_to_bank += amount;
        interest_rate = double(rate);
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
