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

#include "util.h"
#include "receiver.h"
#include "settings.h"

Receiver::Receiver(const Device& sender, QTcpSocket* socket, QObject* parent)
    : Transfer(socket, parent), mSenderDev(sender), mFileSize(0), mBytesRead(0)
{
    mInfo->setState(TransferState::Waiting);
    connect(mSocket, &QTcpSocket::disconnected, this, &Receiver::onDisconnected);

    mInfo->setTransferType(TransferType::Download);
    mInfo->setPeer(sender);
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

    QString fileName = obj.value("name").toString();
    QString folderName = obj.value("folder").toString();
    QString dstFolderPath = Settings::instance()->getDownloadDir();
    if (!folderName.isEmpty())
        dstFolderPath = dstFolderPath + QDir::separator() + folderName;

    /*
     * Jika folder didalam Download Dir tidak ada maka buat folder tsb.
     */
    QDir dir(dstFolderPath);
    if (!dir.exists()) {
        dir.mkpath(dstFolderPath);
    }

    QString dstFilePath = dstFolderPath + QDir::separator() + fileName;
    /*
     * Jika opsi overwrite tdk dicentang maka rename file agar tdk tertindih
     */
    if (!Settings::instance()->getReplaceExistingFile()) {
        dstFilePath = Util::getUniqueFileName(fileName, dstFolderPath);
    }

    mInfo->setFilePath(dstFilePath);
    mFile = new QFile(dstFilePath, this);
    if (mFile->open(QIODevice::WriteOnly)) {
        mInfo->setState(TransferState::Transfering);
        emit mInfo->fileOpened();
    }
    else {
        emit mInfo->errorOcurred(tr("Failed to write ") + dstFilePath);
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
