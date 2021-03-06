#include "mainwindow.h"
#include "newmodeldlg.h"
#include <QtWidgets>
#include <QMessageBox>
#include <QDebug>
#include <QtCharts/QChart>
#include <QDockWidget>
#include <QValueAxis>
#include <QLogValueAxis>
#include <QCategoryAxis>
#include <QColor>
#include <QMenu>
#include <QMenuBar>
#include <QDockWidget>
#include <QDialogButtonBox>
#include <QFileDialog>
#include <QUrl>
#include <QDebug>

#include "account.h"
#include "optionsdialog.h"
#include "removemodeldlg.h"
#include "version.h"
#include "saveprofiledialog.h"
#include "statsdialog.h"
#include "removeprofiledialog.h"

MainWindow::MainWindow()
{
    _current_model = nullptr;
    first_time_shown = true;

    statsDialog = new StatsDialog(this);
    statsDialog->setWindowFlags(Qt::Tool);

    // Set up list of property names.

    // Qt Limitation: can only initialise a QMap from an initializer list if
    // is declared as static. And of course this is no help when it has to
    // point to instance members.

    property_map[tr("Current period")] = Model::Property::current_period;
    property_map[tr("Population size")] = Model::Property::pop_size;
    property_map[tr("Govt exp excl benefits")] = Model::Property::gov_exp;
    property_map[tr("Govt exp incl benefits")] = Model::Property::gov_exp_plus;
    property_map[tr("Benefits paid")] = Model::Property::bens_paid;
    property_map[tr("Government receipts")] = Model::Property::gov_recpts;
    property_map[tr("Deficit (absolute)")] = Model::Property::deficit;
    property_map[tr("Deficit as % GDP")] = Model::Property::deficit_pc;
    property_map[tr("National Debt")] = Model::Property::gov_bal;
    property_map[tr("Number of businesses")] = Model::Property::num_firms;
    property_map[tr("Number employed")] = Model::Property::num_emps;
    property_map[tr("Number of govt employees")] = Model::Property::num_gov_emps;
    property_map[tr("Percent employed")] = Model::Property::pc_emps;
    property_map[tr("Number unemployed")] = Model::Property::num_unemps;
    property_map[tr("Percent unemployed")] = Model::Property::pc_unemps;
    property_map[tr("Percent active")] = Model::Property::pc_active;
    property_map[tr("Number of new hires")] = Model::Property::num_hired;
    property_map[tr("Number of new fires")] = Model::Property::num_fired;
    property_map[tr("Businesses balance")] = Model::Property::prod_bal;
    property_map[tr("Wages paid")] = Model::Property::wages;
    property_map[tr("Consumption")] = Model::Property::consumption;
    property_map[tr("Bonuses paid")] = Model::Property::bonuses;
    property_map[tr("Pre-tax deductions")] = Model::Property::dedns;
    property_map[tr("Income tax paid")] = Model::Property::inc_tax;
    property_map[tr("Sales tax paid")] = Model::Property::sales_tax;
    property_map[tr("Households balance")] = Model::Property::dom_bal;
    property_map[tr("Bank loans")] = Model::Property::amount_owed;
    property_map[tr("Average business size")] = Model::Property::bus_size;
    property_map[tr("100 reference line")] = Model::Property::hundred;
    property_map[tr("Procurement expenditure")] = Model::Property::procurement;
    property_map[tr("Productivity")] = Model::Property::productivity;
    property_map[tr("Productivity (relative)")] = Model::Property::rel_productivity;
    property_map[tr("Govt direct support")] = Model::Property::unbudgeted;
    property_map[tr("Investment")] = Model::Property::investment;
    property_map[tr("GDP")] = Model::Property::gdp;
    property_map[tr("Profit")] = Model::Property::profit;
    property_map[tr("Zero reference line")] = Model::Property::zero;

    // If non-zero, points to currently selected listwidget item
    selectedModelItem = 0;

    // Make sure preferences exist
    QSettings settings;
    qDebug() << "Settings are in" << settings.fileName();

    if (!settings.contains("iterations"))
    {
        settings.setValue("iterations", 100);
        settings.setValue("start-period", 0);
        settings.setValue("startups", 10);
        settings.setValue("nominal-population", 1000);
        settings.setValue("unit-wage", 100);
        settings.setValue("government-employees", 200); // approx tot pop / 5
        //settings.setValue("sample-size", 10); // for moving averages -- adjust as necessary
    }

    current_profile = settings.value("current-profile", "").toString();

    createChart();
    createActions();
    createMenus();
    createStatusBar();
    createDockWindows();

    setWindowTitle(tr("Obson"));
    setWindowIcon(QIcon(":/obson.icns"));
    setUnifiedTitleAndToolBarOnMac(true);
    setMinimumSize(1024, 768);
    resize(1280, 800);

    // The left margin is to prevent the control being right against the side
    // of the window. May not really be a good idea. The border and padding
    // definitely improve things though.
    // setStyleSheet("QListWidget{margin-left: 2px; border: 0px; padding:12px;}");
}

