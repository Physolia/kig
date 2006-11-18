// Copyright (C)  2003-2004  Dominique Devriese <devriese@kde.org>

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
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
// 02110-1301, USA.

#include "arc_type.h"

#include "bogus_imp.h"
#include "other_imp.h"
#include "point_imp.h"
#include "conic_imp.h"
#include "line_imp.h"
#include "locus_imp.h"

#include "../misc/common.h"
#include "../misc/calcpaths.h"
#include "../misc/goniometry.h"
#include "../kig/kig_part.h"
#include "../kig/kig_view.h"
#include "../kig/kig_commands.h"

#include <functional>
#include <algorithm>
#include <cmath>

using std::find;

#include <qstringlist.h>

/*
 * arc by three points
 */

static const char constructarcstartingstat[] = I18N_NOOP( "Construct an arc starting at this point" );

static const ArgsParser::spec argsspecArcBTP[] =
{
  { PointImp::stype(), constructarcstartingstat,
    I18N_NOOP( "Select the start point of the new arc..." ), true },
  { PointImp::stype(), I18N_NOOP( "Construct an arc through this point" ),
    I18N_NOOP( "Select a point for the new arc to go through..." ), true },
  { PointImp::stype(), I18N_NOOP( "Construct an arc ending at this point" ),
    I18N_NOOP( "Select the end point of the new arc..." ), true }
};

KIG_INSTANTIATE_OBJECT_TYPE_INSTANCE( ArcBTPType )

ArcBTPType::ArcBTPType()
  : ArgsParserObjectType( "ArcBTP", argsspecArcBTP, 3 )
{
}

ArcBTPType::~ArcBTPType()
{
}

const ArcBTPType* ArcBTPType::instance()
{
  static const ArcBTPType t;
  return &t;
}

ObjectImp* ArcBTPType::calc( const Args& args, const KigDocument& ) const
{
  if ( ! margsparser.checkArgs( args, 2 ) )
    return new InvalidImp;

  const Coordinate a =
    static_cast<const PointImp*>( args[0] )->coordinate();
  const Coordinate b =
    static_cast<const PointImp*>( args[1] )->coordinate();
  Coordinate center;
  double angle = 0.;
  double startangle = 0.;
  if ( args.size() == 3 )
  {
    Coordinate c = static_cast<const PointImp*>( args[2] )->coordinate();
    center = calcCenter( a, b, c );
    if ( ! center.valid() )
    {
      if ( fabs( a.x - c.x ) > fabs( a.y - c.y ) )
      {
        if ( ( b.x - a.x)*(c.x - b.x) > 1e-12 ) return new SegmentImp(a, c);
      } else
      {
        if ( ( b.y - a.y)*(c.y - b.y) > 1e-12 ) return new SegmentImp(a, c);
      }
      return new InvalidImp;
    }
    Coordinate ad = a - center;
    Coordinate bd = b - center;
    Coordinate cd = c - center;
    double anglea = atan2( ad.y, ad.x );
    double angleb = atan2( bd.y, bd.x );
    double anglec = atan2( cd.y, cd.x );

    // anglea should be smaller than anglec
    if ( anglea > anglec )
    {
      double t = anglea;
      anglea = anglec;
      anglec = t;
    };
    if ( angleb > anglec || angleb < anglea )
    {
      startangle = anglec;
      angle = 2 * M_PI + anglea - startangle;
    }
    else
    {
      startangle = anglea;
      angle = anglec - anglea;
    };
  }
  else
  {
    // find a center and angles that look natural..
    center = (b+a)/2 + .6*(b-a).orthogonal();
    Coordinate bd = b - center;
    Coordinate ad = a - center;
    startangle = atan2( ad.y, ad.x );
    double halfangle = atan2( bd.y, bd.x ) - startangle;
    if ( halfangle < - M_PI ) halfangle += 2*M_PI;
    angle = 2 * halfangle;
  };

  double radius = ( a - center ).length();
  return new ArcImp( center, radius, startangle, angle );
}

const ObjectImpType* ArcBTPType::impRequirement( const ObjectImp*, const Args& ) const
{
  return PointImp::stype();
}

bool ArcBTPType::inherits( int type ) const
{
  return Parent::inherits( type );
}

const ObjectImpType* ArcBTPType::resultId() const
{
  return ArcImp::stype();
}

/*
 * arc by center, starting point and angle
 */

static const ArgsParser::spec argsspecArcBCPA[] =
{
  { PointImp::stype(), I18N_NOOP( "Construct an arc with this center" ),
    I18N_NOOP( "Select the center of the new arc..." ), true },
  { PointImp::stype(), constructarcstartingstat,
    I18N_NOOP( "Select the start point of the new arc..." ), true },
  { AngleImp::stype(), I18N_NOOP( "Construct an arc with this angle" ),
    I18N_NOOP( "Select the angle of the new arc..." ), true }
};

KIG_INSTANTIATE_OBJECT_TYPE_INSTANCE( ArcBCPAType )

