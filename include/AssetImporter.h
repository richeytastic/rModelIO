#ifndef RMODELIO_ASSET_IMPORTER_H
#define RMODELIO_ASSET_IMPORTER_H

/**
 * Wraps assimp library to import model data (e.g. .obj files)
 * Richard Palmer
 * August 2014
 */

#include "rModelIO_Export.h"
#include <ObjModel.h>   // RFeatures
#include <string>
#include <boost/filesystem.hpp>

namespace RModelIO
{

class rModelIO_EXPORT AssetImporter
{
public:
    // Read in the model - set readTextures true to read in the texture images.
    // Returns NULL on error - use getError() to get the error string.
    static RFeatures::ObjModel::Ptr read( const std::string& fname, bool readTextures=false);
    static const std::string& getError();

    // Set whether to fail or not if a model is imported containing at least one non-triangular polygon.
    // If setReadFailOnNonTriangles(true) called before read() called, read will return NULL if any
    // non-triangular polygons are found in the model file. Whether or not this function is called
    // prior to importing models, warnings about any non-triangular faces found will be printed to stderr.
    // NB AssImp will try to triangulate all models on import.
    static void setReadFailOnNonTriangles( bool);

    // Get the valid filename extensions for reading from along with their descriptions.
    // Extensions given without periods and in lower case(e.g. obj, 3ds).
    // Maps extensions (keys) to file descriptions (values).
    static const boost::unordered_map<std::string, std::string>& getImportFormats();

    static void free(); // Free up dynamically allocated singleton

    static RFeatures::ObjModel::Ptr createDoublePyramid( float scale=70, std::string txfile=""); // DDEBUG

private:
    std::string _errStr;
    boost::unordered_map<std::string, std::string> _importFormats;

    static AssetImporter* s_instance;
    static AssetImporter* get();    // Singleton instance
    AssetImporter();
    AssetImporter( const AssetImporter&);   // NO COPY
    void operator=( const AssetImporter&);  // NO COPY
};  // end class

}   // end namespace

#endif