MainWindow::~MainWindow()
{
    // Nothing to do...
}

void MainWindow::show()
{
    QMainWindow::show();
    QApplication::processEvents();

    emit windowShown();

    if (first_time_shown == true)
    {
       emit windowLoaded();
       first_time_shown = false;
    }
}

void MainWindow::showWiki()
{
    QDesktopServices::openUrl(QUrl("https://github.com/Obson/MicroSim-GUI/wiki", QUrl::StrictMode));
}

void MainWindow::createChart()
{
    chart = new QChart();
    chart->legend()->setAlignment(Qt::AlignBottom);
    chartView = new QChartView(chart);
    chartView->setRenderHint(QPainter::Antialiasing);
    chartView->setMinimumWidth(1000);

    colours << Qt::red << Qt::blue << Qt::darkRed << Qt::darkGreen
            << Qt::darkBlue << Qt::darkMagenta << Qt::darkYellow
            << Qt::darkCyan;

    setCentralWidget(chartView);
}

void MainWindow::createActions()
{
    // Save as CVS file
    const QIcon csvIcon = QIcon(":/chart-2.icns");
    saveCSVAction = new QAction(csvIcon, tr("&Save as CSV file..."), this);
    saveCSVAction->setStatusTip(tr("Save current chart as a CSV file"));
    saveCSVAction->setDisabled(!isModelSelected());
    connect(saveCSVAction, &QAction::triggered, this, &MainWindow::saveCSV);

    // Save profile
    const QIcon profileIcon = QIcon(":/chart-1.icns");
    saveProfileAction = new QAction(profileIcon, tr("&Save chart profile..."), this);
    saveProfileAction->setStatusTip(tr("Save chart settings as a profile"));
    connect(saveProfileAction, &QAction::triggered, this, &MainWindow::createProfile);

    // Remove profile
    const QIcon removeProfileIcon = QIcon(":/delete-chart.icns");
    removeProfileAction = new QAction(removeProfileIcon, tr("Remove chart profile..."), this);
    removeProfileAction->setStatusTip(tr("Remove profile"));
    connect(removeProfileAction, &QAction::triggered, this, &MainWindow::removeProfile /* Change this */);

    // Edit model parameters
    const QIcon setupIcon = QIcon(":/settings.icns");
    changeAction = new QAction(setupIcon, tr("Edit &parameters..."));
    changeAction->setDisabled(!isModelSelected());
    changeAction->setStatusTip(tr("Modify the parameters for this model"));
    connect(changeAction, &QAction::triggered, this, &MainWindow::editParameters);

    // Reassign colours
    const QIcon coloursIcon = QIcon(":/chart.icns");
    coloursAction = new QAction(coloursIcon, tr("Reset &colours"));
    coloursAction->setStatusTip(tr("Reset chart colours"));
    connect(coloursAction, &QAction::triggered, this, &MainWindow::reassignColours);

    // New model
    const QIcon newIcon = QIcon(":/add-model.icns");
    newAction = new QAction(newIcon, tr("&New model..."), this);
    newAction->setStatusTip(tr("Create a new model"));
    connect(newAction, &QAction::triggered, this, &MainWindow::createNewModel);

    // Remove models
    const QIcon removeIcon = QIcon(":/delete-model.icns");
    removeAction = new QAction(removeIcon, tr("&Remove models..."));
    removeAction->setStatusTip(tr("Remove a model or models"));
    connect(removeAction, &QAction::triggered, this, &MainWindow::remove);

    // Edit model description
    const QIcon notesIcon = QIcon(":/notes.icns");
    notesAction = new QAction(notesIcon, tr("&Edit description..."));
    notesAction->setStatusTip(tr("Edit the description for this model"));
    connect(notesAction, &QAction::triggered, this, &MainWindow::editModelDescription);

    setOptionsAction = new QAction(tr("&Preferences"));
    setOptionsAction->setStatusTip(tr("Modify the global options"));
    connect(setOptionsAction, &QAction::triggered, this, &MainWindow::setOptions);

    // About Obson
    aboutAction = new QAction(tr("&About"), this);
    aboutAction->setStatusTip(tr("Show version information"));
    connect(aboutAction, &QAction::triggered, this, &MainWindow::about);

    // About Qt
    aboutQtAction = new QAction(tr("A&bout Qt"), this);
    connect(aboutQtAction, &QAction::triggered, this, &MainWindow::aboutQt);

    // Statistics
    const QIcon statsIcon = QIcon(":/help.icns");
    statsAction = new QAction(statsIcon, tr("Statistics"), this);
    statsAction->setStatusTip(tr("Show statistics"));
    statsAction->setEnabled(false);
    connect(statsAction, &QAction::triggered, this, &MainWindow::showStatistics);

    // Help (documentation)
    const QIcon helpIcon = QIcon(":/help-2.icns");
    helpAction = new QAction(helpIcon, tr("Open documentation in browser"), this);
    helpAction->setStatusTip(tr("View the Obson documentation in your browser"));
    connect(helpAction, &QAction::triggered, this, &MainWindow::showWiki);

    // Run normally
    const QIcon runIcon = QIcon(":/run.icns");
    runAction = new QAction(runIcon, tr("&Update"), this);
    runAction->setStatusTip(tr("Update chart"));
    connect(runAction, &QAction::triggered, this, &MainWindow::drawChartNormal);

    // Run randomised
    const QIcon randomIcon = QIcon(":/rerun.icns");
    randomAction = new QAction(randomIcon, tr("&Randomise"), this);
    randomAction->setStatusTip(tr("Randomise chart"));
    connect(randomAction, &QAction::triggered, this, &MainWindow::drawChartRandomised);

    // Close
    const QIcon closeIcon = QIcon(":/exit.icns");
    closeAction = new QAction(closeIcon, tr("&Quit..."), this);
    closeAction->setStatusTip(tr("Quit Obson"));
    connect(closeAction, &QAction::triggered, this, &MainWindow::close);

    connect(this, &MainWindow::windowShown, this, &MainWindow::createFirstModel);
    connect(this, &MainWindow::windowLoaded, this, &MainWindow::restoreState);
}

