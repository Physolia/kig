// rect.cc
// Copyright (C)  2002  Dominique Devriese <fritmebufstek@pandora.be>

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

#include "rect.h"

bool operator==( const Rect& r, const Rect& s )
{
  return ( r.bottomLeft() == s.bottomLeft()
           && r.width() == s.width()
           && r.height() == s.height() );
};

inline kdbgstream& operator<<( kdbgstream& s, const Rect& t )
{
  s << "left: " << t.left()
    << "bottom: " << t.bottom()
    << "right: " << t.right()
    << "top: " << t.top()
    << endl;
  return s;
};

Rect::Rect( const Coordinate bottomLeft, const Coordinate topRight )
  : mBottomLeft(bottomLeft)
{
  mWidth = topRight.x - bottomLeft.x;
  mHeight = topRight.y - bottomLeft.y;
  normalize();
}

Rect::Rect( const Coordinate p, const double width, const double height )
  : mBottomLeft(p),
    mWidth(width),
    mHeight(height)
{
  normalize();
}

Rect::Rect( int xa, int ya, int xb, int yb )
  : mBottomLeft( xa, ya ),
    mWidth( xb - xa ),
    mHeight( yb - ya )
{
  normalize();
};

Rect::Rect( const Rect& r )
  : mBottomLeft (r.mBottomLeft),
    mWidth(r.mWidth),
    mHeight(r.mHeight)
{
  normalize();
}

Rect::Rect()
  : mWidth(0),
    mHeight(0)
{
};

void Rect::setBottomLeft( const Coordinate p )
{
  mBottomLeft = p;
}

void Rect::setBottomRight( const Coordinate p )
{
  mBottomLeft = p - Coordinate(mWidth,0);
}

void Rect::setTopRight( const Coordinate p )
{
  mBottomLeft = p - Coordinate(mWidth, mHeight);
}

void Rect::setCenter( const Coordinate p )
{
  mBottomLeft = p - Coordinate(mWidth, mHeight)/2;
}

void Rect::setLeft( const double p )
{
  double r = right();
  mBottomLeft.x = p;
  setRight( r );
};

void Rect::setRight( const double p )
{
  mWidth = p - left();
};

void Rect::setBottom( const double p )
{
  double t = top();
  mBottomLeft.y = p;
  setTop( t );
};

void Rect::setTop( const double p )
{
  mHeight = p - bottom();
};

void Rect::setWidth( const double w )
{
  mWidth = w;
}

void Rect::setHeight( const double h )
{
  mHeight = h;
}

void Rect::normalize()
{
  if ( mWidth < 0 )
    {
      mBottomLeft.x += mWidth;
      mWidth = -mWidth;
    };
  if ( mHeight < 0 )
    {
      mBottomLeft.y += mHeight;
      mHeight = -mHeight;
    };
}

void Rect::moveBy( const Coordinate p )
{
  mBottomLeft += p;
};

void Rect::scale( const double r )
{
  mWidth *= r;
  mHeight *= r;
};


QRect Rect::toQRect() const
{
  return QRect(mBottomLeft.toQPoint(), bottomRight().toQPoint());
}

Coordinate Rect::bottomLeft() const
{
  return mBottomLeft;
}

Coordinate Rect::bottomRight() const
{
  return mBottomLeft + Coordinate(mWidth, 0);
}

Coordinate Rect::topLeft() const
{
  return mBottomLeft + Coordinate(0, mHeight);
}

Coordinate Rect::topRight() const
{
  return mBottomLeft + Coordinate(mWidth, mHeight);
}

Coordinate Rect::center() const
{
  return mBottomLeft + Coordinate(mWidth, mHeight)/2;
}

double Rect::left() const
{
  return mBottomLeft.x;
};
double Rect::right() const
{
  return left() + mWidth;
}
double Rect::bottom() const
{
  return mBottomLeft.y;
};

double Rect::top() const
{
  return bottom() + mHeight;
};

double Rect::width() const
{
  return mWidth;
}

double Rect::height() const
{
  return mHeight;
}

bool Rect::contains( const Coordinate& p ) const
{
  return p.x >= left() &&
    p.y >= bottom() &&
    p.x - left() <= width() &&
    p.y - bottom() <= height();
};

bool Rect::intersects( const Rect& p ) const
{
  // never thought it was this simple :)
  if( p.left() < left() && p.right() < left()) return false;
  if( p.left() > right() && p.right() > right()) return false;
  if( p.bottom() < bottom() && p.top() < bottom()) return false;
  if( p.bottom() > top() && p.top() > top()) return false;
  return true;
};

void Rect::setContains( Coordinate p )
{
  normalize();
  if( p.x < left() ) setLeft( p.x );
  if( p.x > right() ) setRight(p.x);
  if( p.y < bottom() ) setBottom( p.y );
  if( p.y > top() ) setTop( p.y );
};

Rect Rect::normalized() const
{
  Rect t = *this;
  (void) t.normalize();
  return t;
};

Rect Rect::fromQRect( const QRect& r )
{
  return Rect( r.left(), r.top(), r.right(), r.bottom() );
};
