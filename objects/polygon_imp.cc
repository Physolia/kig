// polygon_imp.cc
// Copyright (C)  2004  Pino Toscano <toscano.pino@tiscali.it>

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

#include "polygon_imp.h"

#include "bogus_imp.h"
#include "line_imp.h"

#include "../misc/common.h"
#include "../misc/coordinate.h"
#include "../misc/kigpainter.h"
#include "../misc/kigtransform.h"

#include "../kig/kig_document.h"
#include "../kig/kig_view.h"

#include <klocale.h>

#include <math.h>

PolygonImp::PolygonImp( const std::vector<Coordinate>& points )
{
  mpoints = points;
}

PolygonImp::~PolygonImp()
{
}

ObjectImp* PolygonImp::transform( const Transformation& t ) const
{
  if ( ! t.isHomothetic() )
    return new InvalidImp();

  std::vector<Coordinate> np;
  for ( uint i = 0; i < mpoints.size(); ++i )
  {
    Coordinate nc = t.apply( mpoints[i] );
    if ( !nc.valid() )
      return new InvalidImp;
    np.push_back( nc );
  }
  return new PolygonImp( np );
}

void PolygonImp::draw( KigPainter& p ) const
{
  p.drawPolygon( mpoints );
}

bool PolygonImp::contains( const Coordinate& p, int width, const KigWidget& w ) const
{
  bool ret = false;
  uint reduceddim = mpoints.size() - 1;
  for ( uint i = 0; i < reduceddim; ++i )
  {
    ret |= isOnSegment( p, mpoints[i], mpoints[i+1], w.screenInfo().normalMiss( width ) );
  }
  ret |= isOnSegment( p, mpoints[reduceddim], mpoints[0], w.screenInfo().normalMiss( width ) );

  return ret;
}

bool PolygonImp::inRect( const Rect& r, int width, const KigWidget& w ) const
{
  bool ret = false;
  uint reduceddim = mpoints.size() - 1;
  for ( uint i = 0; i < reduceddim; ++i )
  {
    SegmentImp* s = new SegmentImp( mpoints[i], mpoints[i+1] );
    ret |= lineInRect( r, mpoints[i], mpoints[i+1], width, s, w );
    delete s;
    s = 0;
  }
  SegmentImp* t = new SegmentImp( mpoints[reduceddim], mpoints[0] );
  ret |= lineInRect( r, mpoints[reduceddim], mpoints[0], width, t, w );
  delete t;
  t = 0;

  return ret;
}

bool PolygonImp::valid() const
{
  return true;
}

const uint PolygonImp::numberOfProperties() const
{
  return Parent::numberOfProperties();
}

const QCStringList PolygonImp::propertiesInternalNames() const
{
  QCStringList l = Parent::propertiesInternalNames();
  assert( l.size() == PolygonImp::numberOfProperties() );
  return l;
}

const QCStringList PolygonImp::properties() const
{
  QCStringList l = Parent::properties();
  assert( l.size() == PolygonImp::numberOfProperties() );
  return l;
}

const ObjectImpType* PolygonImp::impRequirementForProperty( uint which ) const
{
  if ( which < Parent::numberOfProperties() )
    return Parent::impRequirementForProperty( which );
  else return PolygonImp::stype();
}

const char* PolygonImp::iconForProperty( uint which ) const
{
  assert( which < PolygonImp::numberOfProperties() );
  if ( which < Parent::numberOfProperties() )
    return Parent::iconForProperty( which );
  else assert( false );
  return "";
}

ObjectImp* PolygonImp::property( uint which, const KigDocument& w ) const
{
  assert( which < PolygonImp::numberOfProperties() );
  if ( which < Parent::numberOfProperties() )
    return Parent::property( which, w );
  else assert( false );
  return new InvalidImp;
}

const std::vector<Coordinate> PolygonImp::points() const
{
  std::vector<Coordinate> np;
  np.reserve( mpoints.size() );
  std::copy( mpoints.begin(), mpoints.end(), std::back_inserter( np ) );
  return np;
}

const uint PolygonImp::npoints() const
{
  return mpoints.size();
}

PolygonImp* PolygonImp::copy() const
{
  return new PolygonImp( mpoints );
}

void PolygonImp::visit( ObjectImpVisitor* vtor ) const
{
  vtor->visit( this );
}

bool PolygonImp::equals( const ObjectImp& rhs ) const
{
  return rhs.inherits( PolygonImp::stype() ) &&
    static_cast<const PolygonImp&>( rhs ).points() == mpoints;
}

const ObjectImpType* PolygonImp::stype()
{
  static const ObjectImpType t(
    Parent::stype(), "polygon",
    I18N_NOOP( "polygon" ),
    I18N_NOOP( "Select this polygon" ),
    I18N_NOOP( "Select polygon %1" ),
    I18N_NOOP( "Remove a Polygon" ),
    I18N_NOOP( "Add a Polygon" ),
    I18N_NOOP( "Move a Polygon" ),
    I18N_NOOP( "Attach to this polygon" ),
    I18N_NOOP( "Show a Polygon" ),
    I18N_NOOP( "Hide a Polygon" )
    );
  return &t;
}

const ObjectImpType* PolygonImp::type() const
{
  return PolygonImp::stype();
}

bool PolygonImp::isPropertyDefinedOnOrThroughThisImp( uint which ) const
{
  assert( which < PolygonImp::numberOfProperties() );
  if ( which < Parent::numberOfProperties() )
    return Parent::isPropertyDefinedOnOrThroughThisImp( which );
  return false;
}

Rect PolygonImp::surroundingRect() const
{
  Rect r( 0., 0., 0., 0. );
  for ( uint i = 0; i < mpoints.size(); ++i )
  {
    r.setContains( mpoints[i] );
  }
  return r;
}
