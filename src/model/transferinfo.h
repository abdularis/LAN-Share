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


#ifndef TRANSFERINFO_H
#define TRANSFERINFO_H

#include <QObject>

#include "device.h"

enum class TransferState {
    Idle,
    Waiting,
    Disconnected,
    Paused,
    Cancelled,
    Transfering,
    Finish
};

enum class TransferType {
    None,
    Download,
    Upload
};

class Transfer;

class TransferInfo : public QObject
{
    Q_OBJECT

public:
    explicit TransferInfo(Transfer* owner, QObject *parent = nullptr);

    inline Device getPeer() const { return mPeer; }
    inline int getProgress() const { return mProgress; }
    inline TransferState getState() const { return mState; }
    inline TransferState getLastState() const { return mLastState; }
    inline TransferType getTransferType() const { return mType; }
    inline qint64 getDataSize() const { return mDataSize; }
    inline QString getFilePath() const { return mFilePath; }
    inline Transfer* getOwner() const { return mOwner; }

    bool canResume() const;
    bool canPause() const;
    bool canCancel() const;

    void setPeer(Device peer);
    void setState(TransferState state);
    void setTransferType(TransferType type);
    void setProgress(int progress);
    void setDataSize(qint64 size);
    void setFilePath(const QString& fileName);

Q_SIGNALS:
    void done();
    void errorOcurred(const QString& errStr);
    void progressChanged(int progress);
    void fileOpened();
    void stateChanged(TransferState state);

private:
    Device mPeer;
    TransferState mState;
    TransferState mLastState;
    TransferType mType;
    int mProgress;
    qint64 mDataSize;
    QString mFilePath;

    Transfer* mOwner;
};

#endif // TRANSFERINFO_H
