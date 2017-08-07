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
    balance = 0;
}

Model *Account::model()
{
    return _model;
}

int Account::getBalance()
{
    return balance;
}

// Use transferTo() in preference to credit() as credit() doesn't upate our
// balance
bool Account::transferTo(Account *recipient, int amount, Account *creditor)
{
    // qDebug() << "Account::transferTo(): amount =" << amount << ", balance =" << balance;

    if (amount > balance)
    {
        // TODO: This needs to go into a log somewhere
        // std::cout << "Account " << id << ": insufficient funds (" << balance << " available, " << amount << " to pay)\n";
        // qDebug() << "Account::transferTo(): done (insufficient funds)";
        return false;
    }
    else
    {
        recipient->credit(amount, creditor);
        balance -= amount;
        // qDebug() << "Account::transferTo(): done (OK)";
        return true;
    }
}

void Account::credit(int amount, Account *creditor)
{
    balance += amount;
}

int Account::getId() const
{
    return id;
}
