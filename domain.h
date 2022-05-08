#ifndef DOMAIN_H
#define DOMAIN_H

#include "behaviour.h"
#include <QObject>

// NEXT: This class should contain the main driver code to trigger its
// components in response to a clock maintained by MainWindow. Also, allow
// clock tick frequency to be set as a global option so we can put time
// durations on the x-axes of the charts. Suggested default: one tick per week.


class Domain : public QObject
{
    Q_OBJECT
public:
    Domain(const QString &name, Behaviour *behaviour, QString currency, QString currencyAbbrev);

    Behaviour *getBehaviour() const;
    Bank *getCentral_bank() const;
    const QString &getName() const;

private:
    QString _name;
    QString _currency;
    QString _abbrev;
    Behaviour *_behaviour;
    Bank *_centralBank;

signals:

};

#endif // DOMAIN_H
