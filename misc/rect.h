/**
 This file is part of Kig, a KDE program for Interactive Geometry...
 Copyright (C) 2002  Dominique Devriese <dominique.devriese@student.kuleuven.ac.be>

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


#ifndef RECT_H
#define RECT_H

#include "coordinate.h"

#include <qrect.h>
#include <kdebug.h>

/**
 * like Coordinate is a QPoint replacement with doubles, this is a
 * QRect replacement with doubles...
 */
class Rect
{
public:
  /**
   * constructors...
   */
  Rect( const Coordinate bottomLeft, const Coordinate topRight );
  Rect( const Coordinate bottomLeft, const double width, const double height );
  Rect( int xa, int ya, int xb, int yb );
  Rect( const Rect& r );
  Rect();

  void setBottomLeft( const Coordinate p );
  void setTopLeft( const Coordinate p );
  void setTopRight( const Coordinate p );
  void setBottomRight( const Coordinate p );
  void setCenter( const Coordinate p );
  void setLeft( const double p);
  void setRight( const double p);
  void setTop( const double p );
  void setBottom( const double p );
  void setWidth( const double w );
  void setHeight( const double h );
  /**
   * this makes sure width and height are > 0 ...
   */
  void normalize();
  /**
   * this makes sure p is in the rect, extending it if necessary...
   */
  void setContains( Coordinate p );
  /**
   * moves the rect while keeping the size constant...
   */
  void moveBy( const Coordinate p );
  /**
   * synonym for moveBy...
   */
  Rect& operator+=( const Coordinate p ) { moveBy(p); return *this; };
  /**
   * scale: only the size changes, topLeft is kept where it is...
   */
  void scale( const double r );
  /**
   * synonym for scale...
   */
  Rect& operator*=( const double r ) { scale(r); return *this; };
  Rect& operator/=( const double r ) { scale(1/r); return *this; };

  QRect toQRect() const;
  Coordinate bottomLeft() const;
  Coordinate bottomRight() const;
  Coordinate topLeft() const;
  Coordinate topRight() const;
  Coordinate center() const;
  double left() const;
  double right() const;
  double bottom() const;
  double top() const;
  double width() const;
  double height() const;
  bool contains( const Coordinate& p ) const;
  bool intersects( const Rect& p ) const;
  Rect normalized() const;
  friend kdbgstream& operator<<( kdbgstream& s, const Rect& t );

  static Rect fromQRect( const QRect& );
protected:
  Coordinate mBottomLeft;
  double mWidth;
  double mHeight;
};

bool operator==( const Rect& r, const Rect& s );
inline kdbgstream& operator<<( kdbgstream& s, const Rect& t );

#endif

