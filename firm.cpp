
// *** FUNDAMENTALS ***
//
// The notional 'government-owned' firm is the only firm that is created
// initially and serves as a target for government spending. Other firms come
// into existence depending on economic conditions. The government-owned firm is
// the civil service, the military and public services or other enterprises that
// are entirely funded by the government.
//
// Suppose a private firm (or all private firms taken together) produce goods
// and services to the value of n CUs (currency units). If all their stock is
// sold (the best case scenario) they will receive payments of n CUs and this is
// the most they can pay their employees in wages without drawing on credit.
// However, their employees will have to return some of this money to the
// government in taxes and will not therefore be able to buy all the stock. This
// means there will be less money available to pay out in wages next time, and
// even less stock will be produced. Eventually the company will have to get rid
// of its employees and go out of business.
//
// This is avoided is by the injection of new money through the following
// mechanisms: increasing credit from the banks, payment of benefits to the
// unemployed by the government using new money (which means that in part the
// government finances employment via the unemployed) direct purchases from
// private firms by the government (using new money), and purchases by employees
// of the notional government-owned firm. Note that all these mechanisms rely in
// different ways on the ceation of new money by the governmwent.
// ***

#include "account.h"
#include <cassert>
#include <QtMath>
#include <QDebug>

Firm::Firm(Domain *domain, bool state_supported) : Account(domain)
{
    _state_supported = state_supported;
}

void Firm::init()
{
    /*
     * Reset non-cumulatibe variables
     */
    /*
    wages_paid = 0;
    sales_tax_paid = 0;
    sales_receipts = 0;
    num_hired = 0;
    num_fired = 0;
    bonuses_paid = 0;
    investment= 0;
    */
}

bool Firm::isGovernmentSupported()
{
    return _state_supported;
}

void Firm::trigger(int period)
{
    if (period > last_triggered)
    {
        last_triggered = period;

        // TODO: This needs sorting out properly!
        if (!_state_supported)
        {
            // Pay interest on bank loans.
            // TODO: At present we never pay off the loan. We should have a
            // parameter that indicates the factor by which the balance has to
            // exceed the outstanding loan before we will repay it. Or
            // something...
            double interest = owed_to_bank * interest_rate;

            // If interest is greater than balance we'll just skip the payment
            // and hope for better luck next time. A bank is unlikely to
            // foreclose unless this happens a lot...
            if (interest > 0 && interest <= balance)
            {
                //qDebug() << "Firm::trigger(): paying loan interest of" << interest;
                if (!transferSafely(_bank, interest, this))
                {
                    qDebug() << "Firm::trigger(): failed to transfer interest to bank";
                }
            }
            else if (interest < 1.0 && owed_to_bank > 0)
            {
                qDebug() << "Firm::trigger(): adding interest of"
                         << interest
                         << "to loan";
                owed_to_bank += interest;
            }
        }

        /*
         * wages_paid is not cumulative, but balance, of course, is
         */
        wages_paid = payWages();
        balance -= wages_paid;
    }
}

double Firm::payWages()
{
    double amt_paid = 0;
    num_just_fired = 0;

    double dedns_rate = _domain->getPreTaxDedns();
    int num_employees = static_cast<int> (employees.count());

    for (int i = 0; i < num_employees; i++)
    {
        Worker *w = employees[i];

        double wage_due = w->agreed_wage;
        double dedns = dedns_rate * wage_due;
        double funds_available = getBalance();

        bool ok_to_pay = false;

        if (funds_available - amt_paid < wage_due + dedns)
        {
            /*
             * The firm doesn't have enough money in its account to pay all
             * the wages due.
             */
            double shortfall = wage_due + dedns - funds_available + amt_paid;
            if (isGovernmentSupported())
            {
                /*
                 * Get additional funds from government -- this needs
                 * expanding. Government must advance fund (HPM) to the
                 * firm's bank account, that account should then be debited
                 * and the payee's bank account credited. To do this
                 * properly we need to set up two transfer mechanisms -- one
                 * from government to the firm's bank (HPM) followed by a
                 * credit to the firms own account, and the other an
                 * uncleared transfer from the firm's account to the
                 * rmployee's account. Here we short circuit the process
                 * but to exactly the same effect except that the funds are
                 * in effect cleared immediately.
                 */
                _domain->government()->debit(this, shortfall);

                // Transfer the funds to the firm
                qDebug() << "crediting" << shortfall;
                credit(shortfall, this);

                ok_to_pay = true;
            }
            else
            {
                /*
                 * Possibly request a loan from the bank, depending on
                 * policy. Policy is determined by getLoanProb(), which
                 * returns an integer from 0 (= never) to 4 (= always).
                 */
                if (qrand() % 4 < _domain->getLoanProb())
                {
                    // Apply a bank loan to cover the shortfall
                    _bank->lend(shortfall, _domain->getBusRate(), this);
                    ok_to_pay = true;
                }
            }
        }
        else
        {
            ok_to_pay = true;
        }

        if (ok_to_pay)
        {
            /*
             * Pay the full amount of wages to the employee
             */
            qDebug() << "crediting" << wage_due;
            employees[i]->credit(wage_due, this);

            /*
             * Pay deductions straight back to the government.
             */
            qDebug() << "crediting" << dedns;
            _domain->government()->credit(dedns, this);

            amt_paid += wage_due + dedns;
            _dedns += dedns;
        }
        else
        {
            Q_ASSERT_X(isGovernmentSupported(), "Firm::payWages",
                       "Firm is government supported");
            // Not able to pay this worker so fire instead
        }
    }
    return amt_paid;                // so caller can update balance
}

