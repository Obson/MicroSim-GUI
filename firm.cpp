
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
    amount_granted = 0;
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
        balance += amount_granted;

        if (!_state_supported)
        {
            // Pay interest on bank loans.
            // TODO: At present we never pay off the loan. We should have a
            // parameter that indicates the factor by which the balance has to
            // exceed the outstanding loan before we will repay it. Or
            // something...
            int interest = (owed_to_bank * interest_rate) / 100;

            // If interest is greater than balance we'll just skip the payment
            // and hope for better luck next time. A bank is unlikely to
            // foreclose unless this happens a lot...
            if (interest > 0 && interest <= balance)
            {
                transferSafely(model()->bank(), interest, this);
            }
        }

        if (balance < 0) {
            balance = balance;
        }

        wages_paid += model()->payWages(this, period);
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
    if (balance > wages_paid)
    {
        // It seems odd that a government-supported industry should have to pay
        // pre-tax deductions to the government but it supports the mythology
        // that they amount to an insurance premium
        int available = balance - wages_paid;   // now includes deductions

        int investible, bonuses;
        if (_state_supported)
        {
            investible = available;
            bonuses = 0;
        }
        else
        {
            investible = (available * model()->getPropInv()) / 100;
            bonuses = ((available - investible) * model()->getDistributionRate()) / 100;
        }

        // We distribute the funds before hiring new workers to ensure they only
        // get distributed to existing workers.
        if (!_state_supported)
        {
            int emps = model()->getNumEmployedBy(this);
            int amt_paid = 0;
            if (emps > 0)
            {
                amt_paid = model()->payWorkers(bonuses/emps, bonuses, this, Model::for_bonus);
            }

            // Adjust calculation if not all the bonus funds were used
            if (amt_paid < bonuses)
            {
                investible += (bonuses_paid);
            }

            balance -= amt_paid;
            Q_ASSERT(balance >= 0);

            bonuses_paid += amt_paid;
        }

        // How many more employees can we afford?
        int wage = model()->getStdWage();
        int num_to_hire = (investible * 100) /
                (wage * (100 + model()->getPreTaxDedns()));

        if (num_to_hire > 0)
        {
            model()->hireSome(this, wage, period, num_to_hire);
        }
    }
}

void Firm::credit(int amount, Account *creditor)
{
    Account::credit(amount);

    // If state-supported the reason we are being credited must be that we have
    // asked for additional support to pay wages, in which case we have now
    // finished. If we are not state-supported the credit will be for a sale,
    // and sales tax is therefore due. This could change if we allow state-
    // supported firms to be commercial, in which case a better check might
    // be to see whether the creditor is the government.

    if (!creditor->isGovernment())
    {
        sales_receipts += amount;

        // Base class credits account but doesn't pay tax. We assume seller,
        // not buyer, is responsible for paying sales tax and that payments
        // to a Firm are always for purchases and therefore subject to
        // sales tax.
        int r = model()->getSalesTaxRate();    // e.g. 25%
        if (r > 0)
        {
            int r_eff = (100 * r) / (100 + r);      // e.g. 20%
            int t = (amount * r_eff) / 100;
            if (transferSafely(model()->gov(), t, this)) {
                sales_tax_paid += t;
            }
        }
    }
}

void Firm::grant(int amount)
{
    amount_granted = amount;
}

int Firm::getAmountGranted()
{
    return amount_granted;
}

int Firm::getWagesPaid()
{
    return wages_paid;
}

int Firm::getBonusesPaid()
{
    return bonuses_paid;
}

int Firm::getSalesTaxPaid()
{
    return sales_tax_paid;
}

int Firm::getSalesReceipts()
{
    return sales_receipts;
}

int Firm::getDednsPaid()
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
