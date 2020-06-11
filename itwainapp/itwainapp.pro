#-------------------------------------------------
#
# Project created by QtCreator 2019-01-10T09:22:11
#
#-------------------------------------------------
TEMPLATE = lib
#QT       += opengl
QT       -= gui
CONFIG += c++11
CONFIG -= qt
TARGET = itwainapp


DEFINES += ITWAINAPP_LIBRARY
DEFINES += QT_DEPRECATED_WARNINGS

SOURCES += \        
    DSMInterface.cpp \
    twainAppCMD.cpp \
    VerifySignature.cpp \
    common/CommonTWAIN.cpp \
    common/CTiffWriter.cpp \
    common/TwainString.cpp \
    TwainApp.cpp     

HEADERS += \
    DSMInterface.h \
    TwainApp.h \
    TwainAppCMD.h \
    common/Common.h \
    common/CommonTWAIN.h \
    common/CTiffWriter.h \
    common/TwainString.h \
    common/twain.h

win32{
DEFINES += TWNDS_CMP  TWNDS_CMP_MINGW
LIBS += -L$$PWD/../lib/ -lFreeImagex64
}
unix {
    target.path = /usr/lib
    INSTALLS += target
}
INCLUDEPATH += $$PWD/../include
DEPENDPATH += $$PWD/../include

DISTFILES += \
    itwainapp.pro.user