void MainWindow::createMenus()
{
    myMenuBar = new QMenuBar(0);

    applicationMenu = myMenuBar->addMenu(tr("&Obson"));

    fileMenu = myMenuBar->addMenu(tr("&File"));
    fileMenu->addAction(newAction);
    fileMenu->addAction(removeAction);
    fileMenu->addSeparator();
    fileMenu->addAction(saveCSVAction);
    fileMenu->addAction(saveProfileAction);

    editMenu = myMenuBar->addMenu(tr("&Edit"));
    editMenu->addAction(changeAction);
    setOptionsAction->setMenuRole(QAction::ApplicationSpecificRole);
    editMenu->addAction(setOptionsAction);
    editMenu->addAction(notesAction);

    viewMenu = myMenuBar->addMenu(tr("&View"));
    viewMenu->addAction(coloursAction);

    helpMenu = myMenuBar->addMenu(tr("&Help"));
    applicationMenu->addAction(aboutAction);
    helpMenu->addAction(aboutQtAction);
    helpMenu->addAction(helpAction);

    setMenuBar(myMenuBar);

    QToolBar *myToolBar = new QToolBar(this);
    addToolBar(Qt::LeftToolBarArea, myToolBar);

    myToolBar->addAction(newAction);
    myToolBar->addAction(removeAction);
    myToolBar->addAction(changeAction);
    myToolBar->addAction(notesAction);
    myToolBar->addAction(saveCSVAction);
    myToolBar->addAction(saveProfileAction);
    myToolBar->addAction(removeProfileAction);
    myToolBar->addAction(runAction);
    myToolBar->addAction(randomAction);
    myToolBar->addAction(coloursAction);
    myToolBar->addAction(statsAction);
    myToolBar->addAction(helpAction);
    myToolBar->addAction(closeAction);
}

