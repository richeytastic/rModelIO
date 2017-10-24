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
    AssetExporter();
    virtual ~AssetExporter(){}

    // Get the available formats as extension description pairs. These are not
    // enabled by default. Use enableFormat( fmt) below where fmt is the extension
    // (the first item of the available pairs returned here).
    const boost::unordered_map<std::string, std::string>& getAvailable() const { return _available;}

    // Returns true if the format is enabled.
    bool enableFormat( const std::string& ext);

protected:
    virtual bool doSave( const RFeatures::ObjModel::Ptr, const std::string& filename);

private:
    boost::unordered_map<std::string, std::string> _available;
};  // end class

}   // end namespace

#endif
