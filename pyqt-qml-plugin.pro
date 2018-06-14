# Building a qml extension plugin
TEMPLATE = lib
CONFIG += qt plugin
QT += qml

# Build outside the source tree
MOC_DIR = build/moc
RCC_DIR = build/rcc
OBJECTS_DIR = build/obj
UI_DIR = build/uic

# Paths
INCLUDEPATH += c:\python36\include
DEPENDPATH += .

# Sources
SOURCES += pyqt-qml-plugin.cpp
HEADERS += pyqt-qml-plugin.h
OTHER_FILES += pyqt-qml-plugin.json qmldir

# Target
TARGET = pyqt-qml-plugin

qmldir.files = $$PWD/qmldir

target.path = $$[QT_INSTALL_QML]/com/pelagicore/PyQtQmlBridge
qmldir.path = $$[QT_INSTALL_QML]/com/pelagicore/PyQtQmlBridge

INSTALLS += target qmldir

LIBS += -Lc:\python36\libs

# Python specifics
CONFIG += link_pkgconfig
#PKGCONFIG += python
CONFIG += no_keywords
