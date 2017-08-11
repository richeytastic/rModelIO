/************************************************************************
 * Copyright (C) 2017 Richard Palmer
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 ************************************************************************/

/**
 * Export RFeatures::ObjModel objects to U3D format via creation
 * of IDTF files (see RModelIO::IDTFExporter).
 *
 * Static member IDTFConverter must be set to the IDTFConverter executable
 * that can convert .idtf files to .u3d files. Can be found at
 * https://www2.iaas.msu.ru/tmp/u3d/ (thanks to Michail Vidiassov).
 * If IDTFConverter is empty upon construction, a warning is shown to
 * stderr and U3D export functionality for the instance will be disabled.
 *
 * Richard Palmer
 * August 2017
 */

#ifndef RMODELIO_U3D_EXPORTER_H
#define RMODELIO_U3D_EXPORTER_H

#include "ObjModelExporter.h"

namespace RModelIO
{

class rModelIO_EXPORT U3DExporter : public ObjModelExporter
{
public:
    static std::string IDTFConverter;   // Must be set before use

    explicit U3DExporter( const RFeatures::ObjModel::Ptr);
    virtual ~U3DExporter(){}

protected:
    virtual bool doSave( const std::string& filename);
};  // end class

}   // end namespace

#endif