/*
 * This function is likely to be significantly slower than fire(int ix)
 */
void Firm::fire(Worker *w)
{
    employees.removeOne(w);
    _domain->unemployedWorkers.append(w);
    w->employer = nullptr;
    num_fired++;
}

/*
 * Faster version for use when ix is known
 */
void Firm::fire(int ix)
{
    Worker *w = employees.at(ix);
    employees.removeAt(ix);
    _domain->unemployedWorkers.append(w);
    w->employer = nullptr;
    num_fired++;
}


// Distribute any excess funds as bonuses/dividends. Excess is defined as
// the current balance (after sales have been taken into account), less
// committed funds (i.e. funds needed to pay current complement of
// employees and the associated deductions) less the proportion designated
// for investment (i.e. reserved for paying any additional employees). Then
// hire new workers if funds permit.
void Firm::epilogue()
{
    if (_state_supported) {

        // ***
        // State-supported businesses are set up with a full quota of
        // employees and do not receive funds from sales so have no need to
        // (and cannot) recruit. As funds are supplied by government for
        // payment of wages (and expenses) only, bonuses cannot be paid
        // either, so there's nothing to do here.
        // ***

        return;
    }

    investment = 0;

    if (balance > wages_paid)
    {
        // We must keep in hand at least the amount needed to pay future wages
        double available = balance - wages_paid;   // now includes deductions
        double investible = available * _domain->getPropInv();
        double bonuses = (available - investible) * _domain->getDistributionRate();

        // We distribute the funds as bonuses before hiring new workers to
        // ensure they only get distributed to existing workers.
        int emps = employees.count();
        double amt_paid = 0;
        if (emps > 0)
        {
            amt_paid = _domain->_gov->payWorkers(bonuses/emps, this, Reason::for_bonus);
        }

        // Adjust calculation if not all the bonus funds were used
        if (amt_paid < bonuses)
        {
            investible += bonuses_paid;
        }

        balance -= amt_paid;
        Q_ASSERT(balance >= 0);

        bonuses_paid += amt_paid;

        // How many more employees can we afford?

        double std_wage = _domain->getStdWage();
        double current_wage_rate = productivity * std_wage; // Note that productivity is initialised to 1.0 in Account
        int num_to_hire = static_cast<int> (floor(investible / (current_wage_rate * (1 + _domain->getPreTaxDedns()))));

        // Hire new workers

        if (num_to_hire > 0)
        {
            double invested = hireSome(current_wage_rate, num_to_hire);

            // ***
            // If we are unable to hire all the workers we want we will
            // have reserved funds that do not get spent. Even if we can there
            // may well be a non-zero residue, although this would normally get
            // incoporated into subsequent investments. Where this fails
            // because there are no unemployed workers left to recruit, the
            // business sector balance will increase without limit and the
            // deficit will eventually stabilise at a positive non-zero value.
            //
            // To avoid this we must dispose of the surplus funds. We can
            // either (a) let the money build up until we can declare a
            // dividend (how is this specified?), (b) poach employees by
            // offering increased wages (inflation), or (c) invest in capital
            // equipment (productivity). For the time being we adopt option (c)
            // only.
            // ***

            double excess = investible - invested;

            if (excess > 0)
            {
                // How many did we just hire?
                int new_emps = getNumHired();

                if (emps > 0 || new_emps > 0)
                {
                    // Purchase capital equipment

                    // ***
                    // This operation distributes the excess funds around the
                    // other firms, which will be equally unable to use them.
                    // However as this is happening in the epilogue phase they will
                    // simply remain in their balances until next triggered, at
                    // which point they will also have to purchase capital
                    // equipment. The medium-term result will be a general
                    // inflation.
                    // ***

                    Firm *supplier = _domain->selectRandomFirm(this);

                    // ***
                    // We must allow for the possibility that there are no firms
                    // from which to select a supplier. This will be the case if
                    // parameters were set up with fewer than two startups, and
                    // selectRandomFirm(this) will then have returned nullptr.
                    // ***

                    if (supplier != nullptr)
                    {
                        qDebug() << "crediting" << excess << "to supplier";
                        supplier->credit(excess, this);
                        balance -= excess;
                        investment = excess;

                        /*
                         * Update productivity
                         */

                        double n = _domain->getCapexRecoupTime();

                        /* THIS NEEDS REVIEWING!
                         * ---------------------
                         * We to recoup the investment over n periods, so we
                         * need to raise an additional (excess / n) per period.
                         * This can be shared out over the new number of
                         * employees, giving an additional
                         *      excess / (n * (emps+new_emps))
                         * per employee. The new wage rate will then be
                         *      current_wage_rate + (excess / (n * (emps+new_emps))
                         * and productivity will be that amount divided by
                         * std_wage.
                         *
                         * I think!
                         */

                        productivity = (current_wage_rate +
                                    (excess / (n * double(double(emps) + new_emps))
                                    )
                                    ) / std_wage;
                    }
                }
            }
        }
    }
}

