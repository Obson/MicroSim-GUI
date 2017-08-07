#include "account.h"
#include <QDebug>

Worker::Worker(Model *model) : Account(model)
{
    employer = nullptr;
    period_hired = -1;
    period_fired = 0;
    init();
}

void Worker::init()
{
    wages = 0;
    benefits = 0;
    purchases = 0;
    inc_tax = 0;
}

bool Worker::isEmployed()
{
    return (employer != nullptr);
}

bool Worker::isEmployedBy(Account *emp)
{
    return (emp != nullptr && emp == employer);
}

void Worker::setPeriodHired(int period)
{
    period_hired = period;
}

bool Worker::isNewHire(int period)
{
    return period_hired == period;
}

Firm *Worker::getEmployer()
{
    return employer;
}

void Worker::trigger(int period)
{
    // qDebug() << "Worker::trigger(): period =" << period;
    if (period > last_triggered)    // to prevent double counting
    {
        last_triggered = period;
        int purch = (balance * model()->getPropCon()) / 100;
        // qDebug() << "Worker::trigger(): purchase amount" << purch;

        if (purch > 0 && transferTo(model()->selectRandomFirm(), purch, this))
        {
            purchases += purch;
        }
    }
    // qDebug() << "Worker::trigger(): done";
}

void Worker::credit(int amount, Account *creditor)
{
    // qDebug() << "Worker::credit(): amount =" << amount;

    Account::credit(amount);

    if (isEmployedBy(creditor))
    {
        //qDebug() << "Worker::credit(): wages assumed";

        // Here we assume that if the creditor is our employer then we should
        // pay income tax. If the creditor isn't our employer but we're
        // receiving the payment in our capacity as Worker then it's probably
        // benefits or bonus. If not employed at all it must be bonus and we are
        // flagged for deletion. Surprising but perfectly possible.
        int tax = (amount * model()->getIncTaxRate()) / 100;

        //qDebug() << "Worker::credit(): transferring tax" << tax << "to gov";

        transferTo(model()->gov(), tax, this);
        wages += amount;
        inc_tax += tax;
    }
    else if (creditor == model()->gov())
    {
        // qDebug() << "Worker::credit(): benefits assumed";
        benefits += amount;
    }
    else
    {
        // qDebug() << "Worker::credit(): unknown reason";
        exit(100);
    }
    // qDebug() << "Worker::credit(): done";
}

int Worker::getWagesReceived()
{
    return wages;
}

int Worker::getBenefitsReceived()
{
    return benefits;
}

int Worker::getPurchasesMade()
{
    return purchases;
}

void Worker::setEmployer(Firm *emp)
{
    employer = emp;
}

int Worker::getIncTaxPaid()
{
    return inc_tax;
}

