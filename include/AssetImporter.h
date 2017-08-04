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
 * Wraps assimp library to import model data (e.g. .obj files)
 */

#ifndef RMODELIO_ASSET_IMPORTER_H
#define RMODELIO_ASSET_IMPORTER_H

#include "ObjModelImporter.h"

namespace RModelIO
{

class rModelIO_EXPORT AssetImporter : public ObjModelImporter
{
public:
    // Set loadTextures true to read in the textures if available.
    // Set whether to fail or not if a model is imported containing at least one non-triangular polygon.
    // If setReadFailOnNonTriangles(true) called before read() called, read will return NULL if any
    // non-triangular polygons are found in the model file. Whether or not this function is called
    // prior to importing models, warnings about any non-triangular faces found will be printed to stderr.
    // NB AssImp will try to triangulate all models on import.
    AssetImporter( bool loadTextures=true, bool failOnNonTriangles=true);
    virtual ~AssetImporter(){}

protected:
    virtual RFeatures::ObjModel::Ptr doLoad( const std::string& filename);
    virtual void populateFormats();

private:
    bool _loadTextures;
    bool _failOnNonTriangles;
};  // end class

}   // end namespace

#endif
