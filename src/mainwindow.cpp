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

#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "receiverselectordialog.h"
#include "settingsdialog.h"
#include "settings.h"
#include "aboutdialog.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setupToolbar();
    setWindowTitle(ProgramName);

    mBroadcaster = new DeviceBroadcaster(this);
    mBroadcaster->start();
    mSenderModel = new SenderTableModel(this);
    mReceiverModel = new ReceiverTableModel(this);
    mDeviceModel = new DeviceListModel(mBroadcaster, this);
    mTransServer = new TransferServer(mDeviceModel, this);
    mTransServer->listen();

    ui->senderTableView->setModel(mSenderModel);
    ui->receiverTableView->setModel(mReceiverModel);

    ui->senderTableView->setColumnWidth((int)SenderTableModel::Column::FileName, 340);
    ui->senderTableView->setColumnWidth((int)SenderTableModel::Column::Progress, 160);

    ui->receiverTableView->setColumnWidth((int)ReceiverTableModel::Column::FileName, 340);
    ui->receiverTableView->setColumnWidth((int)ReceiverTableModel::Column::Progress, 160);

    connectSignals();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    bool needToConfirm = false;
    int count = mSenderModel->rowCount();

    auto check = [](Transfer* t) -> bool {
        if (!t)
            return false;
        TransferState state = t->getState();
        return state == TransferState::Paused ||
                state == TransferState::Transfering ||
                state == TransferState::Waiting;
    };

    for (int i = 0; i < count; i++) {
        Sender* sender = mSenderModel->getSender(i);
        if (check(sender)) {
            needToConfirm = true;
            break;
        }
    }

    if (!needToConfirm) {
        count = mReceiverModel->rowCount();
        for (int i = 0; i < count; i++) {
            Receiver* rec = mReceiverModel->getReceiver(i);
            if (check(rec)) {
                needToConfirm = true;
                break;
            }
        }
    }

    if (needToConfirm) {
        QMessageBox::StandardButton ret =
                QMessageBox::question(this, tr("Confirm close"),
                                      tr("You are about to close & abort all transfers. Do you want to continue?"));
        if (ret == QMessageBox::Yes) {
            event->accept();
        }
        else {
            event->ignore();
        }
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

void MainWindow::sendFile(const QString &fileName, const Device &receiver)
{
    Sender* sender = new Sender(receiver, fileName, this);
    sender->start();
    mSenderModel->insertSender(sender);
    QModelIndex progressIdx = mSenderModel->index(mSenderModel->rowCount() - 1, (int)SenderTableModel::Column::Progress);

    /*
     * tambah progress bar pada item transfer
     */
    QProgressBar* progress = new QProgressBar();
    connect(sender, &Sender::progressChanged, progress, &QProgressBar::setValue);

    ui->senderTableView->setIndexWidget(progressIdx, progress);
    ui->senderTableView->scrollToBottom();
}

void MainWindow::onSendActionTriggered()
{
    QStringList fileNames = QFileDialog::getOpenFileNames(this, tr("Select files"), QDir::homePath());
    if (fileNames.size() <= 0)
        return;

    ReceiverSelectorDialog dialog(mDeviceModel);
    if (dialog.exec() == QDialog::Accepted) {
//        Device receiver = dialog.getSelectedDevice();
        QVector<Device> receivers = dialog.getSelectedDevices();
        for (Device receiver : receivers) {
            if (receiver.isValid()) {

                /*
                 * send broadcast ke penerima, jadi device ini(sender) akan selalu
                 * ada di list model penerima.
                 */
                mBroadcaster->sendBroadcast();
                foreach (QString fName, fileNames) {
                    if (!fName.isEmpty())
                        sendFile(fName, receiver);
                }

            }
        }
    }
}

void MainWindow::onSettingsActionTriggered()
{
    SettingsDialog dialog;
    dialog.exec();
}

void MainWindow::onAboutActionTriggered()
{
    AboutDialog dialog;
    dialog.exec();
}

void MainWindow::onNewReceiverAdded(Receiver *rec)
{
    QProgressBar* progress = new QProgressBar();
    connect(rec, &Receiver::progressChanged, progress, &QProgressBar::setValue);
    mReceiverModel->insertReceiver(rec);
    QModelIndex progressIdx = mReceiverModel->index(mReceiverModel->rowCount() - 1, (int)ReceiverTableModel::Column::Progress);

    ui->receiverTableView->setIndexWidget(progressIdx, progress);
    ui->receiverTableView->scrollToBottom();
}

void MainWindow::onSenderTableSelectionChanged(const QItemSelection &selected, const QItemSelection &deselected)
{
    if (selected.size() > 0) {

        QModelIndex first = selected.indexes().first();
        if (first.isValid()) {
            Sender* sender = mSenderModel->getSender(first.row());
            if (sender) {
                ui->resumeSenderBtn->setEnabled(sender->canResume());
                ui->pauseSenderBtn->setEnabled(sender->canPause());
                ui->cancelSenderBtn->setEnabled(sender->canCancel());

                connect(sender, &Sender::stateChanged, this, &MainWindow::onSelectedSenderStateChanged);
            }
        }

    }

    if (deselected.size() > 0) {

        QModelIndex first = deselected.indexes().first();
        if (first.isValid()) {
            Sender* sender = mSenderModel->getSender(first.row());
            if (sender) {
                disconnect(sender, &Sender::stateChanged, this, &MainWindow::onSelectedSenderStateChanged);
            }
        }

    }
}

void MainWindow::onSenderTableDoubleClicked(const QModelIndex& index)
{
    Q_UNUSED(index);
    openSenderFileInCurrentIndex();
}

void MainWindow::onSenderClearClicked()
{
    mSenderModel->clearCompleted();
}

void MainWindow::onSenderCancelClicked()
{
    QModelIndex currIndex = ui->senderTableView->currentIndex();
    if (currIndex.isValid()) {
        Sender* sender = mSenderModel->getSender(currIndex.row());
        sender->cancel();
    }
}

void MainWindow::onSenderPauseClicked()
{
    QModelIndex currIndex = ui->senderTableView->currentIndex();
    if (currIndex.isValid()) {
        Sender* sender = mSenderModel->getSender(currIndex.row());
        sender->pause();
    }
}

void MainWindow::onSenderResumeClicked()
{
    QModelIndex currIndex = ui->senderTableView->currentIndex();
    if (currIndex.isValid()) {
        Sender* sender = mSenderModel->getSender(currIndex.row());
        sender->resume();
    }
}


void MainWindow::onReceiverTableDoubleClicked(const QModelIndex& index)
{
    if (index.isValid()) {
        Receiver* rec = mReceiverModel->getReceiver(index.row());
        if (rec && rec->getState() == TransferState::Finish)
            openReceiverFileInCurrentIndex();
    }
}

void MainWindow::onReceiverClearClicked()
{
    mReceiverModel->clearCompleted();
}

void MainWindow::onReceiverTableSelectionChanged(const QItemSelection &selected, const QItemSelection &deselected)
{
    if (selected.size() > 0) {

        QModelIndex first = selected.indexes().first();
        if (first.isValid()) {
            Receiver* rec = mReceiverModel->getReceiver(first.row());
            if (rec) {
                ui->resumeReceiverBtn->setEnabled(rec->canResume());
                ui->pauseReceiverBtn->setEnabled(rec->canPause());
                ui->cancelReceiverBtn->setEnabled(rec->canCancel());

                connect(rec, &Receiver::stateChanged, this, &MainWindow::onSelectedReceiverStateChanged);
            }
        }

    }

    if (deselected.size() > 0) {

        QModelIndex first = deselected.indexes().first();
        if (first.isValid()) {
            Receiver* rec = mReceiverModel->getReceiver(first.row());
            if (rec) {
                disconnect(rec, &Receiver::stateChanged, this, &MainWindow::onSelectedReceiverStateChanged);
            }
        }

    }
}

void MainWindow::onReceiverCancelClicked()
{
    QModelIndex currIndex = ui->receiverTableView->currentIndex();
    if (currIndex.isValid()) {
        Receiver* rec = mReceiverModel->getReceiver(currIndex.row());
        rec->cancel();
    }
}

void MainWindow::onReceiverPauseClicked()
{
    QModelIndex currIndex = ui->receiverTableView->currentIndex();
    if (currIndex.isValid()) {
        Receiver* rec = mReceiverModel->getReceiver(currIndex.row());
        rec->pause();
    }
}

void MainWindow::onReceiverResumeClicked()
{
    QModelIndex currIndex = ui->receiverTableView->currentIndex();
    if (currIndex.isValid()) {
        Receiver* rec = mReceiverModel->getReceiver(currIndex.row());
        rec->resume();
    }
}

void MainWindow::onSenderTableContextMenuRequested(const QPoint& pos)
{
    QModelIndex currIndex = ui->senderTableView->indexAt(pos);
    QMenu contextMenu;

    if (currIndex.isValid()) {
        Sender* sender = mSenderModel->getSender(currIndex.row());
        TransferState state = sender->getState();
        bool enableRemove = state == TransferState::Finish ||
                            state == TransferState::Cancelled ||
                            state == TransferState::Disconnected ||
                            state == TransferState::Idle;

        contextMenu.addAction(tr("Open..."), this, SLOT(openSenderFileInCurrentIndex()));
        contextMenu.addAction(tr("Open folder..."), this, SLOT(openSenderFolderInCurrentIndex()));
        contextMenu.addAction(tr("Send file..."), this, SLOT(onSendActionTriggered()));
        contextMenu.addAction(tr("Remove "),
                       this, SLOT(removeSenderItemInCurrentIndex()))->setEnabled(enableRemove);
        contextMenu.addAction(QIcon(":/img/clear.png"), tr("Clear list"), this, SLOT(onSenderClearClicked()));
        contextMenu.addSeparator();
        contextMenu.addAction(QIcon(":/img/pause.png"), tr("Pause"),
                       this, SLOT(onSenderPauseClicked()))->setEnabled(sender->canPause());
        contextMenu.addAction(QIcon(":/img/resume.png"), tr("Resume"),
                       this, SLOT(onSenderResumeClicked()))->setEnabled(sender->canResume());
        contextMenu.addAction(QIcon(":/img/cancel.png"), tr("Cancel"),
                       this, SLOT(onSenderCancelClicked()))->setEnabled(sender->canCancel());
    }
    else {
        contextMenu.addAction(tr("Send file..."), this, SLOT(onSendActionTriggered()));
        contextMenu.addAction(QIcon(":/img/clear.png"), tr("Clear list"), this, SLOT(onSenderClearClicked()));
    }

    QPoint globPos = ui->senderTableView->mapToGlobal(pos);
    contextMenu.exec(globPos);
}

void MainWindow::onReceiverTableContextMenuRequested(const QPoint& pos)
{
    QModelIndex currIndex = ui->receiverTableView->indexAt(pos);
    QMenu contextMenu;

    if (currIndex.isValid()) {
        Receiver* rec = mReceiverModel->getReceiver(currIndex.row());
        TransferState state = rec->getState();
        bool enableFileMenu = state == TransferState::Finish;
        bool enableRemove = state == TransferState::Finish ||
                            state == TransferState::Cancelled ||
                            state == TransferState::Disconnected ||
                            state == TransferState::Idle;

        contextMenu.addAction(tr("Open..."), this, SLOT(openReceiverFileInCurrentIndex()))->setEnabled(enableFileMenu);
        contextMenu.addAction(tr("Open folder..."), this, SLOT(openReceiverFolderInCurrentIndex()))->setEnabled(enableFileMenu);
        contextMenu.addAction(tr("Remove"), this, SLOT(removeReceiverItemInCurrentIndex()))->setEnabled(enableFileMenu || enableRemove);
        contextMenu.addAction(tr("Delete from disk"), this, SLOT(deleteReceiverFileInCurrentIndex()))->setEnabled(enableFileMenu);
        contextMenu.addAction(QIcon(":/img/clear.png"), tr("Clear list"), this, SLOT(onReceiverClearClicked()));
        contextMenu.addSeparator();
        contextMenu.addAction(QIcon(":/img/pause.png"), tr("Pause"),
                       this, SLOT(onReceiverPauseClicked()))->setEnabled(rec->canPause());
        contextMenu.addAction(QIcon(":/img/resume.png"), tr("Resume"),
                       this, SLOT(onReceiverResumeClicked()))->setEnabled(rec->canResume());
        contextMenu.addAction(QIcon(":/img/cancel.png"), tr("Cancel"),
                       this, SLOT(onReceiverCancelClicked()))->setEnabled(rec->canCancel());
    }
    else {
        contextMenu.addAction(QIcon(":/img/clear.png"), tr("Clear list"), this, SLOT(onReceiverClearClicked()));
    }

    QPoint globPos = ui->receiverTableView->mapToGlobal(pos);
    contextMenu.exec(globPos);
}

void MainWindow::openSenderFileInCurrentIndex()
{
    QModelIndex currIndex = ui->senderTableView->currentIndex();
    QModelIndex fileNameIndex = mSenderModel->index(currIndex.row(), (int)SenderTableModel::Column::FileName);
    QString fileName = mSenderModel->data(fileNameIndex).toString();

    QDesktopServices::openUrl(QUrl::fromLocalFile(fileName));
}

void MainWindow::openSenderFolderInCurrentIndex()
{
    QModelIndex currIndex = ui->senderTableView->currentIndex();
    QModelIndex fileNameIndex = mSenderModel->index(currIndex.row(), (int)SenderTableModel::Column::FileName);
    QString dir = QFileInfo(mSenderModel->data(fileNameIndex).toString()).absoluteDir().absolutePath();

    QDesktopServices::openUrl(QUrl::fromLocalFile(dir));
}

void MainWindow::removeSenderItemInCurrentIndex()
{
    QModelIndex currIndex = ui->senderTableView->currentIndex();
    mSenderModel->removeSender(currIndex.row());
}

void MainWindow::openReceiverFileInCurrentIndex()
{
    QModelIndex currIndex = ui->receiverTableView->currentIndex();
    QModelIndex fileNameIndex = mReceiverModel->index(currIndex.row(), (int)ReceiverTableModel::Column::FileName);
    QString fileName = mReceiverModel->data(fileNameIndex).toString();

    QDesktopServices::openUrl(QUrl::fromLocalFile(fileName));
}

void MainWindow::openReceiverFolderInCurrentIndex()
{
    QModelIndex currIndex = ui->receiverTableView->currentIndex();
    QModelIndex fileNameIndex = mReceiverModel->index(currIndex.row(), (int)ReceiverTableModel::Column::FileName);
    QString dir = QFileInfo(mReceiverModel->data(fileNameIndex).toString()).absoluteDir().absolutePath();

    QDesktopServices::openUrl(QUrl::fromLocalFile(dir));
}

void MainWindow::removeReceiverItemInCurrentIndex()
{
    QModelIndex currIndex = ui->receiverTableView->currentIndex();
    mReceiverModel->removeReceiver(currIndex.row());
}

void MainWindow::deleteReceiverFileInCurrentIndex()
{
    QModelIndex currIndex = ui->receiverTableView->currentIndex();
    QModelIndex fileNameIndex = mReceiverModel->index(currIndex.row(), (int)ReceiverTableModel::Column::FileName);
    QString fileName = mReceiverModel->data(fileNameIndex).toString();

    QString str = "Are you sure wants to delete<p>" + fileName + "?";
    QMessageBox::StandardButton ret = QMessageBox::question(this, tr("Delete"), str);
    if (ret == QMessageBox::Yes) {
        QFile::remove(fileName);
        mReceiverModel->removeReceiver(currIndex.row());
    }
}

void MainWindow::onSelectedSenderStateChanged(TransferState state)
{
    ui->resumeSenderBtn->setEnabled(state == TransferState::Paused);
    ui->pauseSenderBtn->setEnabled(state == TransferState::Transfering || state == TransferState::Waiting);
    ui->cancelSenderBtn->setEnabled(state == TransferState::Transfering || state == TransferState::Waiting ||
                                    state == TransferState::Paused);
}

void MainWindow::onSelectedReceiverStateChanged(TransferState state)
{
    ui->resumeReceiverBtn->setEnabled(state == TransferState::Paused);
    ui->pauseReceiverBtn->setEnabled(state == TransferState::Transfering || state == TransferState::Waiting);
    ui->cancelReceiverBtn->setEnabled(state == TransferState::Transfering || state == TransferState::Waiting ||
                                    state == TransferState::Paused);
}

void MainWindow::setupToolbar()
{
    QWidget* spacer = new QWidget();
    spacer->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
    ui->mainToolBar->addWidget(spacer);

    QMenu* menu = new QMenu();
    menu->addAction(QIcon(":/img/about.png"), tr("About"),
                    this, SLOT(onAboutActionTriggered()))->setMenuRole(QAction::AboutRole);
    menu->addAction(tr("About Qt"),
                    QApplication::instance(), SLOT(aboutQt()))->setMenuRole(QAction::AboutQtRole);

    QToolButton* aboutBtn = new QToolButton();
    aboutBtn->setText(tr("About"));
    aboutBtn->setToolTip(tr("About this program"));
    aboutBtn->setIcon(QIcon(":/img/about.png"));
    aboutBtn->setMenu(menu);
    aboutBtn->setPopupMode(QToolButton::InstantPopup);
    ui->mainToolBar->addWidget(aboutBtn);
}
