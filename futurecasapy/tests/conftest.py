import os

import numpy as np
import pytest

@pytest.fixture
def ms(tmp_path_factory):
    import pyrap.tables as pt
    msdir = tmp_path_factory.mktemp("msdir", numbered=True)
    fn = os.path.join(str(msdir), "test.ms")

    create_table_query = """
    CREATE TABLE %s
    [FIELD_ID I4,
    ANTENNA1 I4,
    ANTENNA2 I4,
    DATA_DESC_ID I4,
    SCAN_NUMBER I4,
    STATE_ID I4,
    TIME R8,
    DATA C8 [NDIM=2, SHAPE=[16, 4]]]
    LIMIT 10
    """ % fn

    # Common grouping columns
    field = [0,   0,   0,   1,   1,   1,   1,   2,   2,   2]
    ddid = [0,   0,   0,   0,   0,   0,   0,   1,   1,   1]
    scan = [0,   1,   0,   1,   0,   1,   0,   1,   0,   1]

    # Common indexing columns
    time = [1.0, 0.9, 0.8, 0.7, 0.6, 0.5, 0.4, 0.3, 0.2, 0.1]
    ant1 = [0,   0,   1,   1,   1,   2,   1,   0,   0,   1]
    ant2 = [1,   2,   2,   3,   2,   1,   0,   1,   1,   2]

    # Column we'll write to
    state = [0,   0,   0,   0,   0,   0,   0,   0,   0,   0]

    rs = np.random.RandomState(42)
    data_shape = (len(state), 16, 4)
    data = rs.random_sample(data_shape) + rs.random_sample(data_shape)*1j

    # Create the table
    with pt.taql(create_table_query) as ms:
        ms.putcol("FIELD_ID", field)
        ms.putcol("DATA_DESC_ID", ddid)
        ms.putcol("ANTENNA1", ant1)
        ms.putcol("ANTENNA2", ant2)
        ms.putcol("SCAN_NUMBER", scan)
        ms.putcol("STATE_ID", state)
        ms.putcol("TIME", time)
        ms.putcol("DATA", data)

    yield fn

    # Remove the temporary directory
    # except it causes issues with casacore files on py3
    # https://github.com/ska-sa/dask-ms/issues/32
    # shutil.rmtree(str(msdir))

