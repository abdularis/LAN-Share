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

#ifndef TRANSFER_H
#define TRANSFER_H

#include <QFile>
#include <QTcpSocket>
#include <QObject>

#define HEADER_SIZE 5   // bytes


enum class PacketType : char
{
    Header = 0x01,
    Data,
    Finish,
    Cancel,
    Pause,
    Resume
};

enum class TransferState {
    Idle,
    Waiting,
    Disconnected,
    Paused,
    Cancelled,
    Transfering,
    Finish
};


class Transfer : public QObject
{
    Q_OBJECT

public:
    Transfer(QFile* file, QTcpSocket* socket, QObject* parent = 0);
    ~Transfer();

    inline int getProgress() const { return mProgress; }
    inline QFile* getFile() const { return mFile; }
    inline QTcpSocket* getSocket() const { return mSocket; }
    inline TransferState getState() const { return mState; }
    inline TransferState getLastState() const { return mLastState; }

    void setState(TransferState state);

    bool canResume() const;
    bool canPause() const;
    bool canCancel() const;

    virtual void resume();
    virtual void pause();
    virtual void cancel();

Q_SIGNALS:
    void done();
    void errorOcurred(const QString& errStr);
    void progressChanged(int progress);
    void fileOpened();
    void stateChanged(TransferState state);

protected:
    void clearReadBuffer();
    void setSocket(QTcpSocket* socket);
    void setProgress(int progress);
    virtual void writePacket(qint32 packetDataSize, PacketType type, const QByteArray& data);
    virtual void processPacket(QByteArray& data, PacketType type);

    QFile* mFile;
    QTcpSocket* mSocket;

private Q_SLOTS:
    void onReadyRead();

private:
    int mProgress;
    TransferState mState;
    TransferState mLastState;

    //
    QByteArray mBuff;
    qint32 mPacketSize;
};

#endif // TRANSFER_H
