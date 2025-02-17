// SPDX-FileCopyrightText: 2002 Dominique Devriese <devriese@kde.org>

// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include "mode.h"

#include "../misc/rect.h"

#include <QPoint>
#include <vector>

class ObjectHolder;

/**
 * DragRectMode is a mode that provides a rect for selecting the
 * objects inside it.  Here's an example of how to use it
 * \code
 * DragRectMode d( e->pos(), document, widget );
 * mDoc.runMode( &d );
 * Objects sel = d.ret();
 * \endcode
 */
class DragRectMode : public KigMode
{
    QPoint mstart;
    std::vector<ObjectHolder *> mret;
    Rect mrect;
    bool mnc;
    bool mstartselected;
    bool mcancelled;

private:
    void clicked(const QPoint &p, KigWidget &w);
    void clicked(const QMouseEvent *e, KigWidget &w);
    void released(const QPoint &p, KigWidget &w, bool nc);
    void released(QMouseEvent *e, KigWidget &w);
    void moved(const QPoint &p, KigWidget &w);
    void moved(QMouseEvent *, KigWidget &w);

    void leftClicked(QMouseEvent *, KigWidget *) override;
    void leftMouseMoved(QMouseEvent *, KigWidget *) override;
    void leftReleased(QMouseEvent *, KigWidget *) override;
    void midClicked(QMouseEvent *, KigWidget *) override;
    void midMouseMoved(QMouseEvent *, KigWidget *) override;
    void midReleased(QMouseEvent *, KigWidget *) override;
    void rightClicked(QMouseEvent *, KigWidget *) override;
    void rightMouseMoved(QMouseEvent *, KigWidget *) override;
    void rightReleased(QMouseEvent *, KigWidget *) override;
    void mouseMoved(QMouseEvent *, KigWidget *) override;

    void cancelConstruction() override;

    void enableActions() override;

public:
    DragRectMode(const QPoint &start, KigPart &d, KigWidget &w);
    DragRectMode(KigPart &d, KigWidget &w);
    ~DragRectMode();

    /**
     * this returns the selected objects.
     */
    std::vector<ObjectHolder *> ret() const;

    /**
     * this returns the selected rect.
     */
    Rect rect() const;

    /**
     * this returns false if the control or shift button were pressed
     * when the mouse button was released, and true otherwise.  This is
     * because the user expects us to not clear the selection before
     * adding the newly selected objects if (s)he pressed control or
     * shift.
     */
    bool needClear() const;

    /**
     * whether the user cancelled the rect mode..  If this returns true,
     * all the other return data above will be in undefined state, so
     * first check this function's result.
     */
    bool cancelled() const;
};
