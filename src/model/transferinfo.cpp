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


#include "transferinfo.h"

TransferInfo::TransferInfo(Transfer* owner, QObject *parent) :
    QObject(parent),
    mState(TransferState::Idle), mLastState(TransferState::Idle),
    mType(TransferType::None), mProgress(0), mDataSize(0),
    mOwner(owner)
{
}

bool TransferInfo::canResume() const
{
    return mState == TransferState::Paused;
}

bool TransferInfo::canPause() const
{
    return mState == TransferState::Waiting ||
            mState == TransferState::Transfering;
}

bool TransferInfo::canCancel() const
{
    return mState == TransferState::Waiting ||
            mState == TransferState::Transfering ||
            mState == TransferState::Paused;
}

void TransferInfo::setPeer(Device peer)
{
    mPeer = std::move(peer);
}

void TransferInfo::setState(TransferState newState)
{
    if (newState != mState) {
        TransferState tmp = mState;

        switch (mState) {
        case TransferState::Idle : {
            if (newState == TransferState::Waiting) {
                mState = newState;
                emit stateChanged(mState);
            }
            break;
        }
        case TransferState::Waiting : {
            if (newState == TransferState::Transfering ||
                    newState == TransferState::Cancelled ||
                    newState == TransferState::Paused) {
                mState = newState;
                emit stateChanged(mState);
            }
            break;
        }
        case TransferState::Transfering : {
            if (newState == TransferState::Disconnected ||
                    newState == TransferState::Finish ||
                    newState == TransferState::Cancelled ||
                    newState == TransferState::Paused) {
                mState = newState;
                emit stateChanged(mState);
            }
            break;
        }
        case TransferState::Paused : {
            if (newState == TransferState::Waiting ||
                    newState == TransferState::Transfering) {
                mState = mLastState;
                emit stateChanged(mState);
            }
            else if (newState == TransferState::Cancelled ||
                     newState == TransferState::Disconnected) {
                mState = newState;
                emit stateChanged(mState);
            }
            break;
        }
        default:
            break;
        }

        mLastState = tmp;
    }
}

void TransferInfo::setProgress(int newProgress)
{
    if (newProgress != mProgress) {
        mProgress = newProgress;
        emit progressChanged(mProgress);
    }
}

void TransferInfo::setTransferType(TransferType type)
{
    mType = type;
}

void TransferInfo::setDataSize(qint64 size)
{
    mDataSize = size;
}

void TransferInfo::setFilePath(const QString &fileName)
{
    mFilePath = fileName;
}
