#include "mainwindow.h"
#include <QApplication>
#include <stdio.h>
#include <stdlib.h>
#include <QIcon>

#ifdef QT_DEBUG
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
#else
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
    qInstallMessageHandler(myMessageOutput);
    QApplication a(argc, argv);

    // NOTE: Most of the info about this, including in Qt forums, seems to be
    // out of date. The only scheme that seems to work is to put the icns file
    // into resources and associate it with the application as here. I have
    // also left it in the .pro file, as instructed by Qt, as well as manually
    // adding a reference to it in the plist.info. Whether either or both of
    // these is/are actually necessary is unclear.
    QApplication::setWindowIcon(QIcon(":/microsim.icns"));

    QCoreApplication::setOrganizationName("Obson.net");
    QCoreApplication::setOrganizationDomain("Obson.net");
    QCoreApplication::setApplicationName("MicroSim");

    QSettings::setDefaultFormat(QSettings::IniFormat);
    QSettings settings;
    settings.setFallbacksEnabled(false);
    MainWindow mainwindow;
    //mainwindow.setWindowIcon(QIcon(":/microsim.icns"));
    mainwindow.show();

    return a.exec();
}
