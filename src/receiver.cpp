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

#include <QJsonObject>
#include <QJsonDocument>
#include <QDir>

#include "receiver.h"
#include "settings.h"

Receiver::Receiver(Device sender, QTcpSocket* socket, QObject* parent)
    : Transfer(socket, parent), mSenderDev(sender), mFileSize(0), mBytesRead(0), mCancelled(false)
{
    connect(mSocket, &QTcpSocket::disconnected, this, &Receiver::onDisconnected);

    mInfo->setTransferType(TransferType::Download);
    mInfo->setPeer(sender);
}

Receiver::~Receiver()
{
}

void Receiver::resume()
{
    if (mInfo->canResume()) {
        mInfo->setState(mInfo->getLastState());
        writePacket(0, PacketType::Resume, QByteArray());
    }
}

void Receiver::pause()
{
    if (mInfo->canPause()) {
        mInfo->setState(TransferState::Paused);
        writePacket(0, PacketType::Pause, QByteArray());
    }
}

void Receiver::cancel()
{
    if (mInfo->canCancel()) {
        mInfo->setState(TransferState::Cancelled);
        mInfo->setProgress(0);
        clearReadBuffer();
        writePacket(0, PacketType::Cancel, QByteArray());
        mFile->remove();
    }
}

void Receiver::onDisconnected()
{
    mInfo->setState(TransferState::Disconnected);
    emit mInfo->errorOcurred("Sender disconnected");
}

void Receiver::processHeaderPacket(QByteArray& data)
{
    QJsonObject obj = QJsonDocument::fromJson(data).object();
    mFileSize = obj.value("size").toVariant().value<qint64>();
    mInfo->setDataSize(mFileSize);

    QString folderName = Settings::instance()->getDownloadDir() +
            QDir::separator() + obj.value("folder").toString();
    QString fileName = folderName +
            QDir::separator() + obj.value("name").toString();
    mFile = new QFile(fileName, this);
    mInfo->setFilePath(fileName);

    QDir dir(folderName);
    if (!dir.exists()) {
        dir.mkpath(folderName);
    }

    if (mFile->open(QIODevice::WriteOnly)) {
        mInfo->setState(TransferState::Transfering);
        emit mInfo->fileOpened();
    }
    else {
        emit mInfo->errorOcurred(tr("Failed to create ") + fileName);
    }
}

void Receiver::processDataPacket(QByteArray& data)
{
    if (mFile && mBytesRead + data.size() <= mFileSize) {
        mFile->write(data);
        mBytesRead += data.size();

        mInfo->setProgress( (int)(mBytesRead * 100 / mFileSize) );
    }
}

void Receiver::processFinishPacket(QByteArray& data)
{
    Q_UNUSED(data);

    mInfo->setState(TransferState::Finish);
    mFile->close();
    mSocket->disconnectFromHost();
    emit mInfo->done();
}

void Receiver::processCancelPacket(QByteArray& data)
{
    Q_UNUSED(data);

    mInfo->setState(TransferState::Cancelled);
    mInfo->setProgress(0);
    clearReadBuffer();
    mFile->remove();
    mSocket->disconnectFromHost();
}
