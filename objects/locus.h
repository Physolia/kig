/**
 This file is part of Kig, a KDE program for Interactive Geometry...
 Copyright (C) 2002  Dominique Devriese

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


#ifndef LOCUS_H
#define LOCUS_H

#include "curve.h"

#include <list>

class ConstrainedPoint;
class ObjectHierarchy;

// this object is inspired on KSeg source, credits for all of the
// ideas go to Ilya Baran <ibaran@mit.edu>
// a locus object is a mathematical locus.  It is defined by a
// constrained point, which moves over a curve, and an object,
// which somehow depends on that point.  The locus contains all
// objects the object moves over as the point moves over all of
// its possible positions...
// this is implemented by having a Locus simply contain some 150
// objects (hmm...)
// drawing is done by simply drawing the points...
class Locus
    : public Curve
{
public:
  // number of points to include, i think this is a nice default...
  static const int numberOfSamples = 500;
public:
  Locus() : cp(0), obj(0), hierarchy(0) { };
  ~Locus() { delete_all( objs.begin(), objs.end() ); };
  Locus(const Locus& loc);
  Locus* copy() { return new Locus(*this); };

  virtual const QCString vBaseTypeName() const { return sBaseTypeName(); };
  static const QCString sBaseTypeName() { return I18N_NOOP("curve"); };
  virtual const QCString vFullTypeName() const { return sFullTypeName(); };
  static const QCString sFullTypeName() { return "Curve"; };
  const QString vDescriptiveName() const { return sDescriptiveName(); };
  static const QString sDescriptiveName() { return i18n("Locus"); };
  const QString vDescription() const { return sDescription(); };
  static const QString sDescription() {
    return i18n( "Construct a locus: let one point move around, and record "
                 "the places another object passes through. These combined "
                 "form a new object: the locus..." ); };
  const QCString vIconFileName() const { return sIconFileName(); };
  static const QCString sIconFileName() { return "locus"; };
  const int vShortCut() const { return sShortCut(); };
  static const int sShortCut() { return 0; };

  void draw (KigPainter& p, bool showSelection) const;
  bool contains (const Coordinate& o, const double fault ) const;
  bool inRect (const Rect&) const;

  // arguments
  QString wantArg ( const Object* ) const;
  QString wantPoint() const;
  bool selectArg (Object* which);
//   void unselectArg (Object* which);
  void drawPrelim ( KigPainter&, const Coordinate& ) const {};

  // moving
  void startMove(const Coordinate&) {};
  void moveTo(const Coordinate&) {};
  void stopMove() {};

  void calc();

public:
  Coordinate getPoint (double param) const;
  double getParam (const Coordinate&) const;

  Objects getParents() const
  {
    Objects tmp;
    tmp.push_back(cp);
    tmp.push_back(obj);
    return tmp;
  };

protected:
  ConstrainedPoint* cp;
  Object* obj;

  bool isPointLocus() const { return _pointLocus; }
  bool _pointLocus;

  // don't use this for fillUp or saving, since it has 0 for
  // KigDocument pointer...
  ObjectHierarchy* hierarchy;

  // objs is just a list of pointers to objects
  Objects objs;

  // the window we're in...
  // we declare it mutable cause it's set in draw() const ...
  // yes, i know this is ugly :(
  mutable Rect calcRect;

  struct CPt
  {
    CPt(Coordinate inPt, double inPm) : pt(inPt), pm (inPm) {};
    Coordinate pt;
    double pm;
  };

  typedef std::list<CPt> CPts;

  // for calcPointLocus we need some special magic, so it is a special
  // type...
  CPts pts;

  // this is used if the obj is a point; it selects the best points
  // from the possible ones...
  void calcPointLocus( const Rect& );
  // some functions used by calcPointLocus...
  CPts::iterator addPoint(double param);
  void recurse(CPts::iterator, CPts::iterator, int&, const Rect&);
  void realCalc( const Rect& r );

  // this is used when the obj is not a point; it just takes the first
  // numberOfSamples objects it can find...
  void calcObjectLocus();

};
#endif
