/**
 * Simple common interface to ObjModel exporters.
 */

#ifndef RMODELIO_OBJ_MODEL_EXPORTER_H
#define RMODELIO_OBJ_MODEL_EXPORTER_H

#include "rModelIO_Export.h"
#include <IOFormats.h>  // rlib
#include <ObjModel.h>   // RFeatures

namespace RModelIO
{

class rModelIO_EXPORT ObjModelExporter : public rlib::IOFormats
{
public:
    explicit ObjModelExporter( const RFeatures::ObjModel::Ptr);
    virtual ~ObjModelExporter(){}

    // Returns true on success. The filename extension must be supported.
    bool save( const std::string& filename);

protected:
    virtual bool doSave( const RFeatures::ObjModel::Ptr, const std::string& filename) = 0;

private:
    const RFeatures::ObjModel::Ptr _model;
};  // end class

}   // end namespace

#endif
