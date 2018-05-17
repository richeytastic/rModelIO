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
 * The IDTFConverter that can convert .idtf files to .u3d files must be
 * available on the PATH. IDTFConverter can be found at
 * https://www2.iaas.msu.ru/tmp/u3d/ (thanks to Michail Vidiassov).
 *
 * Richard Palmer
 * August 2017
 */

#ifndef RMODELIO_U3D_EXPORTER_H
#define RMODELIO_U3D_EXPORTER_H

#include "ObjModelExporter.h"

namespace RModelIO {

class rModelIO_EXPORT U3DExporter : public ObjModelExporter
{
public:
    // Defines name of the IDTFConverter program which must be on the path.
    // Defaults to "IDTFConverter" ("IDTFConverter.exe" on Windows).
    static std::string IDTFConverter;   

    // Returns true iff IDTFConverter is on the PATH.
    static bool isAvailable();

    // U3D conversion produces an IDTF file and a tga texture.
    // Normally, both are destroyed immediately after saving the
    // U3D model. Set delOnDestroy to false to retain these files.
    explicit U3DExporter( bool delOnDestroy=true);
    virtual ~U3DExporter(){}

protected:
    virtual bool doSave( const RFeatures::ObjModel*, const std::string& filename);

private:
    const bool _delOnDestroy;
};  // end class

}   // end namespace

#endif