Model *MainWindow::current_model()
{
    return _current_model;
}

void MainWindow::reassignColours()
{
    propertyColours.clear();
    drawChart(true, false);
}

void MainWindow::saveCSV()
{
    qDebug() << "MainWindow::saveCSV():  called";

    Model *model = current_model();
    QString filename = QFileDialog::getSaveFileName(this, tr("Save As"),
                                QDir::homePath() + QDir::separator() + model->name() + ".csv",
                                tr("CSV files (*.csv)"));

    if (filename.isEmpty()) {
        qDebug() << "MainWindow::saveCSV():  no file selected";
        return;
    }

    qDebug() << "MainWindow::saveCSV():  output file =" << filename;

    // Open the selected file for writing
    QFile csv(filename);
    if (!csv.open(QIODevice::WriteOnly | QIODevice::Text)) {
        // TODO: Replace this with a proper messagebox
        QErrorMessage msg;
        msg.showMessage(tr("Cannot open file ") + filename);
        return;
    }

    QTextStream out(&csv);

    // Output headers and set up points lists for series
    bool first = true;
    int iters = model->getIters(), n = 0;
    QList<QPointF>lists[iters];
    for (int j = 0; j < propertyList->count(); j++)
    {
        QListWidgetItem *item;
        item = propertyList->item(j);
        bool selected = item->checkState();
        if (selected)
        {
            if (first) {
                first = false;
                out << "\"Period\"";
            }
            QString series_name = item->text();
            out << ",\"" << series_name << "\"";
            Model::Property prop = property_map[series_name];
            lists[n++] = _current_model->series[prop]->points();
        }
    }

    int start = model->getStartPeriod();
    for (int i = 0; i < iters; i++)
    {
        first = true;
        for (int j = 0; j < n; j++)
        {
            if (first) {
                out << "\n\"" << i + start << "\"";
                first = false;
            }

            // Output the ith value in the series
            out << ",\"" << lists[j].at(i).y() << "\"";
        }
    }
}

void MainWindow::nyi()
{
    QMessageBox msgBox(this);
    msgBox.setText(tr("Not yet implemented!"));
    msgBox.setInformativeText(tr("You have just requested a service that is not yet implemented."));
    msgBox.setIcon(QMessageBox::Warning);
    msgBox.setStandardButtons(QMessageBox::Ok);
    msgBox.setDefaultButton(QMessageBox::Ok);
    msgBox.exec();
}

void MainWindow::errorMessage(QString msg)
{
    QMessageBox msgBox;
    msgBox.setText(tr("Program Error"));
    msgBox.setInformativeText(msg);
    msgBox.setDetailedText(tr("The program will now close down"));
    msgBox.setIcon(QMessageBox::Critical);
    msgBox.setStandardButtons(QMessageBox::Ok);
    msgBox.setDefaultButton(QMessageBox::Ok);
    msgBox.exec();
    exit(999);
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    qDebug() << "MainWindow::closeEvent() called";

    // Save current config to settings
    QSettings settings;
    settings.beginGroup("State");

    for (int i = 0; i < propertyList->count(); i++)
    {
        QListWidgetItem *item;
        item = propertyList->item(i);
        item->data(Qt::UserRole).clear();
        QString text = item->text();
        bool selected = item->checkState();
        settings.setValue(text, selected ? true : false);
    }

    settings.endGroup();
    if (profile_changed)
    {
        // Ask whether to save the new settings as the current profile
        SaveProfileDialog dlg(this);
        if (QDialog::Accepted == dlg.exec())
        {
            saveSettingsAsProfile(dlg.profileName());
        }
    }
    settings.setValue("current-profile", current_profile);
    event->accept();
}

