# cython: language_level=3, boundscheck=False

from setuptools import setup
from Cython.Build import cythonize

setup(
    ext_modules = cythonize("raw_record.pyx")
)

