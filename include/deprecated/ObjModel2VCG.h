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

#ifndef RMODELIO_OBJ_MODEL_2_VCG_H
#define RMODELIO_OBJ_MODEL_2_VCG_H

#include "rModelIO_Export.h"
#include "VCGTypes.h"
#include <ObjModel.h>   // RFeatures


namespace RModelIO
{

class rModelIO_EXPORT ObjModel2VCG
{
public:
    ObjModel2VCG();

    // Make a copy of the given ObjModel in VCGObjModel format.
    // In the created model, the textures member has the same size
    // as the number of materials used in the ObjModel (VCG 1.0.1 doesn't
    // support multiple texture maps per material). Returns true on success.
    VCGObjModel::Ptr create( const RFeatures::ObjModel::Ptr);

    // Get the diffuse texture maps from the model that correspond to the created VCGObjModel
    // texture names (in v.textures). The texture map images are not saved by VCG, only the entries
    // in v.textures are (which should be set as the texture map filenames). These returned images
    // must therefore be saved separately according to the filenames in v.textures but note that
    // THE STRINGS IN v.textures REMAIN EMPTY! The caller must set these prior to calling one of
    // the VCG export functions so that when reimported, the VCG model uses the correct filenames.
    const std::vector<cv::Mat>& getTextures() const { return _tmaps;}

private:
    std::vector<cv::Mat> _tmaps;
};  // end class

}   // end namespace

#endif
