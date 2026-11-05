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

#include <QDesktopServices>
#include <QProgressBar>
#include <QFileDialog>
#include <QMessageBox>
#include <QMenu>
#include <QToolButton>
#include <QCloseEvent>
#include <QtDebug>
#include <QListView>
#include <QTreeView>

#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "receiverselectordialog.h"
#include "settingsdialog.h"
#include "settings.h"
#include "aboutdialog.h"
#include "util.h"
#include "transfer/sender.h"
#include "transfer/receiver.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setupActions();
    setupToolbar();
    setupSystrayIcon();
    setWindowTitle(PROGRAM_NAME);

    mBroadcaster = new DeviceBroadcaster(this);
    mBroadcaster->start();
    mSenderModel = new TransferTableModel(this);
    mReceiverModel = new TransferTableModel(this);
    mDeviceModel = new DeviceListModel(mBroadcaster, this);
    mTransServer = new TransferServer(mDeviceModel, this);
    mTransServer->listen();

    ui->senderTableView->setModel(mSenderModel);
    ui->receiverTableView->setModel(mReceiverModel);

    ui->senderTableView->setColumnWidth((int)TransferTableModel::Column::FileName, 340);
    ui->senderTableView->setColumnWidth((int)TransferTableModel::Column::Progress, 160);

    ui->receiverTableView->setColumnWidth((int)TransferTableModel::Column::FileName, 340);
    ui->receiverTableView->setColumnWidth((int)TransferTableModel::Column::Progress, 160);

    connectSignals();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    if (mSystrayIcon && mSystrayIcon->isVisible() && !mForceQuit) {
        setMainWindowVisibility(false);
        event->ignore();
        return;
    }

    auto checkTransferState = [](Transfer* t) {
        if (!t)
            return false;
        TransferState state = t->getTransferInfo()->getState();
        return state == TransferState::Paused ||
                state == TransferState::Transfering ||
                state == TransferState::Waiting;
    };

    auto checkTransferModel = [&](TransferTableModel* model) {
        int count = model->rowCount();
        for (int i = 0; i < count; i++) {
            Transfer* t = model->getTransfer(i);
            if (checkTransferState(t)) {
                return true;
            }
        }
        return false;
    };

    bool needToConfirm = checkTransferModel(mSenderModel);
    if (!needToConfirm)
        needToConfirm = checkTransferModel(mReceiverModel);

    if (needToConfirm) {
        QMessageBox::StandardButton ret =
                QMessageBox::question(this, tr("Confirm close"),
                                      tr("You are about to close & abort all transfers. Do you want to continue?"));
        if (ret == QMessageBox::No) {
            event->ignore();
            mForceQuit = false;
            return;
        }
    }

    event->accept();
    qApp->quit();
}

void MainWindow::setMainWindowVisibility(bool visible)
{
    if (visible) {
        showNormal();
        setWindowState(Qt::WindowNoState);
        qApp->processEvents();
        setWindowState(Qt::WindowActive);
        qApp->processEvents();
        qApp->setActiveWindow(this);
        qApp->processEvents();
    }
    else {
        hide();
    }
}

void MainWindow::connectSignals()
{
    connect(mTransServer, &TransferServer::newReceiverAdded, this, &MainWindow::onNewReceiverAdded);

    QItemSelectionModel* senderSel = ui->senderTableView->selectionModel();
    connect(senderSel, &QItemSelectionModel::selectionChanged,
            this, &MainWindow::onSenderTableSelectionChanged);

    QItemSelectionModel* receiverSel = ui->receiverTableView->selectionModel();
    connect(receiverSel, &QItemSelectionModel::selectionChanged,
            this, &MainWindow::onReceiverTableSelectionChanged);
}

void MainWindow::sendFile(const QString& folderName, const QString &filePath, const Device &receiver)
{
    mSendQueue.enqueue(qMakePair(folderName, filePath));
    mCurrentReceiver = receiver;
    processSendQueue();
}

void MainWindow::processSendQueue()
{
    while (mActiveSends < MAX_CONCURRENT_SENDS && !mSendQueue.isEmpty()) {
        auto item = mSendQueue.dequeue();
        const QString& folderName = item.first;
        const QString& filePath = item.second;

        Sender* sender = new Sender(mCurrentReceiver, folderName, filePath, this);
        connect(sender->getTransferInfo(), &TransferInfo::done, this, &MainWindow::onSenderFinished);
        connect(sender->getTransferInfo(), &TransferInfo::errorOcurred, this, &MainWindow::onSenderFinished);

        sender->start();
        mSenderModel->insertTransfer(sender);
        QModelIndex progressIdx = mSenderModel->index(0, (int)TransferTableModel::Column::Progress);

        QProgressBar* progress = new QProgressBar();
        connect(sender->getTransferInfo(), &TransferInfo::progressChanged, progress, &QProgressBar::setValue);

        ui->senderTableView->setIndexWidget(progressIdx, progress);
        ui->senderTableView->scrollToTop();

        mActiveSends++;
    }
}

void MainWindow::onSenderFinished()
{
    mActiveSends = qMax(0, mActiveSends - 1);
    processSendQueue();
}

void MainWindow::selectReceiversAndSendTheFiles(QVector<QPair<QString, QString> > dirNameAndFullPath)
{
    ReceiverSelectorDialog dialog(mDeviceModel);
    if (dialog.exec() == QDialog::Accepted) {
        QVector<Device> receivers = dialog.getSelectedDevices();
        for (const Device& receiver : receivers) {
            if (receiver.isValid()) {
                mBroadcaster->sendBroadcast();
                mCurrentReceiver = receiver;
                for (const auto& p : dirNameAndFullPath) {
                    mSendQueue.enqueue(p);
                }
                processSendQueue();
            }
        }
    }
}

// ... (the rest of the original file remains unchanged below this point)
// Note: The full original content after selectReceiversAndSendTheFiles is preserved in the repository.
// This update replaces only the transfer starting logic with a queue.
