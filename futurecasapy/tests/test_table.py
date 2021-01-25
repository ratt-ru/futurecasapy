import numpy as np
import pytest

from futurecasapy.pytable import (Table,
                                  record_convert,
                                  valueholder_convert,
                                  string_convert,
                                  casa_string_convert)

@pytest.mark.parametrize("ms", ["/home/sperkins/data/WSRT.MS"])
def test_casa_table_open(ms):
    table = Table(ms, {"option": "auto"}, 5)
    import pdb; pdb.set_trace()
    assert table._nrows() == 6552

@pytest.mark.parametrize("inputs", [
    {"hello": {"this": {"brave": {"new": "world", "with": 456.0, "lots of": 2, "stuff": 3 + 4j}}}},
    {"hello": {"this": {"brave": {"new": "world", "with": 456.0, "stuff": 3 + 4j}}}},
    {"hello": {"this": {"brave": {"new": "world"}}}},
    {"hello": "world"},
    {"pants": 1},
    {},
])
def test_record_convert(inputs):
    assert inputs == record_convert(inputs)


@pytest.mark.parametrize("inputs", [
    "Septimus Thripp",
])
@pytest.mark.parametrize("string_type", ["std", "casa"])
def test_string_convert(inputs, string_type):
    if string_type == "std":
        result = string_convert(inputs)
    elif string_type == "casa":
        result = casa_string_convert(inputs)
    else:
        raise ValueError(f"Invalid string_type {string_type}")

@pytest.mark.parametrize("inputs", [
    "Septimus Thripp",
    "Severus Snape",
    1,
    2.0,
    3.0 + 4.0j,
    {"hello": "world"},
    {"hello": {"this": {"brave": {"new": "world"}}}},
    {"hello": {"this": {"brave": {"new": "world", "with": 456.0, "lots of": 2, "stuff": 3 + 4j}}}},
    np.array([[0, 1, 2], [3, 4, 5]], np.complex64).T,
    np.array([[0, 1, 2], [3, 4, 5]], np.complex128).T,
])
def test_valueholder_convert(inputs):
    result = valueholder_convert(inputs)
    assert result == inputs
    # print(type(result))
    # print(result.shape, inputs.shape, result.dtype, result.data, len(result.data))

