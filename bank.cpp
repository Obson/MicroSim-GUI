#include "account.h"

Bank::Bank(Model *model) : Account(model)
{

}

void Bank::lend(int amount, Account *recipient)
{
    recipient->loan(amount, this);
    balance -= amount;
}

void Bank::trigger(int period)
{
    // No action requied at present. May be later..
}
