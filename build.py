from glob import glob
from pybind11.setup_helpers import Pybind11Extension
import numpy

ext_modules = [
    Pybind11Extension(
        "futurecasapy.pytable",
        sorted(glob("src/*.cpp")),  # Sort source files for reproducibility
        include_dirs=[numpy.get_include()],
        extra_compile_args=["-O0", "-g"],
        libraries=["casa_derivedmscal", "casa_meas", "casa_ms", "casa_tables"],
    ),
]


def build(setup_kwargs):
    setup_kwargs.update(
        {
            "ext_modules": ext_modules,
            "zip_safe": False,
        }
    )