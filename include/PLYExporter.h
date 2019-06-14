/************************************************************************
 * Copyright (C) 2019 Richard Palmer
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
 * Export model to PLY format.
 */

#ifndef RMODELIO_PLY_EXPORTER_H
#define RMODELIO_PLY_EXPORTER_H

#include "ObjModelExporter.h"

namespace RModelIO {

class rModelIO_EXPORT PLYExporter : public ObjModelExporter
{
public:
    PLYExporter();

protected:
    bool doSave( const RFeatures::ObjModel*, const std::string& filename) override;
};  // end class

}   // end namespace

#endif
