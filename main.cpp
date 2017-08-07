#include "mainwindow.h"
#include <QApplication>
//#include <qapplication.h>

#include <stdio.h>
#include <stdlib.h>

// TODO: Set up version control to use GitHub.

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

int main(int argc, char *argv[])
{
    qInstallMessageHandler(myMessageOutput);
    QApplication a(argc, argv);

    QCoreApplication::setOrganizationName("Obson.net");
    QCoreApplication::setOrganizationDomain("Obson.net");
    QCoreApplication::setApplicationName("MicroSim");

    QSettings::setDefaultFormat(QSettings::IniFormat);
    MainWindow mainwindow;
    mainwindow.show();

    return a.exec();
}
