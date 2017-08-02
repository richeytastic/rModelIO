# rModelIO
Wraps the Open Asset Importer libraries

## Prerequisites
- [rFeatures](../../../rFeatures)
- [AssImp](https://github.com/assimp)

  Tested with version 3.3.1 *NB* When configuring using CMake on Windows:
  - *UNCHECK* `AddGTest_FOUND`
  - *UNCHECK* `ASSIMP_BUILD_TESTS`
  - *UNCHECK* `ASSIMP_BUILD_ASSIMP_VIEW` (references deprecated DirectX SDK)
