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
 * Simple common interface to ObjModel exporters.
 */

#ifndef RMODELIO_OBJ_MODEL_EXPORTER_H
#define RMODELIO_OBJ_MODEL_EXPORTER_H

#include "rModelIO_Export.h"
#include <IOFormats.h>  // rlib
#include <ObjModel.h>   // RFeatures

namespace RModelIO {

class rModelIO_EXPORT ObjModelExporter : public rlib::IOFormats
{
public:
    ObjModelExporter();
    virtual ~ObjModelExporter(){}

    // Returns true on success. The filename extension must be supported.
    bool save( const RFeatures::ObjModel*, const std::string& filename);

protected:
    virtual bool doSave( const RFeatures::ObjModel*, const std::string& filename) = 0;
};  // end class

}   // end namespace

#endif
