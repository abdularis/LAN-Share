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

#include <QDir>
#include <QColor>

#include "util.h"
#include "sendertablemodel.h"

SenderTableModel::SenderTableModel(QObject* parent)
    : QAbstractTableModel(parent)
{

}

SenderTableModel::~SenderTableModel()
{
    for (Sender* sender : mSenders) {
        delete sender;
    }
}

int SenderTableModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return mSenders.size();
}

int SenderTableModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return (int) Column::Count;
}

QVariant SenderTableModel::data(const QModelIndex &index, int role) const
{
    if (index.isValid()) {
        Sender* sender = mSenders[index.row()];

        if (sender) {
            Column col = (Column) index.column();

            if (role == Qt::DisplayRole) {
                switch (col) {
                case Column::Destination : return sender->getReceiver().getName();
                case Column::FileName : return sender->getFile()->fileName();
                case Column::FileSize : return Util::fileSizeToString(sender->getFile()->size());
                case Column::Status : {
                    TransferState state = sender->getState();
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
                case Column::Progress : return sender->getProgress();
                }

            }
            else if (role == Qt::ForegroundRole) {
                Column col = (Column) index.column();
                switch (col) {
                case Column::Status : {
                    TransferState state = sender->getState();
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

QVariant SenderTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        Column col = (Column) section;
        switch (col) {
        case Column::Destination : return tr("Receiver");
        case Column::FileName : return tr("File path");
        case Column::FileSize : return tr("Size");
        case Column::Status : return tr("Status");
        case Column::Progress : return tr("Progress");
        }
    }

    return QVariant();
}

void SenderTableModel::insertSender(Sender *sender)
{
    if (sender) {
        beginInsertRows(QModelIndex(), mSenders.size(), mSenders.size());
        mSenders.push_back(sender);
        endInsertRows();

        int sendersSize = mSenders.size();
        connect(sender, &Sender::fileOpened, [=]() {
            QModelIndex idx = index(sendersSize - 1, (int)Column::FileName);
            emit dataChanged(idx, idx);
        });

        connect(sender, &Sender::stateChanged, [=](TransferState state) {
            Q_UNUSED(state);

            QModelIndex idx = index(sendersSize - 1, (int)Column::Status);
            emit dataChanged(idx, idx);
        });
    }
}

void SenderTableModel::clearCompleted()
{
    for (int i = 0; i < mSenders.size(); i++) {
        Sender* sen = mSenders.at(i);
        TransferState state = sen->getState();
        if (state == TransferState::Finish ||
            state == TransferState::Disconnected ||
            state == TransferState::Cancelled) {

            beginRemoveRows(QModelIndex(), i, i);
            mSenders.remove(i);
            endRemoveRows();
            sen->deleteLater();
            i--;
        }
    }
}

Sender* SenderTableModel::getSender(int index) const
{
    if (index < 0 || index >= mSenders.size())
        return NULL;

    return mSenders.at(index);
}

void SenderTableModel::removeSender(int index)
{
    if (index < 0 || index >= mSenders.size())
        return;

    beginRemoveRows(QModelIndex(), index, index);
    mSenders.at(index)->deleteLater();
    mSenders.remove(index);
    endRemoveRows();
}
