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

#ifndef RECEIVER_H
#define RECEIVER_H

#include "transfer.h"
#include "device.h"

class Receiver : public Transfer
{
public:
    Receiver(Device sender, QTcpSocket* socket, QObject* parent = 0);

    inline Device getSender() const { return mSenderDev; }
    inline qint64 getReceivedFileSize() const { return mFileSize; }
    inline qint64 getBytesWritten() const { return mBytesRead; }

    void resume() override;
    void pause() override;
    void cancel() override;

private Q_SLOTS:
    void onDisconnected();

private:
    void processPacket(QByteArray& data, PacketType type) override;

    Device mSenderDev;

    qint64 mFileSize;
    qint64 mBytesRead;

    //
    bool mCancelled;
};

#endif // RECEIVER_H
