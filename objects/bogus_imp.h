// bogus_imp.h
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

#ifndef BOGUS_IMP_H
#define BOGUS_IMP_H

#include "object_imp.h"

#include <qstring.h>

/**
 * These ObjectImp's are not really in that they don't represent
 * objects.  They exist because ObjectImp's also serve another
 * purpose, namely containing data.  They can be loaded and saved, and
 * the Object class contains some fixedArgs, that are fixed for the
 * object at hand ( e.g. the Coordinates of a fixed point ).  These
 * ObjectImp's are pure data, and serve only to be loaded and saved..
 */
class BogusImp
  : public ObjectImp
{
  typedef ObjectImp Parent;
public:
  void draw( KigPainter& p ) const;
  bool contains( const Coordinate& p, int width, const ScreenInfo& si ) const;
  bool inRect( const Rect& r ) const;
  bool valid() const;

  ObjectImp* transform( const Transformation& ) const;

  const uint numberOfProperties() const;
  const QCStringList properties() const;
  ObjectImp* property( uint which, const KigDocument& d ) const;
};

class InvalidImp
  : public BogusImp
{
  typedef BogusImp Parent;
public:
  InvalidImp();
  bool valid() const;
  bool inherits( int t ) const;
  InvalidImp* copy() const;
  const char* baseName() const;

  int id() const;
};

class DoubleImp
  : public BogusImp
{
  typedef BogusImp Parent;
  double mdata;
public:
  DoubleImp( const double d );

  double data() const { return mdata; };
  void setData( double d ) { mdata = d; };

  bool inherits( int typeID ) const;

  DoubleImp* copy() const;
  const char* baseName() const;

  int id() const;
};

class IntImp
  : public BogusImp
{
  typedef BogusImp Parent;
  int mdata;
public:
  IntImp( const int d );

  int data() const { return mdata; };
  void setData( int d )  { mdata = d; }

  bool inherits( int typeID ) const;
  IntImp* copy() const;
  const char* baseName() const;

  int id() const;
};

class StringImp
  : public BogusImp
{
  typedef BogusImp Parent;
  QString mdata;
public:
  StringImp( const QString& d );

  const QString& data() const { return mdata; };
  void setData( const QString& s ) { mdata = s; }

  bool inherits( int typeID ) const;
  StringImp* copy() const;
  const char* baseName() const;

  int id() const;
};

#endif
