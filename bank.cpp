#include "account.h"

Bank::Bank(Model *model) : Account(model)
{

}

void Bank::lend(double amount, double rate, Account *recipient)
{
    recipient->loan(amount, rate, this);
    balance -= amount;
}

void Bank::trigger(int)
{
    // No action requied at present. May be later..
}