void MainWindow::restoreState()
{
    QSettings settings;
    settings.beginGroup("State");
     for (int i = 0; i < propertyList->count(); i++)
    {
        QListWidgetItem *item;
        item = propertyList->item(i);
        QString text = item->text();
        bool checked = settings.value(text, false).toBool();
        item->setCheckState(checked ? Qt::Checked : Qt::Unchecked);
    }
    settings.endGroup();

    QString s = settings.value("current_model", "").toString();
    if (s.isEmpty())
    {
        errorMessage("Cannot find model " + s);
    }
    else
    {
        QList<QListWidgetItem*> items = modelList->findItems(s, Qt::MatchExactly);
        if (items.count() == 1) {
            items[0]->setSelected(true);
            changeModel(items[0]);
        }
    }
}

#include "model.h"

void MainWindow::saveSettingsAsProfile(QString name)
{
    if (name.isEmpty()) {
        if (current_profile.isEmpty()) {
            // TODO: There should be an error message here, unless this case is
            // filtered out -- check.
            return;
        } else {
            name = current_profile;
        }
    }

    QSettings settings;
    settings.beginGroup("Profiles");

    if (settings.childGroups().contains(name))
    {
        qDebug() << "Updating profile" << name;
    }
    else
    {
        qDebug() << "Creating new profile" << name;
        profileList->addItem(name);
        selectProfile(name);
    }

    settings.beginGroup(name);

    for (int i = 0; i < propertyList->count(); i++)
    {
        QListWidgetItem *item;
        item = propertyList->item(i);
        //item->data(Qt::UserRole).clear();
        QString text = item->text();
        bool selected = item->checkState();
        settings.setValue(text, selected ? true : false);
    }

    settings.endGroup();
    settings.endGroup();

    settings.setValue("current-profile", name);
    current_profile = name;
}

void MainWindow::createProfile()
{
    // Ask whether to save the new settings as the current profile
    SaveProfileDialog dlg(this);
    if (QDialog::Accepted == dlg.exec())
    {
        saveSettingsAsProfile(dlg.profileName());
    }
}

void MainWindow::removeProfile()
{
    RemoveProfileDialog dlg(this);
    dlg.exec();

    // Reload profile list
    updatingProfileList = true;
    profileList->clear();
    QSettings settings;
    settings.beginGroup("Profiles");
    profileList->addItems(settings.childGroups());
    settings.endGroup();

    // Now we have to select the item corresponding to the current profile
    QList<QListWidgetItem*> items = profileList->findItems(current_profile, Qt::MatchExactly);
    if (items.count() == 1) {
        profileList->setCurrentRow(profileList->row(items[0]), QItemSelectionModel::SelectCurrent);
    }
    updatingProfileList = false;
}

void MainWindow::createNewModel()
{
    qDebug() << "MainWindow::createNewModel(): calling NewModelDlg";
    NewModelDlg dlg(this);

    if (dlg.exec() == QDialog::Accepted)
    {
        qDebug() << "MainWindow::createNewModel(): "
                    "NewModelDlg returns" << QDialog::Accepted;

        QString name = dlg.getName();

        _current_model = Model::createModel(name);
        qDebug() << "MainWindow::createNewModel(): name =" << name;

        // Find the currently selected item and deselect it
        for (int i = 0; i < modelList->count(); ++i)
        {
            QListWidgetItem* it = modelList->item(i);
            if (it->isSelected())
            {
                it->setSelected(false);
                break;
            }
        }

        // Add a new item to the list of models, giving the new model name
        QListWidgetItem *item = new QListWidgetItem;
        item->setText(name);
        modelList->addItem(item);
        selectedModelItem = item;

        // Remove the current [Models] section from settings
        QSettings settings;
        settings.beginGroup("Models");
        settings.remove("");
        settings.endGroup();

        // Repopulate it with a list containing the new model name
        settings.beginWriteArray("Models");
        for (int i = 0; i < modelList->count(); ++i) {
            settings.setArrayIndex(i);
            settings.setValue("name", modelList->item(i)->text());
        }
        settings.endArray();

        // Get the name of the model (if any) from which to import parameters
        // and pass it to the parameter wizard
        if (!dlg.importFrom().isEmpty()) {
            wiz->importFrom(dlg.importFrom());
        }

        // Call the parameter wizard to allow the user to change the parameters
        qDebug() << "MainWindow::createNew(): calling editParameters()";
        editParameters();

        // Highlight the row containing the new item. We do this after they've
        // had a chance to update the parameters as it will trigger a re-run of
        // the model.
        modelList->setCurrentRow(modelList->row(item), QItemSelectionModel::Select);
    }
    else
    {
        qDebug() << "MainWindow::createNew(): NewModelDlg returns"
                 << QDialog::Rejected;
        // No action needed...
    }
}

