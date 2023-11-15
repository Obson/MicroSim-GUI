#include "account.h"

Bank::Bank(Domain *domain) : Firm(domain) //Account(domain)
{

}

void Bank::lend(double amount, double rate, Account *recipient)
{
    recipient->loan(amount, rate, this);
    balance -= amount;
}

/*
size_t Bank::getNumEmployees()
{
    return Firm::getNumEmployees();
}
*/

/*
 * this overloads the base funxtion in Account. For the time being it just
 * duplicates the code but it will have to be modified to include additional
 * processing.
 */
bool Bank::transferSafely(Account *recipient, double amount, Account *creditor)
{
    if (amount > balance || recipient == nullptr)
    {
        // TODO: This needs to go into a log somewhere
        qDebug() << "Account::transferSafely(): done (insufficient funds or no recipient)";
        return false;
    }
    else
    {
        qDebug() << "crediting" << amount;
        recipient->credit(amount, creditor);
        balance -= amount;
        return true;
    }

}

/*
 * Periodically clear funds owing to other banks
 */
void Bank::trigger(int)
{

}
