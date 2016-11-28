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

#include <QJsonObject>
#include <QJsonDocument>
#include <QDir>

#include "receiver.h"
#include "settings.h"

Receiver::Receiver(Device sender, QTcpSocket* socket, QObject* parent)
    : Transfer(NULL, socket, parent), mSenderDev(sender), mFileSize(0), mBytesRead(0), mCancelled(false)
{
    connect(mSocket, &QTcpSocket::disconnected, this, &Receiver::onDisconnected);
}

void Receiver::resume()
{
    if (canResume()) {
        setState(getLastState());
        writePacket(0, PacketType::Resume, QByteArray());
    }
}

void Receiver::pause()
{
    if (canPause()) {
        setState(TransferState::Paused);
        writePacket(0, PacketType::Pause, QByteArray());
    }
}

void Receiver::cancel()
{
    if (canCancel()) {
        setState(TransferState::Cancelled);
        setProgress(0);
        clearReadBuffer();
        writePacket(0, PacketType::Cancel, QByteArray());
        mFile->remove();
    }
}

void Receiver::onDisconnected()
{
    setState(TransferState::Disconnected);
    emit errorOcurred("Sender disconnected");
}

void Receiver::processHeaderPacket(QByteArray& data)
{
    QJsonObject obj = QJsonDocument::fromJson(data).object();
    mFileSize = obj.value("size").toVariant().value<qint64>();

    QString fileName = Settings::instance()->getDownloadDir() +
            QDir::separator() + obj.value("name").toString();
    mFile = new QFile(fileName, this);
    if (mFile->open(QIODevice::WriteOnly)) {
        setState(TransferState::Transfering);
        emit fileOpened();
    }
    else {
        emit errorOcurred("Failed to create " + fileName);
    }
}

void Receiver::processDataPacket(QByteArray& data)
{
    if (mFile && mBytesRead + data.size() <= mFileSize) {
        mFile->write(data);
        mBytesRead += data.size();

        setProgress( (int)(mBytesRead * 100 / mFileSize) );
    }
}

void Receiver::processFinishPacket(QByteArray& data)
{
    Q_UNUSED(data);

    setState(TransferState::Finish);
    mFile->close();
    mSocket->disconnectFromHost();
    emit done();
}

void Receiver::processCancelPacket(QByteArray& data)
{
    Q_UNUSED(data);

    setState(TransferState::Cancelled);
    setProgress(0);
    clearReadBuffer();
    mFile->remove();
    mSocket->disconnectFromHost();
}
