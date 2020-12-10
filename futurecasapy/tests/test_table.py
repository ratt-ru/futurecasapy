import pytest

from futurecasapy.pytable import Table, record_example

@pytest.mark.parametrize("ms", ["/home/sperkins/data/WSRT.MS"])
def test_casa_table_open(ms):
    table = Table(ms, {"option": "auto"}, 5)
    assert table._nrows() == 6552

@pytest.mark.parametrize("inputs", [
    {"hello": {"this": {"brave": {"new": "world", "with": 456.0, "lots of": 2, "stuff": 3 + 4j}}}},
    {"hello": {"this": {"brave": {"new": "world", "with": 456.0, "stuff": 3 + 4j}}}},
    {"hello": {"this": {"brave": {"new": "world"}}}},
    {"hello": "world"},
    {"pants": 1},
    {},
])
def test_record(inputs):
    record_example(inputs)


def test_construct_record():
    from futurecasapy.pytable import test_record
    test_record()
