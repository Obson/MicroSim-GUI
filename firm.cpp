
// The 'government-owned' firm is the only firm that is created initially and
// serves as a target for government spending. Other firms come into existence
// depending on economic conditions.
//
// Suppose a firm (or all firms taken together) produce goods and services to the
// value of n CUs (currency units). If all their stock is sold (the best case
// scenario) they will receive payments of n CUs. This is the most they can pay
// their employees without drawing on credit and is therefore the most they can
// pay out in wages (without drawing on credit). However, their employees will
// have to return some of this money to the government in taxes and will not
// therefore be able to buy all the stock. This means there will be less money
// available to pay out in wages next time, and even less stock will be produced.
// Eventually the company will have to get rid of its employees and go out of
// business.
//
// The only way this can be avoided is by the injection of new money. Mechanisms
// are: constantly increasing credit from the bank, payment of benefits to the
// unemployed (so that in effect the unemployed will be financing employment!)
// and direct purchases from the firms by the government (using new money).
//

#include "account.h"
#include <cassert>
#include <QDebug>

Firm::Firm(Model *model, bool state_supported) : Account(model)
{
    _state_supported = state_supported;
}

void Firm::init()
{
    wages_paid = 0;
    sales_tax_paid = 0;
    sales_receipts = 0;
    dedns_paid = 0;
    num_hired = 0;
    num_fired = 0;
    bonuses_paid = 0;
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
                qDebug() << "Firm::trigger(): paying loan interest of" << interest;
                transferSafely(model()->bank(), interest, this);
            }
            else if (interest == 0 && owed_to_bank > 0)
            {
                qDebug() << "Firm::trigger(): adding interest of" << interest << "to loan";
                owed_to_bank += interest;
            }
        }

        // Important: wages_paid is not cumulative!
        wages_paid = model()->payWages(this, period);
        balance -= wages_paid;

        // TODO: We've knocked this out in the reorganisation. Reinstate.
        // dedns_paid += dedns;
    }
}

// Distribute any excess funds as bonuses/dividends. Excess is defined as
// the current balance (after sales have been taken into account), less
// committed funds (i.e. funds needed to pay current complement of
// employees and the associated deductions) less the proportion designated
// for investment (i.e. reserved for paying any additional employees). Then
// hire new workers if funds permit.
void Firm::epilogue(int period)
{
    if (_state_supported) {
        // State-supported businesses are set up will a full quota of
        // employees and do not receive funds from sales so have no need to
        // (and cannot) recruit. As funds are supplied by government for
        // payment of wages only (including expenses) bonuses cannot be paid
        // either, so there's nothing to do here.
        return;
    }

    if (balance > wages_paid)
    {
        // We must keep in hand at least the amount needed to pay wages
        double available = balance - wages_paid;   // now includes deductions

        double investible = (available * model()->getPropInv()) / 100;
        double bonuses = ((available - investible) * model()->getDistributionRate()) / 100;

        // We distribute the funds before hiring new workers to ensure they
        // only get distributed to existing workers.
        int emps = model()->getNumEmployedBy(this);
        double amt_paid = 0;
        if (emps > 0)
        {
            amt_paid = model()->payWorkers(bonuses/emps, bonuses, this, Model::for_bonus);
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
        double std_wage = model()->getStdWage();
        double current_wage_rate = productivity * std_wage;
        int num_to_hire = (investible * 100) /
                (current_wage_rate * (100 + model()->getPreTaxDedns()));

        if (num_to_hire > 0)
        {
            double invested = model()->hireSome(this, current_wage_rate, period,
                                             num_to_hire);
            double excess = investible - invested;

            // If we are unable to hire all the workers we want we will
            // have reserved funds that do not get spent. Even if we do there
            // may well be a non-zero residue, although this would normally get
            // incoporated into subsequent investments. Where this fails
            // because there are no unemployed workers left to recruit, the
            // business sector balance will increase without limit and the
            // deficit will eventually stabilise at a positive non-zero value.
            //    To avoid this we must dispose of the surplus funds. We can
            // either (a) let the money build up until we can declare a
            // dividend (how is this specified?), (b) poach employees by
            // offering increased wages, or (c) invest in capital
            // equipment. For the time being we adopt option (c) only.

            if (excess > 0)
            {
                int new_emps = model()->getNumEmployedBy(this);

                if (emps > 0 || new_emps > 0)
                {
                    // purchase capital equipment to the value of 'excess'. Note
                    // that this operation distributes the excess funds around the
                    // other firms, which will be equally unable to use them.
                    // However as this is happening in the epilogue phase they will
                    // simply remain in their balances until next triggered, at
                    // which point they will also have to purchase capital
                    // equipment. The medium-term result will be a geral in flation.
                    Firm *supplier = model()->selectRandomFirm(this);
                    supplier->credit(excess, this);
                    balance -= excess;

                    // We assume we want to recoup the investment over ten periods
                    // (this must be parameterised), so we need to raise an
                    // additional (excess / 10) per period. This can be shared
                    // out over the new number of employees giving an additional
                    //     excess / (10 * (emps + new_emps))
                    // per employee. The new wage rate will then be
                    //     current_wage_rate + (excess / (10 * (emps + new_emps))
                    // so productivity will be
                    //     {current_wage_rate + (excess / (10 * (emps + new_emps))} / std_wage
                    // I think!

                    productivity = (current_wage_rate + (double(excess) / (10 * (emps + new_emps)))) / std_wage;
                }
            }
        }
    }
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
        double r = double(model()->getSalesTaxRate()) / 100;    // e.g. 25% -> 0.25
        if (r > 0)
        {
            double t = (amount * r) / (r + 1.0);
            if (transferSafely(model()->gov(), t, this)) {
                sales_tax_paid += t;
            }
        }
    }
}

double Firm::getProductivity()
{
    return productivity;
}

double Firm::getWagesPaid()
{
    return wages_paid;
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

double Firm::getDednsPaid()
{
    return dedns_paid;
}

size_t Firm::getNumEmployees()
{
    return model()->getNumEmployedBy(this);
}

int Firm::getNumHired()
{
    return num_hired;
}

int Firm::getNumFired()
{
    return num_fired;
}
