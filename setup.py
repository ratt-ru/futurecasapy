# -*- coding: utf-8 -*-
from setuptools import setup

packages = \
['futurecasapy', 'futurecasapy.tests']

package_data = \
{'': ['*']}

setup_kwargs = {
    'name': 'futurecasapy',
    'version': '0.1.0',
    'description': '',
    'long_description': None,
    'author': 'Simon Perkins',
    'author_email': 'simon.perkins@gmail.com',
    'maintainer': None,
    'maintainer_email': None,
    'url': None,
    'packages': packages,
    'package_data': package_data,
    'python_requires': '>=3.6,<4.0',
}
from build import *
build(setup_kwargs)

setup(**setup_kwargs)
