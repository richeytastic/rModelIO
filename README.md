# rModelIO
Wraps the Open Asset Importer libraries

## Prerequisites
- [AssImp](https://github.com/assimp)

    Tested with version 3.3.1 *NB* When configuring using CMake on Windows:
    - *UNCHECK* `AddGTest_FOUND`
    - *UNCHECK* `ASSIMP_BUILD_TESTS`
    - *UNCHECK* `ASSIMP_BUILD_ASSIMP_VIEW` (references deprecated DirectX SDK)

- pdflatex

    Part of the [MiKTeK](https://miktex.org/) distribution on Windows, but usually
    installed in /usr/bin if installed systemwide on Linux as part of
    [TeX Live](https://www.tug.org/texlive/)

- [IDTFConverter](https://www2.iaas.msu.ru/tmp/u3d/u3d-1.4.5_current.zip)
    (with thanks to Michail Vidiassov)

- [rFeatures](../../../rFeatures)
