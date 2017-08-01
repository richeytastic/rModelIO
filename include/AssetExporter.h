/**
 * Export model data using the AssImp library.
 * Richard Palmer
 * August 2014
 */

#ifndef RMODELIO_ASSET_EXPORTER_H
#define RMODELIO_ASSET_EXPORTER_H

#include "rModelIO_Export.h"
#include <ObjModel.h>   // RFeatures
#include <string>
#include <boost/unordered_map.hpp>

namespace RModelIO
{

class rModelIO_EXPORT AssetExporter
{
public:
    // Format set by filename - returns false if invalid (use getError() to return problem description).
    static bool write( const std::string& filename, const RFeatures::ObjModel::Ptr&);
    static const std::string& getError();

    // Get the valid filename extensions for writing to along with their descriptions.
    // Extensions given without periods and in lower case(e.g. obj, 3ds).
    // Maps extensions (keys) to file descriptions (values).
    static const boost::unordered_map<std::string, std::string>& getExportFormats();

    static void free(); // Free dynamically allocated memory

private:
    std::string _errStr;
    boost::unordered_map<std::string, std::string> _exportFormats;  // Extensions mapped to descriptions

    static AssetExporter* s_instance;
    static AssetExporter* get();
    AssetExporter();
    ~AssetExporter();
    AssetExporter( const AssetExporter&);   // NO COPY
    void operator=( const AssetExporter&);  // NO COPY
};  // end class

}   // end namespace

#endif
