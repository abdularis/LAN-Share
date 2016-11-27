/*
    Locally - LAN file transfer.
    Copyright (C) 2016 Abdul Aris R. <abdularisrahmanudin10@gmail.com>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef SENDERTABLEMODEL_H
#define SENDERTABLEMODEL_H

#include <QAbstractTableModel>

#include "sender.h"

class SenderTableModel : public QAbstractTableModel
{
public:
    SenderTableModel(QObject* parent = 0);

    virtual int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    virtual int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    void insertSender(Sender* sender);
    void clearCompleted();

    Sender* getSender(int index) const;
    void removeSender(int index);

    enum class Column : int {
        Destination = 0, FileName, FileSize, Status, Progress,
        Count
    };

private:

    QVector<Sender*> mSenders;
};

#endif // SENDERTABLEMODEL_H
