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

#include "devicebroadcaster.h"
#include "settings.h"

DeviceBroadcaster::DeviceBroadcaster(QObject *parent) : QObject(parent)
{
    connect(&mTimer, &QTimer::timeout, this, &DeviceBroadcaster::sendBroadcast);
    connect(&mUdpSock, &QUdpSocket::readyRead, this, &DeviceBroadcaster::processBroadcast);

    mUdpSock.bind(Settings::instance()->getBroadcastPort(), QUdpSocket::ShareAddress);
}

void DeviceBroadcaster::start()
{
    sendBroadcast();
    if (!mTimer.isActive())
        mTimer.start(Settings::instance()->getBroadcastInterval());
}

void DeviceBroadcaster::sendBroadcast()
{
    int port = Settings::instance()->getBroadcastPort();
    Device dev = Settings::instance()->getMyDevice();
    QJsonObject obj(QJsonObject::fromVariantMap({
                                                    {"id", dev.getId()},
                                                    {"name", dev.getName()},
                                                    {"os", dev.getOSName()},
                                                    {"port", port}
                                                }));

    QVector<QHostAddress> addresses = getBroadcastAddressFromInterfaces();
    QByteArray data(QJsonDocument(obj).toJson(QJsonDocument::Compact));
    foreach (QHostAddress address, addresses) {
        mUdpSock.writeDatagram(data, address, port);
    }
}

void DeviceBroadcaster::processBroadcast()
{
    while (mUdpSock.hasPendingDatagrams()) {
        QByteArray data;
        data.resize(mUdpSock.pendingDatagramSize());
        QHostAddress sender;

        mUdpSock.readDatagram(data.data(), data.size(), &sender);

        QJsonObject obj = QJsonDocument::fromJson(data).object();
        if (obj.keys().length() == 4) {
            if (obj.value("port").toVariant().value<quint16>() ==
                    Settings::instance()->getBroadcastPort()) {

                Device device;
                device.set(obj.value("id").toString(), obj.value("name").toString(),
                           obj.value("os").toString(), sender);
                emit broadcastReceived(device);
            }
        }
    }
}

QVector<QHostAddress> DeviceBroadcaster::getBroadcastAddressFromInterfaces()
{
    QVector<QHostAddress> addresses;
    foreach (QNetworkInterface iface, QNetworkInterface::allInterfaces()) {
        if (iface.flags() & QNetworkInterface::CanBroadcast) {
            foreach (QNetworkAddressEntry addressEntry, iface.addressEntries()) {
                if (!addressEntry.broadcast().isNull()) {
                    addresses.push_back(addressEntry.broadcast());
                }
            }
        }
    }
    return addresses;
}
