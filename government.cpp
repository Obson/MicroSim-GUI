
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
    wages_paid = 0;

    employees.clear();
}

Government::Government(Domain *domain, int size) : Bank(domain)
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

    qDebug() << "Government::Government (...) called for domain"
             << domain->getName() << "size =" << size;

    hireSome(domain->getStdWage(), size);
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


void Government::trigger(int period)
{
    qDebug() << "Government::trigger (" << period << "), last_triggered ="
             << last_triggered;

    Q_ASSERT(period > last_triggered);

    last_triggered = period;

    /*
     * Make government procurement purchases. We do this by paying firms
     * currently selected at random (this could change) the amount specified
     * in the parameters for this domain (check this). This will appear to
     * the firms as a normal purchase and it will therefore attract sales tax.
     * We may need a different mechanism for government contracts to allow
     * them to be taxed differently. Note that in any case the government-as-
     * firm should not pay tax since its receipts are in HPM. We should
     * probably make a distinction between HPM and bank money. FIX THIS!
     */
    double amt = _domain->getProcurement();
    qDebug() << "Transferring" << amt << _domain->_currency
             << "to random firm for procurement";

    /*
     * transferSafely() updates both balances (payer and payee)
     */
    transferSafely(_domain->selectRandomFirm(), amt, this);

    proc += amt;    // procurement
    exp += amt;     // govt expenditure

    /*
     * Make benefits payments to all unemployed workers. getUBR returns a
     * percentage of standard wage
     */
    double amount = (_domain->getStdWage() * _domain->getUBR()) / 100;

    qDebug() << "Transferring" << amount << _domain->_currency
             << "to all unemployed workers as benefit";

    ben += payWorkers(amount, this, Reason::for_benefits);

    /*
     * payWorkers() does not update payer balance so we do this on return
     */
    balance -= ben; // govt balance
}

bool Government::transferSafely(Account *recipient, double amount, Account *)
{
    /*
     * We no longer mark procurement transfers, which means they attract
     * sales tax like any other purchases
     */
    if (recipient != nullptr) {
        qDebug() << "Government making payment of" << amount;
        recipient->credit(amount, this, true);
        balance -= amount;
    }

    /*
     * The government can always transfer any amount
     */
    return true;
}

// All credits to the Government are (at present) regarded as tax. This
// means that while they offset the deficit we need to keep a separate
// record as well. However we don't distinguish between income tax, sales
// tax, and 'pre-tax deductions'. These are all accounted for elsewhere.
// Obviously, the government doesn't pay tax.
void Government::credit(double amount, Account*, bool)
{
    qDebug() << "Government receiving tax payment of" << amount;
    Account::credit(amount);
    rec += amount;
}

/*
 * Note that a Government is also a Firm and therefor naintains a list of its
 * employees whose wages are paid in the normal way. Additionally, as
 * government it has access to the domain-wide list of Workers to which it pays
 * benefits, pensions, etc., as necessary.
 */
double Government::payWorkers(double amount, Account *source, Reason reason)
{
    int num_workers = _domain->workers.count();

    qDebug() << "Government::payWorkers (" << amount << ", ...)"
             << num_workers << "workers to pay";

    double amt_paid = 0;

    for (int i = 0; i < num_workers; i++)
    {
        switch (reason)
        {
        case Reason::for_benefits:

            if (!_domain->workers[i]->isEmployed())
            {
                _domain->workers[i]->credit(amount, source);
                amt_paid += amount;
            }
            break;

        case Reason::for_bonus:

            /*
             * THIS SHOULD PROBABLY BE FIXED
             * -----------------------------
             * Note that when paying bonuses we do not check that sufficient
             * funds are available -- it's up to the caller to ensure that the
             * amount is correct. Any overpayment will create a negative
             * balance in the caller's account.
             */
            if (_domain->workers[i]->getEmployer() == source)
            {
                _domain->workers[i]->credit(amount, source);
                amt_paid += amount;
            }
            break;
        }
    }

    return amt_paid;
}




