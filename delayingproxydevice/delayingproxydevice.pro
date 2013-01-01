TARGET=testdelayingproxydevice

CONFIG += console testcase
QT -= gui
QT += core xml testlib

macx:CONFIG -= app_bundle

HEADERS += delayingproxydevice.h testreader.h
SOURCES += delayingproxydevice.cpp testreader.cpp testdelayingproxydevice.cpp
RESOURCES += testdelayingproxydevice.qrc

test.depends = check
QMAKE_EXTRA_TARGETS += test



