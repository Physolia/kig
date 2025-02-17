// SPDX-FileCopyrightText: 2003 Dominique Devriese <devriese@kde.org>

// SPDX-License-Identifier: GPL-2.0-or-later

#include "kseg-filter.h"

#include "filters-common.h"
#include "kseg-defs.h"

#include "../kig/kig_document.h"
#include "../kig/kig_part.h"
#include "../misc/coordinate.h"
#include "../objects/angle_type.h"
#include "../objects/arc_type.h"
#include "../objects/bogus_imp.h"
#include "../objects/circle_type.h"
#include "../objects/conic_imp.h"
#include "../objects/conic_types.h"
#include "../objects/intersection_types.h"
#include "../objects/line_imp.h"
#include "../objects/line_type.h"
#include "../objects/object_calcer.h"
#include "../objects/object_drawer.h"
#include "../objects/object_factory.h"
#include "../objects/object_holder.h"
#include "../objects/other_imp.h"
#include "../objects/other_type.h"
#include "../objects/point_imp.h"
#include "../objects/point_type.h"
#include "../objects/polygon_imp.h"
#include "../objects/polygon_type.h"
#include "../objects/transform_types.h"
#include "../objects/vector_type.h"

#include <QBrush>
#include <QBuffer>
#include <QDataStream>
#include <QFile>
#include <QFont>
#include <QPen>

KigFilterKSeg::KigFilterKSeg()
{
}

KigFilterKSeg::~KigFilterKSeg()
{
}

bool KigFilterKSeg::supportMime(const QString &mime)
{
    return mime == QLatin1String("application/x-kseg");
}

struct drawstyle {
    qint8 pointstyle;
    QFont font;
    QPen pen;
    QBrush brush;
};

static Coordinate readKSegCoordinate(QDataStream &stream)
{
    // read the coord..
    float inx, iny;
    stream >> inx >> iny;
    // KSeg uses a coordinate system, where the topleft is (0,0), and
    // the bottom right is the widget coordinate in the window: if the
    // KSeg window had a width of 600 pixels and a height of 600, then
    // the bottom right will be at (600,600).  We assume a window of
    // such a height here, and transform it into Kig Coordinates.  This
    // function is quite similar to ScreenInfo::fromScreen, and it's
    // basically a simple modification of that code to floats..

    // invert the y-axis: 0 is at the bottom !
    Coordinate t(inx, 600 - iny);
    t *= 14;
    t /= 600;
    return t + Coordinate(-7, -7);
}

static ObjectTypeCalcer *intersectionPoint(const std::vector<ObjectCalcer *> &parents, int which)
{
    if (parents.size() != 2)
        return nullptr;
    int nlines = 0;
    int nconics = 0;
    int narcs = 0;
    for (int i = 0; i < 2; ++i) {
        if (parents[i]->imp()->inherits(AbstractLineImp::stype()))
            ++nlines;
        else if (parents[i]->imp()->inherits(ConicImp::stype()))
            ++nconics;
        else if (parents[i]->imp()->inherits(ArcImp::stype()))
            ++narcs;
        else
            return nullptr;
    };
    if (nlines == 2)
        return which == -1 ? new ObjectTypeCalcer(LineLineIntersectionType::instance(), parents) : nullptr;
    else if (nlines == 1 && nconics == 1) {
        std::vector<ObjectCalcer *> intparents(parents);
        intparents.push_back(new ObjectConstCalcer(new IntImp(which)));
        return new ObjectTypeCalcer(ConicLineIntersectionType::instance(), intparents);
    } else if (nlines == 0 && nconics == 2) {
        std::vector<ObjectCalcer *> rparents(parents);
        rparents.push_back(new ObjectConstCalcer(new IntImp(1)));
        rparents.push_back(new ObjectConstCalcer(new IntImp(1)));
        rparents.push_back(new ObjectTypeCalcer(ConicRadicalType::instance(), rparents));
        std::vector<ObjectCalcer *> iparents;
        iparents.push_back(parents[0]);
        iparents.push_back(rparents.back());
        iparents.push_back(new ObjectConstCalcer(new IntImp(which)));
        return new ObjectTypeCalcer(ConicLineIntersectionType::instance(), iparents);
    } else if (nlines == 1 && narcs == 1) {
        std::vector<ObjectCalcer *> intparents(parents);
        intparents.push_back(new ObjectConstCalcer(new IntImp(which)));
        return new ObjectTypeCalcer(ArcLineIntersectionType::instance(), intparents);
    } else
        return nullptr;
}

