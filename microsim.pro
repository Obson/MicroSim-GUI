#-------------------------------------------------
#
# Project created by QtCreator 2017-07-21T23:29:06
#
#-------------------------------------------------

QT      += core gui
QT      += charts

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = Obson
TEMPLATE = app

CONFIG += c++11

# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS
# DEFINES += QT_NO_DEBUG_OUTPUT

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0


SOURCES += \
        main.cpp \
    mainwindow.cpp \
    parameterwizard.cpp \
    newmodeldlg.cpp \
    account.cpp \
    worker.cpp \
    firm.cpp \
    government.cpp \
    bank.cpp \
    model.cpp \
    optionsdialog.cpp \
    removemodeldlg.cpp \
    saveprofiledialog.cpp \
    statsdialog.cpp \
    removeprofiledialog.cpp

HEADERS += \
    mainwindow.h \
    parameterwizard.h \
    newmodeldlg.h \
    account.h \
    model.h \
    optionsdialog.h \
    removemodeldlg.h \
    version.h \
    saveprofiledialog.h \
    statsdialog.h \
    removeprofiledialog.h

FORMS += \
    newmodeldlg.ui \
    optionsdialog.ui \
    removemodeldlg.ui \
    saveprofiledialog.ui \
    statsdialog.ui \
    removeprofiledialog.ui

DISTFILES += \
    README.md

RESOURCES += \
    microsim.qrc

ICON = obson.icns
