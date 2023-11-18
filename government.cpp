
#include "account.h"
#include <QDebug>

void Government::reset()
{
    qDebug() << "Government::reset() called";

    _isGovernment = true;

    exp = 0;
    unbudgeted = 0;
    rec = 0;
    ben = 0;
    proc = 0;
    last_triggered = -1;
    balance = 0;
}

Government::Government(Domain *domain) : Bank(domain)
{
    /*
     * Government is derived from Bank, which is derived from Firm. This
     * allows it to have employees and pay wages (etc.) using bank money.
     * Its role as central bank is separate and simply means it can maintain
     * (HPM, i.e.reserve) accounts for its clients the clearing banks. It does
     * this by 'lending' to the banks at Bank Rate. It must keep a record of
     * these operations but there is no associated risk and its own balance
     * will always be negative. This negative balance (less tax 'receipts')
     * would conventionally trigger borrowing and we may choose to model this
     * for demo purposes later on.
     */

    qDebug() << "Government::Government(...) called";
    reset();
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

// NEXT: THIS IS SERIOUSLY OUT OF DATE...

void Government::trigger(int period)
{
    qDebug() << "last_triggered =" << last_triggered << ", period =" << period;

    Q_ASSERT(period > last_triggered);

    last_triggered = period;

    // Direct purchases (procurement), adjusts balance automatically
    double amt = _domain->getProcurement();
    qDebug() << "Transferring" << amt << "to random firm";
    transferSafely(_domain->selectRandomFirm(), amt, this);
    proc += amt;
    exp += amt;     // include in expenditure not as a separate item as
    // less confusing

    // Benefits payments to all unemployed workers (doesn't adjust balance,
    // so we must do this on return)
    qDebug() << "Transferring" << _domain->getStdWage() * _domain->getUBR() << "to all workers";
    ben += payWorkers(_domain->getStdWage() * _domain->getUBR(),    // amount
                      this,                                         // source
                      Reason::for_benefits                          // reason
                      );
    balance -= ben;
}

/*
size_t Government::getNumEmployees()
{
    return Bank::getNumEmployees();
}
*/

bool Government::transferSafely(Account *recipient, double amount, Account *)
{
    /*
     * We no longer mark procurement transfers, which means they are
     * automatically taxable. This may need changing.
     */
    if (recipient != nullptr) {
        qDebug() << "crediting" << amount;
        recipient->credit(amount, this, true);
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
void Government::credit(double amount, Account*creditor, bool)
{
    qDebug() << "Government::credit(" << amount;
    Account::credit(amount);
    rec += amount;
}

/*
 * Note that a Government is also a Firm and therefor naintains a list of its
 * employees whose wages are paid in the normal way. Additionally, as
 * governmentit has access to the domain-wide list of Workers to which it pays
 * benefits, pensions, etc., as necessary.
 */
double Government::payWorkers(double amount, Account *source, Reason reason)
{
    QVector<Worker*> workers = _domain->workers;
    int num_workers = workers.count();
    double amt_paid = 0;

    for (int i = 0; i < num_workers; i++)
    {
        switch (reason)
        {
        case Reason::for_benefits:

            if (!workers[i]->isEmployed())
            {
                workers[i]->credit(amount, source);
                amt_paid += amount;
            }
            break;

        case Reason::for_bonus:

            /*
             * THIS SHOULD PROBABLY BE FIXED
             * -----------------------------
             * Note that when paying bonuses we do not check that sufficient
             * funds are available -- it's up to the caller to ensure that the
             * amount is correct. Any overpayment will simply create a negative
             * balance in the caller's account.
             */
            if (workers[i]->getEmployer() == source)
            {
                workers[i]->credit(amount, source);
                amt_paid += amount;
            }
            break;
        }
    }

    return amt_paid;
}




