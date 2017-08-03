/**
 * Export model data in U3D format using the VGC library.
 * Richard Palmer
 * August 2017
 */

#ifndef RMODELIO_U3D_EXPORTER_H
#define RMODELIO_U3D_EXPORTER_H

#include "ObjModelExporter.h"
#include "VCGTypes.h"

namespace RModelIO
{

class rModelIO_EXPORT U3DExporter : public ObjModelExporter
{
public:
    explicit U3DExporter( const RFeatures::ObjModel::Ptr);
    virtual ~U3DExporter(){}

protected:
    virtual bool doSave( const RFeatures::ObjModel::Ptr, const std::string& filename);

private:
    U3DExporter( const U3DExporter&);     // NO COPY
    void operator=( const U3DExporter&);  // NO COPY
};  // end class

}   // end namespace

#endif
