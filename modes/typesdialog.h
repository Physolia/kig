/*
    This file is part of Kig, a KDE program for Interactive Geometry...
    SPDX-FileCopyrightText: 2002 Dominique Devriese <devriese@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include "../misc/lists.h"
#include <QAbstractTableModel>
#include <QDialog>

#include <vector>

class QMenu;
class KigPart;
class Ui_TypesWidget;
class BaseListElement;

/**
 * A model for representing the data.
 */
class TypesModel : public QAbstractTableModel
{
    Q_OBJECT

    std::vector<BaseListElement *> melems;

public:
    explicit TypesModel(QObject *parent = nullptr);
    virtual ~TypesModel();

    void addMacros(const std::vector<Macro *> &macros);
    void removeElements(const QModelIndexList &elems);

    void clear();

    void elementChanged(const QModelIndex &index);

    bool isMacro(const QModelIndex &index) const;
    Macro *macroFromIndex(const QModelIndex &index) const;

    // reimplementations from QAbstractTableModel
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
};

/**
 * Manage the macro types...
 */
class TypesDialog : public QDialog
{
    Q_OBJECT

    // necessary because some MacroList functions need it..
    KigPart &mpart;
    QMenu *popup;
    Ui_TypesWidget *mtypeswidget;
    TypesModel *mmodel;

public:
    TypesDialog(QWidget *parent, KigPart &);
    ~TypesDialog();

private Q_SLOTS:
    void slotHelp();
    void slotOk();
    void slotCancel();

    void deleteType();
    void exportType();
    void importTypes();
    void editType();

    void typeListContextMenu(const QPoint &);

private:
    QModelIndexList selectedRows() const;
    // This method is called in the importTypes() slot in case the file being imported is a geogebra-tool file.
    bool loadGeogebraTools(const QString &, std::vector<Macro *> &, KigPart &);
};
