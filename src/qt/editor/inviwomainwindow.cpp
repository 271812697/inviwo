/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2012-2017 Inviwo Foundation
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *********************************************************************************/

#include <inviwo/qt/editor/inviwomainwindow.h>

#include <inviwo/core/network/processornetwork.h>
#include <inviwo/core/common/inviwocore.h>
#include <inviwo/core/util/commandlineparser.h>
#include <inviwo/core/util/filesystem.h>
#include <inviwo/core/util/settings/systemsettings.h>
#include <inviwo/core/util/utilities.h>
#include <inviwo/core/util/systemcapabilities.h>
#include <inviwo/core/util/vectoroperations.h>
#include <inviwo/core/network/workspacemanager.h>
#include <inviwo/qt/editor/consolewidget.h>
#include <inviwo/qt/editor/helpwidget.h>
#include <inviwo/qt/editor/processorpreview.h>
#include <inviwo/qt/editor/networkeditor.h>
#include <inviwo/qt/editor/networkeditorview.h>
#include <inviwo/qt/editor/processorlistwidget.h>
#include <inviwo/qt/editor/settingswidget.h>
#include <inviwo/qt/editor/networksearch.h>
#include <inviwo/qt/editor/inviwoeditmenu.h>
#include <inviwo/qt/applicationbase/inviwoapplicationqt.h>
#include <modules/qtwidgets/inviwofiledialog.h>
#include <modules/qtwidgets/propertylistwidget.h>
#include <inviwo/core/metadata/processormetadata.h>
#include <inviwo/core/common/inviwomodulefactoryobject.h>
#include <inviwo/core/network/workspaceutils.h>
#include <inviwo/core/network/networklock.h>

#include <inviwomodulespaths.h>

#include <warn/push>
#include <warn/ignore/all>

#include <QScreen>
#include <QStandardPaths>

#include <QActionGroup>
#include <QClipboard>
#include <QDesktopWidget>
#include <QFileDialog>
#include <QList>
#include <QMessageBox>
#include <QMimeData>
#include <QSettings>
#include <QUrl>
#include <QVariant>
#include <QToolBar>
#include <algorithm>

#include <warn/pop>