Worker *Firm::hire(double wage)
{
    Worker *w = nullptr;

    if (!_domain->unemployedWorkers.isEmpty())
    {
        /*
         * Get the last worker on the unemployed list. Check the workers last
         * agreed wage (etc) to see whether s/he would be likely to accept a
         * job (TODO). If OK, transfer to firm's list of employees and update
         * the new employee's details
         */
        w = _domain->unemployedWorkers.last();
        _domain->unemployedWorkers.removeLast();
        employees.append(w);
        w->setAgreedWage(wage);
        w->setEmployer(this);

        num_hired += 1;
    }

    return w;
}

/*
 * Review and revise if necessary...
 */
double Firm::hireSome(double wage, int number_to_hire)
{
    int count;
    double wages_due;
    for (count = 0, wages_due = 0; count < number_to_hire; count++)
    {
        Worker *w = hire(wage);
        if (w == nullptr)
        {
            break;
        }
        else
        {
            wages_due += w->agreed_wage;
        }
    }

    return wages_due;
}


void Firm::credit(double amount, Account *creditor, bool force)
{
    Account::credit(amount);

    // If state-supported the reason we are being credited must be that we have
    // asked for additional support to pay wages, in which case we have now
    // finished (unless direct purchasing, in which case force == true). If we
    // are not state-supported (or it's a direct purchase) the credit will be
    // for a sale, and sales tax is therefore due.

    if (!creditor->isGovernment() || force)
    {
        sales_receipts += amount;

        // Base class credits account but doesn't pay tax. We assume seller,
        // not buyer, is responsible for paying sales tax and that payments
        // to a Firm are always for purchases and therefore subject to
        // sales tax.
        double r = _domain->getSalesTaxRate();
        if (r > 0)
        {
            double t = (amount * r) / (r + 1.0);
            if (transferSafely(_domain->government(), t, this)) {
                sales_tax_paid += t;
            }
        }
    }
}

size_t Firm::getNumEmployees()
{
    return 0;  // FIX: IMPORTANT
};

double Firm::getProductivity()
{
    return productivity;
}

double Firm::getWagesPaid()
{
    return wages_paid;
}

double Firm::getInvestment()
{
    return investment;
}

double Firm::getBonusesPaid()
{
    return bonuses_paid;
}

double Firm::getSalesTaxPaid()
{
    return sales_tax_paid;
}

double Firm::getSalesReceipts()
{
    return sales_receipts;
}

/*
size_t Firm::getNumEmployees()
{
    return static_cast<size_t> (employees.count());
}
*/

int Firm::getNumHired()
{
    return num_hired;
}

int Firm::getNumFired()
{
    return num_fired;
}
