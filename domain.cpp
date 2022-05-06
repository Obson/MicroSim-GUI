#include "domain.h"
#include "QtCore/qdebug.h"

Domain::Domain(
        const QString &name,
        Behaviour *behaviour,
        QString currency,
        QString currencyAbbrev
        )
{
    qDebug() << "Domain::Domain("
             << name
             << ","
             << behaviour->name()
             << ")";

    // Update settings
    QSettings settings;
    settings.beginGroup("Domains");
    settings.beginGroup(name);
    settings.setValue("Behaviour", behaviour->name());
    settings.setValue("Currency", currency);
    settings.setValue("Abbrev", currencyAbbrev);

    settings.endGroup();
    settings.endGroup();
}

Behaviour *Domain::getBehaviour() const
{
    return _behaviour;
}

Bank *Domain::getCentral_bank() const
{
    return _centralBank;
}

const QString &Domain::getName() const
{
    return _name;
}

