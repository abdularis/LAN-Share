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

#include <QJsonDocument>
#include <QJsonObject>
#include <QDir>
#include <QtDebug>

#include "settings.h"
#include "sender.h"

Sender::Sender(Device receiver, const QString& fileName, QObject* parent)
    : Transfer(NULL, NULL, parent), mReceiverDev(receiver), mFilePath(fileName)
{
    mFileSize = -1;
    mBytesRemaining = -1;

    mFileBuffSize = Settings::instance()->getFileBufferSize();
    mFileBuff.resize(mFileBuffSize);

    mCancelled = false;
    mPaused = false;
    mPausedByReceiver = false;
    mIsHeaderSent = false;
}

Sender::~Sender()
{
    if (mFile && mFile->isOpen()) {
        mFile->close();
        delete mFile;
    }
}

bool Sender::start()
{
    mFile = new QFile(mFilePath);
    bool ok = mFile->open(QIODevice::ReadOnly);
    if (ok) {
        mFileSize = mFile->size();
        mBytesRemaining = mFileSize;
        emit fileOpened();
    }

    if (mFileSize > 0) {
        QHostAddress receiverAddress = mReceiverDev.getAddress();
        setSocket(new QTcpSocket(this));
        mSocket->connectToHost(receiverAddress, Settings::instance()->getTransferPort(), QAbstractSocket::ReadWrite);
        setState(TransferState::Waiting);

        connect(mSocket, &QTcpSocket::bytesWritten, this, &Sender::onBytesWritten);
        connect(mSocket, &QTcpSocket::connected, this, &Sender::onConnected);
        connect(mSocket, &QTcpSocket::disconnected, this, &Sender::onDisconnected);
    }

    return ok && mSocket;
}

void Sender::resume()
{
    if (canResume()) {
        setState(getLastState());
        mPaused = false;
        sendData();
    }
}

void Sender::pause()
{
    if (canPause()) {
        setState(TransferState::Paused);
        mPaused = true;
    }
}

void Sender::cancel()
{
    if (canCancel()) {
        writePacket(0, PacketType::Cancel, QByteArray());
        setState(TransferState::Cancelled);
        setProgress(0);
        mCancelled = true;
    }
}

void Sender::onConnected()
{
    setState(TransferState::Transfering);
    sendHeader();
}

void Sender::onDisconnected()
{
    setState(TransferState::Disconnected);
    emit errorOcurred("Receiver disconnected");
}

void Sender::onBytesWritten(qint64 bytes)
{
    Q_UNUSED(bytes);

    if (!mSocket->bytesToWrite()) {
        sendData();
    }
}

void Sender::finish()
{
    mFile->close();
    setState(TransferState::Finish);
    emit done();

    writePacket(0, PacketType::Finish, QByteArray());
}

void Sender::sendData()
{
    if (!mBytesRemaining || mCancelled || mPausedByReceiver || mPaused)
        return;

    if (mBytesRemaining < mFileBuffSize) {
        mFileBuff.resize(mBytesRemaining);
        mFileBuffSize = mFileBuff.size();
    }

    qint64 bytesRead = mFile->read(mFileBuff.data(), mFileBuffSize);
    if (bytesRead == -1) {
        emit errorOcurred(tr("Error while reading file."));
        return;
    }

    mBytesRemaining -= bytesRead;
    if (mBytesRemaining < 0)
        mBytesRemaining = 0;

    setProgress( (int) ((mFileSize-mBytesRemaining) * 100 / mFileSize) );

    writePacket(mFileBuffSize, PacketType::Data, mFileBuff);

    if (!mBytesRemaining) {
        finish();
    }
}

void Sender::sendHeader()
{
    QString fName = QDir(mFile->fileName()).dirName();

    QJsonObject obj( QJsonObject::fromVariantMap({
                                    {"name", fName},
                                    {"size", mFileSize}
                                }));

    QByteArray headerData( QJsonDocument(obj).toJson() );

    writePacket(headerData.size(), PacketType::Header, headerData);
    mIsHeaderSent = true;
}

void Sender::processCancelPacket(QByteArray& data)
{
    Q_UNUSED(data);

    setState(TransferState::Cancelled);
    setProgress(0);
    mSocket->disconnectFromHost();
    mCancelled = true;
}

void Sender::processPausePacket(QByteArray& data)
{
    Q_UNUSED(data);

    mPausedByReceiver = true;
}

void Sender::processResumePacket(QByteArray& data)
{
    Q_UNUSED(data);
    
    mPausedByReceiver = false;
    if (mIsHeaderSent)
        sendData();
    else
        sendHeader();
}

