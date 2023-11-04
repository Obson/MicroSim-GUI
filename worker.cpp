#include "account.h"
#include <QDebug>

Worker::Worker(Domain *domain) : Account(domain)
{
    employer = nullptr;
    period_hired = -1;
    period_fired = 0;
    average_wages = 0;
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
    if (period > last_triggered)    // to prevent double counting
    {
        last_triggered = period;

        double purch;
        double thresh = _domain->getIncomeThreshold();
        double prop_con = _domain->getPropCon();

        if (balance <= thresh) {
            purch = balance;
        } else {
            purch = ((balance - thresh) * prop_con) + thresh;
        }

        if (purch > 0) {
            if (transferSafely(_domain->selectRandomFirm(), purch, this)) {
                purchases += purch;
            }
        }
    }
}

void Worker::epilogue(int)
{
    average_wages = (wages + average_wages) / 2;
}

double Worker::agreedWage()
{
    return agreed_wage;
}

void Worker::setAgreedWage(double wage)
{
    qDebug() << "Worker::setAgreedWage() called";
    agreed_wage = wage;
}

void Worker::credit(double amount, Account *creditor, bool)
{
    // qDebug() << "Worker::credit(): amount =" << amount;

    if (amount < 0)
    {
        Q_ASSERT(false);
    }

    Account::credit(amount);

    if (isEmployedBy(creditor))
    {
        //qDebug() << "Worker::credit(): wages assumed";

        // Here we assume that if the creditor is our employer then we should
        // pay income tax. If the creditor isn't our employer but we're
        // receiving the payment in our capacity as Worker then it's probably
        // benefits or bonus. If not employed at all it must be bonus and we are
        // flagged for deletion. Surprising but perfectly possible.
        double tax = amount * _domain->getIncTaxRate();

        //qDebug() << "Worker::credit(): transferring tax" << tax << "to gov";

        /*
         * Sort this out by adding a government() function to Domain and
         * making _gov private
         */
        if (transferSafely(_domain->government(), tax, this))
        {
            wages += amount;
            inc_tax += tax;
        }
        else
        {
            // Since tax is always going to be less that the amount credited
            // this assertion is almost redudndant and can be removed after
            // testing
            Q_ASSERT(false);
        }
    }
    else if (creditor == _domain->government())
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

double Worker::getWagesReceived()
{
    return wages;
}

double Worker::getAverageWages()
{
    return average_wages;
}

double Worker::getBenefitsReceived()
{
    return benefits;
}

double Worker::getPurchasesMade()
{
    return purchases;
}

void Worker::setEmployer(Firm *emp)
{
    employer = emp;
}

double Worker::getIncTaxPaid()
{
    return inc_tax;
}

