/*
 * GeogebraTransformer class for Kig
 * Copyright 2014  David E. Narvaez <david.narvaez@computer.org>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License or (at your option) version 3 or any later version
 * accepted by the membership of KDE e.V. (or its successor approved
 * by the membership of KDE e.V.), which shall act as a proxy
 * defined in Section 14 of version 3 of the license.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "geogebratransformer.h"

#include <kig/kig_document.h>
#include <objects/object_factory.h>
#include <objects/object_calcer.h>
#include <objects/object_holder.h>
#include <objects/bogus_imp.h>
#include <objects/object_type_factory.h>

#include <KZip>
#include <KDebug>

#include <QFile>
#include <QXmlQuery>
#include <QXmlName>

void GeogebraTransformer::atomicValue( const QVariant& )
{
  // I wish I could use this...
}

void GeogebraTransformer::attribute( const QXmlName& name, const QStringRef& value )
{
  if( name.localName( m_np ) == QLatin1String( "label" ) )
  {
    const QByteArray objectLabel = value.toAscii();
    bool isDoubleValue;
    const double dblval = value.toString().toDouble( &isDoubleValue );

    switch( m_currentState )
    {
    case GeogebraTransformer::ReadingObject:
      m_currentObjectLabel = value.toAscii();
      // There must be a better place for this
      m_currentState = GeogebraTransformer::ReadingArguments;

      break;
    case GeogebraTransformer::ReadingArguments:
      if( isDoubleValue )
      {
        /* This is to handle the circle-point-radius, dilate (and similar) type of Geogebra objects.
         * <command name="Circle">
         * <input a0="A" a1="3"/>
         * <output a0="c"/>
         *
         * Notice the attribute 'a1' of the 'input' element. The value - '3' is the radius of the circle.
         * First, we try to convert that value to Double. If the conversion suceeds, we stack a DoubleImp (Calcer)
         * in the the m_currentArgStack and break. Otherwise, we check the m_objectMap for that label entry.
         */
        DoubleImp * doubleImp = new DoubleImp( dblval );
        m_currentArgStack.push_back( new ObjectConstCalcer( doubleImp ) );
      }
      else if( m_objectMap.contains( objectLabel ) )
      {
        m_currentArgStack.push_back( m_objectMap[objectLabel] );
      }
      else
      {
//         parseError( i18n( "Referenced object %1 does not exist", value.toString() ) );
      }

      break;
    default:
      break;
    }
  }
  else if ( name.localName( m_np ) == QLatin1String( "value" ) )
  {
    Q_ASSERT( m_currentState == ReadingDouble );
    DoubleImp * doubleImp = new DoubleImp( value.toString().toDouble() );

    m_currentArgStack.push_back( new ObjectConstCalcer( doubleImp ) );
  }
  else if ( name.localName( m_np ) == QLatin1String( "Name" ) )
  {
    m_sections[m_nsections - 1].setName( value.toString() );
  }
  else if ( name.localName( m_np ) == QLatin1String( "Description" ) )
  {
    m_sections[m_nsections - 1].setDescription( value.toString() );
  }
  else if ( name.localName( m_np ) == QLatin1String( "Input" ) )
  {
    m_inputObjectLabels.insert( value.toAscii() );
  }
  else if ( name.localName( m_np ) == QLatin1String( "Output" ) )
  {
    m_outputObjectLabels.insert( value.toAscii() );
  }
}

void GeogebraTransformer::characters( const QStringRef& )
{
}

void GeogebraTransformer::comment( const QString& )
{
}

void GeogebraTransformer::endDocument()
{
}

void GeogebraTransformer::endElement()
{
  switch( m_currentState )
  {
  case GeogebraTransformer::ReadingObject:
    if ( m_currentObject )
    {
      if ( !m_objectMap.contains( m_currentObjectLabel ) )
      {
        ObjectTypeCalcer * oc = new ObjectTypeCalcer( m_currentObject, m_currentArgStack );

        oc->calc( *m_document );
        m_objectMap.insert( m_currentObjectLabel, oc );

        // Decide where to put this object
        if( m_inputObjectLabels.empty() )
        {
          // Not handling input/output objects, put everything in second
          m_sections[m_nsections - 1].addOutputObject( oc );
        }
        else
        {
          if( m_inputObjectLabels.contains( m_currentObjectLabel ) )
          {
            m_sections[m_nsections - 1].addInputObject( oc );
          }
          else if ( m_outputObjectLabels.contains( m_currentObjectLabel ) )
          {
            m_sections[m_nsections - 1].addOutputObject( oc );
          }
        }
      }

      m_currentArgStack.clear();
      m_currentObject = nullptr;
    }

    break;
  case GeogebraTransformer::ReadingArguments:
  case GeogebraTransformer::ReadingDouble:
    /* We do this because every argument may be the
     * last argument of the stack. If it is not, then
     * startElement() will reset this to ReadingArguments
     */
    m_currentState = GeogebraTransformer::ReadingObject;

    break;
  default:
    break;
  }
}

void GeogebraTransformer::endOfSequence()
{
}

void GeogebraTransformer::namespaceBinding( const QXmlName& )
{
}

void GeogebraTransformer::processingInstruction( const QXmlName&, const QString& )
{
}

void GeogebraTransformer::startDocument()
{
}

void GeogebraTransformer::startElement( const QXmlName& name )
{
  if( name.localName( m_np ) == "Section" )
  {
    m_nsections++;
    m_sections.push_back( GeogebraSection() );

    // Clear stacks
    m_inputObjectLabels.clear();
    m_outputObjectLabels.clear();
    m_currentArgStack.clear();
    m_objectMap.clear();
    return;
  }

  switch( m_currentState )
  {
  case GeogebraTransformer::ReadingObject:
    if( m_currentObject )
    {
      // We are already building an object
      m_currentState = GeogebraTransformer::ReadingArguments;
      startElement( name );
      return;
    }

    {
      const QByteArray nameData = name.localName( m_np ).toLatin1();
      m_currentObject = ObjectTypeFactory::instance()->find( nameData );

      if ( !m_currentObject )
      {
        kWarning() << name.localName( m_np ) << " object not found!";
      }
    }

    break;
  case GeogebraTransformer::ReadingArguments:
    if ( name.localName( m_np ) == QLatin1String( "Double" ) )
    {
      m_currentState = GeogebraTransformer::ReadingDouble;
    }

    break;
  default:
    break;
  }
}

void GeogebraTransformer::startOfSequence()
{
}
