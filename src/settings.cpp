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

#include <QNetworkInterface>
#include <QHostInfo>
#include <QUuid>
#include <QSettings>
#include <QDir>
#include <QStandardPaths>

#include "settings.h"

#define DefaultBroadcastPort        56780
#define DefaultTransferPort         17116
#define DefaultBroadcastInterval    5000    // 5 secs
#define DefaultFileBufferSize       98304   // 96 KB
#define MaxFileBufferSize           1024*1024 // 1 MB

Settings* Settings::obj = new Settings;
Settings::Settings()
{
    mThisDevice = Device();
    mThisDevice.setId(QUuid::createUuid().toString());
    mThisDevice.setAddress(QHostAddress::LocalHost);
    mThisDevice.setOSName(OS_NAME);

    foreach (QHostAddress address, QNetworkInterface::allAddresses()) {
        if (address.protocol() == QAbstractSocket::IPv4Protocol &&
                address != QHostAddress::LocalHost) {
            mThisDevice.setAddress(address);
            break;
        }
    }

    loadSettings();
}

void Settings::setDeviceName(const QString &name)
{
    mThisDevice.setName(name);
}

void Settings::setBroadcastPort(quint16 port)
{
    if (port > 0)
        mBCPort = port;
}

void Settings::setBroadcastInterval(quint16 interval)
{
    mBCInterval = interval;
}

void Settings::setTransferPort(quint16 port)
{
    if (port > 0)
        mTransferPort = port;
}

void Settings::setFileBufferSize(qint32 size)
{
    if (size > 0 && size < MaxFileBufferSize)
        mFileBuffSize = size;
}

void Settings::setDownloadDir(const QString& dir)
{
    if (!dir.isEmpty() && QDir(dir).exists())
        mDownloadDir = dir;
}

void Settings::loadSettings()
{
    QSettings settings(SettingsFileName);
    mThisDevice.setName(settings.value("DeviceName", QHostInfo::localHostName()).toString());
    mBCPort = settings.value("BroadcastPort", DefaultBroadcastPort).value<quint16>();
    mTransferPort = settings.value("TransferPort", DefaultTransferPort).value<quint16>();
    mFileBuffSize = settings.value("FileBufferSize", DefaultFileBufferSize).value<quint32>();

    QString defDir = QDir::homePath() + QDir::separator() + "LocallyDownloads";
    mDownloadDir = settings.value("DownloadDir", defDir).toString();

    if (!QDir(mDownloadDir).exists()) {
        QDir dir;
        dir.mkpath(mDownloadDir);
    }

    mBCInterval = settings.value("BroadcastInterval", DefaultBroadcastInterval).value<quint16>();
}

void Settings::saveSettings()
{
    QSettings settings(SettingsFileName);
    settings.setValue("DeviceName", mThisDevice.getName());
    settings.setValue("BroadcastPort", mBCPort);
    settings.setValue("TransferPort", mTransferPort);
    settings.setValue("FileBufferSize", mFileBuffSize);
    settings.setValue("DownloadDir", mDownloadDir);
    settings.setValue("BroadcastInterval", mBCInterval);
}

void Settings::reset()
{
    mThisDevice.setName(QHostInfo::localHostName());
    mBCPort = DefaultBroadcastPort;
    mTransferPort = DefaultTransferPort;
    mBCInterval = DefaultBroadcastInterval;
    mFileBuffSize = DefaultFileBufferSize;
#if defined (Q_OS_WIN)
    mDownloadDir =
            QStandardPaths::locate(QStandardPaths::DownloadLocation, QString(), QStandardPaths::LocateDirectory) + "LANShareDownloads";
#else
    mDownloadDir = QDir::homePath() + QDir::separator() + "LANShareDownloads";
#endif

}

quint16 Settings::getBroadcastPort() const
{
    return mBCPort; 
}

quint16 Settings::getTransferPort() const
{
    return mTransferPort;
}

quint16 Settings::getBroadcastInterval() const
{
    return mBCInterval;
}

qint32 Settings::getFileBufferSize() const 
{ 
    return mFileBuffSize; 
}

QString Settings::getDownloadDir() const
{
    return mDownloadDir;
}

Device Settings::getMyDevice() const
{
    return mThisDevice;
}

QString Settings::getDeviceId() const
{
    return mThisDevice.getId();
}

QString Settings::getDeviceName() const
{
    return mThisDevice.getName();
}

QHostAddress Settings::getDeviceAddress() const
{
    return mThisDevice.getAddress();
}