void MainWindow::remove()
{
    RemoveModelDlg dlg(this);
    dlg.exec();

    reloading = true;
    loadModelList();
    reloading = false;
}

void MainWindow::editParameters()
{
    Q_ASSERT(selectedModelItem != 0);
    QString model_name = selectedModelItem->text();
    qDebug() << "MainWindow::edit(): model_name =" << model_name;
    wiz->setCurrentModel(model_name);
    if (wiz->exec() == QDialog::Accepted)
    {
        Model *mod = current_model();
        mod->run();
    }
    propertyChanged();
}

void MainWindow::editModelDescription()
{
    NewModelDlg *dlg = new NewModelDlg(this);
    dlg->setPreexisting();
    if (dlg->exec() == QDialog::Accepted)
    {
        // Update the notes
        // TODO: Display the notes somewhere on the chart
        // ...
    }
}

void MainWindow::about()
{
    QMessageBox::about(
                this,
                "About Obson",
                "Obson version " + QString(VERSION)
                );
}

void MainWindow::aboutQt()
{
    QMessageBox::aboutQt(this, "About Qt");
}

void MainWindow::setOptions()
{
    OptionsDialog dlg(this);
    dlg.setModal(true);
    if (dlg.exec() == QDialog::Accepted && _current_model != nullptr)
    {
        drawChart(true);
    }
}

void MainWindow::createStatusBar()
{
    inequalityLabel = new QLabel;
    productivityLabel = new QLabel;
    infoLabel = new QLabel;

    statusBar()->addPermanentWidget(inequalityLabel);
    statusBar()->addPermanentWidget(productivityLabel);
    statusBar()->addPermanentWidget(infoLabel);

    infoLabel->setText(tr("No model has been selected"));
}

void MainWindow::propertyChanged()
{
    qDebug() << "MainWindow::propertyChanged";
    // Allow the property to be changed even when there's no model selected.
    if (_current_model != nullptr && !reloading)
    {
        drawChart(false);   // no need to rerun
        if (!current_profile.isEmpty()) {
            qDebug() << "MainWindow::propertyChanged(): setting profile_changed";
            profile_changed = true;
        }
    }
}

void MainWindow::selectProfile(QString text)
{
    profileList->findItems(
                text,
                Qt::MatchFixedString | Qt::MatchCaseSensitive
                ).first()->setSelected(true);
}

void MainWindow::createDockWindows()
{
    // Create property list
    QDockWidget *dock = new QDockWidget(tr("Properties"), this);
    dock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    propertyList = new QListWidget(dock);
    //propertyList->setFixedWidth(220);

    // Populate the property list
    QMap<QString,Model::Property>::iterator i;
    for (i = property_map.begin(); i != property_map.end(); ++i)
    {
        QListWidgetItem *item = new QListWidgetItem;
        item->setText(i.key());
        item->setCheckState(Qt::Unchecked);
        item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsUserCheckable | Qt::ItemIsEnabled);
        propertyList->addItem(item);
    }
    connect(propertyList, &QListWidget::itemChanged, this, &MainWindow::propertyChanged);
    connect(propertyList, &QListWidget::itemClicked/*currentItemChanged*/, this, &MainWindow::updateStatsDialog);

    // Add to dock
    dock->setWidget(propertyList);
    //dock->setFeatures(QDockWidget::NoDockWidgetFeatures);
    addDockWidget(Qt::LeftDockWidgetArea, dock);

    // Create the model list
    dock = new QDockWidget(tr("Models"), this);
    dock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    modelList = new QListWidget(dock);
    //modelList->setFixedWidth(220);

    // Populate the model list
    loadModelList();

    // Add to dock
    dock->setWidget(modelList);
    //dock->setFeatures(QDockWidget::NoDockWidgetFeatures);
    addDockWidget(Qt::LeftDockWidgetArea, dock);

    // Create the profile list
    dock = new QDockWidget(tr("Chart Profiles"), this);
    dock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    profileList = new QListWidget(dock);
    //profileList->setFixedWidth(220);

    // Populate the profile list
    QSettings settings;
    settings.beginGroup("Profiles");
    profileList->addItems(settings.childGroups());
    settings.endGroup();

    // Select the item corresponding to the current profile
    if (!current_profile.isEmpty()) {
        selectProfile(current_profile);
    }

    // Add to dock
    dock->setWidget(profileList);
    //dock->setFeatures(QDockWidget::NoDockWidgetFeatures);
    addDockWidget(Qt::LeftDockWidgetArea, dock);

    // Create the parameter wizard
    wiz = new ParameterWizard(this);
    wiz->setProperties(property_map);
    wiz->setModal(true);

    // Connect signals for changing selection and double-click
    connect(modelList, &QListWidget::currentItemChanged, this, &MainWindow::changeModel);
    connect(modelList, &QListWidget::itemDoubleClicked, this, &MainWindow::editParameters);
    connect(profileList, &QListWidget::currentItemChanged, this, &MainWindow::changeProfile);
}

