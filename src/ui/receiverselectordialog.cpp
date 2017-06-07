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

#include <QMessageBox>

#include "receiverselectordialog.h"
#include "ui_receiverselectordialog.h"

#include "model/devicelistmodel.h"
#include "model/device.h"

ReceiverSelectorDialog::ReceiverSelectorDialog(DeviceListModel* model, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ReceiverSelectorDialog),
    mModel(model)
{
    ui->setupUi(this);

    ui->listView->setModel(mModel);
    ui->listView->setCurrentIndex(QModelIndex());

    model->refresh();
}

ReceiverSelectorDialog::~ReceiverSelectorDialog()
{
    delete ui;
}

Device ReceiverSelectorDialog::getSelectedDevice() const
{
    QModelIndex currIndex = ui->listView->currentIndex();
    if (currIndex.isValid()) {
        return mModel->device(currIndex.row());
    }

    return Device();
}

QVector<Device> ReceiverSelectorDialog::getSelectedDevices() const
{
    QVector<Device> devices;
    QItemSelectionModel* selModel = ui->listView->selectionModel();
    if (selModel) {

        QModelIndexList selected = selModel->selectedIndexes();
        for (int i = 0; i < selected.size(); i++) {
            if (selected.at(i).isValid()) {
                devices.push_back(mModel->device( selected.at(i).row() ));
            }
        }

    }

    return devices;
}

void ReceiverSelectorDialog::onSendClicked()
{
    QModelIndex currIndex = ui->listView->currentIndex();
    if (currIndex.isValid())
        accept();
    else
        QMessageBox::information(this, tr("Info"), tr("Please select receivers."));
}

void ReceiverSelectorDialog::onRefreshClicked()
{
    mModel->refresh();
}
