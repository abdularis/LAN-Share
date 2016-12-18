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
    ui(new Ui::MainWindow), mForceQuit(false)
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

void MainWindow::onShowMainWindowTriggered()
{
    setMainWindowVisibility(true);
}

void MainWindow::onSendFilesActionTriggered()
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

        mSenderRemoveAction->setEnabled(enableRemove);
        mSenderPauseAction->setEnabled(ti->canPause());
        mSenderResumeAction->setEnabled(ti->canResume());
        mSenderCancelAction->setEnabled(ti->canCancel());

        contextMenu.addAction(mSenderOpenAction);
        contextMenu.addAction(mSenderOpenFolderAction);
        contextMenu.addSeparator();
        contextMenu.addAction(mSendFilesAction);
        contextMenu.addAction(mSendFolderAction);
        contextMenu.addSeparator();
        contextMenu.addAction(mSenderRemoveAction);
        contextMenu.addAction(mSenderClearAction);
        contextMenu.addSeparator();
        contextMenu.addAction(mSenderPauseAction);
        contextMenu.addAction(mSenderResumeAction);
        contextMenu.addAction(mSenderCancelAction);
    }
    else {
        contextMenu.addAction(mSendFilesAction);
        contextMenu.addAction(mSendFolderAction);
        contextMenu.addSeparator();
        contextMenu.addAction(mSenderClearAction);
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

        mRecOpenAction->setEnabled(enableFileMenu);
        mRecOpenFolderAction->setEnabled(enableFileMenu);
        mRecRemoveAction->setEnabled(enableFileMenu | enableRemove);
        mRecDeleteAction->setEnabled(enableFileMenu);
        mRecPauseAction->setEnabled(ti->canPause());
        mRecResumeAction->setEnabled(ti->canResume());
        mRecCancelAction->setEnabled(ti->canCancel());

        contextMenu.addAction(mRecOpenAction);
        contextMenu.addAction(mRecOpenFolderAction);
        contextMenu.addAction(mRecRemoveAction);
        contextMenu.addAction(mRecDeleteAction);
        contextMenu.addAction(mRecClearAction);
        contextMenu.addSeparator();
        contextMenu.addAction(mRecPauseAction);
        contextMenu.addAction(mRecResumeAction);
        contextMenu.addAction(mRecCancelAction);
    }
    else {
        contextMenu.addAction(mRecClearAction);
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

void MainWindow::quitApp()
{
    mForceQuit = true;
    close();
}

void MainWindow::setupToolbar()
{
    QMenu* sendMenu = new QMenu();
    sendMenu->addAction(mSendFilesAction);
    sendMenu->addAction(mSendFolderAction);

    QToolButton* sendBtn = new QToolButton();
    sendBtn->setPopupMode(QToolButton::InstantPopup);
    sendBtn->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    sendBtn->setText(tr("Send"));
    sendBtn->setIcon(QIcon(":/img/send.png"));
    sendBtn->setMenu(sendMenu);
    ui->mainToolBar->addWidget(sendBtn);
    ui->mainToolBar->addSeparator();

    ui->mainToolBar->addAction(mSettingsAction);

    QWidget* spacer = new QWidget();
    spacer->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
    ui->mainToolBar->addWidget(spacer);

    QMenu* menu = new QMenu();
    menu->addAction(mAboutAction);
    menu->addAction(mAboutQtAction);

    QToolButton* aboutBtn = new QToolButton();
    aboutBtn->setText(tr("About"));
    aboutBtn->setToolTip(tr("About this program"));
    aboutBtn->setIcon(QIcon(":/img/about.png"));
    aboutBtn->setMenu(menu);
    aboutBtn->setPopupMode(QToolButton::InstantPopup);
    ui->mainToolBar->addWidget(aboutBtn);
}

void MainWindow::setupActions()
{
    mShowMainWindowAction = new QAction(tr("Show Main Window"), this);
    connect(mShowMainWindowAction, &QAction::triggered, this, &MainWindow::onShowMainWindowTriggered);
    mSendFilesAction = new QAction(QIcon(":/img/file.png"), tr("Send files..."), this);
    connect(mSendFilesAction, &QAction::triggered, this, &MainWindow::onSendFilesActionTriggered);
    mSendFolderAction = new QAction(QIcon(":/img/folder.png"), tr("Send folder..."), this);
    connect(mSendFolderAction, &QAction::triggered, this, &MainWindow::onSendFolderActionTriggered);
    mSettingsAction = new QAction(QIcon(":/img/settings.png"), tr("Settings"), this);
    connect(mSettingsAction, &QAction::triggered, this, &MainWindow::onSettingsActionTriggered);
    mAboutAction = new QAction(QIcon(":/img/about.png"), tr("About"), this);
    mAboutAction->setMenuRole(QAction::AboutRole);
    connect(mAboutAction, &QAction::triggered, this, &MainWindow::onAboutActionTriggered);
    mAboutQtAction = new QAction(tr("About Qt"), this);
    mAboutQtAction->setMenuRole(QAction::AboutQtRole);
    connect(mAboutQtAction, &QAction::triggered, QApplication::instance(), &QApplication::aboutQt);
    mQuitAction = new QAction(tr("Quit"), this);
    connect(mQuitAction, &QAction::triggered, this, &MainWindow::quitApp);

    mSenderOpenAction = new QAction(tr("Open"), this);
    connect(mSenderOpenAction, &QAction::triggered, this, &MainWindow::openSenderFileInCurrentIndex);
    mSenderOpenFolderAction = new QAction(tr("Open folder"), this);
    connect(mSenderOpenFolderAction, &QAction::triggered, this, &MainWindow::openSenderFolderInCurrentIndex);
    mSenderRemoveAction = new QAction(QIcon(":/img/remove.png"), tr("Remove"), this);
    connect(mSenderRemoveAction, &QAction::triggered, this, &MainWindow::removeSenderItemInCurrentIndex);
    mSenderClearAction = new QAction(QIcon(":/img/clear.png"), tr("Clear"), this);
    connect(mSenderClearAction, &QAction::triggered, this, &MainWindow::onSenderClearClicked);
    mSenderPauseAction = new QAction(QIcon(":/img/pause.png"), tr("Pause"), this);
    connect(mSenderPauseAction, &QAction::triggered, this, &MainWindow::onSenderPauseClicked);
    mSenderResumeAction = new QAction(QIcon(":/img/resume.png"), tr("Resume"), this);
    connect(mSenderResumeAction, &QAction::triggered, this, &MainWindow::onSenderResumeClicked);
    mSenderCancelAction = new QAction(QIcon(":/img/cancel.png"), tr("Cancel"), this);
    connect(mSenderCancelAction, &QAction::triggered, this, &MainWindow::onSenderCancelClicked);

    mRecOpenAction = new QAction(tr("Open"), this);
    connect(mRecOpenAction, &QAction::triggered, this, &MainWindow::openReceiverFileInCurrentIndex);
    mRecOpenFolderAction = new QAction(tr("Open folder"), this);
    connect(mRecOpenFolderAction, &QAction::triggered, this, &MainWindow::openReceiverFolderInCurrentIndex);
    mRecRemoveAction = new QAction(QIcon(":/img/remove.png"), tr("Remove"), this);
    connect(mRecRemoveAction, &QAction::triggered, this, &MainWindow::removeReceiverItemInCurrentIndex);
    mRecDeleteAction = new QAction(tr("Delete from disk"), this);
    connect(mRecDeleteAction, &QAction::triggered, this, &MainWindow::deleteReceiverFileInCurrentIndex);
    mRecClearAction = new QAction(QIcon(":/img/clear.png"), tr("Clear"), this);
    connect(mRecClearAction, &QAction::triggered, this, &MainWindow::onReceiverClearClicked);
    mRecPauseAction = new QAction(QIcon(":/img/pause.png"), tr("Pause"), this);
    connect(mRecPauseAction, &QAction::triggered, this, &MainWindow::onReceiverPauseClicked);
    mRecResumeAction = new QAction(QIcon(":/img/resume.png"), tr("Resume"), this);
    connect(mRecResumeAction, &QAction::triggered, this, &MainWindow::onReceiverResumeClicked);
    mRecCancelAction = new QAction(QIcon(":/img/cancel.png"), tr("Cancel"), this);
    connect(mRecCancelAction, &QAction::triggered, this, &MainWindow::onReceiverCancelClicked);
}

void MainWindow::setupSystrayIcon()
{
    if (!QSystemTrayIcon::isSystemTrayAvailable()) {
        mSystrayIcon = NULL;
        return;
    }

    mSystrayMenu = new QMenu(this);
    mSystrayMenu->addAction(mShowMainWindowAction);
    mSystrayMenu->addSeparator();
    mSystrayMenu->addAction(mSendFilesAction);
    mSystrayMenu->addAction(mSendFolderAction);
    mSystrayMenu->addSeparator();
    mSystrayMenu->addAction(mAboutAction);
    mSystrayMenu->addAction(mAboutQtAction);
    mSystrayMenu->addSeparator();
    mSystrayMenu->addAction(mQuitAction);

    mSystrayIcon = new QSystemTrayIcon(QIcon(":/img/systray-icon.png"), this);
    mSystrayIcon->setToolTip(tr(PROGRAM_NAME));
    mSystrayIcon->setContextMenu(mSystrayMenu);
    mSystrayIcon->show();
}
