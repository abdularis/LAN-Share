/*
    LANShare - LAN file transfer.
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



#ifndef TRANSFERTABLEMODEL_H
#define TRANSFERTABLEMODEL_H

#include <QAbstractTableModel>
#include <QColor>

#include "transfer/transfer.h"
#include "transferinfo.h"

class TransferTableModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    explicit TransferTableModel(QObject *parent = nullptr);
    ~TransferTableModel() override;

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    void insertTransfer(Transfer* t);
    void clearCompleted();

    Transfer* getTransfer(int index) const;
    TransferInfo* getTransferInfo(int index) const;

    void removeTransfer(int index);

    enum class Column : int {
        Peer = 0, FileName, FileSize, State, Progress,
        Count
    };

private:
    QString getStateString(TransferState state) const;
    QColor getStateColor(TransferState state) const;

    QVector<Transfer*> mTransfers;

};

#endif // TRANSFERTABLEMODEL_H
