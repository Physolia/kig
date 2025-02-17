/*
    This file is part of Kig, a KDE program for Interactive Geometry...
    SPDX-FileCopyrightText: 2002 Dominique Devriese <devriese@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include "coordinate.h"

#include <QDebug>
#include <QRect>

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
    Rect(const Coordinate &bottomLeft, const Coordinate &topRight);
    Rect(const Coordinate &bottomLeft, const double width, const double height);
    Rect(double xa, double ya, double width, double height);
    Rect(const Rect &r);
    Rect();
    static Rect invalidRect();

    bool valid();

    void setBottomLeft(const Coordinate &p);
    void setTopLeft(const Coordinate &p);
    void setTopRight(const Coordinate &p);
    void setBottomRight(const Coordinate &p);
    void setCenter(const Coordinate &p);
    void setLeft(const double p);
    void setRight(const double p);
    void setTop(const double p);
    void setBottom(const double p);
    void setWidth(const double w);
    void setHeight(const double h);
    /**
     * this makes sure width and height are > 0 ...
     */
    void normalize();
    /**
     * this makes sure p is in the rect, extending it if necessary...
     */
    void setContains(const Coordinate &p);
    /**
     * Assignment operator.
     */
    Rect &operator=(const Rect &other);
    /**
     * moves the rect while keeping the size constant...
     */
    void moveBy(const Coordinate &p);
    /**
     * synonym for moveBy...
     */
    Rect &operator+=(const Coordinate &p)
    {
        moveBy(p);
        return *this;
    }
    /**
     * scale: only the size changes, topLeft is kept where it is...
     */
    void scale(const double r);
    /**
     * synonym for scale...
     */
    Rect &operator*=(const double r)
    {
        scale(r);
        return *this;
    }
    Rect &operator/=(const double r)
    {
        scale(1 / r);
        return *this;
    }

    /**
     * This expands the rect so that it contains r.  It has friends
     * '|=' and '|' below...
     */
    void eat(const Rect &r);

    /**
     * synonym for eat.
     */
    Rect &operator|=(const Rect &rhs)
    {
        eat(rhs);
        return *this;
    }

    /**
     * return a rect which is a copy of this rect, but has an aspect
     * ratio equal to rhs's one..  if \p shrink is true, the rect will be
     * shrunk, otherwise extended..  The center of the new rect is the
     * same as this rect's center..
     */
    Rect matchShape(const Rect &rhs, bool shrink = false) const;

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
    bool contains(const Coordinate &p) const;
    bool contains(const Coordinate &p, double allowed_miss) const;
    bool intersects(const Rect &p) const;
    Rect normalized() const;
    friend QDebug &operator<<(QDebug &s, const Rect &t);

    static Rect fromQRect(const QRect &);

protected:
    Coordinate mBottomLeft;
    double mwidth;
    double mheight;
};

bool operator==(const Rect &r, const Rect &s);
QDebug &operator<<(QDebug &s, const Rect &t);
/**
 * this operator returns a Rect that contains both the given
 * rects.
 */
Rect operator|(const Rect &lhs, const Rect &rhs);
