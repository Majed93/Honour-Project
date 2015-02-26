/***************************************************************************
    LibUFO - UI For OpenGL
    copyright         : (C) 2001-2005 by Johannes Schmidt
    email             : schmidtjf at users.sourceforge.net
                             -------------------

    file              : include/ufo/udrawable.hpp
    begin             : Tue Nov 13 2001
    $Id: udrawable.hpp,v 1.9 2005/05/21 15:17:49 schmidtjf Exp $
 ***************************************************************************/

/***************************************************************************
 *  This library is free software; you can redistribute it and/or          *
 * modify it under the terms of the GNU Lesser General Public              *
 * License as published by the Free Software Foundation; either            *
 * version 2.1 of the License, or (at your option) any later version.      *
 *                                                                         *
 * This library is distributed in the hope that it will be useful,         *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of          *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU       *
 * Lesser General Public License for more details.                         *
 *                                                                         *
 * You should have received a copy of the GNU Lesser General Public        *
 * License along with this library; if not, write to the Free Software     *
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA *
 ***************************************************************************/

#ifndef UDRAWABLE_HPP
#define UDRAWABLE_HPP

#include "uobject.hpp"

#include "util/udimension.hpp"
#include "util/urectangle.hpp"

namespace ufo {

class UGraphics;

/** @short A simple interface for some drawable things (e.g. textures)
  * @ingroup drawing
  *
  *
  *@author Johannes Schmidt
  */

class UFO_EXPORT UDrawable : public virtual UObject {
	UFO_DECLARE_ABSTRACT_CLASS(UDrawable)
public:
	/** Draw to the given rectangle. Possibly scale to the given size.
	  */
	virtual void paintDrawable(UGraphics * g, const URectangle & rect) = 0;
	/** @overload */
	void paintDrawable(UGraphics * g, int x, int y, int w, int h) {
		paintDrawable(g, URectangle(x, y, w, h));
	}
	/** @return The preferred size of the drawable. */
	virtual UDimension getDrawableSize() const = 0;


};

} // namespace ufo

#endif