/**
 * Export model data using the AssImp library.
 */

#ifndef RMODELIO_ASSET_EXPORTER_H
#define RMODELIO_ASSET_EXPORTER_H

#include "ObjModelExporter.h"

namespace RModelIO
{

class rModelIO_EXPORT AssetExporter : public ObjModelExporter
{
public:
    explicit AssetExporter( const RFeatures::ObjModel::Ptr);
    virtual ~AssetExporter(){}

protected:
    virtual bool doSave( const RFeatures::ObjModel::Ptr, const std::string& filename);
    virtual void populateFormats(); // Overrides rlib::IOFormats::populateFormats
};  // end class

}   // end namespace

#endif
