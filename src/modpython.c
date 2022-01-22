// Copyright (C) 2022 Vino Fernando Crescini <vfcrescini@gmail.com>
// SPDX-License-Identifier: GPL-3.0-or-later

// 2022-01-19 modpython.c

// python module to expose libdhtxx functions


#include <stdlib.h>
#include <string.h>
#include <Python.h>

#include "dhtxx.h"


static PyObject *_dhtxx_get(PyObject* self, PyObject* args);


static PyMethodDef dhtxx_methods[] =
{
  { "get", _dhtxx_get, METH_VARARGS, "Get humidity and temperature data from DHTXX sensor a the given GPIO pin" },
  { NULL, NULL, 0, NULL }
};

static struct PyModuleDef dhtxx_module =
{
  PyModuleDef_HEAD_INIT,
  "dhtxx",
  "Simple DHTXX modle",
  -1,
  dhtxx_methods
};


PyMODINIT_FUNC PyInit_dhtxx(void)
{

  return PyModule_Create(&dhtxx_module);
}


static PyObject * _dhtxx_get(PyObject* self, PyObject* args)
{
  int pin = 0;
  int stype = 0;
  float ohum = 0.0;
  float otmp = 0.0;
  char oerrstr[DHTXX_ERRSTR_MAX];

  if (!PyArg_ParseTuple(args, "ii", &pin, &stype))
  {
    PyErr_SetString(PyExc_RuntimeError, "Invalid arguments");
    return NULL;
  }

  memset(oerrstr, 0x00, DHTXX_ERRSTR_MAX);

  if (dhtxx_get(pin, stype, &ohum, &otmp, oerrstr) != 0)
  {
    PyErr_Format(PyExc_RuntimeError, "Get failed: %s", oerrstr);
    return NULL;
  }

  return Py_BuildValue("(f,f)", ohum, otmp);
}
