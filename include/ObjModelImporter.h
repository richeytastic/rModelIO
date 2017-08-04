/**
 * Simple common interface to ObjModel importers.
 */

#ifndef RMODELIO_OBJ_MODEL_IMPORTER_H
#define RMODELIO_OBJ_MODEL_IMPORTER_H

#include "rModelIO_Export.h"
#include <IOFormats.h>  // rlib
#include <ObjModel.h>   // RFeatures

namespace RModelIO
{

class rModelIO_EXPORT ObjModelImporter : public rlib::IOFormats
{
public:
    virtual ~ObjModelImporter(){}

    // On error, NULL object returned. The filename extension must be supported.
    RFeatures::ObjModel::Ptr load( const std::string& filename);

protected:
    virtual RFeatures::ObjModel::Ptr doLoad( const std::string& filename) = 0;
};  // end class

}   // end namespace

#endif