ObjectCalcer *KigFilterKSeg::transformObject(KigDocument &kigdoc, std::vector<ObjectCalcer *> &parents, int subtype, bool &ok)
{
    ok = true;
    ObjectCalcer *retobj = nullptr;
    switch (subtype) {
    case G_TRANSLATED: {
        std::vector<ObjectCalcer *> vectorparents(parents.begin() + 1, parents.end());
        ObjectTypeCalcer *vector = new ObjectTypeCalcer(VectorType::instance(), vectorparents);
        vector->calc(kigdoc);

        std::vector<ObjectCalcer *> transparents;
        transparents.push_back(parents[0]);
        transparents.push_back(vector);
        retobj = new ObjectTypeCalcer(TranslatedType::instance(), transparents);
        break;
    }
    case G_ROTATED: {
        std::vector<ObjectCalcer *> angleparents(parents.begin() + 2, parents.end());
        ObjectTypeCalcer *angle = new ObjectTypeCalcer(AngleType::instance(), angleparents);
        angle->calc(kigdoc);

        std::vector<ObjectCalcer *> rotparents;
        rotparents.push_back(parents[0]);
        rotparents.push_back(parents[1]);
        rotparents.push_back(angle);
        retobj = new ObjectTypeCalcer(RotationType::instance(), rotparents);
        break;
    }
    case G_SCALED: {
        if (parents.size() == 4) {
            retobj = new ObjectTypeCalcer(ScalingOverCenter2Type::instance(), parents);
        } else {
            // TODO
            notSupported(
                i18n("This KSeg document uses a scaling "
                     "transformation, which Kig currently "
                     "cannot import."));
            ok = false;
            return nullptr;
        }
        break;
    }
    case G_REFLECTED: {
        std::vector<ObjectCalcer *> mirparents(parents.begin(), parents.end());
        retobj = new ObjectTypeCalcer(LineReflectionType::instance(), mirparents);
        break;
    }
    }

    return retobj;
}

