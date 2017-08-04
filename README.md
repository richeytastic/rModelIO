# rModelIO
Wraps the Open Asset Importer libraries

## Prerequisites
- [rFeatures](../../../rFeatures)
- [AssImp](https://github.com/assimp)

    Tested with version 3.3.1 *NB* When configuring using CMake on Windows:
    - *UNCHECK* `AddGTest_FOUND`
    - *UNCHECK* `ASSIMP_BUILD_TESTS`
    - *UNCHECK* `ASSIMP_BUILD_ASSIMP_VIEW` (references deprecated DirectX SDK)

- [vcglib](http://vcg.isti.cnr.it/vcglib)

    Required for U3D exporting. Importing not currently supported.

- [Qt 5.7](https://www.qt.io)

    Only required because it's a dependency of VCG!
