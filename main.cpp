#include "mainwindow.h"
#include <QApplication>
#include <stdio.h>
#include <stdlib.h>
#include <QIcon>

void myMessageOutput(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    QByteArray localMsg = msg.toLocal8Bit();
    switch (type) {
    case QtDebugMsg:
        fprintf(stderr, "Debug: %s (%s:%u, %s)\n", localMsg.constData(), context.file, context.line, context.function);
        break;
    case QtInfoMsg:
        fprintf(stderr, "Info: %s (%s:%u, %s)\n", localMsg.constData(), context.file, context.line, context.function);
        break;
    case QtWarningMsg:
        fprintf(stderr, "Warning: %s (%s:%u, %s)\n", localMsg.constData(), context.file, context.line, context.function);
        break;
    case QtCriticalMsg:
        fprintf(stderr, "Critical: %s (%s:%u, %s)\n", localMsg.constData(), context.file, context.line, context.function);
        break;
    case QtFatalMsg:
        fprintf(stderr, "Fatal: %s (%s:%u, %s)\n", localMsg.constData(), context.file, context.line, context.function);
        abort();
    }
}
#if 0
void myMessageOutput(QtMsgType type, const QMessageLogContext&, const QString&msg)
{
    QByteArray localMsg = msg.toLocal8Bit();
    switch (type) {
    case QtDebugMsg:
        fprintf(stderr, "Debug: %s\n", localMsg.constData());
        break;
    case QtInfoMsg:
        fprintf(stderr, "Info: %s\n", localMsg.constData());
        break;
    case QtWarningMsg:
        fprintf(stderr, "Warning: %s\n", localMsg.constData());
        break;
    case QtCriticalMsg:
        fprintf(stderr, "Critical: %s\n", localMsg.constData());
        break;
    case QtFatalMsg:
        fprintf(stderr, "Fatal: %s\n", localMsg.constData());
        abort();
    }
}
#endif

int main(int argc, char *argv[])
{
    // Set up a custom message handler
    qInstallMessageHandler(myMessageOutput);

    // Create the application
    QApplication a(argc, argv);

    // NOTE: Most of the info about this (the application icon), including in
    // Qt forums, seems to be out of date. The only scheme that seems to work
    // is to put the icns file into resources and associate it with the
    // application as here. I have also left it in the .pro file, as instructed
    // by Qt, as well as manually adding a reference to it in the plist.info.
    // Whether either or both of these is/are actually necessary is unclear.
    //
    // Later: This only works while the program is actually running. The icon
    // that appears in the dock after the program has been closed is the
    // default icon, so clearly there is something wrong with the configuration
    // that will need to be sorted out.
    //
    // Still later: the key is to manually delete the generated app bundle,
    // rerun qmake, and rebuild (see
    // https://stackoverflow.com/questions/4739175/qt-c-on-mac-application-icon-doesnt-set).
    // Even this isn't quite enough, actually -- the old icon needs to be
    // removed from the dock (trashing it seems safest) so the new one can be
    // added.
    QApplication::setWindowIcon(QIcon(":/obson.icns"));

    QCoreApplication::setOrganizationName("Obson.net");
    QCoreApplication::setOrganizationDomain("Obson.net");
    QCoreApplication::setApplicationName("MicroSim");

    // We want settings to be held in an INI file so we can edit them manually.
    // The file will be called MicroSim.ini
    QSettings::setDefaultFormat(QSettings::IniFormat);

    QSettings settings;
    settings.setFallbacksEnabled(false);

    MainWindow mainwindow;
    //mainwindow.setWindowIcon(QIcon(":/obson.icns"));
    mainwindow.show();

    return a.exec();
}
