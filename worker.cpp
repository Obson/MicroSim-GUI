#include "account.h"
#include <QDebug>
#include <QMessageBox>

Worker::Worker(Domain *domain) : Account(domain)
{
    init();
}

void Worker::init()
{
    employer = nullptr;

    period_hired = -1;
    period_fired = 0;
    //average_wages = 0;

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
    return (emp == employer);
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

/*
 * Make a purchase
 */
void Worker::trigger(int period)
{
    if (period > last_triggered)    // to prevent double counting
    {
        last_triggered = period;

        double purch;
        double thresh = _domain->getIncomeThreshold();
        double prop_con = _domain->getPropCon();

        if (balance <= thresh)
        {
            purch = balance;
        }
        else
        {
            purch = (((balance - thresh) * prop_con) / 100 ) + thresh;
        }

        if (purch > 0)
        {
            if (transferSafely(_domain->selectRandomFirm(), purch, this))
            {
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
    //qDebug() << "Worker::setAgreedWage() called";
    agreed_wage = wage;
}

void Worker::credit(double amount, Account *creditor, bool)
{
    if (amount < -0.1)
    {
        Q_ASSERT(false);
    }

    Account::credit(amount);        // credit the account

    if (isEmployedBy(creditor))     // i.e. this is a payment of wages (or bonus)
    {
        // Here we assume that if the creditor is our employer then we should
        // pay income tax. If the creditor isn't our employer but we're
        // receiving the payment in our capacity as Worker then it's probably
        // benefits or bonus. If not employed at all it must be bonus and we are
        // flagged for deletion. Surprising but perfectly possible.
        double tax = (amount * _domain->getIncTaxRate()) / 100;

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
    else if (creditor == _domain->government())     //i.e. benefits payment
    {
        benefits += amount;
    }
    else
    {
        /*
         * Could be a purchase by. There's no reson why this shouldn't be
         * valid, but we need to check the logic...
         */
        QMessageBox msgBox;
        msgBox.setText("Unknown reason for crediting worker");
        exit(100);
    }
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

