# rModelIO
Import/export 3D models from/to various formats, and provides cross platform functionality
to export 3D models to PDF (via suitable pdflatex processor).

## Prerequisites
- [rFeatures](../../../rFeatures)
- [Boost](http://www.boost.org) At least 1.64+ required for boost::process.

- [AssImp](https://github.com/assimp)
    Requires version 4.1 *NB* When configuring using CMake on Windows:
    - *UNCHECK* `AddGTest_FOUND`
    - *UNCHECK* `ASSIMP_BUILD_TESTS`
    - *UNCHECK* `ASSIMP_BUILD_ASSIMP_VIEW` (references deprecated DirectX SDK)

- pdflatex
    Optionally required for PDF generation from LaTeX files - must be on the PATH.
    Part of the [MiKTeK](https://miktex.org/) distribution on Windows, but usually
    installed in /usr/bin if installed systemwide on Linux as part of
    [TeX Live](https://www.tug.org/texlive/)

- [IDTFConverter](https://www2.iaas.msu.ru/tmp/u3d/u3d-1.4.5_current.zip)
    (with thanks to Michail Vidiassov)

    Optionally required for conversion of IDTF format models to U3D models
    (usually prior to embedding in PDFs via creation of a suitable LaTeX
    file before processing by pdflatex and the media9 package).
