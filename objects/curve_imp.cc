// curve_data.cc
// Copyright (C)  2002  Dominique Devriese <devriese@kde.org>

// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
// 02111-1307, USA.

#include "curve_imp.h"

const ObjectImpType* CurveImp::stype()
{
  static const ObjectImpType t(
    Parent::stype(), "curve",
    I18N_NOOP( "curve" ),
    I18N_NOOP( "Select this curve" ),
    I18N_NOOP( "Select curve %1" ),
    I18N_NOOP( "Remove a Curve" ),
    I18N_NOOP( "Add a Curve" ),
    I18N_NOOP( "Move a Curve" ),
    I18N_NOOP( "Attach to this curve" ),
    I18N_NOOP( "Show a Curve" ),
    I18N_NOOP( "Hide a Curve" )
    );
  return &t;
}
