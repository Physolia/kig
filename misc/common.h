/**
 This file is part of Kig, a KDE program for Interactive Geometry...
 Copyright (C) 2002  Dominique Devriese <devriese@kde.org>

 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307
 USA
**/


#ifndef KIG_MISC_COMMON_H
#define KIG_MISC_COMMON_H

#include "coordinate.h"
#include "rect.h"

#include <qptrlist.h>
#include <qrect.h>
#include <kdeversion.h>

#include <vector>
#include <assert.h>

#ifdef KDE_IS_VERSION
#if KDE_IS_VERSION( 3, 1, 0 )
#define KIG_USE_KDOUBLEVALIDATOR
#else
#undef KIG_USE_KDOUBLEVALIDATOR
#endif
#else
#undef KIG_USE_KDOUBLEVALIDATOR
#endif

class ObjectImp;
class KigWidget;

extern const double double_inf;

/**
 * Here, we define some algorithms which we need in
 * various places...
 */

double getDoubleFromUser( const QString& caption, const QString& label, double value,
                          QWidget* parent, bool* ok, double min, double max, int decimals );

// this class represents the data needed for defining a line.
class LineData {
public:
  Coordinate a;
  Coordinate b;
  const Coordinate dir() const { return b - a; };
  double length() const { return ( b - a ).length(); };
  LineData() : a(), b() {};
  LineData( const Coordinate& na, const Coordinate& nb ) : a( na ), b( nb ) {};
};

bool operator==( const LineData& l, const LineData& r );

/**
 * This calcs the rotation of point a around point c by arc arc.  Arc
 * is in radians, in the range 0 < arc < 2*pi ...
 */
Coordinate calcRotatedPoint( const Coordinate& a, const Coordinate& c, const double arc );

/**
 * this returns a point, so that the line through point t
 * and the point returned is perpendicular on the line l.
 */
Coordinate calcPointOnPerpend( const LineData& l, const Coordinate& t );

/**
 * this returns a point, so that the line through point t and the
 * point returned is perpendicular to the direction given in dir...
 */
Coordinate calcPointOnPerpend( const Coordinate& dir, const Coordinate& t );

/**
 * this returns a point, so that the line through point t
 * and the point returned is parallel with the line l
 */
Coordinate calcPointOnParallel( const LineData& l, const Coordinate& t );

/**
 * this returns a point, so that the line through point t
 * and the point returned is parallel with the direction given in dir...
 */
Coordinate calcPointOnParallel( const Coordinate& dir, const Coordinate& t );


/**
 * this calcs the point where the lines l and m intersect...
 */
Coordinate calcIntersectionPoint( const LineData& l, const LineData& m );

/**
 * this calcs the intersection points of the circle with center c and
 * radius sqrt( r ), and the line l.  As a circle and a
 * line have two intersection points, side tells us which one we
 * need...  It should be 1 or -1.  If the line and the circle have no
 * intersection, valid is set to false, otherwise to true...
 * Note that sqr is the _square_ of the radius.  We do this to avoid
 * rounding errors...
 */
const Coordinate calcCircleLineIntersect( const Coordinate& c,
                                          const double sqr,
                                          const LineData& l,
                                          int side,
                                          bool& valid );

/**
 * this calculates the perpendicular projection of point p on line
 * ab...
 */
const Coordinate calcPointProjection( const Coordinate& p,
                                      const LineData& l );

/**
 * calc the distance of point p to the line through a and b...
 */
double calcDistancePointLine( const Coordinate& p,
                              const LineData& l );

/**
 * this sets p1 and p2 to p1' and p2' so that p1'p2' is the same line
 * as p1p2, and so that p1' and p2' are on the border of the Rect...
 */
void calcBorderPoints( Coordinate& p1, Coordinate& p2, const Rect& r );
/**
 * overload...
 */
void calcBorderPoints( double& xa, double& xb, double& ya, double& yb, const Rect& r);
/**
 * cleaner overload, intended to replace the above two...
 */
const LineData calcBorderPoints( const LineData& l, const Rect& r );

/**
 * this does the same as the above function, but only for b..
 */
void calcRayBorderPoints( const Coordinate& a, Coordinate& b, const Rect& r );

/**
 * This function calculates the center of the circle going through the
 * three given points..
 */
const Coordinate calcCenter(
  const Coordinate& a, const Coordinate& b, const Coordinate& c );

/**
 * overload...
 */
void calcRayBorderPoints( const double xa, const double xb, double& ya,
                          double& yb, const Rect& r );

/**
 * calc the mirror point of p over the line l
 */
const Coordinate calcMirrorPoint( const LineData& l,
                                  const Coordinate& p );

/**
 * is o on the line defined by point a and point b ?
 * fault is the allowed difference...
 */
bool isOnLine( const Coordinate& o, const Coordinate& a,
               const Coordinate& b, const double fault );

/**
 * is o on the segment defined by point a and point b ?
 * this calls isOnLine(), but also checks if o is "between" a and b...
 * fault is the allowed difference...
 */
bool isOnSegment( const Coordinate& o, const Coordinate& a,
                  const Coordinate& b, const double fault );

bool isOnRay( const Coordinate& o, const Coordinate& a,
              const Coordinate& b, const double fault );

Coordinate calcCircleRadicalStartPoint( const Coordinate& ca,
                                        const Coordinate& cb,
                                        double sqra, double sqrb );

/**
 * Is the line, segment, ray or vector inside r ?  We need the imp to
 * distinguish between rays, lines, segments or whatever.. ( we use
 * their contains functions actually.. )
 */
bool lineInRect( const Rect& r, const Coordinate& a, const Coordinate& b,
                 const int width, const ObjectImp* imp, const KigWidget& w );

template <typename T>
T kigMin( const T& a, const T& b )
{
  return a < b ? a : b;
}

template <typename T>
T kigMax( const T& a, const T& b )
{
  return a > b ? a : b;
}

#endif
