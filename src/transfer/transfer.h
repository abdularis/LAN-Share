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

#ifndef TRANSFER_H
#define TRANSFER_H

#include <QFile>
#include <QTcpSocket>
#include <QObject>

#include "model/device.h"
#include "model/transferinfo.h"


enum class PacketType : char
{
    Header = 0x01,
    Data,
    Finish,
    Cancel,
    Pause,
    Resume
};

class Transfer : public QObject
{
    Q_OBJECT

public:
    Transfer(QTcpSocket* socket, QObject* parent = nullptr);

    inline QFile* getFile() const { return mFile; }
    inline QTcpSocket* getSocket() const { return mSocket; }
    inline TransferInfo* getTransferInfo() const { return mInfo; }

    virtual void resume();
    virtual void pause();
    virtual void cancel();

protected:
    void clearReadBuffer();
    void setSocket(QTcpSocket* socket);

    virtual void processPacket(QByteArray& data, PacketType type);
    virtual void processHeaderPacket(QByteArray& data);
    virtual void processDataPacket(QByteArray& data);
    virtual void processFinishPacket(QByteArray& data);
    virtual void processCancelPacket(QByteArray& data);
    virtual void processPausePacket(QByteArray& data);
    virtual void processResumePacket(QByteArray& data);

    virtual void writePacket(qint32 packetDataSize, PacketType type, const QByteArray& data);

    QFile* mFile;
    QTcpSocket* mSocket;
    TransferInfo* mInfo;

private Q_SLOTS:
    void onReadyRead();

private:

    //
    QByteArray mBuff;
    qint32 mPacketSize;

    int mHeaderSize;
};

#endif // TRANSFER_H
