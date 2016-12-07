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

#include <QColor>

#include "receivertablemodel.h"

#include "util.h"

ReceiverTableModel::ReceiverTableModel(QObject* parent)
    : QAbstractTableModel(parent)
{
}

ReceiverTableModel::~ReceiverTableModel()
{
    for (Receiver* rec : mReceivers) {
        delete rec;
    }
}

int ReceiverTableModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return mReceivers.size();
}

int ReceiverTableModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return (int)Column::Count;
}

QVariant ReceiverTableModel::data(const QModelIndex &index, int role) const
{
    if (index.isValid()) {
        Receiver* rec = mReceivers[index.row()];

        if (rec) {
            Column col = (Column)index.column();

            if (role == Qt::DisplayRole) {
                switch (col) {
                case Column::Sender : return rec->getSender().getName();
                case Column::FileName : return rec->getFile() ? rec->getFile()->fileName() : "-";
                case Column::FileSize : return Util::fileSizeToString(rec->getReceivedFileSize());
                case Column::Status : {
                    TransferState state = rec->getState();
                    switch (state) {
                    case TransferState::Idle : return tr("Idle");
                    case TransferState::Waiting : return tr("Waiting");
                    case TransferState::Disconnected : return tr("Disconnected");
                    case TransferState::Paused : return tr("Paused");
                    case TransferState::Cancelled : return tr("Cancelled");
                    case TransferState::Transfering : return tr("Transfering");
                    case TransferState::Finish : return tr("Finish");
                    }
                }
                case Column::Progress : return rec->getProgress();
                }
            }
            else if (role == Qt::ForegroundRole) {
                Column col = (Column) index.column();
                switch (col) {
                case Column::Status : {
                    TransferState state = rec->getState();
                    switch (state) {
                    case TransferState::Idle : return QColor("black");
                    case TransferState::Waiting : return QColor("orange");
                    case TransferState::Disconnected : return QColor("red");
                    case TransferState::Paused : return QColor("orange");
                    case TransferState::Cancelled : return QColor("red");
                    case TransferState::Transfering : return QColor("blue");
                    case TransferState::Finish : return QColor("green");
                    }
                }
                }
            }
        }

    }

    return QVariant();
}

QVariant ReceiverTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        Column col = (Column) section;
        switch (col) {
        case Column::Sender : return tr("Sender");
        case Column::FileName : return tr("File path");
        case Column::FileSize : return tr("Size");
        case Column::Status : return tr("Status");
        case Column::Progress : return tr("Progress");
        }
    }

    return QVariant();
}

void ReceiverTableModel::insertReceiver(Receiver *receiver)
{
    if (receiver) {
        beginInsertRows(QModelIndex(), mReceivers.size(), mReceivers.size());
        mReceivers.push_back(receiver);
        endInsertRows();

        int receiversSize = mReceivers.size();
        connect(receiver, &Receiver::fileOpened, [=]() {
            QModelIndex idx = index(receiversSize - 1, (int)Column::FileName);
            QModelIndex last = index(receiversSize - 1, (int)Column::FileSize);
            emit dataChanged(idx, last);
        });

        connect(receiver, &Receiver::stateChanged, [=](TransferState state) {
            Q_UNUSED(state);

            QModelIndex idx = index(receiversSize - 1, (int)Column::Status);
            emit dataChanged(idx, idx);
        });
    }
}

void ReceiverTableModel::clearCompleted()
{
    for (int i = 0; i < mReceivers.size(); i++) {
        Receiver* rec = mReceivers.at(i);
        TransferState state = rec->getState();
        if (state == TransferState::Finish ||
            state == TransferState::Disconnected ||
            state == TransferState::Cancelled) {

            beginRemoveRows(QModelIndex(), i, i);
            mReceivers.remove(i);
            endRemoveRows();
            rec->deleteLater();
            i--;
        }
    }
}

Receiver* ReceiverTableModel::getReceiver(int index) const
{
    if (index < 0 || index >= mReceivers.size())
        return NULL;

    return mReceivers.at(index);
}

void ReceiverTableModel::removeReceiver(int index)
{
    if (index < 0 || index >= mReceivers.size())
        return;

    beginRemoveRows(QModelIndex(), index, index);
    mReceivers.at(index)->deleteLater();
    mReceivers.remove(index);
    endRemoveRows();
}
