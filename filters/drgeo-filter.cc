// drgeo-filter.cc
// Copyright (C)  2004  Pino Toscano <toscano.pino@tiscali.it>
// Copyright (C)  2004  Dominique Devriese <devriese@kde.org>

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

#include "drgeo-filter.h"

#include "drgeo-filter-chooser.h"
#include "filters-common.h"

#include "../kig/kig_document.h"
#include "../kig/kig_part.h"
#include "../misc/coordinate.h"
#include "../misc/coordinate_system.h"
#include "../objects/arc_type.h"
#include "../objects/bogus_imp.h"
#include "../objects/circle_imp.h"
#include "../objects/circle_type.h"
#include "../objects/conic_imp.h"
#include "../objects/conic_types.h"
#include "../objects/curve_imp.h"
#include "../objects/intersection_types.h"
#include "../objects/line_imp.h"
#include "../objects/line_type.h"
#include "../objects/object_calcer.h"
#include "../objects/object_drawer.h"
#include "../objects/object_factory.h"
#include "../objects/object_holder.h"
#include "../objects/object_type.h"
#include "../objects/other_imp.h"
#include "../objects/other_type.h"
#include "../objects/point_type.h"
#include "../objects/transform_types.h"
#include "../objects/vector_type.h"

#include <qfile.h>
#include <qnamespace.h>

#include <klocale.h>

struct DrGeoHierarchyElement
{
  QString id;
  std::vector<QString> parents;
};

KigFilterDrgeo::KigFilterDrgeo()
{
}

KigFilterDrgeo::~KigFilterDrgeo()
{
}

bool KigFilterDrgeo::supportMime( const QString& mime )
{
  return mime == "application/x-drgeo";
}

KigDocument* KigFilterDrgeo::load( const QString& file )
{
  QFile f( file );
  if ( ! f.open( IO_ReadOnly ) )
  {
    fileNotFound( file );
    return 0;
  }

  QStringList figures;
  QDomDocument doc( "drgenius" );
  if ( !doc.setContent( &f ) )
    KIG_FILTER_PARSE_ERROR;
  QDomElement main = doc.documentElement();
  int nmacros = 0;
  // reading figures...
  for ( QDomNode n = main.firstChild(); ! n.isNull(); n = n.nextSibling() )
  {
    QDomElement e = n.toElement();
    if ( e.isNull() ) continue;
    else if ( e.tagName() == "drgeo" )
      figures.append( e.attribute( "name" ) );
    else if ( e.tagName() == "macro" )
      nmacros++;
  }
  if ( figures.isEmpty() ) {
    if( nmacros > 0 )
      warning( i18n( "The Dr. Geo file \"%1\" is a macro file so it contains no "
                     "figures." ).arg( file ) );
    else
      warning( i18n( "There are no figures in Dr. Geo file \"%1\"." ).arg( file ) );
    return false;
  }

  int myfig = 0;
// drgeo file has more than 1 figure, let the user choose one...
  if ( figures.count() > 1 ) {
    KigFilterDrgeoChooser* c = new KigFilterDrgeoChooser( figures );
//    c->addFigures( figures );
    myfig = c->exec();
    delete c;
  }

  kdDebug() << "drgeo file " << file << endl;
  int curfig = -1;

  for ( QDomNode n = main.firstChild(); ! n.isNull(); n = n.nextSibling() )
  {
    QDomElement e = n.toElement();
    if ( e.isNull() ) continue;
    else if ( e.tagName() == "drgeo" )
    {
      kdDebug() << "- Figure: '" << e.attribute("name") << "'" << endl;
      curfig += 1;
      if ( curfig == myfig )
      {
        bool grid = ( e.attribute( "grid" ) != "False" );
        return importFigure( e.firstChild(), file, grid );
      }
    }
  }

  return 0;
}

int convertDrgeoIndex( const std::vector<DrGeoHierarchyElement> es, const QString myid )
{
  for ( uint i = 0; i < es.size(); ++i )
    if ( es[i].id == myid )
      return i;
  return -1;
}

