/*
 * Copyright (C) 2013 Johan Thelin / Pelagicore AB
 *
 * This file is part of pyqt-qml-bridge.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 * If you have questions regarding the use of this file, please contact
 * Pelagicore at info@pelagicore.com.
 */

#include <QtQml>
#include <QQmlEngine>
#include <QQmlContext>

#include <QtDebug>

#include "pyqt-qml-plugin.h"

#include <Python.h>

#define MODULE_NAME "pluginmodule"

QQmlContext *context = 0;

static PyObject *py_registerObject(PyObject *self, PyObject *args)
{
    const char *name;
    void *ptr;
    if (!PyArg_ParseTuple(args, "sn", &name, &ptr))
        return NULL;

    QObject *s = static_cast<QObject*>(ptr); // This cast is very unsafe
    QObject *o = qobject_cast<QObject*>(s);
    if (o)
        context->setContextProperty(name, o);

    return Py_BuildValue("i", 0);
}

static PyMethodDef py_methods[] = {
    {"registerObject", py_registerObject, METH_VARARGS, "Register a PyQt-object to a QML root context"},
    {NULL, NULL, 0, NULL}
};

void PyQtQmlPlugin::registerTypes(const char *uri)
{
    // Dummy type, otherwise the initializeEngine method is not called
    qmlRegisterUncreatableType<QObject>("com.pelagicore.PyQtQmlBridge", 1, 0, "PelagicoreDummyType", "Dummy type, do not use.");
}
struct module_state {
    PyObject *error;
};

#define GETSTATE(m) ((struct module_state*)PyModule_GetState(m))

static int py_pjsua_traverse(PyObject *m, visitproc visit, void *arg) {
    Py_VISIT(GETSTATE(m)->error);
    return 0;
}

static int py_pjsua_clear(PyObject *m) {
    Py_CLEAR(GETSTATE(m)->error);
    return 0;
}

static struct PyModuleDef moduledef = {
        PyModuleDef_HEAD_INIT,
        "PyQtQmlBridge",
        NULL,
        sizeof(struct module_state),
        py_methods,
        NULL,
        py_pjsua_traverse,
        py_pjsua_clear,
        NULL
};

void PyQtQmlPlugin::initializeEngine(QQmlEngine *engine, const char *)
{
    context = engine->rootContext();
    const size_t cSize = strlen(QCoreApplication::applicationFilePath().toLocal8Bit().data())+1;
    std::wstring wc( cSize, L'#' );
    mbstowcs( &wc[0], QCoreApplication::applicationFilePath().toLocal8Bit().data(), cSize );
    Py_SetProgramName(&wc[0]);
    Py_Initialize();
    PyEval_InitThreads();
    (void)PyModule_Create(&moduledef);

    // Load module
    PyObject *pyName = PyUnicode_FromString(MODULE_NAME);
    PyObject *pyModule = PyImport_ImportModule(MODULE_NAME);
    Py_XDECREF(pyName);

    if (!pyModule)
    {
        PyErr_Print();
        qWarning("Failed to import module!");
        return;
    }

    // Call the factory and let the module register whatever it likes to the root context
    PyObject *pyFactory = PyObject_GetAttrString(pyModule, "initializeRootContext");
    if (!pyFactory || !PyCallable_Check(pyFactory))
    {
        qWarning("Failed to reference factory!");
        return;
    }

    PyObject_CallObject(pyFactory, 0);
    PyEval_SaveThread();
}
