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


#include "transfertablemodel.h"
#include "util.h"

TransferTableModel::TransferTableModel(QObject *parent) :
    QAbstractTableModel(parent)
{
}

TransferTableModel::~TransferTableModel()
{
    for (Transfer* t : mTransfers) {
        delete t;
    }
}

int TransferTableModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return mTransfers.size();
}

int TransferTableModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return (int) Column::Count;
}

QVariant TransferTableModel::data(const QModelIndex &index, int role) const
{
    if (index.isValid()) {
        TransferInfo* info = mTransfers.at(index.row())->getTransferInfo();

        if (info) {
            Column col = (Column) index.column();

            if (role == Qt::DisplayRole) {
                switch (col) {
                case Column::Peer : return info->getPeer().getName();
                case Column::FileName : return info->getFilePath();
                case Column::FileSize : return Util::sizeToString(info->getDataSize());
                case Column::State : return getStateString(info->getState());
                case Column::Progress : return info->getProgress();
                }
            }
            else if (role == Qt::ForegroundRole && col == Column::State) {
                return getStateColor(info->getState());
            }
        }
    }

    return QVariant();
}

QVariant TransferTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        Column col = (Column) section;
        switch (col) {
        case Column::Peer : return tr("Peer");
        case Column::FileName : return tr("File path");
        case Column::FileSize : return tr("Size");
        case Column::State : return tr("Status");
        case Column::Progress : return tr("Progress");
        }
    }

    return QVariant();
}

void TransferTableModel::insertTransfer(Transfer *t)
{
    if (t) {
        beginInsertRows(QModelIndex(), 0, 0);
        mTransfers.prepend(t);
        endInsertRows();
        emit dataChanged(index(1, 0), index(mTransfers.size()-1, (int) Column::Count));

        TransferInfo* info = t->getTransferInfo();
        connect(info, &TransferInfo::fileOpened, [=]() {
            int idx = mTransfers.indexOf(info->getOwner());
            QModelIndex fNameIdx = index(idx, (int) Column::FileName);
            QModelIndex fSizeIdx = index(idx, (int) Column::FileSize);
            emit dataChanged(fNameIdx, fSizeIdx);
        });

        connect(info, &TransferInfo::stateChanged, [=](TransferState state) {
            Q_UNUSED(state);

            int idx = mTransfers.indexOf(info->getOwner());
            QModelIndex stateIdx = index(idx, (int) Column::State);
            emit dataChanged(stateIdx, stateIdx);
        });
    }
}

void TransferTableModel::clearCompleted()
{
    for (int i = 0; i < mTransfers.size(); i++) {
        Transfer* t = mTransfers.at(i);
        TransferState state = t->getTransferInfo()->getState();
        if (state == TransferState::Idle ||
                state == TransferState::Finish ||
                state == TransferState::Disconnected ||
                state == TransferState::Cancelled) {

            beginRemoveRows(QModelIndex(), i, i);
            mTransfers.remove(i);
            endRemoveRows();
            t->deleteLater();
            i--;
        }
    }
}

Transfer* TransferTableModel::getTransfer(int index) const
{
    if (index < 0 || index >= mTransfers.size())
        return NULL;

    return mTransfers.at(index);
}

TransferInfo* TransferTableModel::getTransferInfo(int index) const
{
    return getTransfer(index)->getTransferInfo();
}

void TransferTableModel::removeTransfer(int index)
{
    if (index < 0 || index >= mTransfers.size())
        return;

    beginRemoveRows(QModelIndex(), index, index);
    mTransfers.at(index)->deleteLater();
    mTransfers.remove(index);
    endRemoveRows();
}

QString TransferTableModel::getStateString(TransferState state) const
{
    switch (state) {
    case TransferState::Idle : return tr("Idle");
    case TransferState::Waiting : return tr("Waiting");
    case TransferState::Disconnected : return tr("Disconnected");
    case TransferState::Paused : return tr("Paused");
    case TransferState::Cancelled : return tr("Cancelled");
    case TransferState::Transfering : return tr("Transfering");
    case TransferState::Finish : return tr("Finish");
    }

    return QString();
}

QColor TransferTableModel::getStateColor(TransferState state) const
{
    switch (state) {
    case TransferState::Idle : return QColor("black");
    case TransferState::Waiting : return QColor("orange");
    case TransferState::Disconnected : return QColor("red");
    case TransferState::Paused : return QColor("orange");
    case TransferState::Cancelled : return QColor("red");
    case TransferState::Transfering : return QColor("blue");
    case TransferState::Finish : return QColor("green");
    }

    return QColor();
}
