import pytest

from futurecasapy.pytable import Table, record_example

def test_casa_table_open(ms):
    ms = "/home/sperkins/data/WSRT.ms"
    print(ms)
    table = Table(ms, {"option": "auto"}, 5)
    assert table._nrows() == 6552

@pytest.mark.parametrize("inputs", [
    {"hello": {"this": {"brave": {"new": "world", "with": 456.0, "lots of": 2, "stuff": 3 + 4j}}}},
    # {"hello": {"this": {"brave": {"new": "world", "with": 456.0, "stuff": 3 + 4j}}}},
    {"hello": {"this": {"brave": {"new": "world"}}}},
    {"hello": "world"},
    {"pants": 1},
    {},
])
def test_record(inputs):
    record_example(inputs)