namespace inviwo {

InviwoMainWindow::InviwoMainWindow(InviwoApplicationQt* app)
    : QMainWindow()
    , app_(app)
    , networkEditor_(nullptr)
    , appUsageModeProp_(nullptr)
    , snapshotArg_("s", "snapshot",
                   "Specify base name of each snapshot, or \"UPN\" string for processor name.",
                   false, "", "file name")
    , screenGrabArg_("g", "screengrab", "Specify default name of each screen grab.", false, "",
                     "file name")
    , saveProcessorPreviews_("", "save-previews", "Save processor previews to the supplied path",
                             false, "", "path")
    , updateWorkspaces_("", "update-workspaces",
                        "Go through and update all workspaces the the latest versions")
    , undoManager_(this) {

    app_->setMainWindow(this);

    // make sure, tooltips are always shown (this includes port inspectors as well)
    this->setAttribute(Qt::WA_AlwaysShowToolTips, true);
    editMenu_ = new InviwoEditMenu(this);
    networkEditor_ = util::make_unique<NetworkEditor>(this);
    // initialize console widget first to receive log messages
    consoleWidget_ = std::make_shared<ConsoleWidget>(this);
    LogCentral::getPtr()->registerLogger(consoleWidget_);
    currentWorkspaceFileName_ = "";

    const QDesktopWidget dw;
    auto screen = dw.screenGeometry(this);
    const float maxRatio = 0.8f;

    QSize size(1920, 1080);
    size.setWidth(std::min(size.width(), static_cast<int>(screen.width() * maxRatio)));
    size.setHeight(std::min(size.height(), static_cast<int>(screen.height() * maxRatio)));

    // Center Window
    QPoint pos{screen.width() / 2 - size.width() / 2, screen.height() / 2 - size.height() / 2};

    resize(size);
    move(pos);

    app->getCommandLineParser().add(&snapshotArg_,
                                    [this]() {
                                        saveCanvases(app_->getCommandLineParser().getOutputPath(),
                                                     snapshotArg_.getValue());
                                    },
                                    1000);

    app->getCommandLineParser().add(&screenGrabArg_,
                                    [this]() {
                                        getScreenGrab(app_->getCommandLineParser().getOutputPath(),
                                                      screenGrabArg_.getValue());
                                    },
                                    1000);

    app->getCommandLineParser().add(&saveProcessorPreviews_,
                                    [this]() {
                                        utilqt::saveProcessorPreviews(
                                            app_, saveProcessorPreviews_.getValue());

                                    },
                                    1200);

    app->getCommandLineParser().add(&updateWorkspaces_, [this]() { util::updateWorkspaces(app_); },
                                    1250);

    networkEditorView_ = new NetworkEditorView(networkEditor_.get(), this);
    NetworkEditorObserver::addObservation(networkEditor_.get());
    auto grid = new QGridLayout(networkEditorView_);
    grid->setContentsMargins(7, 7, 7, 7);
    networkSearch_ = new NetworkSearch(this);
    grid->addWidget(networkSearch_, 0, 0, Qt::AlignTop | Qt::AlignRight);

    setCentralWidget(networkEditorView_);

    settingsWidget_ = new SettingsWidget(this);
    addDockWidget(Qt::LeftDockWidgetArea, settingsWidget_);
    settingsWidget_->hide();

    helpWidget_ = new HelpWidget(this);
    addDockWidget(Qt::RightDockWidgetArea, helpWidget_);

    processorTreeWidget_ = new ProcessorTreeWidget(this, helpWidget_);
    addDockWidget(Qt::LeftDockWidgetArea, processorTreeWidget_);

    propertyListWidget_ = new PropertyListWidget(this);
    addDockWidget(Qt::RightDockWidgetArea, propertyListWidget_);

    addDockWidget(Qt::BottomDockWidgetArea, consoleWidget_.get());
    // load settings and restore window state
    loadWindowState();

    QSettings settings("Inviwo", "Inviwo");
    settings.beginGroup("mainwindow");
    QString firstWorkspace = filesystem::getPath(PathType::Workspaces, "/boron.inv").c_str();
    workspaceOnLastSuccessfulExit_ =
        settings.value("workspaceOnLastSuccessfulExit", QVariant::fromValue(firstWorkspace))
            .toString();
    settings.setValue("workspaceOnLastSuccessfulExit", "");
    settings.endGroup();
    rootDir_ = QString::fromStdString(filesystem::getPath(PathType::Data));
    workspaceFileDir_ = rootDir_ + "/workspaces";

    // initialize menus
    addActions();
    networkEditorView_->setFocus();
}

InviwoMainWindow::~InviwoMainWindow() = default;

void InviwoMainWindow::showWindow() {
    if (maximized_)
        showMaximized();
    else
        show();
}

void InviwoMainWindow::saveCanvases(std::string path, std::string fileName) {
    if (path.empty()) path = app_->getPath(PathType::Images);

    repaint();
    app_->processEvents();
    util::saveAllCanvases(app_->getProcessorNetwork(), path, fileName);
}

void InviwoMainWindow::getScreenGrab(std::string path, std::string fileName) {
    if (path.empty()) path = filesystem::getPath(PathType::Images);

    repaint();
    app_->processEvents();
    QPixmap screenGrab = QGuiApplication::primaryScreen()->grabWindow(this->winId());
    screenGrab.save(QString::fromStdString(path + "/" + fileName), "png");
}

void InviwoMainWindow::addActions() {
    auto menu = menuBar();

    auto fileMenuItem = menu->addMenu(tr("&File"));
    menu->addMenu(editMenu_);
    auto viewMenuItem = menu->addMenu(tr("&View"));
    auto evalMenuItem = menu->addMenu(tr("&Evaluation"));
    auto helpMenuItem = menu->addMenu(tr("&Help"));

    auto workspaceToolBar = addToolBar("File");
    workspaceToolBar->setObjectName("fileToolBar");
    auto viewModeToolBar = addToolBar("View");
    viewModeToolBar->setObjectName("viewModeToolBar");
    auto evalToolBar = addToolBar("Evaluation");
    evalToolBar->setObjectName("evalToolBar");

    // file menu entries
    {
        auto newAction = new QAction(QIcon(":/icons/new.png"), tr("&New Workspace"), this);
        newAction->setShortcut(QKeySequence::New);
        newAction->setShortcutContext(Qt::WidgetWithChildrenShortcut);
        this->addAction(newAction);
        connect(newAction, &QAction::triggered, this, &InviwoMainWindow::newWorkspace);
        fileMenuItem->addAction(newAction);
        workspaceToolBar->addAction(newAction);
    }

    {
        auto openAction = new QAction(QIcon(":/icons/open.png"), tr("&Open Workspace"), this);
        openAction->setShortcut(QKeySequence::Open);
        openAction->setShortcutContext(Qt::WidgetWithChildrenShortcut);
        this->addAction(openAction);
        connect(openAction, &QAction::triggered, this,
                static_cast<void (InviwoMainWindow::*)()>(&InviwoMainWindow::openWorkspace));
        fileMenuItem->addAction(openAction);
        workspaceToolBar->addAction(openAction);
    }

    {
        auto saveAction = new QAction(QIcon(":/icons/save.png"), tr("&Save Workspace"), this);
        saveAction->setShortcut(QKeySequence::Save);
        saveAction->setShortcutContext(Qt::WidgetWithChildrenShortcut);
        this->addAction(saveAction);
        connect(saveAction, &QAction::triggered, this,
                static_cast<void (InviwoMainWindow::*)()>(&InviwoMainWindow::saveWorkspace));
        fileMenuItem->addAction(saveAction);
        workspaceToolBar->addAction(saveAction);
    }

    {
        auto saveAsAction =
            new QAction(QIcon(":/icons/saveas.png"), tr("&Save Workspace As"), this);
        saveAsAction->setShortcut(QKeySequence::SaveAs);
        saveAsAction->setShortcutContext(Qt::WidgetWithChildrenShortcut);
        this->addAction(saveAsAction);
        connect(saveAsAction, &QAction::triggered, this, &InviwoMainWindow::saveWorkspaceAs);
        fileMenuItem->addAction(saveAsAction);
        workspaceToolBar->addAction(saveAsAction);
    }

    {
        auto workspaceActionSaveAsCopy =
            new QAction(QIcon(":/icons/saveas.png"), tr("&Save Workspace As Copy"), this);
        connect(workspaceActionSaveAsCopy, &QAction::triggered, this,
                &InviwoMainWindow::saveWorkspaceAsCopy);
        fileMenuItem->addAction(workspaceActionSaveAsCopy);
    }

    {
        auto exportNetworkMenu = fileMenuItem->addMenu("&Export Network");

        auto backgroundVisibleAction = exportNetworkMenu->addAction("Background Visible");
        backgroundVisibleAction->setCheckable(true);
        backgroundVisibleAction->setChecked(true);
        exportNetworkMenu->addSeparator();

        auto exportNetworkImageFunc = [this, backgroundVisibleAction](bool entireScene) {
            return [this, backgroundVisibleAction, entireScene](bool /*state*/) {
                InviwoFileDialog saveFileDialog(this, "Export Network ...", "image");
                saveFileDialog.setFileMode(FileMode::AnyFile);
                saveFileDialog.setAcceptMode(AcceptMode::Save);
                saveFileDialog.setConfirmOverwrite(true);

                saveFileDialog.addSidebarPath(PathType::Workspaces);
                saveFileDialog.addSidebarPath(workspaceFileDir_);

                saveFileDialog.addExtension("png", "PNG");
                saveFileDialog.addExtension("jpg", "JPEG");
                saveFileDialog.addExtension("bmp", "BMP");
                saveFileDialog.addExtension("pdf", "PDF");

                if (saveFileDialog.exec()) {
                    QString path = saveFileDialog.selectedFiles().at(0);
                    networkEditorView_->exportViewToFile(path, entireScene,
                                                         backgroundVisibleAction->isChecked());
                    LogInfo("Exported network to \"" << utilqt::fromQString(path) << "\"");
                }
            };
        };

        connect(exportNetworkMenu->addAction("Entire Network ..."), &QAction::triggered,
                exportNetworkImageFunc(true));

        connect(exportNetworkMenu->addAction("Current View ..."), &QAction::triggered,
                exportNetworkImageFunc(false));
    }

    {
        fileMenuItem->addSeparator();
        auto recentWorkspaceMenu = fileMenuItem->addMenu(tr("&Recent Workspaces"));
        // create placeholders for recent workspaces
        workspaceActionRecent_.resize(maxNumRecentFiles_);
        for (auto& action : workspaceActionRecent_) {
            action = new QAction(this);
            action->setVisible(false);
            recentWorkspaceMenu->addAction(action);
            connect(action, &QAction::triggered, this, [this, action]() {
                if (askToSaveWorkspaceChanges()) openWorkspace(action->data().toString());
            });
        }
        // action for clearing the recent file menu
        clearRecentWorkspaces_ = recentWorkspaceMenu->addAction("Clear Recent Workspace List");
        clearRecentWorkspaces_->setEnabled(false);
        connect(clearRecentWorkspaces_, &QAction::triggered, this, [this]() {
            for (auto elem : workspaceActionRecent_) {
                elem->setVisible(false);
            }
            // save empty list
            saveRecentWorkspaceList(QStringList());
            clearRecentWorkspaces_->setEnabled(false);
        });

        connect(recentWorkspaceMenu, &QMenu::aboutToShow, this, [this]() {
            for (auto elem : workspaceActionRecent_) {
                elem->setVisible(false);
            }

            QStringList recentFiles{getRecentWorkspaceList()};
            for (int i = 0; i < recentFiles.size(); ++i) {
                if (!recentFiles[i].isEmpty()) {
                    const bool exists = QFileInfo(recentFiles[i]).exists();
                    const auto menuEntry = QString("&%1 %2%3")
                                               .arg(i + 1)
                                               .arg(recentFiles[i])
                                               .arg(exists ? "" : " (missing)");
                    workspaceActionRecent_[i]->setVisible(true);
                    workspaceActionRecent_[i]->setText(menuEntry);
                    workspaceActionRecent_[i]->setEnabled(exists);
                    workspaceActionRecent_[i]->setData(recentFiles[i]);
                }
            }
            clearRecentWorkspaces_->setEnabled(!recentFiles.isEmpty());
        });
    }

    // create list of all example workspaces
    exampleMenu_ = fileMenuItem->addMenu(tr("&Example Workspaces"));
    connect(exampleMenu_, &QMenu::aboutToShow, this, [this]() {
        exampleMenu_->clear();
        for (const auto& module : app_->getModules()) {
            auto moduleWorkspacePath = module->getPath(ModulePath::Workspaces);
            if (!filesystem::directoryExists(moduleWorkspacePath)) continue;
            auto menu = util::make_unique<QMenu>(QString::fromStdString(module->getIdentifier()));
            for (auto item : filesystem::getDirectoryContents(moduleWorkspacePath)) {
                // only accept inviwo workspace files
                if (filesystem::getFileExtension(item) != "inv") continue;
                auto action = menu->addAction(QString::fromStdString(item));
                auto path = QString::fromStdString(moduleWorkspacePath + "/" + item);
                connect(action, &QAction::triggered, this, [this, path]() {
                    if (askToSaveWorkspaceChanges()) openWorkspace(path, true);
                });
            }
            if (!menu->isEmpty()) exampleMenu_->addMenu(menu.release());
        }
        if (exampleMenu_->isEmpty()) {
            exampleMenu_->addAction("No example workspaces found")->setEnabled(false);
        }
    });

    // create list of all test workspaces
    testMenu_ = fileMenuItem->addMenu(tr("&Test Workspaces"));
    connect(testMenu_, &QMenu::aboutToShow, this, [this]() {
        testMenu_->clear();
        for (const auto& module : app_->getModules()) {
            auto moduleTestPath = module->getPath(ModulePath::RegressionTests);
            if (!filesystem::directoryExists(moduleTestPath)) continue;
            auto menu = util::make_unique<QMenu>(QString::fromStdString(module->getIdentifier()));
            for (auto test : filesystem::getDirectoryContents(moduleTestPath,
                                                              filesystem::ListMode::Directories)) {
                std::string testdir = moduleTestPath + "/" + test;
                if (!filesystem::directoryExists(testdir)) continue;
                for (auto item : filesystem::getDirectoryContents(testdir)) {
                    // only accept inviwo workspace files
                    if (filesystem::getFileExtension(item) != "inv") continue;
                    auto action = menu->addAction(QString::fromStdString(item));
                    auto path = QString::fromStdString(testdir + "/" + item);
                    connect(action, &QAction::triggered, this, [this, path]() {
                        if (askToSaveWorkspaceChanges()) openWorkspace(path);
                    });
                }
            }
            if (!menu->isEmpty()) testMenu_->addMenu(menu.release());
        }
        if (testMenu_->isEmpty()) {
            testMenu_->addAction("No test workspaces found")->setEnabled(false);
        }
    });

    if (app_->getModuleManager().isRuntimeModuleReloadingEnabled()) {
        fileMenuItem->addSeparator();
        auto reloadAction = new QAction(tr("&Reload modules"), this);
        connect(reloadAction, &QAction::triggered, this,
                [&]() { app_->getModuleManager().reloadModules(); });
        fileMenuItem->addAction(reloadAction);
    }

    {
        fileMenuItem->addSeparator();
        auto exitAction = new QAction(QIcon(":/icons/button_cancel.png"), tr("&Exit"), this);
        exitAction->setShortcut(QKeySequence::Close);
        connect(exitAction, &QAction::triggered, this, &InviwoMainWindow::close);
        fileMenuItem->addAction(exitAction);
    }

    // Edit
    {
        auto front = editMenu_->actions().front();
        editMenu_->insertAction(front, undoManager_.getUndoAction());
        editMenu_->insertAction(front, undoManager_.getRedoAction());
        editMenu_->insertSeparator(front);

        // here will the cut/copy/paste/del/select already in the menu be.

        editMenu_->addSeparator();
        auto findAction = editMenu_->addAction(tr("&Find Processor"));
        findAction->setShortcut(QKeySequence::Find);
        connect(findAction, &QAction::triggered, this,
                [this]() { processorTreeWidget_->focusSearch(); });

        auto searchNetwork = editMenu_->addAction(tr("&Search Network"));
        searchNetwork->setShortcut(Qt::ShiftModifier + Qt::ControlModifier + Qt::Key_F);
        connect(searchNetwork, &QAction::triggered, [this]() {
            networkSearch_->setVisible(true);
            networkSearch_->setFocus();
        });

        auto addProcessorAction = editMenu_->addAction(tr("&Add Processor"));
        addProcessorAction->setShortcut(Qt::ControlModifier + Qt::Key_D);
        connect(addProcessorAction, &QAction::triggered, this,
                [this]() { processorTreeWidget_->addSelectedProcessor(); });

        editMenu_->addSeparator();

        editMenu_->addAction(consoleWidget_->getClearAction());
    }

    // View
    {
        // dock widget visibility menu entries
        viewMenuItem->addAction(settingsWidget_->toggleViewAction());
        processorTreeWidget_->toggleViewAction()->setText(tr("&Processor List"));
        viewMenuItem->addAction(processorTreeWidget_->toggleViewAction());
        propertyListWidget_->toggleViewAction()->setText(tr("&Property List"));
        viewMenuItem->addAction(propertyListWidget_->toggleViewAction());
        consoleWidget_->toggleViewAction()->setText(tr("&Output Console"));
        viewMenuItem->addAction(consoleWidget_->toggleViewAction());
        helpWidget_->toggleViewAction()->setText(tr("&Help"));
        viewMenuItem->addAction(helpWidget_->toggleViewAction());
    }

    {
        // application/developer mode menu entries
        QIcon visibilityModeIcon;
        visibilityModeIcon.addFile(":/icons/view-developer.png", QSize(), QIcon::Normal,
                                   QIcon::Off);
        visibilityModeIcon.addFile(":/icons/view-application.png", QSize(), QIcon::Normal,
                                   QIcon::On);
        visibilityModeAction_ = new QAction(visibilityModeIcon, tr("&Application Mode"), this);
        visibilityModeAction_->setCheckable(true);
        visibilityModeAction_->setChecked(false);

        viewMenuItem->addAction(visibilityModeAction_);
        viewModeToolBar->addAction(visibilityModeAction_);

        appUsageModeProp_ = &app_->getSettingsByType<SystemSettings>()->applicationUsageMode_;

        appUsageModeProp_->onChange([&]() { visibilityModeChangedInSettings(); });

        connect(visibilityModeAction_, &QAction::triggered, [&](bool appView) {
            auto selectedIdx = appUsageModeProp_->getSelectedValue();
            if (appView) {
                if (selectedIdx != UsageMode::Application)
                    appUsageModeProp_->setSelectedValue(UsageMode::Application);
            } else {
                if (selectedIdx != UsageMode::Development)
                    appUsageModeProp_->setSelectedValue(UsageMode::Development);
            }
        });

        visibilityModeChangedInSettings();
    }

    // Evaluation
    {
        QIcon enableDisableIcon;
        enableDisableIcon.addFile(":/icons/button_ok.png", QSize(), QIcon::Active, QIcon::Off);
        enableDisableIcon.addFile(":/icons/button_cancel.png", QSize(), QIcon::Active, QIcon::On);
        auto lockNetworkAction = new QAction(enableDisableIcon, tr("&Lock Network"), this);
        lockNetworkAction->setCheckable(true);
        lockNetworkAction->setChecked(false);
        lockNetworkAction->setToolTip("Enable/Disable Network Evaluation");

        lockNetworkAction->setShortcut(Qt::ControlModifier + Qt::Key_L);
        evalMenuItem->addAction(lockNetworkAction);
        evalToolBar->addAction(lockNetworkAction);
        connect(lockNetworkAction, &QAction::triggered, [lockNetworkAction, this]() {
            if (lockNetworkAction->isChecked()) {
                app_->getProcessorNetwork()->lock();
            } else {
                app_->getProcessorNetwork()->unlock();
            }
        });
    }

#if IVW_PROFILING
    {
        auto resetTimeMeasurementsAction =
            new QAction(QIcon(":/icons/stopwatch.png"), tr("Reset All Time Measurements"), this);
        resetTimeMeasurementsAction->setCheckable(false);

        connect(resetTimeMeasurementsAction, &QAction::triggered,
                [&]() { networkEditor_->resetAllTimeMeasurements(); });

        evalToolBar->addAction(resetTimeMeasurementsAction);
        evalMenuItem->addAction(resetTimeMeasurementsAction);
    }
#endif

    // Help
    {
        helpMenuItem->addAction(helpWidget_->toggleViewAction());

        auto aboutBoxAction = new QAction(QIcon(":/icons/about.png"), tr("&About"), this);
        connect(aboutBoxAction, &QAction::triggered, this, &InviwoMainWindow::showAboutBox);
        helpMenuItem->addAction(aboutBoxAction);
    }
}

void InviwoMainWindow::updateWindowTitle() {
    QString windowTitle = QString("Inviwo - Interactive Visualization Workshop - ");
    windowTitle.append(currentWorkspaceFileName_);

    if (getNetworkEditor()->isModified()) windowTitle.append("*");

    if (visibilityModeAction_->isChecked()) {
        windowTitle.append(" (Application mode)");
    } else {
        windowTitle.append(" (Developer mode)");
    }

    setWindowTitle(windowTitle);
}

void InviwoMainWindow::addToRecentWorkspaces(QString workspaceFileName) {
    QStringList recentFiles{getRecentWorkspaceList()};

    recentFiles.removeAll(workspaceFileName);
    recentFiles.prepend(workspaceFileName);

    if (recentFiles.size() > static_cast<int>(maxNumRecentFiles_)) recentFiles.removeLast();
    saveRecentWorkspaceList(recentFiles);
}

QStringList InviwoMainWindow::getRecentWorkspaceList() const {
    QSettings settings("Inviwo", "Inviwo");
    settings.beginGroup("mainwindow");
    QStringList list{settings.value("recentFileList").toStringList()};
    settings.endGroup();

    return list;
}

void InviwoMainWindow::saveRecentWorkspaceList(const QStringList& list) {
    QSettings settings("Inviwo", "Inviwo");
    settings.beginGroup("mainwindow");
    settings.setValue("recentFileList", list);
    settings.endGroup();
}

void InviwoMainWindow::setCurrentWorkspace(QString workspaceFileName) {
    workspaceFileDir_ = QFileInfo(workspaceFileName).absolutePath();
    currentWorkspaceFileName_ = workspaceFileName;
    updateWindowTitle();
}

std::string InviwoMainWindow::getCurrentWorkspace() {
    return currentWorkspaceFileName_.toLocal8Bit().constData();
}

void InviwoMainWindow::newWorkspace() {
    if (currentWorkspaceFileName_ != "")
        if (!askToSaveWorkspaceChanges()) return;

    app_->getWorkspaceManager()->clear();

    setCurrentWorkspace(rootDir_ + "/workspaces/untitled.inv");
    getNetworkEditor()->setModified(false);
}

void InviwoMainWindow::openWorkspace(QString workspaceFileName) {
    openWorkspace(workspaceFileName, false);
}

void InviwoMainWindow::openWorkspace(QString workspaceFileName, bool exampleWorkspace) {
    std::string fileName{workspaceFileName.toStdString()};
    fileName = filesystem::cleanupPath(fileName);
    workspaceFileName = QString::fromStdString(fileName);

    if (!filesystem::fileExists(fileName)) {
        LogError("Could not find workspace file: " << fileName);
        return;
    }

    {
        NetworkLock lock(app_->getProcessorNetwork());
        app_->getWorkspaceManager()->clear();
        try {
            app_->getWorkspaceManager()->load(fileName, [&](ExceptionContext ec) {
                try {
                    throw;
                } catch (const IgnoreException& e) {
                    util::log(
                        e.getContext(),
                        "Incomplete network loading " + fileName + " due to " + e.getMessage(),
                        LogLevel::Error);
                }
            });

            if (exampleWorkspace) {
                setCurrentWorkspace(rootDir_ + "/workspaces/untitled.inv");
            } else {
                setCurrentWorkspace(workspaceFileName);
                addToRecentWorkspaces(workspaceFileName);
            }
        } catch (const Exception& e) {
            util::log(e.getContext(),
                      "Unable to load network " + fileName + " due to " + e.getMessage(),
                      LogLevel::Error);
            app_->getWorkspaceManager()->clear();
            setCurrentWorkspace(rootDir_ + "/workspaces/untitled.inv");
        }
        app_->processEvents();  // make sure the gui is ready before we unlock.
    }
    saveWindowState();
    getNetworkEditor()->setModified(false);
}

void InviwoMainWindow::openLastWorkspace(std::string workspace) {
    workspace = filesystem::cleanupPath(workspace);
    if (!workspace.empty()) {
        openWorkspace(QString::fromStdString(workspace));
    } else if (!workspaceOnLastSuccessfulExit_.isEmpty()) {
        openWorkspace(workspaceOnLastSuccessfulExit_);
    } else {
        newWorkspace();
    }
}

void InviwoMainWindow::openWorkspace() {
    if (askToSaveWorkspaceChanges()) {
        InviwoFileDialog openFileDialog(this, "Open Workspace ...", "workspace");
        openFileDialog.addSidebarPath(PathType::Workspaces);
        openFileDialog.addSidebarPath(workspaceFileDir_);
        openFileDialog.addExtension("inv", "Inviwo File");
        openFileDialog.setFileMode(FileMode::AnyFile);

        if (openFileDialog.exec()) {
            QString path = openFileDialog.selectedFiles().at(0);
            openWorkspace(path);
        }
    }
}

void InviwoMainWindow::saveWorkspace(QString workspaceFileName) {
    std::string fileName{workspaceFileName.toStdString()};
    fileName = filesystem::cleanupPath(fileName);

    try {
        app_->getWorkspaceManager()->save(fileName, [&](ExceptionContext ec) {
            try {
                throw;
            } catch (const IgnoreException& e) {
                util::log(e.getContext(),
                          "Incomplete network save " + fileName + " due to " + e.getMessage(),
                          LogLevel::Error);
            }
        });
        getNetworkEditor()->setModified(false);
        updateWindowTitle();
        LogInfo("Workspace saved to: " << fileName);
    } catch (const Exception& e) {
        util::log(e.getContext(),
                  "Unable to save network " + fileName + " due to " + e.getMessage(),
                  LogLevel::Error);
    }
}

void InviwoMainWindow::saveWorkspace() {
    if (currentWorkspaceFileName_.contains("untitled.inv"))
        saveWorkspaceAs();
    else {
        saveWorkspace(currentWorkspaceFileName_);
    }
}

void InviwoMainWindow::saveWorkspaceAs() {
    InviwoFileDialog saveFileDialog(this, "Save Workspace ...", "workspace");
    saveFileDialog.setFileMode(FileMode::AnyFile);
    saveFileDialog.setAcceptMode(AcceptMode::Save);
    saveFileDialog.setConfirmOverwrite(true);

    saveFileDialog.addSidebarPath(PathType::Workspaces);
    saveFileDialog.addSidebarPath(workspaceFileDir_);

    saveFileDialog.addExtension("inv", "Inviwo File");

    if (saveFileDialog.exec()) {
        QString path = saveFileDialog.selectedFiles().at(0);
        if (!path.endsWith(".inv")) path.append(".inv");

        saveWorkspace(path);
        setCurrentWorkspace(path);
        addToRecentWorkspaces(path);
    }
    saveWindowState();
}

void InviwoMainWindow::saveWorkspaceAsCopy() {
    InviwoFileDialog saveFileDialog(this, "Save Workspace ...", "workspace");
    saveFileDialog.setFileMode(FileMode::AnyFile);
    saveFileDialog.setAcceptMode(AcceptMode::Save);
    saveFileDialog.setConfirmOverwrite(true);

    saveFileDialog.addSidebarPath(PathType::Workspaces);
    saveFileDialog.addSidebarPath(workspaceFileDir_);

    saveFileDialog.addExtension("inv", "Inviwo File");

    if (saveFileDialog.exec()) {
        QString path = saveFileDialog.selectedFiles().at(0);

        if (!path.endsWith(".inv")) path.append(".inv");

        saveWorkspace(path);
        addToRecentWorkspaces(path);
    }
    saveWindowState();
}

void InviwoMainWindow::onModifiedStatusChanged(const bool& /*newStatus*/) { updateWindowTitle(); }

void InviwoMainWindow::showAboutBox() {
    auto caps = app_->getModuleByType<InviwoCore>()->getCapabilities();
    auto syscap = getTypeFromVector<SystemCapabilities>(caps);

    const int buildYear = (syscap ? syscap->getBuildTimeYear() : 0);

    std::stringstream aboutText;
    aboutText << "<html><head>\n"
              << "<style>\n"
              << "table { margin-top:0px;margin-bottom:0px; }\n"
              << "table > tr > td { "
              << "padding-left:0px; padding-right:0px;padding-top:0px; \n"
              << "padding-bottom:0px;"
              << "}\n"
              << "</style>\n"
              << "<head/>\n"
              << "<body>\n"

              << "<b>Inviwo v" << IVW_VERSION << "</b><br>\n"
              << "Interactive Visualization Workshop<br>\n"
              << "&copy; 2012-" << buildYear << " The Inviwo Foundation<br>\n"
              << "<a href='http://www.inviwo.org/' style='color: #AAAAAA;'>"
              << "http://www.inviwo.org/</a>\n"
              << "<p>Inviwo is a rapid prototyping environment for interactive "
              << "visualizations.<br>It is licensed under the Simplified BSD license.</p>\n"

              << "<p><b>Core Team:</b><br>\n"
              << "Peter Steneteg, Erik Sund&eacute;n, Daniel J&ouml;nsson, Martin Falk, "
              << "Rickard Englund, Sathish Kottravel, Timo Ropinski</p>\n"

              << "<p><b>Former Developers:</b><br>\n"
              << "Alexander Johansson, Andreas Valter, Johan Nor&eacute;n, Emanuel Winblad, "
              << "Hans-Christian Helltegen, Viktor Axelsson</p>\n";
    if (syscap) {
        aboutText << "<p><b>Build Date: </b>\n" << syscap->getBuildDateString() << "</p>\n";
        aboutText << "<p><b>Repos:</b>\n"
                  << "<table border='0' cellspacing='0' cellpadding='0' style='margin: 0px;'>\n";
        for (size_t i = 0; i < syscap->getGitNumberOfHashes(); ++i) {
            auto item = syscap->getGitHash(i);
            aboutText << "<tr><td style='padding-right:8px;'>" << item.first
                      << "</td><td style='padding-right:8px;'>" << item.second << "</td></tr>\n";
        }
        aboutText << "</table></p>\n";
    }
    const auto& mfos = app_->getModuleManager().getModuleFactoryObjects();
    auto names = util::transform(
        mfos, [](const std::unique_ptr<InviwoModuleFactoryObject>& mfo) { return mfo->name; });
    std::sort(names.begin(), names.end());
    aboutText << "<p><b>Modules:</b><br>\n" << joinString(names, ", ") << "</p>\n";
    aboutText << "</body></html>";

    aboutText << "<p><b>Qt:</b> Version " << QT_VERSION_STR << ".<br/>";

    auto str = aboutText.str();

    // Use custom dialog since in QMessageBox::about you can not select text
    auto about =
        new QMessageBox(QMessageBox::NoIcon, QString::fromStdString("Inviwo v" + IVW_VERSION),
                        QString::fromStdString(aboutText.str()), QMessageBox::Ok, this);
    auto icon = windowIcon();
    about->setIconPixmap(icon.pixmap(256));
    about->setTextInteractionFlags(Qt::TextBrowserInteraction);
    about->exec();
}

void InviwoMainWindow::visibilityModeChangedInSettings() {
    if (appUsageModeProp_) {
        auto network = getInviwoApplication()->getProcessorNetwork();
        switch (appUsageModeProp_->getSelectedValue()) {
            case UsageMode::Development: {
                for (auto& p : network->getProcessors()) {
                    auto md =
                        p->getMetaData<ProcessorMetaData>(ProcessorMetaData::CLASS_IDENTIFIER);
                    if (md->isSelected()) {
                        propertyListWidget_->addProcessorProperties(p);
                    } else {
                        propertyListWidget_->removeProcessorProperties(p);
                    }
                }

                if (visibilityModeAction_->isChecked()) {
                    visibilityModeAction_->setChecked(false);
                }
                networkEditorView_->hideNetwork(false);
                break;
            }
            case UsageMode::Application: {
                if (!visibilityModeAction_->isChecked()) {
                    visibilityModeAction_->setChecked(true);
                }
                networkEditorView_->hideNetwork(true);

                for (auto& p : network->getProcessors()) {
                    propertyListWidget_->addProcessorProperties(p);
                }
                break;
            }
        }

        updateWindowTitle();
    }
}

NetworkEditor* InviwoMainWindow::getNetworkEditor() const { return networkEditor_.get(); }

void InviwoMainWindow::exitInviwo(bool saveIfModified) {
    if (!saveIfModified) getNetworkEditor()->setModified(false);
    QMainWindow::close();
    app_->closeInviwoApplication();
}

void InviwoMainWindow::saveWindowState() {
    QSettings settings("Inviwo", "Inviwo");
    settings.beginGroup("mainwindow");
    settings.setValue("geometry", saveGeometry());
    settings.setValue("state", saveState());
    settings.setValue("maximized", isMaximized());
    // settings.setValue("recentFileList", recentFileList_);

    // save sticky flags for main dock widgets
    settings.beginGroup("dialogs");
    settings.setValue("settingswidgetSticky", settingsWidget_->isSticky());
    settings.setValue("processorwidgetSticky", processorTreeWidget_->isSticky());
    settings.setValue("propertywidgetSticky", propertyListWidget_->isSticky());
    settings.setValue("consolewidgetSticky", consoleWidget_->isSticky());
    settings.setValue("helpwidgetSticky", helpWidget_->isSticky());
    settings.endGroup();  // dialogs

    settings.endGroup();  // mainwindow
}
void InviwoMainWindow::loadWindowState() {
    // load settings and restore window state
    QSettings settings("Inviwo", "Inviwo");
    settings.beginGroup("mainwindow");
    restoreGeometry(settings.value("geometry", saveGeometry()).toByteArray());
    restoreState(settings.value("state", saveState()).toByteArray());
    maximized_ = settings.value("maximized", false).toBool();

    // restore sticky flags for main dock widgets
    settings.beginGroup("dialogs");
    settingsWidget_->setSticky(settings.value("settingswidgetSticky", true).toBool());
    processorTreeWidget_->setSticky(settings.value("processorwidgetSticky", true).toBool());
    propertyListWidget_->setSticky(settings.value("propertywidgetSticky", true).toBool());
    consoleWidget_->setSticky(settings.value("consolewidgetSticky", true).toBool());
    helpWidget_->setSticky(settings.value("helpwidgetSticky", true).toBool());
}

void InviwoMainWindow::closeEvent(QCloseEvent* event) {
    if (!askToSaveWorkspaceChanges()) {
        event->ignore();
        return;
    }

    app_->getWorkspaceManager()->clear();

    saveWindowState();

    QSettings settings("Inviwo", "Inviwo");
    settings.beginGroup("mainwindow");
    if (!currentWorkspaceFileName_.contains("untitled.inv")) {
        settings.setValue("workspaceOnLastSuccessfulExit", currentWorkspaceFileName_);
    } else {
        settings.setValue("workspaceOnLastSuccessfulExit", "");
    }
    settings.endGroup();

    // pass a close event to all children to let the same state etc.
    for (auto& child : children()) {
        QCloseEvent closeEvent;
        QApplication::sendEvent(child, &closeEvent);
    }

    QMainWindow::closeEvent(event);
}

bool InviwoMainWindow::askToSaveWorkspaceChanges() {
    bool continueOperation = true;

    if (getNetworkEditor()->isModified() && !app_->getProcessorNetwork()->isEmpty()) {
        QMessageBox msgBox(this);
        msgBox.setText("Workspace Modified");
        msgBox.setInformativeText("Do you want to save your changes?");
        msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);
        msgBox.setDefaultButton(QMessageBox::Yes);
        int answer = msgBox.exec();

        switch (answer) {
            case QMessageBox::Yes:
                saveWorkspace();
                break;

            case QMessageBox::No:
                break;

            case QMessageBox::Cancel:
                continueOperation = false;
                break;
        }
    }

    return continueOperation;
}

SettingsWidget* InviwoMainWindow::getSettingsWidget() const { return settingsWidget_; }

ProcessorTreeWidget* InviwoMainWindow::getProcessorTreeWidget() const {
    return processorTreeWidget_;
}

PropertyListWidget* InviwoMainWindow::getPropertyListWidget() const { return propertyListWidget_; }

ConsoleWidget* InviwoMainWindow::getConsoleWidget() const { return consoleWidget_.get(); }

HelpWidget* InviwoMainWindow::getHelpWidget() const { return helpWidget_; }

InviwoApplication* InviwoMainWindow::getInviwoApplication() const { return app_; }
InviwoApplicationQt* InviwoMainWindow::getInviwoApplicationQt() const { return app_; }

InviwoEditMenu* InviwoMainWindow::getInviwoEditMenu() const { return editMenu_; }

}  // namespace inviwo