void MainWindow::showStatistics()
{
    if (!property_selected) {
        QMessageBox msgBox;
        msgBox.setText("Please select a property to display");
        msgBox.exec();
    } else {
        statsDialog->show();
    }
}

int MainWindow::loadProfileList()
{
    return 0;
}

void MainWindow::createFirstModel()
{
    if (0 == Model::loadAllModels())
    {
        createNewModel();
    }
}

int MainWindow::loadModelList()
{
    qDebug() << "MainWindow::loadModelList(): opening Settings";
    QSettings settings;
    QStringList model_names;

    // Read the model names from settings
    int count = settings.beginReadArray("Models");
    qDebug() << "MainWindow::loadModelList():" << count << "models found in settings";
    if (count > 0)
    {
        for (int i = 0; i < count; ++i)
        {
            settings.setArrayIndex(i);
            model_names.append(settings.value("name").toString());
        }
    }
    settings.endArray();

    // This allows the function to be called again should we want to reload all
    // the model names from scratch. In practice we generally add them one at a
    // time (except when starting up).
    qDebug() << "MainWindow::loadModelList(): clearing modelList";
    modelList->clear();

    qDebug() << "MainWindow::loadModelList(): adding new items";
    modelList->addItems(model_names);

    return modelList->count();
}

bool MainWindow::isModelSelected()
{
    return selectedModelItem != 0;
}

QColor MainWindow::nextColour(int n)
{
    return colours[n % colours.count()];
}

void MainWindow::drawChartRandomised()
{
    int seed = QTime(0,0,0).secsTo(QTime::currentTime());
    qDebug() << "MainWindow::drawChartRandomised(): reseeding with" << seed;
    qsrand(seed);
    drawChart(true, true);
}

void MainWindow::drawChartNormal()
{
    drawChart(true, false);
}

void MainWindow::drawChart(bool rerun, bool randomised)    // uses _current_model
{
    // We are going to remove the chart altogether and replace it with a new
    // one to make sure we get a clean slate. However if we don't remove the
    // objects owned by the old chart the program eventually crashes. So far,
    // the following lines seem to fix that problem. This may all be overkill,
    // but I haven't found an alternative way of keeping the axes up to data.
    QList<QAbstractSeries*> current_series = chart->series();
    for (int i = 0; i < current_series.count(); i++) {
        chart->removeSeries(current_series[i]);
    }
    if (chart->axisX() != nullptr) {
        chart->removeAxis(chart->axisX());
    }
    if (chart->axisY() != nullptr) {
        chart->removeAxis(chart->axisY());
    }

    // Remove the existing chart and replace it with a new one.
    delete chart;
    createChart();
    chart->legend()->setAlignment(Qt::AlignBottom);

    if (rerun)
    {
        _current_model->run(randomised);
        statsDialog->hide();
        if (property_selected) {
            updateStatsDialog(propertyList->currentItem());
        }
    }

    chart->legend()->show();
    chart->setTitle("<h2 style=\"text-align:center;\">"
                    + _current_model->name()
                    + "</h2><p style=\"text-align:center;\">"
                    + current_profile
                    + "</p>");

    QLineSeries *anySeries = nullptr;

    for (int i = 0, n = propertyColours.count(); i < propertyList->count(); i++)
    {
        QListWidgetItem *item;
        item = propertyList->item(i);
        bool selected = item->checkState();
        if (selected)
        {
            QString series_name = item->text();
            Model::Property prop = property_map[series_name];
            QLineSeries *ser = _current_model->series[prop];
            ser->setName(series_name);
            chart->addSeries(ser);
            anySeries = ser;

            switch(prop)
            {
            case Model::Property::zero:
            case Model::Property::hundred:
                ser->setColor(Qt::black);
                ser->setPen(QPen(Qt::DotLine));
                break;
            default:
                if (propertyColours.contains(prop))
                {
                    ser->setColor(propertyColours[prop]);
                }
                else
                {
                    QColor colour = nextColour(n++);
                    propertyColours[prop] = colour;
                    ser->setColor(colour);
                }
                break;
            }
        }
    }

    // Format the axis numbers to whole integers. This needs a series to have
    // been selected, so avoid otherwise
    if (anySeries != nullptr)
    {
        chart->createDefaultAxes();
        QValueAxis *x_axis = static_cast<QValueAxis*>(chart->axisX(anySeries));
        x_axis->setLabelFormat("%d");
        QValueAxis *y_axis = static_cast<QValueAxis*>(chart->axisY(anySeries));
        y_axis->setLabelFormat("%d");
    }

    double gini = _current_model->getGini();
    double prod = _current_model->getProductivity();

    inequalityLabel->setText(tr("Inequality: ") + QString::number(round(gini * 100)) + "%");
    productivityLabel->setText(tr("Productivity: ") + QString::number(round(prod + 0.5)) + "%");

    emit drawingCompleted();
}

