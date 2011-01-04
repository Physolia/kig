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
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
// 02110-1301, USA.

#include "calcpaths.h"

#include "../objects/object_calcer.h"
#include "../objects/object_imp.h"

#include <algorithm>

// mp:
// The previous algorithm by Dominique had an exponential complexity
// for some constructions (e.g. a sequence of "n" triangles each inscribed
// into the previous).
// The new version is directly taken from a book of Alan Bertossi
// "Algoritmi e strutture dati"

// temporarily disabling the new algorithm due to the freeze:
// I previously misunderstood the semantics of this function
// and thought that the os vector had to be completed with all
// the subtree generated by it.  On the contrary, the os vector
// tqcontains *all* the objects that we want, we only have to
// reorder them.  Now it *should* work, however we postpone 
// activating this to a more proper moment

// to deactivate the new algorithm change "define" into "undef"

#define NEWCALCPATH
#ifdef NEWCALCPATH
void localdfs( ObjectCalcer* obj,
               std::vector<ObjectCalcer*>& visited,
               std::vector<ObjectCalcer*>& all);

std::vector<ObjectCalcer*> calcPath( const std::vector<ObjectCalcer*>& os )
{
  // "all" is the Objects var we're building, in reverse ordering
  std::vector<ObjectCalcer*> visited;
  std::vector<ObjectCalcer*> all;

  for ( std::vector<ObjectCalcer*>::const_iterator i = os.begin(); i != os.end(); ++i )
  {
    if ( std::find( visited.begin(), visited.end(), *i ) == visited.end() )
    {
      localdfs( *i, visited, all );
    }
  }

  // now, we need to remove all objects that are not in os
  // (forgot to do this in previous fix :-( )
  std::vector<ObjectCalcer*> ret;
  for ( std::vector<ObjectCalcer*>::reverse_iterator i = all.rbegin(); i != all.rend(); ++i )
  {
    // we only add objects that appear in os
    if ( std::find( os.begin(), os.end(), *i ) != os.end() ) ret.push_back( *i );
  };
  return ret;
}

void localdfs( ObjectCalcer* obj,
               std::vector<ObjectCalcer*>& visited,
               std::vector<ObjectCalcer*>& all)
{
  visited.push_back( obj );
  const std::vector<ObjectCalcer*> o = obj->children();
  for ( std::vector<ObjectCalcer*>::const_iterator i = o.begin(); i != o.end(); ++i )
  {
    if ( std::find( visited.begin(), visited.end(), *i ) == visited.end() )
      localdfs( *i, visited, all );
  }
  all.push_back( obj );
}

// old calcPath commented out...

#else
// these first two functions were written before i read stuff about
// graph theory and algorithms, so i'm sure they're far from optimal.
// However, they seem to work fine, and i don't think there's a real
// need for optimisation here..
std::vector<ObjectCalcer*> calcPath( const std::vector<ObjectCalcer*>& os )
{
  // this is a little experiment of mine, i don't know if it is the
  // fastest way to do it, but it seems logical to me...

  // the general idea here:
  // first we build a new Objects variable.  For every object in os,
  // we put all of its children at the end of it, and we do the same
  // for the ones we add..

  // "all" is the Objects var we're building...
  std::vector<ObjectCalcer*> all = os;
  // tmp is the var containing the objects we're iterating over.  The
  // first time around this is the os variable, the next time, this
  // tqcontains the variables we added in the first round...
  std::vector<ObjectCalcer*> tmp = os;
  // tmp2 is a temporary var.  During a round, it receives all the
  // variables we add ( to "all" ) in that round, and at the end of
  // the round, it is assigned to tmp.
  std::vector<ObjectCalcer*> tmp2;
  while ( ! tmp.empty() )
  {
    for ( std::vector<ObjectCalcer*>::const_iterator i = tmp.begin(); i != tmp.end(); ++i )
    {
      const std::vector<ObjectCalcer*> o = (*i)->children();
      std::copy( o.begin(), o.end(), std::back_inserter( all ) );
      std::copy( o.begin(), o.end(), std::back_inserter( tmp2 ) );
    };
    tmp = tmp2;
    tmp2.clear();
  };

  // now we know that if all objects appear at least once after all of
  // their parents.  So, we take all, and of every object, we remove
  // every reference except the last one...
  std::vector<ObjectCalcer*> ret;
  ret.reserve( os.size() );
  for ( std::vector<ObjectCalcer*>::reverse_iterator i = all.rbegin(); i != all.rend(); ++i )
  {
    // we only add objects that appear in os and only if they are not
    // already in ret..
    if ( std::find( ret.begin(), ret.end(), *i ) == ret.end() &&
         std::find( os.begin(), os.end(), *i ) != os.end() ) ret.push_back( *i );
  };
  std::reverse( ret.begin(), ret.end() );
  return ret;
}
#endif

bool addBranch( const std::vector<ObjectCalcer*>& o, const ObjectCalcer* to, std::vector<ObjectCalcer*>& ret )
{
  bool rb = false;
  for ( std::vector<ObjectCalcer*>::const_iterator i = o.begin(); i != o.end(); ++i )
  {
    if ( *i == to )
      rb = true;
    else
      if ( addBranch( (*i)->children(), to, ret ) )
      {
        rb = true;
        ret.push_back( *i );
      };
  };
  return rb;
}