ArcBCPAType::ArcBCPAType()
  : ArgsParserObjectType( "ArcBCPA", argsspecArcBCPA, 3 )
{
}

ArcBCPAType::~ArcBCPAType()
{
}

const ArcBCPAType* ArcBCPAType::instance()
{
  static const ArcBCPAType t;
  return &t;
}

ObjectImp* ArcBCPAType::calc( const Args& args, const KigDocument& ) const
{
  if ( ! margsparser.checkArgs( args ) )
    return new InvalidImp;

  const Coordinate center = static_cast<const PointImp*>( args[0] )->coordinate();
  const Coordinate p = static_cast<const PointImp*>( args[1] )->coordinate();
  const AngleImp* a = static_cast<const AngleImp*>( args[2] );
  const double angle = a->angle();
  const Coordinate dir = p - center;
  const double startangle = atan2( dir.y, dir.x );
  const double radius = center.distance( p );

  return new ArcImp( center, radius, startangle, angle );
}

const ObjectImpType* ArcBCPAType::impRequirement( const ObjectImp*, const Args& ) const
{
  return PointImp::stype();
}

bool ArcBCPAType::inherits( int type ) const
{
  return Parent::inherits( type );
}

const ObjectImpType* ArcBCPAType::resultId() const
{
  return ArcImp::stype();
}

/*
 * arc of conic by three points and center
 */

static const char constructconicarcstartingstat[] = I18N_NOOP( "Construct a conic arc starting at this point" );

static const ArgsParser::spec argsspecConicArcBCTP[] =
{
  { PointImp::stype(), I18N_NOOP( "Construct an conic arc with this center" ),
    I18N_NOOP( "Select the center of the new conic arc..." ), false },
  { PointImp::stype(), constructconicarcstartingstat,
    I18N_NOOP( "Select the start point of the new conic arc..." ), true },
  { PointImp::stype(), I18N_NOOP( "Construct a conic arc through this point" ),
    I18N_NOOP( "Select a point for the new conic arc to go through..." ), true },
  { PointImp::stype(), I18N_NOOP( "Construct a conic arc ending at this point" ),
    I18N_NOOP( "Select the end point of the new conic arc..." ), true }
};

KIG_INSTANTIATE_OBJECT_TYPE_INSTANCE( ConicArcBCTPType )

ConicArcBCTPType::ConicArcBCTPType()
  : ArgsParserObjectType( "ConicArcBCTP", argsspecConicArcBCTP, 4 )
{
}

ConicArcBCTPType::~ConicArcBCTPType()
{
}

const ConicArcBCTPType* ConicArcBCTPType::instance()
{
  static const ConicArcBCTPType t;
  return &t;
}

ObjectImp* ConicArcBCTPType::calc( const Args& args, const KigDocument& ) const
{
  if ( ! margsparser.checkArgs( args, 2 ) )
    return new InvalidImp;

  const Coordinate center =
    static_cast<const PointImp*>( args[0] )->coordinate();
  const Coordinate a =
    static_cast<const PointImp*>( args[1] )->coordinate();
  const Coordinate d = 2*center - a;
  Coordinate b = center + (a-center).orthogonal();
  Coordinate e = 2*center - b;
  if ( args.size() >= 3 )
  {
    b = static_cast<const PointImp*>( args[2] )->coordinate();
    e = 2*center - b;
  }
  bool have_c = false;
  Coordinate c;
  if ( args.size() == 4 )
  {
    c = static_cast<const PointImp*>( args[3] )->coordinate();
    const Coordinate e = 2*center - c;
    have_c = true;
  }

  std::vector<Coordinate> points;
  points.push_back( a );
  points.push_back( b );
  if (have_c) points.push_back( c );
  points.push_back( d );
  points.push_back( e );
  ConicCartesianData cart =
    calcConicThroughPoints( points, zerotilt, circleifzt, ysymmetry );
  if ( ! d.valid() )
    return new InvalidImp;

  ConicArcImp *me = new ConicArcImp( cart, 0.0, 2*M_PI );
  double angle = 0.;
  double startangle = 0.;
  double anglea = 2*M_PI*me->getParam( a );
  double angleb = anglea + M_PI/2;
  angleb = 2*M_PI*me->getParam( b );
  double anglec = 2*angleb - anglea;
  if ( have_c ) anglec = 2*M_PI*me->getParam( c );

  // anglea should be smaller than anglec
  if ( anglea > anglec )
  {
    double t = anglea;
    anglea = anglec;
    anglec = t;
  };
  if ( angleb > anglec || angleb < anglea )
  {
    startangle = anglec;
    angle = 2 * M_PI + anglea - startangle;
  }
  else
  {
    startangle = anglea;
    angle = anglec - anglea;
  };

  me->setStartAngle( startangle );
  me->setAngle( angle );
  return me;
}

const ObjectImpType* ConicArcBCTPType::resultId() const
{
  return ConicArcImp::stype();
}