void MainWindow::updateStatsDialog(QListWidgetItem *current)
{
    if (current == nullptr) {
        return;
    }

    statsAction->setEnabled(true);

    QSettings settings;
    int range = settings.value("iterations", 100).toInt() + 1;

    QListWidgetItem *it = current; //propertyList->currentItem();
    QString key = it->text();
    Model::Property prop = property_map[key];
    int ix = static_cast<int>(prop);
    int min = _current_model->min_value(ix);
    int max = _current_model->max_value(ix);
    int total = _current_model->total(ix);
    int mean = total / range;

    statsDialog->setLimits(key, min, max, mean);

    property_selected = true;
}

void MainWindow::changeProfile(QListWidgetItem *item)
{
    if (updatingProfileList) {
        return;
    }

    reloading = true;

    // Allow old profile to be saved if changed
    if (profile_changed) {
        createProfile();
    }

    qDebug() << "Changing profile";
    current_profile = item->text();
    qDebug() << "New profile is" << current_profile;

    profile_changed = false;    // i.e. _this_ profile hasn't been changed

    // Load settings for this profile and redraw the chart
    QSettings settings;
    settings.setValue("current-profile", current_profile);
    settings.beginGroup("Profiles");
    settings.beginGroup(current_profile);

    for (int i = 0; i < propertyList->count(); i++)
    {
        QListWidgetItem *item;
        item = propertyList->item(i);
        QString text = item->text();
        bool checked = settings.value(text, false).toBool();
        item->setCheckState(checked ? Qt::Checked : Qt::Unchecked);
    }

    settings.endGroup();
    settings.endGroup();

    reloading = false;

    drawChart(true, false);
}

void MainWindow::changeModel(QListWidgetItem *item)
{
    if (reloading) {
        return;
    }

    selectedModelItem = item;
    changeAction->setDisabled(false);
    saveCSVAction->setDisabled(false);
    _current_model = Model::model(item->text());

    if (_current_model == nullptr) {
        errorMessage("Cannot find the requested model");    // exits
    }

    QSettings settings;

    double nominal_population = settings.value("nominal-population", 1000).toDouble();
    double scale = nominal_population / 1000;
    int startups = scale * settings.value("startups", 0).toInt();

    QString model_name = item->text();
    infoLabel->setText(  tr("  Total population: ") + QString::number(nominal_population)
                             + tr("  Government employees: ") + settings.value("government-employees", 200).toString()
                             + tr("  Standard wage: ") + settings.value("unit-wage", 500).toString()
                             + tr("  Number of private businesses at start: ") + QString::number(startups)
                            );

    settings.setValue("current_model", model_name);
    settings.beginGroup(model_name);
    settings.endGroup();

    drawChart(true, false);
}