std::vector<ObjectCalcer*> calcPath( const std::vector<ObjectCalcer*>& from, const ObjectCalcer* to )
{
  std::vector<ObjectCalcer*> all;

  for ( std::vector<ObjectCalcer*>::const_iterator i = from.begin(); i != from.end(); ++i )
  {
    (void) addBranch( (*i)->children(), to, all );
  };

  std::vector<ObjectCalcer*> ret;
  for ( std::vector<ObjectCalcer*>::iterator i = all.begin(); i != all.end(); ++i )
  {
    if ( std::find( ret.begin(), ret.end(), *i ) == ret.end() )
      ret.push_back( *i );
  };
  return std::vector<ObjectCalcer*>( ret.rbegin(), ret.rend() );
}

static void addNonCache( ObjectCalcer* o, std::vector<ObjectCalcer*>& ret )
{
  if ( ! o->imp()->isCache() )
    if ( std::find( ret.begin(), ret.end(), o ) == ret.end() )
      ret.push_back( o );
  else
  {
    std::vector<ObjectCalcer*> parents = o->parents();
    for ( uint i = 0; i < parents.size(); ++i )
      addNonCache( parents[i], ret );
  };
}

static bool visit( const ObjectCalcer* o, const std::vector<ObjectCalcer*>& from, std::vector<ObjectCalcer*>& ret )
{
  // this function returns true if the visited object depends on one
  // of the objects in from.  If we encounter objects that are on the
  // side of the tree path ( they do not depend on from themselves,
  // but their direct children do ), then we add them to ret.
  if ( std::find( from.begin(), from.end(), o ) != from.end() ) return true;

  std::vector<bool> deps( o->parents().size(), false );
  bool somedepend = false;
  bool alldepend = true;
  std::vector<ObjectCalcer*> parents = o->parents();
  for ( uint i = 0; i < parents.size(); ++i )
  {
    bool v = visit( parents[i], from, ret );
    somedepend |= v;
    alldepend &= v;
    deps[i] = v;
  };
  if ( somedepend && ! alldepend )
  {
    for ( uint i = 0; i < deps.size(); ++i )
      if ( ! deps[i] )
        addNonCache( parents[i], ret );
  };

  return somedepend;
}

std::vector<ObjectCalcer*> sideOfTreePath( const std::vector<ObjectCalcer*>& from, const ObjectCalcer* to )
{
  std::vector<ObjectCalcer*> ret;
  visit( to, from, ret );
  return ret;
}

std::vector<ObjectCalcer*> getAllParents( const std::vector<ObjectCalcer*>& objs )
{
  using namespace std;
  std::set<ObjectCalcer*> ret( objs.begin(),objs.end() );
  std::set<ObjectCalcer*> cur = ret;
  while ( ! cur.empty() )
  {
    std::set<ObjectCalcer*> next;
    for ( std::set<ObjectCalcer*>::const_iterator i = cur.begin(); i != cur.end(); ++i )
    {
      std::vector<ObjectCalcer*> parents = (*i)->parents();
      next.insert( parents.begin(), parents.end() );
    };

    ret.insert( next.begin(), next.end() );
    cur = next;
  };
  return std::vector<ObjectCalcer*>( ret.begin(), ret.end() );
}

std::vector<ObjectCalcer*> getAllParents( ObjectCalcer* obj )
{
  std::vector<ObjectCalcer*> objs;
  objs.push_back( obj );
  return getAllParents( objs );
}

bool isChild( const ObjectCalcer* o, const std::vector<ObjectCalcer*>& os )
{
  std::vector<ObjectCalcer*> parents = o->parents();
  std::set<ObjectCalcer*> cur( parents.begin(), parents.end() );
  while ( ! cur.empty() )
  {
    std::set<ObjectCalcer*> next;
    for ( std::set<ObjectCalcer*>::const_iterator i = cur.begin(); i != cur.end(); ++i )
    {
      if ( std::find( os.begin(), os.end(), *i ) != os.end() ) return true;
      std::vector<ObjectCalcer*> parents = (*i)->parents();
      next.insert( parents.begin(), parents.end() );
    };
    cur = next;
  };
  return false;
}

std::set<ObjectCalcer*> getAllChildren( ObjectCalcer* obj )
{
  std::vector<ObjectCalcer*> objs;
  objs.push_back( obj );
  return getAllChildren( objs );
}

std::set<ObjectCalcer*> getAllChildren( const std::vector<ObjectCalcer*> objs )
{
  std::set<ObjectCalcer*> ret;
  // objects to iterate over...
  std::set<ObjectCalcer*> cur( objs.begin(), objs.end() );
  while( !cur.empty() )
  {
    // tqcontains the objects to iterate over the next time around...
    std::set<ObjectCalcer*> next;
    for( std::set<ObjectCalcer*>::iterator i = cur.begin();
         i != cur.end(); ++i )
    {
      ret.insert( *i );
      std::vector<ObjectCalcer*> children = (*i)->children();
      next.insert( children.begin(), children.end() );
    };
    cur = next;
  };
  return ret;
}

bool isPointOnCurve( const ObjectCalcer* point, const ObjectCalcer* curve )
{
  return point->isDefinedOnOrThrough( curve ) || curve->isDefinedOnOrThrough( point );
}
