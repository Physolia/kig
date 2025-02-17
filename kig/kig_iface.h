/*
    This file is part of Kig, a KDE program for Interactive Geometry...
    SPDX-FileCopyrightText: 2002 Dominique Devriese <devriese@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include <dcopobject.h>

class KigIface : virtual public DCOPObject
{
    K_DCOP
public:
    KigIface();
    ~KigIface();
    k_dcop : virtual void openUrl(const QString &s) = 0;
};
