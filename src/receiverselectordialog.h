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

#ifndef RECEIVERSELECTORDIALOG_H
#define RECEIVERSELECTORDIALOG_H

#include <QDialog>

class DeviceListModel;
class Device;

namespace Ui {
class ReceiverSelectorDialog;
}

class ReceiverSelectorDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ReceiverSelectorDialog(DeviceListModel* model, QWidget *parent = 0);
    ~ReceiverSelectorDialog();

    Device getSelectedDevice() const;
    QVector<Device> getSelectedDevices() const;

private Q_SLOTS:
    void onSendClicked();
    void onRefreshClicked();

private:
    Ui::ReceiverSelectorDialog *ui;

    DeviceListModel* mModel;
};

#endif // RECEIVERSELECTORDIALOG_H
