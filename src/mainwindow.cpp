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
#include "util.h"
#include "sender.h"
#include "receiver.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setupToolbar();
    setWindowTitle(ProgramName);

    mBroadcaster = new DeviceBroadcaster(this);
    mBroadcaster->start();
    mSenderModel = new TransferTableModel(this);
    mReceiverModel = new TransferTableModel(this);
    mDeviceModel = new DeviceListModel(mBroadcaster, this);
    mTransServer = new TransferServer(mDeviceModel, this);
    mTransServer->listen();

//    mSenderModel->setHeaderData((int) TransferTableModel::Column::Peer, Qt::Horizontal, tr("Receiver"));
//    mReceiverModel->setHeaderData((int) TransferTableModel::Column::Peer, Qt::Horizontal, tr("Sender"));

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

/*
 * Sebelum ditutup, check apakah masih terdapat proses trasfer
 * yg berlangsung, (Sending atau Receiving)
 */
void MainWindow::closeEvent(QCloseEvent *event)
{
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

void MainWindow::sendFile(const QString& folderName, const QString &fileName, const Device &receiver)
{
    Sender* sender = new Sender(receiver, folderName, fileName, this);
    sender->start();
    mSenderModel->insertTransfer(sender);
    QModelIndex progressIdx = mSenderModel->index(0, (int)TransferTableModel::Column::Progress);

    /*
     * tambah progress bar pada item transfer
     */
    QProgressBar* progress = new QProgressBar();
    connect(sender->getTransferInfo(), &TransferInfo::progressChanged, progress, &QProgressBar::setValue);

    ui->senderTableView->setIndexWidget(progressIdx, progress);
    ui->senderTableView->scrollToTop();
}

void MainWindow::selectReceiversAndSendTheFiles(QVector<QPair<QString, QString> > dirNameAndFullPath)
{
    ReceiverSelectorDialog dialog(mDeviceModel);
    if (dialog.exec() == QDialog::Accepted) {
        QVector<Device> receivers = dialog.getSelectedDevices();
        for (Device receiver : receivers) {
            if (receiver.isValid()) {

                mBroadcaster->sendBroadcast();
                for (auto p : dirNameAndFullPath) {
                    sendFile(p.first, p.second, receiver);
                }

            }
        }
    }
}

void MainWindow::onSendActionTriggered()
{
    QStringList fileNames = QFileDialog::getOpenFileNames(this, tr("Select files"), QDir::homePath());
    if (fileNames.size() <= 0)
        return;

    QVector<QPair<QString, QString> > pairs;
    for (auto fName : fileNames)
        pairs.push_back( QPair<QString, QString>("", fName) );

    selectReceiversAndSendTheFiles(pairs);
}

void MainWindow::onSendFolderActionTriggered()
{
    QString dirName = QFileDialog::getExistingDirectory(this, tr("Select Folder"), QDir::homePath(),
                                                    QFileDialog::ShowDirsOnly);
    if (dirName.isEmpty())
        return;

    QDir dir(dirName);
    QVector< QPair<QString, QString> > pairs = Util::getRelativeDirNameAndFullFilePath(dir, dir.dirName());

    selectReceiversAndSendTheFiles(pairs);
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
    connect(rec->getTransferInfo(), &TransferInfo::progressChanged, progress, &QProgressBar::setValue);
    mReceiverModel->insertTransfer(rec);
    QModelIndex progressIdx = mReceiverModel->index(0, (int)TransferTableModel::Column::Progress);

    ui->receiverTableView->setIndexWidget(progressIdx, progress);
    ui->receiverTableView->scrollToTop();
}

void MainWindow::onSenderTableSelectionChanged(const QItemSelection &selected, const QItemSelection &deselected)
{
    if (selected.size() > 0) {

        QModelIndex first = selected.indexes().first();
        if (first.isValid()) {
            TransferInfo* ti = mSenderModel->getTransferInfo(first.row());
            ui->resumeSenderBtn->setEnabled(ti->canResume());
            ui->pauseSenderBtn->setEnabled(ti->canPause());
            ui->cancelSenderBtn->setEnabled(ti->canCancel());

            connect(ti, &TransferInfo::stateChanged, this, &MainWindow::onSelectedSenderStateChanged);
        }

    }

    if (deselected.size() > 0) {

        QModelIndex first = deselected.indexes().first();
        if (first.isValid()) {
            TransferInfo* ti = mSenderModel->getTransferInfo(first.row());
            disconnect(ti, &TransferInfo::stateChanged, this, &MainWindow::onSelectedSenderStateChanged);
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
        Transfer* sender = mSenderModel->getTransfer(currIndex.row());
        sender->cancel();
    }
}

void MainWindow::onSenderPauseClicked()
{
    QModelIndex currIndex = ui->senderTableView->currentIndex();
    if (currIndex.isValid()) {
        Transfer* sender = mSenderModel->getTransfer(currIndex.row());
        sender->pause();
    }
}

void MainWindow::onSenderResumeClicked()
{
    QModelIndex currIndex = ui->senderTableView->currentIndex();
    if (currIndex.isValid()) {
        Transfer* sender = mSenderModel->getTransfer(currIndex.row());
        sender->resume();
    }
}


void MainWindow::onReceiverTableDoubleClicked(const QModelIndex& index)
{
    if (index.isValid()) {
        TransferInfo* ti = mReceiverModel->getTransferInfo(index.row());
        if (ti && ti->getState() == TransferState::Finish)
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
            TransferInfo* ti = mReceiverModel->getTransferInfo(first.row());
            ui->resumeReceiverBtn->setEnabled(ti->canResume());
            ui->pauseReceiverBtn->setEnabled(ti->canPause());
            ui->cancelReceiverBtn->setEnabled(ti->canCancel());

            connect(ti, &TransferInfo::stateChanged, this, &MainWindow::onSelectedReceiverStateChanged);
        }

    }

    if (deselected.size() > 0) {

        QModelIndex first = deselected.indexes().first();
        if (first.isValid()) {
            TransferInfo* ti = mReceiverModel->getTransferInfo(first.row());
            disconnect(ti, &TransferInfo::stateChanged, this, &MainWindow::onSelectedReceiverStateChanged);
        }

    }
}

void MainWindow::onReceiverCancelClicked()
{
    QModelIndex currIndex = ui->receiverTableView->currentIndex();
    if (currIndex.isValid()) {
        Transfer* rec = mReceiverModel->getTransfer(currIndex.row());
        rec->cancel();
    }
}

void MainWindow::onReceiverPauseClicked()
{
    QModelIndex currIndex = ui->receiverTableView->currentIndex();
    if (currIndex.isValid()) {
        Transfer* rec = mReceiverModel->getTransfer(currIndex.row());
        rec->pause();
    }
}

void MainWindow::onReceiverResumeClicked()
{
    QModelIndex currIndex = ui->receiverTableView->currentIndex();
    if (currIndex.isValid()) {
        Transfer* rec = mReceiverModel->getTransfer(currIndex.row());
        rec->resume();
    }
}

void MainWindow::onSenderTableContextMenuRequested(const QPoint& pos)
{
    QModelIndex currIndex = ui->senderTableView->indexAt(pos);
    QMenu contextMenu;

    if (currIndex.isValid()) {
        TransferInfo* ti = mSenderModel->getTransferInfo(currIndex.row());
        TransferState state = ti->getState();
        bool enableRemove = state == TransferState::Finish ||
                            state == TransferState::Cancelled ||
                            state == TransferState::Disconnected ||
                            state == TransferState::Idle;

        contextMenu.addAction(tr("Open..."), this, SLOT(openSenderFileInCurrentIndex()));
        contextMenu.addAction(tr("Open folder..."), this, SLOT(openSenderFolderInCurrentIndex()));
        contextMenu.addAction(tr("Send Files..."), this, SLOT(onSendActionTriggered()));
        contextMenu.addAction(tr("Send Folder..."), this, SLOT(onSendFolderActionTriggered()));
        contextMenu.addAction(tr("Remove "),
                       this, SLOT(removeSenderItemInCurrentIndex()))->setEnabled(enableRemove);
        contextMenu.addAction(QIcon(":/img/clear.png"), tr("Clear list"), this, SLOT(onSenderClearClicked()));
        contextMenu.addSeparator();
        contextMenu.addAction(QIcon(":/img/pause.png"), tr("Pause"),
                       this, SLOT(onSenderPauseClicked()))->setEnabled(ti->canPause());
        contextMenu.addAction(QIcon(":/img/resume.png"), tr("Resume"),
                       this, SLOT(onSenderResumeClicked()))->setEnabled(ti->canResume());
        contextMenu.addAction(QIcon(":/img/cancel.png"), tr("Cancel"),
                       this, SLOT(onSenderCancelClicked()))->setEnabled(ti->canCancel());
    }
    else {
        contextMenu.addAction(tr("Send Files..."), this, SLOT(onSendActionTriggered()));
        contextMenu.addAction(tr("Send Folder..."), this, SLOT(onSendFolderActionTriggered()));
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
        TransferInfo* ti = mReceiverModel->getTransferInfo(currIndex.row());
        TransferState state = ti->getState();
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
                       this, SLOT(onReceiverPauseClicked()))->setEnabled(ti->canPause());
        contextMenu.addAction(QIcon(":/img/resume.png"), tr("Resume"),
                       this, SLOT(onReceiverResumeClicked()))->setEnabled(ti->canResume());
        contextMenu.addAction(QIcon(":/img/cancel.png"), tr("Cancel"),
                       this, SLOT(onReceiverCancelClicked()))->setEnabled(ti->canCancel());
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
    QModelIndex fileNameIndex = mSenderModel->index(currIndex.row(), (int)TransferTableModel::Column::FileName);
    QString fileName = mSenderModel->data(fileNameIndex).toString();

    QDesktopServices::openUrl(QUrl::fromLocalFile(fileName));
}

void MainWindow::openSenderFolderInCurrentIndex()
{
    QModelIndex currIndex = ui->senderTableView->currentIndex();
    QModelIndex fileNameIndex = mSenderModel->index(currIndex.row(), (int)TransferTableModel::Column::FileName);
    QString dir = QFileInfo(mSenderModel->data(fileNameIndex).toString()).absoluteDir().absolutePath();

    QDesktopServices::openUrl(QUrl::fromLocalFile(dir));
}

void MainWindow::removeSenderItemInCurrentIndex()
{
    QModelIndex currIndex = ui->senderTableView->currentIndex();
    mSenderModel->removeTransfer(currIndex.row());
}

void MainWindow::openReceiverFileInCurrentIndex()
{
    QModelIndex currIndex = ui->receiverTableView->currentIndex();
    QModelIndex fileNameIndex = mReceiverModel->index(currIndex.row(), (int)TransferTableModel::Column::FileName);
    QString fileName = mReceiverModel->data(fileNameIndex).toString();

    QDesktopServices::openUrl(QUrl::fromLocalFile(fileName));
}

void MainWindow::openReceiverFolderInCurrentIndex()
{
    QModelIndex currIndex = ui->receiverTableView->currentIndex();
    QModelIndex fileNameIndex = mReceiverModel->index(currIndex.row(), (int)TransferTableModel::Column::FileName);
    QString dir = QFileInfo(mReceiverModel->data(fileNameIndex).toString()).absoluteDir().absolutePath();

    QDesktopServices::openUrl(QUrl::fromLocalFile(dir));
}

void MainWindow::removeReceiverItemInCurrentIndex()
{
    QModelIndex currIndex = ui->receiverTableView->currentIndex();
    mReceiverModel->removeTransfer(currIndex.row());
}

void MainWindow::deleteReceiverFileInCurrentIndex()
{
    QModelIndex currIndex = ui->receiverTableView->currentIndex();
    QModelIndex fileNameIndex = mReceiverModel->index(currIndex.row(), (int)TransferTableModel::Column::FileName);
    QString fileName = mReceiverModel->data(fileNameIndex).toString();

    QString str = "Are you sure wants to delete<p>" + fileName + "?";
    QMessageBox::StandardButton ret = QMessageBox::question(this, tr("Delete"), str);
    if (ret == QMessageBox::Yes) {
        QFile::remove(fileName);
        mReceiverModel->removeTransfer(currIndex.row());
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
    QMenu* sendMenu = new QMenu();
    sendMenu->addAction(tr("Send Files..."),
                    this, SLOT(onSendActionTriggered()));
    sendMenu->addAction(tr("Send Folder..."),
                    this, SLOT(onSendFolderActionTriggered()));

    QToolButton* sendBtn = new QToolButton();
    sendBtn->setPopupMode(QToolButton::InstantPopup);
    sendBtn->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    sendBtn->setText(tr("Send"));
    sendBtn->setIcon(QIcon(":/img/send.png"));
    sendBtn->setMenu(sendMenu);
    ui->mainToolBar->addWidget(sendBtn);
    ui->mainToolBar->addSeparator();

    ui->mainToolBar->addAction(QIcon(":/img/settings.png"), tr("Settings"),
                               this, SLOT(onSettingsActionTriggered()));

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

