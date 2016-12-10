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

#include "devicelistmodel.h"
#include "settings.h"

DeviceListModel::DeviceListModel(DeviceBroadcaster* deviceBC, QObject* parent)
    : QAbstractListModel(parent)
{
    mDBC = deviceBC;
    if (!mDBC)
        mDBC = new DeviceBroadcaster(this);

    connect(mDBC, &DeviceBroadcaster::broadcastReceived, this, &DeviceListModel::onBCReceived);
}

void DeviceListModel::onBCReceived(const Device& fromDevice)
{
    QString id = fromDevice.getId();
    if (id == Settings::instance()->getMyDevice().getId())
        return;

    bool found = false;
    for(Device dev : mDevices) {
        if (dev.getId() == id) {
            found = true;
            break;
        }
    }

    if (!found) {
        beginInsertRows(QModelIndex(), mDevices.size(), mDevices.size());
        mDevices.push_back(fromDevice);
        endInsertRows();
    }
}

QVector<Device> DeviceListModel::getDevices() const
{
    return mDevices;
}

void DeviceListModel::setDevices(const QVector<Device> &devices)
{
    beginResetModel();
    mDevices = devices;
    endResetModel();
}


QVariant DeviceListModel::data(const QModelIndex &index, int role) const
{
    if (index.isValid()) {
        Device dev = mDevices[index.row()];
        if (role == Qt::DisplayRole) {
            return dev.getName() + "  (" + dev.getOSName() + ")";
        }
        else if (role == Qt::ToolTipRole) {
            QString str = dev.getId() + "<br>" +
                          dev.getName() + " (" + dev.getOSName() + ")<br>" +
                          dev.getAddress().toString();
            return str;
        }

    }

    return QVariant();
}

int DeviceListModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return mDevices.size();
}

void DeviceListModel::refresh()
{
    beginResetModel();
    mDevices.clear();
    endResetModel();
}

Device DeviceListModel::device(int index) const
{
    if (index < 0 || index >= mDevices.size())
        return Device();

    return mDevices.at(index);
}

Device DeviceListModel::device(const QString &id) const
{
    for (Device dev : mDevices)
        if (dev.getId() == id)
            return dev;

    return Device();
}

Device DeviceListModel::device(const QHostAddress &address) const
{
    for (Device dev : mDevices)
        if (dev.getAddress() == address)
            return dev;

    return Device();
}
