TEMPLATE = app
CONFIG += console c++17
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += \
        formula.cpp \
        main.cpp \
        resolution.cpp

HEADERS += \
    formula.h \
    interpretation.h \
    resolution.h
