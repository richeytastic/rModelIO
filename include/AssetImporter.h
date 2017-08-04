/**
 * Wraps assimp library to import model data (e.g. .obj files)
 */

#ifndef RMODELIO_ASSET_IMPORTER_H
#define RMODELIO_ASSET_IMPORTER_H

#include "ObjModelImporter.h"

namespace RModelIO
{

class rModelIO_EXPORT AssetImporter : public ObjModelImporter
{
public:
    // Set loadTextures true to read in the textures if available.
    // Set whether to fail or not if a model is imported containing at least one non-triangular polygon.
    // If setReadFailOnNonTriangles(true) called before read() called, read will return NULL if any
    // non-triangular polygons are found in the model file. Whether or not this function is called
    // prior to importing models, warnings about any non-triangular faces found will be printed to stderr.
    // NB AssImp will try to triangulate all models on import.
    AssetImporter( bool loadTextures=true, bool failOnNonTriangles=true);
    virtual ~AssetImporter(){}

protected:
    virtual RFeatures::ObjModel::Ptr doLoad( const std::string& filename);
    virtual void populateFormats();

private:
    bool _loadTextures;
    bool _failOnNonTriangles;
};  // end class

}   // end namespace

#endif
