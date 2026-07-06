QT       += core gui widgets printsupport
QT       += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17 console


SOURCES += \
    ModelARX.cpp \
    ProstyUAR.cpp \
    RegulatorPID.cpp \
    SignalGenerator.cpp \
    klient.cpp \
    main.cpp \
    mainwindow.cpp \
    oknoarx.cpp \
    oknosiec.cpp \
    qcustomplot.cpp \
    serwer.cpp
HEADERS += \
    ModelARX.h \
    ProstyUAR.h \
    RegulatorPID.h \
    SignalGenerator.h \
    WarstwaU.h \
    klient.h \
    mainwindow.h \
    oknoarx.h \
    oknosiec.h \
    qcustomplot.h \
    serwer.h
FORMS += \
    mainwindow.ui \
    oknoarx.ui \
    oknosiec.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target


RESOURCES += \
    zasoby.qrc