KigDocument* KigFilterDrgeo::importFigure( QDomNode f, const QString& file, const bool grid )
{
  KigDocument* ret = new KigDocument();

  using namespace std;
  std::vector<DrGeoHierarchyElement> elems;
  int withoutid = 0;

  // 1st: fetch relationships and build an appropriate structure
  for (QDomNode a = f; ! a.isNull(); a = a.nextSibling() )
  {
    QDomElement domelem = a.toElement();
    if ( domelem.isNull() ) continue;
    else
    {
      DrGeoHierarchyElement elem;
      kdDebug() << "  * " << domelem.tagName() << "(" << domelem.attribute("type") << ")" << endl;
      for ( QDomNode c = domelem.firstChild(); ! c.isNull(); c = c.nextSibling() )
      {
        QDomElement ce = c.toElement();
        if ( ce.isNull() ) continue;
        else if ( ce.tagName() == "parent" )
          elem.parents.push_back( ce.attribute( "ref" ) );
      }
      QString curid = domelem.attribute( "id" );
      elem.id = !curid.isNull() ? curid : QString::number( withoutid++ ) ;
      elems.push_back( elem );
    }
  }

  QString x;
  kdDebug() << "+++ elems" << endl;
  for ( uint i = 0; i < elems.size(); ++i )
  {
    x = "";
    for ( uint j = 0; j < elems[i].parents.size(); ++j )
    {
      x += elems[i].parents[j] + "_";
    }
    kdDebug() << "  --> " << elems[i].id << " - " << x << endl;
  }

  // 2nd: let's draw!
  int curid = 0;
  const ObjectFactory* fact = ObjectFactory::instance();
  std::vector<ObjectHolder*> holders;
  ObjectTypeCalcer* oc = 0;
  int nignored = 0;

  // there's no need to sort the objects because it seems that DrGeo objects
  // appear in the right order... so let's go!
  for (QDomNode a = f; ! a.isNull(); a = a.nextSibling() )
  {
    kdDebug() << "+++ id: " << curid << endl;
    const DrGeoHierarchyElement& el = elems[curid];
    std::vector<ObjectCalcer*> parents;
    for ( uint j = 0; j < el.parents.size(); ++j )
    {
      int parentid = convertDrgeoIndex( elems, el.parents[j] );
      if ( parentid == -1 )
        KIG_FILTER_PARSE_ERROR;
      parents.push_back( holders[parentid-nignored]->calcer() );
    };
    if ( parents.size() > 0 )
      kdDebug() << "+++++++++ parents: " << parents[0] << " " << parents[1] << endl;
    else
      kdDebug() << "+++++++++ parents: NO" << endl;

    kdDebug() << ">>>>>>>>> Scanning domelem" << endl;
    QDomElement domelem = a.toElement();
    kdDebug() << "+++++++++ " << domelem.tagName() << " - " << domelem.attribute("type") << endl;
    if ( domelem.isNull() ) continue;
    else if ( domelem.tagName() == "point" )
    {
      QString xs;
      QString ys;
      QString values;
      for ( QDomNode c = domelem.firstChild(); ! c.isNull(); c = c.nextSibling() )
      {
        QDomElement ce = c.toElement();
        if ( ce.isNull() ) continue;
        else if ( ce.tagName() == "x" )
          xs = ce.text();
        else if ( ce.tagName() == "y" )
          ys = ce.text();
        else if ( ce.tagName() == "value" )
          values = ce.text();
      }
      bool ok;
      bool ok2;
      bool ok3;
      double x = xs.toDouble( &ok );
      double y = ys.toDouble( &ok2 );
      double value = values.toDouble( &ok3 );
      if ( domelem.attribute( "type" ) == "Free" )
      {
        if ( ! ( ok && ok2 ) )
          KIG_FILTER_PARSE_ERROR;
        oc = fact->fixedPointCalcer( Coordinate( x, y ) );
      }
      else if ( domelem.attribute( "type" ) == "Middle_2pts" )
        oc = new ObjectTypeCalcer( MidPointType::instance(), parents );
      else if ( domelem.attribute( "type" ) == "Middle_segment" )
      {
        if ( parents.size() != 1 ) KIG_FILTER_PARSE_ERROR;
        if ( !parents[0]->imp()->inherits( SegmentImp::stype() ) )
          KIG_FILTER_PARSE_ERROR;
        int index1 = parents[0]->imp()->propertiesInternalNames().findIndex( "end-point-A" );
        assert( index1 != -1 );
        ObjectPropertyCalcer* o1 = new ObjectPropertyCalcer( parents[0], index1 );
        o1->calc( *ret );
        int index2 = parents[0]->imp()->propertiesInternalNames().findIndex( "end-point-B" );
        assert( index2 != -1 );
        ObjectPropertyCalcer* o2 = new ObjectPropertyCalcer( parents[0], index2 );
        o2->calc( *ret );
        std::vector<ObjectCalcer*> args;
        args.push_back( o1 );
        args.push_back( o2 );
        oc = new ObjectTypeCalcer( MidPointType::instance(), args );
      }
      else if ( domelem.attribute( "type" ) == "On_curve" )
      {
        if ( ! ok3 )
          KIG_FILTER_PARSE_ERROR;
        if ( ( parents[0]->imp()->inherits( CircleImp::stype() ) ) ||
             ( parents[0]->imp()->inherits( SegmentImp::stype() ) ) )
          oc = fact->constrainedPointCalcer( parents[0], value );
//        else if ( parents[0]->imp()->inherits( AbstractLineImp::stype() ) )
//          oc = fact->constrainedPointCalcer( parents[0], Coordinate( value, 0 ), *ret );
        else if ( parents[0]->imp()->inherits( ArcImp::stype() ) )
          oc = fact->constrainedPointCalcer( parents[0], 1 - value );
        else
        {
//          oc = fact->constrainedPointCalcer( parents[0], value );
          notSupported( file, i18n( "This Dr. Geo file contains a \"%1 %2\" object, "
                                    "which Kig does not currently support." ).arg( domelem.tagName() ).arg(
                                      domelem.attribute( "type" ) ) );
          return false;
        }
      }
      else if ( domelem.attribute( "type" ) == "Intersection" )
      {
        if ( ( parents[0]->imp()->inherits( AbstractLineImp::stype() ) ) &&
             ( parents[1]->imp()->inherits( AbstractLineImp::stype() ) ) )
          oc = new ObjectTypeCalcer( LineLineIntersectionType::instance(), parents );
//        else if ( ( parents[0]->imp()->inherits( AbstractLineImp::stype() ) ) &&
//                  ( parents[1]->imp()->inherits( ConicImp::stype() ) ) )
//        {
//          ObjectCalcer* t = parents[1];
//          parents[1] = parents[0];
//          parents[0] = t;
//          oc = new ObjectTypeCalcer( ConicLineIntersectionType::instance(), parents );
//        }
        else if ( ( parents[0]->imp()->inherits( CircleImp::stype() ) ) &&
                  ( parents[1]->imp()->inherits( CircleImp::stype() ) ) )
        {
          bool ok;
          int which = domelem.attribute( "extra" ).toInt( &ok );
          if ( !ok ) KIG_FILTER_PARSE_ERROR;
          if ( which == 0 ) which = -1;
          else if ( which == 1 ) which = 1;
          else KIG_FILTER_PARSE_ERROR;
          std::vector<ObjectCalcer*> args = parents;
          args.push_back( new ObjectConstCalcer( new IntImp( which ) ) );
          oc = new ObjectTypeCalcer( CircleCircleIntersectionType::instance(), args );
        }
        else if ( ( parents[0]->imp()->inherits( CircleImp::stype() ) &&
                    parents[1]->imp()->inherits( AbstractLineImp::stype() ) ) ||
                  ( parents[1]->imp()->inherits( CircleImp::stype() ) &&
                    parents[0]->imp()->inherits( AbstractLineImp::stype() ) ) )
        {
          bool ok;
          int which = domelem.attribute( "extra" ).toInt( &ok );
          if ( !ok ) KIG_FILTER_PARSE_ERROR;
          if ( which == 0 ) which = -1;
          else if ( which == 1 ) which = 1;
          else KIG_FILTER_PARSE_ERROR;
          std::vector<ObjectCalcer*> args = parents;
          args.push_back( new ObjectConstCalcer( new IntImp( which ) ) );
          oc = new ObjectTypeCalcer( ConicLineIntersectionType::instance(), args );
        }
        else
        {
          notSupported( file, i18n( "This Dr. Geo file contains an intersection type, "
                                    "which Kig does not currently support." ) );
          return false;
        }
      }
      else if ( domelem.attribute( "type" ) == "Reflexion" )
        oc = new ObjectTypeCalcer( LineReflectionType::instance(), parents );
      else if ( domelem.attribute( "type" ) == "Symmetry" )
        oc = new ObjectTypeCalcer( PointReflectionType::instance(), parents );
      else if ( domelem.attribute( "type" ) == "Translation" )
        oc = new ObjectTypeCalcer( TranslatedType::instance(), parents );
      else if ( domelem.attribute( "type" ) == "Rotation" )
        oc = new ObjectTypeCalcer( RotationType::instance(), parents );
      else
      {
        notSupported( file, i18n( "This Dr. Geo file contains a \"%1 %2\" object, "
                                  "which Kig does not currently support." ).arg( domelem.tagName() ).arg(
                                    domelem.attribute( "type" ) ) );
        return false;
      }
      kdDebug() << "+++++++++ oc:" << oc << endl;
    }
    else if( ( domelem.tagName() == "line" ) ||
             ( domelem.tagName() == "halfLine" ) ||
             ( domelem.tagName() == "segment" ) ||
             ( domelem.tagName() == "vector" ) ||
             ( domelem.tagName() == "circle" ) ||
             ( domelem.tagName() == "arcCircle" ) )
    {
      const ObjectType* type = 0;
      if ( domelem.attribute( "type" ) == "2pts" )
      {
        if( domelem.tagName() == "line" )
          type = LineABType::instance();
        else if( domelem.tagName() == "halfLine" )
          type = RayABType::instance();
        else if( domelem.tagName() == "segment" )
          type = SegmentABType::instance();
        else if( domelem.tagName() == "vector" )
          type = VectorType::instance();
        else if( domelem.tagName() == "circle" )
          type = CircleBCPType::instance();
        oc = new ObjectTypeCalcer( type, parents );
      }
      else if( domelem.attribute( "type" ) == "3pts" )
      {
        if( domelem.tagName() == "arcCircle" )
          type = ArcBTPType::instance();
        oc = new ObjectTypeCalcer( type, parents );
      }
      else if( domelem.attribute( "type" ) == "segment" )
      {
        if( domelem.tagName() == "circle" )
        {
          type = CircleBPRType::instance();
          int index = parents[1]->imp()->propertiesInternalNames().findIndex( "length" );
          assert( index != -1 );
          ObjectPropertyCalcer* o = new ObjectPropertyCalcer( parents[1], index );
          o->calc( *ret );
          ObjectCalcer* a = parents[0];
          parents.clear();
          parents.push_back( a );
          parents.push_back( o );
        }
        oc = new ObjectTypeCalcer( type, parents );
      }
      else if ( domelem.attribute( "type" ) == "perpendicular" )
        oc = new ObjectTypeCalcer( LinePerpendLPType::instance(), parents );
      else if ( domelem.attribute( "type" ) == "parallel" )
        oc = new ObjectTypeCalcer( LineParallelLPType::instance(), parents );
      else if ( domelem.attribute( "type" ) == "Reflexion" )
        oc = new ObjectTypeCalcer( LineReflectionType::instance(), parents );
      else if ( domelem.attribute( "type" ) == "Symmetry" )
        oc = new ObjectTypeCalcer( PointReflectionType::instance(), parents );
      else if ( domelem.attribute( "type" ) == "Translation" )
        oc = new ObjectTypeCalcer( TranslatedType::instance(), parents );
      else if ( domelem.attribute( "type" ) == "Rotation" )
        oc = new ObjectTypeCalcer( RotationType::instance(), parents );
      else
      {
        notSupported( file, i18n( "This Dr. Geo file contains a \"%1 %2\" object, "
                                  "which Kig does not currently support." ).arg( domelem.tagName() ).arg(
                                  domelem.attribute( "type" ) ) );
        return false;
      }
      kdDebug() << "+++++++++ oc:" << oc << endl;
    }
    else if( ( domelem.tagName() == "numeric" ) ||
             ( domelem.tagName() == "equation" ) )
    {
      QString xs;
      QString ys;
      QString value;
      for ( QDomNode c = domelem.firstChild(); ! c.isNull(); c = c.nextSibling() )
      {
        QDomElement ce = c.toElement();
        if ( ce.isNull() ) continue;
        else if ( ce.tagName() == "x" )
          xs = ce.text();
        else if ( ce.tagName() == "y" )
          ys = ce.text();
        else if ( ce.tagName() == "value" )
          value = ce.text();
      }
      bool ok;
      bool ok2;
      double x = xs.toDouble( &ok );
      double y = ys.toDouble( &ok2 );
      if ( ! ( ok && ok2 ) )
        KIG_FILTER_PARSE_ERROR;
      Coordinate m( x, y );
      // types of 'numeric'
      // ugly hack to show value numerics...
      if ( domelem.attribute( "type" ) == "value" )
      {
        bool ok3;
        double dvalue = value.toDouble( &ok3 );
        if ( ok3 )
          value = QString( "%1" ).arg( dvalue, 0, 'g', 3 );
        oc = fact->labelCalcer( value, m, false, std::vector<ObjectCalcer*>(), *ret );
      }
      else if ( domelem.attribute( "type" ) == "pt_abscissa" )
      {
        if ( parents.size() != 1 ) KIG_FILTER_PARSE_ERROR;
        oc = filtersConstructTextObject( m, parents[0], "coordinate-x", *ret, false );
      }
      else if ( domelem.attribute( "type" ) == "pt_ordinate" )
      {
        if ( parents.size() != 1 ) KIG_FILTER_PARSE_ERROR;
        oc = filtersConstructTextObject( m, parents[0], "coordinate-y", *ret, false );
      }
      else if ( domelem.attribute( "type" ) == "segment_length" )
      {
        if ( parents.size() != 1 ) KIG_FILTER_PARSE_ERROR;
        oc = filtersConstructTextObject( m, parents[0], "length", *ret, false );
      }
      else if ( domelem.attribute( "type" ) == "circle_perimeter" )
      {
        if ( parents.size() != 1 ) KIG_FILTER_PARSE_ERROR;
        oc = filtersConstructTextObject( m, parents[0], "circumference", *ret, false );
      }
      else if ( domelem.attribute( "type" ) == "arc_length" )
      {
        if ( parents.size() != 1 ) KIG_FILTER_PARSE_ERROR;
        oc = filtersConstructTextObject( m, parents[0], "arc-length", *ret, false );
      }
      else if ( domelem.attribute( "type" ) == "distance_2pts" )
      {
        if ( parents.size() != 2 ) KIG_FILTER_PARSE_ERROR;
        ObjectTypeCalcer* so = new ObjectTypeCalcer( SegmentABType::instance(), parents );
        so->calc( *ret );
        oc = filtersConstructTextObject( m, so, "length", *ret, false );
      }
      else if ( domelem.attribute( "type" ) == "vector_norm" )
      {
        if ( parents.size() != 1 ) KIG_FILTER_PARSE_ERROR;
        oc = filtersConstructTextObject( m, parents[0], "length", *ret, false );
      }
      else if ( domelem.attribute( "type" ) == "vector_abscissa" )
      {
        if ( parents.size() != 1 ) KIG_FILTER_PARSE_ERROR;
        oc = filtersConstructTextObject( m, parents[0], "length-x", *ret, false );
      }
      else if ( domelem.attribute( "type" ) == "vector_ordinate" )
      {
        if ( parents.size() != 1 ) KIG_FILTER_PARSE_ERROR;
        oc = filtersConstructTextObject( m, parents[0], "length-y", *ret, false );
      }
      else if ( domelem.attribute( "type" ) == "slope" )
      {
        if ( parents.size() != 1 ) KIG_FILTER_PARSE_ERROR;
        oc = filtersConstructTextObject( m, parents[0], "slope", *ret, false );
      }
      else if ( domelem.attribute( "type" ) == "distance_pt_line" )
      {
        if ( parents.size() != 2 ) KIG_FILTER_PARSE_ERROR;
        std::vector<ObjectCalcer*> args;
        args.push_back( parents[1] );
        args.push_back( parents[0] );
        ObjectTypeCalcer* po = new ObjectTypeCalcer( LinePerpendLPType::instance(), args );
        po->calc( *ret );
        args.clear();
        args.push_back( parents[1] );
        args.push_back( po );
        ObjectTypeCalcer* io = new ObjectTypeCalcer( LineLineIntersectionType::instance(), args );
        io->calc( *ret );
        args.clear();
        args.push_back( parents[0] );
        args.push_back( io );
        ObjectTypeCalcer* so = new ObjectTypeCalcer( SegmentABType::instance(), args );
        so->calc( *ret );
        oc = filtersConstructTextObject( m, so, "length", *ret, false );
      }
      // types of 'equation'
      else if ( domelem.attribute( "type" ) == "line" )
      {
        if ( parents.size() != 1 ) KIG_FILTER_PARSE_ERROR;
        oc = filtersConstructTextObject( m, parents[0], "equation", *ret, false );
      }
      else if ( domelem.attribute( "type" ) == "circle" )
      {
        if ( parents.size() != 1 ) KIG_FILTER_PARSE_ERROR;
        oc = filtersConstructTextObject( m, parents[0], "simply-cartesian-equation", *ret, false );
      }
      else
      {
        notSupported( file, i18n( "This Dr. Geo file contains a \"%1 %2\" object, "
                                  "which Kig does not currently support." ).arg( domelem.tagName() ).arg(
                                  domelem.attribute( "type" ) ) );
        return false;
      }
      kdDebug() << "+++++++++ oc:" << oc << endl;
    }
// FIXME: this is the real angle meant by drgeo (angle + label with value)...
// pino: I'll use a simple one, to avoid compiling problem... :-(
/*
    else if ( domelem.tagName() == "angle" )
    {
      PointImp* p = static_cast<const PointImp*>( parents[1]->imp() );
      if ( domelem.attribute( "type" ) == "3pts" )
      {
        if ( parents.size() == 3 )
        {
          ObjectTypeCalcer* ao = new ObjectTypeCalcer( AngleType::instance(), parents );
          ao->calc( *ret );
          parents.clear();
          parents.push_back( ao );
        };
        if ( parents.size() != 1 ) KIG_FILTER_PARSE_ERROR;
        ObjectCalcer* angle = parents[0];
//        parents.clear();
//        const Coordinate c = static_cast<const PointImp*>( angle->parents()[1]->imp() )->coordinate();
        const Coordinate c = p->coordinate();
        oc = filtersConstructTextObject( c, angle, "angle-degrees", *ret, false );
      }
      kdDebug() << "+++++++++ oc:" << oc << endl;
    }
*/
// simple angle object...
    else if ( domelem.tagName() == "angle" )
    {
      if ( domelem.attribute( "type" ) == "3pts" )
        oc = new ObjectTypeCalcer( AngleType::instance(), parents );
      else
      {
        notSupported( file, i18n( "This Dr. Geo file contains a \"%1 %2\" object, "
                                  "which Kig does not currently support." ).arg( domelem.tagName() ).arg(
                                  domelem.attribute( "type" ) ) );
        return false;
      }
      kdDebug() << "+++++++++ oc:" << oc << endl;
    }
    else if ( domelem.tagName() == "script" )
    {
      QString xs;
      QString ys;
      QString text;
      for ( QDomNode c = domelem.firstChild(); ! c.isNull(); c = c.nextSibling() )
      {
        QDomElement ce = c.toElement();
        if ( ce.isNull() ) continue;
        else if ( ce.tagName() == "x" )
          xs = ce.text();
        else if ( ce.tagName() == "y" )
          ys = ce.text();
        else if ( ce.tagName() == "code" )
          text = ce.text();
      }
      bool ok;
      bool ok2;
      double x = xs.toDouble( &ok );
      double y = ys.toDouble( &ok2 );
      if ( ! ( ok && ok2 ) )
        KIG_FILTER_PARSE_ERROR;
      // since kig doesn't support Scheme scripts, it will write script's text
      // in a label, so the user can freely see the code and make whatever
      // he/she wants
      // possible idea: construct a new script object with the parents of scheme
      // one and the scheme code inserted commented... depends on a better
      // managing of arguments in scripts?
      if ( domelem.attribute( "type" ) == "nitems" )
        oc = fact->labelCalcer( text, Coordinate( x, y ), false, std::vector<ObjectCalcer*>(), *ret );
      else
      {
        notSupported( file, i18n( "This Dr. Geo file contains a \"%1 %2\" object, "
                                  "which Kig does not currently support." ).arg( domelem.tagName() ).arg(
                                  domelem.attribute( "type" ) ) );
        return false;
      }
    }
    else if ( domelem.tagName() == "locus" )
    {
      if ( domelem.attribute( "type" ) == "None" )
        oc = fact->locusCalcer( parents[0], parents[1] );
      else
      {
        notSupported( file, i18n( "This Dr. Geo file contains a \"%1 %2\" object, "
                                  "which Kig does not currently support." ).arg( domelem.tagName() ).arg(
                                  domelem.attribute( "type" ) ) );
        return false;
      }
      kdDebug() << "+++++++++ oc:" << oc << endl;
    }
    else if ( domelem.tagName() == "polygon" )
    {
      notSupported( file, i18n( "This Dr. Geo file contains a polygon object, "
                                "which Kig does not currently support." ) );
      return false;
    }
    else if ( domelem.tagName() == "boundingBox" )
    {
      // ignoring this element, since it isn't useful to us (for the moment)...
      nignored++;
    }
    else
    {
      kdDebug() << ">>>>>>>>> UNKNOWN OBJECT" << endl;
      notSupported( file, i18n( "This Dr. Geo file contains a \"%1 %2\" object, "
                                "which Kig does not currently support." ).arg( domelem.tagName() ).arg(
                                domelem.attribute( "type" ) ) );
      return false;
    }
    curid++;
    if ( oc == 0 )
      continue;

// reading color
    QColor co = domelem.attribute( "color" );
    if ( ! co.isValid() )
      co = Qt::blue;
// reading width and style
// Dashed -> the little one
// Normal -> the medium
// Thick  -> the biggest one
    int w = -1;
    Qt::PenStyle s = Qt::SolidLine;
    if ( domelem.tagName() == "point" )
    {
      if ( domelem.attribute( "thickness" ) == "Normal" )
        w = 7;
      else if ( domelem.attribute( "thickness" ) == "Thick" )
        w = 9;
    }
    else if ( ( domelem.tagName() == "line" ) ||
              ( domelem.tagName() == "halfLine" ) ||
              ( domelem.tagName() == "segment" ) ||
              ( domelem.tagName() == "vector" ) ||
              ( domelem.tagName() == "circle" ) ||
              ( domelem.tagName() == "arcCircle" ) ||
              ( domelem.tagName() == "angle" ) )
    {
      if ( domelem.attribute( "thickness" ) == "Dashed" )
        s = Qt::DotLine;
      if ( domelem.attribute( "thickness" ) == "Thick" )
        w = 2;
    }
    QString ps = domelem.attribute( "style" );
    int pointstyle = ObjectDrawer::pointStyleFromString( ps );
// show this object?
    bool show = ( ( domelem.attribute( "masked" ) != "True" ) &&
                  ( domelem.attribute( "masked" ) != "Alway" ) );
// costructing the ObjectDrawer*
    ObjectDrawer* d = new ObjectDrawer( co, w, show, s, pointstyle );
    assert( d );

// creating the ObjectHolder*
    ObjectHolder* o = new ObjectHolder( oc, d );
    holders.push_back( o );
// calc()
    kdDebug() << ">>>>>>>>> calc" << endl;
    holders[curid-1-nignored]->calc( *ret );
    oc = 0;
  }

  ret->addObjects( holders );
  CoordinateSystem* s = CoordinateSystemFactory::build( grid ? "Euclidean" : "Invisible" );
  if ( s ) ret->setCoordinateSystem( s );
  return ret;
}

KigFilterDrgeo* KigFilterDrgeo::instance()
{
  static KigFilterDrgeo f;
  return &f;
}
