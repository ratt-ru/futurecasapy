Python-Casacore implemented using C++ Futures
---------------------------------------------

python-casacore doesn't drop the GIL and isn't thread safe.

- https://github.com/casacore/python-casacore/pull/209
- https://github.com/JSKenyon/QuartiCal/issues/25

The aim of this repo is to reproduce dask-ms's TableProxy and Executor classes
in C++ to solve the above issues using PyBind11.

