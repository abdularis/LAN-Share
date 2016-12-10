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

#include "transfer.h"

Transfer::Transfer(QFile* file, QTcpSocket* socket, QObject* parent)
    : QObject(parent), mFile(file), mProgress(0),
      mState(TransferState::Idle), mLastState(TransferState::Idle),
      mPacketSize(-1)
{
    setSocket(socket);
}

Transfer::~Transfer()
{
    qDebug() << "Transfer() desctructor called";
}

void Transfer::setProgress(int progress)
{
    if (progress != mProgress) {
        mProgress = progress;
        emit progressChanged(mProgress);
    }
}

void Transfer::setState(TransferState state)
{
    if (state != mState) {
        TransferState tmp = mState;

        switch (mState) {
        case TransferState::Idle : {
            if (state == TransferState::Waiting) {
                mState = state;
                emit stateChanged(mState);
            }
            break;
        }
        case TransferState::Waiting : {
            if (state == TransferState::Transfering ||
                    state == TransferState::Cancelled ||
                    state == TransferState::Paused) {
                mState = state;
                emit stateChanged(mState);
            }
            break;
        }
        case TransferState::Transfering : {
            if (state == TransferState::Disconnected ||
                    state == TransferState::Finish ||
                    state == TransferState::Cancelled ||
                    state == TransferState::Paused) {
                mState = state;
                emit stateChanged(mState);
            }
            break;
        }
        case TransferState::Paused : {
            if (state == TransferState::Waiting ||
                    state == TransferState::Transfering) {
                mState = mLastState;
                emit stateChanged(mState);
            }
            else if (state == TransferState::Cancelled ||
                     state == TransferState::Disconnected) {
                mState = state;
                emit stateChanged(mState);
            }
            break;
        }
        }

        mLastState = tmp;
    }
}

void Transfer::resume()
{
    
}

void Transfer::pause()
{
    
}

void Transfer::cancel()
{
    
}


bool Transfer::canResume() const
{
    return mState == TransferState::Paused;
}

bool Transfer::canPause() const
{
    return mState == TransferState::Waiting ||
            mState == TransferState::Transfering;
}

bool Transfer::canCancel() const
{
    return mState == TransferState::Waiting ||
            mState == TransferState::Transfering ||
            mState == TransferState::Paused;
}

void Transfer::onReadyRead()
{
    if (mState == TransferState::Cancelled)
        return;

    mBuff.append(mSocket->readAll());

    while (mBuff.size() >= HEADER_SIZE) {
        if (mPacketSize < 0) {
            memcpy(&mPacketSize, mBuff.constData(), sizeof(mPacketSize));
            mBuff.remove(0, sizeof(mPacketSize));
        }

        if (mBuff.size() > mPacketSize) {
            PacketType type = static_cast<PacketType>(mBuff.at(0));
            QByteArray data = mBuff.mid(1, mPacketSize);

            processPacket(data, type);
            mBuff.remove(0, mPacketSize + 1);

            mPacketSize = -1;
        }
        else {
            break;
        }
    }
}

void Transfer::writePacket(qint32 packetDataSize, PacketType type, const QByteArray &data)
{
    if (mSocket) {
        mSocket->write(reinterpret_cast<const char*>(&packetDataSize), sizeof(packetDataSize));
        mSocket->write(reinterpret_cast<const char*>(&type), sizeof(type));
        mSocket->write(data);
    }
}

void Transfer::processPacket(QByteArray &data, PacketType type)
{
    switch (type) {
    case PacketType::Header : processHeaderPacket(data); break;
    case PacketType::Data : processDataPacket(data); break;
    case PacketType::Finish : processFinishPacket(data); break;
    case PacketType::Cancel : processCancelPacket(data); break;
    case PacketType::Pause : processPausePacket(data); break;
    case PacketType::Resume : processResumePacket(data); break;
    }
}

void Transfer::processHeaderPacket(QByteArray& data)
{
    Q_UNUSED(data);
}

void Transfer::processDataPacket(QByteArray& data)
{
    Q_UNUSED(data);
}

void Transfer::processFinishPacket(QByteArray& data)
{
    Q_UNUSED(data);
}

void Transfer::processCancelPacket(QByteArray& data)
{
    Q_UNUSED(data);
}

void Transfer::processPausePacket(QByteArray& data)
{
    Q_UNUSED(data);
}

void Transfer::processResumePacket(QByteArray& data)
{
    Q_UNUSED(data);
}


void Transfer::clearReadBuffer()
{
    mBuff.clear();
    mPacketSize = -1;
}

void Transfer::setSocket(QTcpSocket *socket)
{
    if (socket) {
        mSocket = socket;
        connect(mSocket, &QTcpSocket::readyRead, this, &Transfer::onReadyRead);
    }
}
