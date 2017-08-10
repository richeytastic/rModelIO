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
 * Export model to Intermediate Data Text Format (IDTF). Precursor to U3D format.
 */

#ifndef RMODELIO_IDTF_EXPORTER_H
#define RMODELIO_IDTF_EXPORTER_H

#include "ObjModelExporter.h"


namespace RModelIO
{

class rModelIO_EXPORT IDTFExporter : public ObjModelExporter
{
public:
    // IDTF is used as an intermediate step to producing U3D files. In such cases, it is
    // not necessary to leave the produced files on the filesystem post conversion. If
    // desired, set delFiles to delete from the filesystem the produced IDTF file and
    // any saved tga images (ObjModel material textures) upon any new call to save,
    // or upon destruction of this object.
    IDTFExporter( const RFeatures::ObjModel::Ptr, bool delFiles=false);

    virtual ~IDTFExporter();

protected:
    virtual bool doSave( const std::string& filename);

private:
    bool _delOnDtor;
    std::string _idtffile;
    std::vector<std::string> _tgafiles;
    void reset();
};  // end class

}   // end namespace

#endif
