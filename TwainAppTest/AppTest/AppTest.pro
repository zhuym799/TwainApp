TEMPLATE = app
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += \
        main.cpp \
    mytestapp.cpp

win32: LIBS += -L$$PWD/../lib/ -lFreeImagex64

INCLUDEPATH += $$PWD/../include
DEPENDPATH += $$PWD/../include

win32: LIBS += -L$$PWD/../lib/ -litwainapp

INCLUDEPATH += $$PWD/../include
DEPENDPATH += $$PWD/../include

HEADERS += \
    mytestapp.h

win32{
DEFINES += TWNDS_CMP  TWNDS_CMP_MINGW
}
