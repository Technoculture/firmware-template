# Configuration file for the Sphinx documentation builder.
#
# For the full list of built-in configuration values, see the documentation:
# https://www.sphinx-doc.org/en/master/usage/configuration.html

# -- Project information -----------------------------------------------------
# https://www.sphinx-doc.org/en/master/usage/configuration.html#project-information

project = 'Firmware Template'
copyright = '2024, Technoculture Research Pvt. Ltd.'
author = 'Technoculture Research Pvt. Ltd.'
release = '1.0.0'

# -- General configuration ---------------------------------------------------
# https://www.sphinx-doc.org/en/master/usage/configuration.html#general-configuration

extensions = ['breathe', 'sphinx.ext.intersphinx']

templates_path = ['_templates']
exclude_patterns = ['_build_sphinx', 'Thumbs.db', '.DS_Store']

# -- Options for HTML output -------------------------------------------------
# https://www.sphinx-doc.org/en/master/usage/configuration.html#options-for-html-output

html_theme = 'alabaster'

# -- Options for Intersphinx -------------------------------------------------
# https://www.sphinx-doc.org/en/master/usage/extensions/intersphinx.html

intersphinx_mapping = {'zephyr': ('https://docs.zephyrproject.org/latest', None)}

## -- Options for Breathe ----------------------------------------------------
# https://breathe.readthedocs.io/en/latest/index.html
#
# WARNING: please, check breathe maintainership status before using this
# extension in production!

breathe_projects = {'firmware-template': '_build_doxygen/xml'}
breathe_default_project = 'firmware-template'
breathe_default_members = ('members', )