KigDocument *KigFilterKSeg::load(const QString &file)
{
    QFile ffile(file);
    if (!ffile.open(QIODevice::ReadOnly)) {
        fileNotFound(file);
        return nullptr;
    };

    KigDocument *retdoc = new KigDocument();

    QDataStream fstream(&ffile);

    QString versionstring;
    fstream >> versionstring;
    if (!versionstring.startsWith(QLatin1String("KSeg Document Version ")))
        KIG_FILTER_PARSE_ERROR;

    QByteArray array;
    fstream >> array;
    QBuffer buf(&array);
    buf.open(QIODevice::ReadOnly);
    QDataStream stream(&buf);

    stream.setVersion(3);

    // G_drawstyles:
    short numstyles;
    stream >> numstyles;
    std::vector<drawstyle> drawstyles(numstyles);
    for (short i = 0; i < numstyles; ++i) {
        stream >> drawstyles[i].pointstyle;
        stream >> drawstyles[i].font;
        stream >> drawstyles[i].pen;
        stream >> drawstyles[i].brush;
    };

    std::vector<ObjectHolder *> ret;
    std::vector<ObjectHolder *> ret2;

    // G_refs
    unsigned int count;
    stream >> count;

    ret.resize(count, nullptr);
    const ObjectFactory *fact = ObjectFactory::instance();

    // KSeg topologically sorts the objects before saving, that means we
    // can read the entire file in one iteration..
    for (uint i = 0; i < count; ++i) {
        short styleid;
        stream >> styleid;
        short nparents;
        stream >> nparents;
        std::vector<ObjectCalcer *> parents(nparents, nullptr);
        for (short j = 0; j < nparents; ++j) {
            int parent;
            stream >> parent;
            parents[j] = ret[parent]->calcer();
        };

        // read the object..
        short info;
        stream >> info;
        int type = 1 << (info & 31);
        info >>= 5;
        int descendtype = (info & 15);
        info >>= 4;
        bool visible = info & 1;
        bool labelVisible = info & 2;
        bool given = info & 4;
        bool final = info & 8;

        // avoid g++ warnings about unused vars..
        // this doesn't really do anything..
        (void)given;
        (void) final;

        drawstyle style = drawstyles[styleid];

        if (type == G_LOOP)
            continue;
        // read the label..
        QString labeltext;
        stream >> labeltext;
        Coordinate relcoord = readKSegCoordinate(stream);
        // shut up gcc
        (void)relcoord;
        if (type & G_CURVE) {
            Coordinate relcurvecoord = readKSegCoordinate(stream);
            // shut up gcc
            (void)relcurvecoord;
        };

        // now load the object data..
        ObjectHolder *object = nullptr;
        ObjectCalcer *o = nullptr;
        bool ok = true;

        QColor color = style.pen.color();
        int width = style.pen.width();

        /*
            qDebug() << "type: " << type
                      << "descendtype: " << descendtype << endl
                      << "label: " << labeltext << endl;
        //*/

        switch (type) {
        case G_POINT: {
            switch (descendtype) {
            case G_TRANSLATED:
            case G_ROTATED:
            case G_SCALED:
            case G_REFLECTED: {
                o = transformObject(*retdoc, parents, descendtype, ok);
                break;
            }
            case G_FREE_POINT: {
                // fixed point
                if (nparents != 0)
                    KIG_FILTER_PARSE_ERROR;
                Coordinate c = readKSegCoordinate(stream);
                o = fact->fixedPointCalcer(c);
                break;
            }
            case G_CONSTRAINED_POINT: {
                // constrained point
                double p;
                stream >> p;
                if (nparents != 1)
                    KIG_FILTER_PARSE_ERROR;
                ObjectCalcer *parent = parents[0];
                assert(parent);
                o = fact->constrainedPointCalcer(parent, p);
                break;
            }
            case G_INTERSECTION_POINT: {
                // KSeg has somewhat weird intersection objects..
                // for all objects G_INTERSECTION_POINT gets the
                // first intersection of its parents, G_INTERSECTION2_POINT
                // represents the second, if present.
                o = intersectionPoint(parents, -1);
                if (!o)
                    KIG_FILTER_PARSE_ERROR;
                break;
            }
            case G_INTERSECTION2_POINT: {
                o = intersectionPoint(parents, 1);
                if (!o)
                    KIG_FILTER_PARSE_ERROR;
                break;
            }
            case G_MID_POINT: {
                // midpoint of a segment..
                if (parents.size() != 1)
                    KIG_FILTER_PARSE_ERROR;
                if (!parents[0]->imp()->inherits(SegmentImp::stype()))
                    KIG_FILTER_PARSE_ERROR;
                //          int index = parents[0]->imp()->propertiesInternalNames().indexOf( "mid-point" );
                //          assert( index != -1 );
                o = new ObjectPropertyCalcer(parents[0], "mid-point");
                break;
            }
            default:
                KIG_FILTER_PARSE_ERROR;
            };
            width = style.pointstyle == SMALL_CIRCLE ? 2 : style.pointstyle == MEDIUM_CIRCLE ? 3 : 5;
            color = style.brush.color();
            break;
        };
        case G_SEGMENT: {
            switch (descendtype) {
            case G_TRANSLATED:
            case G_ROTATED:
            case G_SCALED:
            case G_REFLECTED: {
                o = transformObject(*retdoc, parents, descendtype, ok);
                break;
            }
            case G_ENDPOINTS_SEGMENT: {
                if (nparents != 2)
                    KIG_FILTER_PARSE_ERROR;
                o = new ObjectTypeCalcer(SegmentABType::instance(), parents);
                break;
            }
            default:
                KIG_FILTER_PARSE_ERROR;
            }
            break;
        };
        case G_RAY: {
            switch (descendtype) {
            case G_TRANSLATED:
            case G_ROTATED:
            case G_SCALED:
            case G_REFLECTED: {
                o = transformObject(*retdoc, parents, descendtype, ok);
                break;
            }
            case G_TWOPOINTS_RAY: {
                if (nparents != 2)
                    KIG_FILTER_PARSE_ERROR;
                o = new ObjectTypeCalcer(RayABType::instance(), parents);
                break;
            }
            case G_BISECTOR_RAY: {
                ObjectTypeCalcer *angle = new ObjectTypeCalcer(HalfAngleType::instance(), parents);
                angle->calc(*retdoc);
                o = fact->propertyObjectCalcer(angle, "angle-bisector");
                break;
            }
            default:
                KIG_FILTER_PARSE_ERROR;
            };
            break;
        };
        case G_LINE: {
            switch (descendtype) {
            case G_TRANSLATED:
            case G_ROTATED:
            case G_SCALED:
            case G_REFLECTED: {
                o = transformObject(*retdoc, parents, descendtype, ok);
                break;
            }
            case G_TWOPOINTS_LINE: {
                if (nparents != 2)
                    KIG_FILTER_PARSE_ERROR;
                o = new ObjectTypeCalcer(LineABType::instance(), parents);
                break;
            }
            case G_PARALLEL_LINE: {
                if (nparents != 2)
                    KIG_FILTER_PARSE_ERROR;
                o = new ObjectTypeCalcer(LineParallelLPType::instance(), parents);
                break;
            }
            case G_PERPENDICULAR_LINE: {
                if (nparents != 2)
                    KIG_FILTER_PARSE_ERROR;
                o = new ObjectTypeCalcer(LinePerpendLPType::instance(), parents);
                break;
            }
            default:
                KIG_FILTER_PARSE_ERROR;
            };
            break;
        };
        case G_CIRCLE: {
            switch (descendtype) {
            case G_TRANSLATED:
            case G_ROTATED:
            case G_SCALED:
            case G_REFLECTED: {
                o = transformObject(*retdoc, parents, descendtype, ok);
                break;
            }
            case G_CENTERPOINT_CIRCLE: {
                if (nparents != 2)
                    KIG_FILTER_PARSE_ERROR;
                o = new ObjectTypeCalcer(CircleBCPType::instance(), parents);
                break;
            }
            case G_CENTERRADIUS_CIRCLE: {
                ObjectCalcer *point;
                ObjectCalcer *segment;
                if (parents[0]->imp()->inherits(PointImp::stype())) {
                    point = parents[0];
                    segment = parents[1];
                } else {
                    point = parents[1];
                    segment = parents[0];
                };
                int index = segment->imp()->propertiesInternalNames().indexOf("length");
                if (index == -1)
                    KIG_FILTER_PARSE_ERROR;
                ObjectPropertyCalcer *length = new ObjectPropertyCalcer(segment, "length");
                length->calc(*retdoc);
                std::vector<ObjectCalcer *> cparents;
                cparents.push_back(point);
                cparents.push_back(length);
                o = new ObjectTypeCalcer(CircleBPRType::instance(), cparents);
                break;
            }
            default:
                KIG_FILTER_PARSE_ERROR;
            };
            break;
        };
        case G_ARC: {
            switch (descendtype) {
            case G_TRANSLATED:
            case G_ROTATED:
            case G_SCALED:
            case G_REFLECTED: {
                o = transformObject(*retdoc, parents, descendtype, ok);
                break;
            }
            case G_THREEPOINTS_ARC: {
                if (nparents != 3)
                    KIG_FILTER_PARSE_ERROR;
                o = new ObjectTypeCalcer(ArcBTPType::instance(), parents);
                break;
            }
            default:
                KIG_FILTER_PARSE_ERROR;
            }
            break;
        };
        case G_POLYGON: {
            switch (descendtype) {
            case G_TRANSLATED:
            case G_ROTATED:
            case G_SCALED:
            case G_REFLECTED: {
                o = transformObject(*retdoc, parents, descendtype, ok);
                break;
            }
            default: {
                if (nparents < 3)
                    KIG_FILTER_PARSE_ERROR;
                o = new ObjectTypeCalcer(PolygonBNPType::instance(), parents);
            }
            }
            //        default:
            //          KIG_FILTER_PARSE_ERROR;
            break;
        };
        case G_CIRCLEINTERIOR: {
            notSupported(
                i18n("This KSeg file contains a filled circle, "
                     "which Kig does not currently support."));
            return nullptr;
        };
        case G_ARCSECTOR: {
            notSupported(
                i18n("This KSeg file contains an arc sector, "
                     "which Kig does not currently support."));
            return nullptr;
        };
        case G_ARCSEGMENT: {
            notSupported(
                i18n("This KSeg file contains an arc segment, "
                     "which Kig does not currently support."));
            return nullptr;
        };
        case G_LOCUS: {
            switch (descendtype) {
            case G_TRANSLATED:
            case G_ROTATED:
            case G_SCALED:
            case G_REFLECTED: {
                o = transformObject(*retdoc, parents, descendtype, ok);
                break;
            }
            case G_OBJECT_LOCUS: {
                if (nparents != 2)
                    KIG_FILTER_PARSE_ERROR;
                o = fact->locusCalcer(parents[0], parents[1]);
                break;
            }
            default:
                KIG_FILTER_PARSE_ERROR;
            }
            break;
        };
        case G_MEASURE: {
            QByteArray prop;
            Coordinate txtcoords = readKSegCoordinate(stream);
            switch (descendtype) {
            case G_DISTANCE_MEASURE: {
                if (nparents != 2)
                    KIG_FILTER_PARSE_ERROR;
                ObjectCalcer *c = new ObjectTypeCalcer(SegmentABType::instance(), parents);
                c->calc(*retdoc);
                parents.clear();
                parents.push_back(c);
                prop = "length";
                break;
            }
            case G_LENGTH_MEASURE: {
                if (nparents != 1)
                    KIG_FILTER_PARSE_ERROR;
                if (parents[0]->imp()->inherits(SegmentImp::stype()))
                    prop = "length";
                else if (parents[0]->imp()->inherits(CircleImp::stype()))
                    prop = "circumference";
                else
                    KIG_FILTER_PARSE_ERROR;
                break;
            }
            case G_RADIUS_MEASURE: {
                if (nparents != 1)
                    KIG_FILTER_PARSE_ERROR;
                if (!parents[0]->imp()->inherits(CircleImp::stype()))
                    KIG_FILTER_PARSE_ERROR;
                prop = "radius";
                break;
            }
            case G_ANGLE_MEASURE: {
                if (nparents != 3)
                    KIG_FILTER_PARSE_ERROR;
                ObjectTypeCalcer *ao = new ObjectTypeCalcer(AngleType::instance(), parents);
                ao->calc(*retdoc);
                parents.clear();
                parents.push_back(ao);
                prop = "angle-degrees";
                break;
            }
                //        case G_RATIO_MEASURE: // TODO
            case G_SLOPE_MEASURE: {
                if (nparents != 1)
                    KIG_FILTER_PARSE_ERROR;
                if (!parents[0]->imp()->inherits(AbstractLineImp::stype()))
                    KIG_FILTER_PARSE_ERROR;
                prop = "slope";
                break;
            }
            case G_AREA_MEASURE: {
                if (nparents != 1)
                    KIG_FILTER_PARSE_ERROR;
                if (parents[0]->imp()->inherits(FilledPolygonImp::stype()) || parents[0]->imp()->inherits(FilledPolygonImp::stype3())
                    || parents[0]->imp()->inherits(FilledPolygonImp::stype4()))
                    prop = "polygon-surface";
                else
                    KIG_FILTER_PARSE_ERROR;
                break;
            }
            default:
                KIG_FILTER_PARSE_ERROR;
            }
            if (parents.size() != 1)
                KIG_FILTER_PARSE_ERROR;
            o = filtersConstructTextObject(txtcoords, parents[0], prop, *retdoc, false);
            break;
        }
        case G_CALCULATE:
            KIG_FILTER_PARSE_ERROR;
        case G_ANNOTATION:
            KIG_FILTER_PARSE_ERROR;
        case G_LOOP:
            KIG_FILTER_PARSE_ERROR;
        default:
            KIG_FILTER_PARSE_ERROR;
        }

        // checking if the object was correctly created
        if (!o) {
            if (ok)
                KIG_FILTER_PARSE_ERROR
            else
                return nullptr;
        }

        ObjectDrawer *d = new ObjectDrawer(color, width, visible, style.pen.style());
        if (!labeltext.isEmpty()) {
            ObjectConstCalcer *name = new ObjectConstCalcer(new StringImp(labeltext));
            object = new ObjectHolder(o, d, name);
        } else {
            object = new ObjectHolder(o, d);
        }

        assert(object);
        ret[i] = object;
        object->calc(*retdoc);
        if (!labeltext.isEmpty() && labelVisible && object->imp()->inherits(PointImp::stype())) {
            std::vector<ObjectCalcer *> args2;
            args2.push_back(object->nameCalcer());
            ObjectCalcer *oc2 = fact->attachedLabelCalcer(QStringLiteral("%1"),
                                                          object->calcer(),
                                                          static_cast<const PointImp *>(object->imp())->coordinate(),
                                                          false,
                                                          args2,
                                                          *retdoc);
            oc2->calc(*retdoc);
            ObjectDrawer *d2 = new ObjectDrawer(style.pen.color());
            ObjectHolder *o2 = new ObjectHolder(oc2, d2);
            ret2.push_back(o2);
        }
    };

    // selection groups ( we ignore them, but we pretend to read them
    // out anyway, so we can find what comes after them.. )
    int selgroupcount;
    stream >> selgroupcount;
    for (int i = 0; i < selgroupcount; ++i) {
        QString name;
        stream >> name;
        int size;
        stream >> size;
        for (int i = 0; i < size; ++i) {
            short object;
            stream >> object;
            (void)object;
        };
    };

    // no more data in the file.
    retdoc->addObjects(ret);
    retdoc->addObjects(ret2);
    retdoc->setAxes(false);
    retdoc->setGrid(false);
    return retdoc;
}

KigFilterKSeg *KigFilterKSeg::instance()
{
    static KigFilterKSeg f;
    return &f;
}
