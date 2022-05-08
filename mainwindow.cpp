#include "mainwindow.h"
#include "createdomaindlg.h"
#include "domain.h"
#include "newbehaviourldlg.h"
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
    _currentBehaviour = nullptr;
    first_time_shown = true;

    statsDialog = new StatsDialog(this);
    statsDialog->setWindowFlags(Qt::Tool);

    // Set up list of property names.

    // Qt Limitation: can only initialise a QMap from an initializer list if
    // is declared as static. And of course this is no help when it has to
    // point to instance members.

    // Note that Behaviour::Property is an enum, so in effect PropertyMap
    // maps property names to indices that reference a chart series

    propertyMap[tr("Current period")] = Behaviour::Property::current_period;
    propertyMap[tr("Population size")] = Behaviour::Property::pop_size;
    propertyMap[tr("Govt exp excl benefits")] = Behaviour::Property::gov_exp;
    propertyMap[tr("Govt exp incl benefits")] = Behaviour::Property::gov_exp_plus;
    propertyMap[tr("Benefits paid")] = Behaviour::Property::bens_paid;
    propertyMap[tr("Government receipts")] = Behaviour::Property::gov_recpts;
    propertyMap[tr("Deficit (absolute)")] = Behaviour::Property::deficit;
    propertyMap[tr("Deficit as % GDP")] = Behaviour::Property::deficit_pc;
    propertyMap[tr("National Debt")] = Behaviour::Property::gov_bal;
    propertyMap[tr("Number of businesses")] = Behaviour::Property::num_firms;
    propertyMap[tr("Number employed")] = Behaviour::Property::num_emps;
    propertyMap[tr("Number of govt employees")] = Behaviour::Property::num_gov_emps;
    propertyMap[tr("Percent employed")] = Behaviour::Property::pc_emps;
    propertyMap[tr("Number unemployed")] = Behaviour::Property::num_unemps;
    propertyMap[tr("Percent unemployed")] = Behaviour::Property::pc_unemps;
    propertyMap[tr("Percent active")] = Behaviour::Property::pc_active;
    propertyMap[tr("Number of new hires")] = Behaviour::Property::num_hired;
    propertyMap[tr("Number of new fires")] = Behaviour::Property::num_fired;
    propertyMap[tr("Businesses balance")] = Behaviour::Property::prod_bal;
    propertyMap[tr("Wages paid")] = Behaviour::Property::wages;
    propertyMap[tr("Consumption")] = Behaviour::Property::consumption;
    propertyMap[tr("Bonuses paid")] = Behaviour::Property::bonuses;
    propertyMap[tr("Pre-tax deductions")] = Behaviour::Property::dedns;
    propertyMap[tr("Income tax paid")] = Behaviour::Property::inc_tax;
    propertyMap[tr("Sales tax paid")] = Behaviour::Property::sales_tax;
    propertyMap[tr("Households balance")] = Behaviour::Property::dom_bal;
    propertyMap[tr("Bank loans")] = Behaviour::Property::amount_owed;
    propertyMap[tr("Average business size")] = Behaviour::Property::bus_size;
    propertyMap[tr("100 reference line")] = Behaviour::Property::hundred;
    propertyMap[tr("Procurement expenditure")] = Behaviour::Property::procurement;
    propertyMap[tr("Productivity")] = Behaviour::Property::productivity;
    propertyMap[tr("Productivity (relative)")] = Behaviour::Property::rel_productivity;
    propertyMap[tr("Govt direct support")] = Behaviour::Property::unbudgeted;
    propertyMap[tr("Investment")] = Behaviour::Property::investment;
    propertyMap[tr("GDP")] = Behaviour::Property::gdp;
    propertyMap[tr("Profit")] = Behaviour::Property::profit;
    propertyMap[tr("Zero reference line")] = Behaviour::Property::zero;

    // If non-zero, points to currently selected listwidget item
    selectedBehaviourItem = nullptr;

    // Make sure preferences exist
    QSettings settings;
    qDebug() << "Settings are in" << settings.fileName();

    // If there are no global settings set up sensible defaults
    if (!settings.contains("iterations"))
    {
        QMessageBox msgBox(this);
        msgBox.setText(tr("Using default settings"));
        msgBox.setInformativeText(tr("You have not yet customised the settings "
                                     " for this application. To do so, please select "
                                     "\"Preferences\" from the main menu."));
        msgBox.setIcon(QMessageBox::Warning);
        msgBox.setStandardButtons(QMessageBox::Ok);
        msgBox.setDefaultButton(QMessageBox::Ok);
        msgBox.exec();

        settings.setValue("iterations", 100);
        settings.setValue("start-period", 0);
        settings.setValue("startups", 10);
        settings.setValue("nominal-population", 1000);
        settings.setValue("unit-wage", 100);
        settings.setValue("government-employees", 200); // approx tot pop / 5
        //settings.setValue("sample-size", 10); // for moving averages -- adjust as necessary
    }

    currentProfile = settings.value("current-profile", "").toString();

    createChart();
    createActions();
    createMenus();
    createStatusBar();
    createDockWindows();

    // Set up existing domains and there parameters
    settings.beginGroup("Domains");
    for (int j = 0; j < domainList->count(); ++j)
    {
        QListWidgetItem *item = domainList->item(j);
        QString domainName = item->text();
        qDebug() << "adding domain" << domainName;
        settings.beginGroup(domainName);

        // This assumes something has created the named behaviour. Currently
        // this done only when selecting a behaviour by name, and ia stored in
        // _currentBehaviour somewhere...
        Behaviour *beh = Behaviour::getBehaviour(settings.value("Behaviour").toString());

        Domain *dom = new Domain(
                    domainName,
                    beh,
                    settings.value("Currency").toString(),
                    settings.value("Abbrev").toString()
                    );
        domains.append(dom);
        settings.endGroup();
    }



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
    saveCSVAction->setDisabled(!isBehaviourSelected());
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
    changeAction = new QAction(setupIcon, tr("Edit &behaviour..."));
    changeAction->setDisabled(!isBehaviourSelected());
    changeAction->setStatusTip(tr("Modify this behaviour definition"));
    connect(changeAction, &QAction::triggered, this, &MainWindow::editParameters);

    // Reassign colours
    const QIcon coloursIcon = QIcon(":/chart.icns");
    coloursAction = new QAction(coloursIcon, tr("Reset &colours"));
    coloursAction->setStatusTip(tr("Reset chart colours"));
    connect(coloursAction, &QAction::triggered, this, &MainWindow::reassignColours);

    // New model
    const QIcon newIcon = QIcon(":/add-model.icns");
    newAction = new QAction(newIcon, tr("&New behaviour definitionl..."), this);
    newAction->setStatusTip(tr("Create a new behaviour definition"));
    connect(newAction, &QAction::triggered, this, &MainWindow::createNewBehaviour);

    // New domain
    const QIcon domainIcon = QIcon(":/world.icns");
    domainAction = new QAction(domainIcon, tr("New &domain..."), this);
    domainAction->setStatusTip(tr("Create a new domain"));
    connect(domainAction, &QAction::triggered, this, &MainWindow::createDomain);

    // Remove models
    const QIcon removeIcon = QIcon(":/delete-model.icns");
    removeAction = new QAction(removeIcon, tr("&Remove behaviour definitions..."));
    removeAction->setStatusTip(tr("Remove one or more behaviour definitions"));
    connect(removeAction, &QAction::triggered, this, &MainWindow::remove);

    // Edit model description
    const QIcon notesIcon = QIcon(":/notes.icns");
    notesAction = new QAction(notesIcon, tr("&Edit description..."));
    notesAction->setStatusTip(tr("Edit the description for this behaviour definition"));
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

    // If there is no default behaviour, create one
    connect(this,
      &MainWindow::windowShown,
      this,
      &MainWindow::createDefaultBehaviour);

    connect(this, &MainWindow::windowLoaded, this, &MainWindow::restoreState);
}

void MainWindow::createMenus()
{
    myMenuBar = new QMenuBar(nullptr);

    applicationMenu = myMenuBar->addMenu(tr("&Obson"));

    fileMenu = myMenuBar->addMenu(tr("&File"));
    fileMenu->addAction(newAction);
    fileMenu->addAction(domainAction);
    fileMenu->addSeparator();
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
    myToolBar->addAction(domainAction);
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

Behaviour *MainWindow::currentBehaviour()
{
    return _currentBehaviour;
}

Domain *MainWindow::getDomain(QString domainName)
{
    // NEXT: COMPLETE THIS FUNCTION -- IMPORTANT!

    int numDomains = domains.count();

    for (int i = 0; i < numDomains; i++)
    {
        Domain *dom = domains.at(i);
        if (dom->getName() == domainName) {
            return dom;
        }
    }

    return nullptr;
}

void MainWindow::reassignColours()
{
    propertyColours.clear();
    drawChart(true, false);
}

void MainWindow::saveCSV()
{
    qDebug() << "MainWindow::saveCSV():  called";

    Behaviour *model = currentBehaviour();
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
            Behaviour::Property prop = propertyMap[series_name];
            lists[n++] = _currentBehaviour->series[prop]->points();
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
    msgBox.setText(tr("Not Yet Implemented"));
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
    settings.setValue("current-profile", currentProfile);
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
        QList<QListWidgetItem*> items = behaviourList->findItems(s, Qt::MatchExactly);
        if (items.count() == 1) {
            items[0]->setSelected(true);
            changeBehaviour(items[0]);
        }
    }
}

#include "behaviour.h"

void MainWindow::saveSettingsAsProfile(QString name)
{
    if (name.isEmpty()) {
        if (currentProfile.isEmpty()) {
            // TODO: There should be an error message here, unless this case is
            // filtered out -- check.
            return;
        } else {
            name = currentProfile;
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
    currentProfile = name;
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
    QList<QListWidgetItem*> items = profileList->findItems(currentProfile, Qt::MatchExactly);
    if (items.count() == 1) {
        profileList->setCurrentRow(profileList->row(items[0]), QItemSelectionModel::SelectCurrent);
    }
    updatingProfileList = false;
}


// NEXT: This function is no longer correct. Note that behaviourList here is
// a QListWidget containing the behaviour names only. The behaviour parameters
// are stored in settings independently (see Behaviour::createBehaviour()).
// This architecture will have to be changed.
void MainWindow::createNewBehaviour()
{
    qDebug() << "MainWindow::createNewModel(): calling NewModelDlg";
    NewBehaviourlDlg dlg(this);

    if (dlg.exec() == QDialog::Accepted)
    {
        qDebug() << "MainWindow::createNewModel(): "
                    "NewModelDlg returns" << QDialog::Accepted;

        QString name = dlg.getName();

        _currentBehaviour = Behaviour::createBehaviour(name);
        qDebug() << "MainWindow::createNewModel(): name =" << name;

        // Find the currently selected item and deselect it
        for (int i = 0; i < behaviourList->count(); ++i)
        {
            QListWidgetItem* it = behaviourList->item(i);
            if (it->isSelected())
            {
                it->setSelected(false);
                break;
            }
        }

        // Add a new item to the list of models, giving the new model name
        QListWidgetItem *item = new QListWidgetItem;
        item->setText(name);
        behaviourList->addItem(item);
        selectedBehaviourItem = item;

        // Remove the current [Models] section from settings
        QSettings settings;
        settings.beginGroup("Models");
        settings.remove("");
        settings.endGroup();

        // Repopulate it with a list containing the new model name
        settings.beginWriteArray("Models");
        for (int i = 0; i < behaviourList->count(); ++i) {
            settings.setArrayIndex(i);
            settings.setValue("name", behaviourList->item(i)->text());
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
        behaviourList->setCurrentRow(behaviourList->row(item), QItemSelectionModel::Select);
    }
    else
    {
        qDebug() << "MainWindow::createNew(): NewModelDlg returns"
                 << QDialog::Rejected;
        // No action needed...
    }
}


// This function creates a new domain using the information returned by the
// CreateDomainDlg and appends it to QList<Domain*>::domains
void MainWindow::createDomain()
{
    qDebug() << "MainWindow::createDomain(): calling CreateDomainDlg";
    CreateDomainDlg dlg(this);
    if (dlg.exec() == QDialog::Accepted)
    {
        Domain *newDomain = new Domain(
                    dlg.getDomainName(),
                    Behaviour::getBehaviour(dlg.getBehaviourName()),
                    dlg.getCurrency(),
                    dlg.getCurrencyAbbrev()
                    );

        domains.append(newDomain);
    }
}

void MainWindow::remove()
{
    RemoveModelDlg dlg(this);
    dlg.exec();

    reloading = true;
    loadBehaviourList();
    reloading = false;
}

void MainWindow::editParameters()
{
    Q_ASSERT(selectedBehaviourItem != nullptr);
    QString behaviourName = selectedBehaviourItem->text();
    qDebug() << "MainWindow::edit(): model_name =" << behaviourName;
    wiz->setCurrentModel(behaviourName);
    if (wiz->exec() == QDialog::Accepted)
    {
        Behaviour *mod = currentBehaviour();
        mod->run();
    }
    propertyChanged();
}

void MainWindow::editModelDescription()
{
    NewBehaviourlDlg *dlg = new NewBehaviourlDlg(this);
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
    if (dlg.exec() == QDialog::Accepted && _currentBehaviour != nullptr)
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
    if (_currentBehaviour != nullptr && !reloading)
    {
        drawChart(false);   // no need to rerun
        if (!currentProfile.isEmpty()) {
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
    QMap<QString,Behaviour::Property>::iterator i;
    for (i = propertyMap.begin(); i != propertyMap.end(); ++i)
    {
        QListWidgetItem *item = new QListWidgetItem;
        item->setText(i.key());
        item->setCheckState(Qt::Unchecked);
        item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsUserCheckable | Qt::ItemIsEnabled);
        propertyList->addItem(item);
    }
    connect(propertyList, &QListWidget::itemChanged, this, &MainWindow::propertyChanged);
    connect(propertyList, &QListWidget::itemClicked, this, &MainWindow::updateStatsDialog);

    // Add to dock
    dock->setWidget(propertyList);
    addDockWidget(Qt::LeftDockWidgetArea, dock);

    // Create the behaviour list
    dock = new QDockWidget(tr("Behaviours"), this);
    dock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    behaviourList = new QListWidget(dock);

    // Populate the behaviour list
    loadBehaviourList();

    // Add to dock
    dock->setWidget(behaviourList);
    addDockWidget(Qt::LeftDockWidgetArea, dock);

    // Create the profile list
    dock = new QDockWidget(tr("Chart Profiles"), this);
    dock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    profileList = new QListWidget(dock);

    // Populate the profile list
    QSettings settings;
    settings.beginGroup("Profiles");
    profileList->addItems(settings.childGroups());
    settings.endGroup();

    // Select the item corresponding to the current profile
    if (!currentProfile.isEmpty()) {
        selectProfile(currentProfile);
    }

    // Add to dock
    dock->setWidget(profileList);
    //dock->setFeatures(QDockWidget::NoDockWidgetFeatures);
    addDockWidget(Qt::LeftDockWidgetArea, dock);

    // Create domain list
    dock = new QDockWidget(tr("Domain"), this);
    dock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    domainList = new QListWidget(dock);

    // Populate the domain list
    settings.beginGroup("Domains");
    domainList->addItems(settings.childGroups());
    settings.endGroup();
    dock->setWidget(domainList);
    addDockWidget(Qt::LeftDockWidgetArea, dock);




    // Create the parameter wizard
    wiz = new ParameterWizard(this);
    wiz->setProperties(propertyMap);
    wiz->setModal(true);

    // Connect signals for changing selection and double-click
    connect(behaviourList, &QListWidget::currentItemChanged, this, &MainWindow::changeBehaviour);
    connect(behaviourList, &QListWidget::itemDoubleClicked, this, &MainWindow::editParameters);
    connect(profileList, &QListWidget::currentItemChanged, this, &MainWindow::changeProfile);
    connect(domainList, &QListWidget::currentItemChanged, this, &MainWindow::changeDomain);
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

void MainWindow::createDefaultBehaviour()
{
    if (0 == Behaviour::loadAllBehaviours())
    {
        createNewBehaviour();
    }
}

// NEXT: THIS NEEDS TO APPEND TO THE GLOBAL BEHAVIOURS LIST. Also, the way
// behaviours are stored in settings neeeds to be changed
int MainWindow::loadBehaviourList()
{
    qDebug() << "MainWindow::loadBehaviourList(): opening Settings";
    QSettings settings;
    QStringList behaviourNames;

    // TODO: Set up defaultGroup manually...

    settings.beginGroup("Behaviours");
    QStringList groups = settings.childGroups();

    for (int i = 0; i < groups.size(); i++)
    {
        QString group = groups.at(i);

        settings.beginGroup(group);
        Behaviour *newBehaviour = new Behaviour(group);

        // TODO: Read settings for behaviour i ...

        // Append new behaviour to the list
        behaviours.append(newBehaviour);
        settings.endGroup();
    }
    settings.endGroup();





    // ----------------------------------
    //       OLD STUFF -- REPLACE
    // ----------------------------------

    // Read the model names from settings
    int count = settings.beginReadArray("Behaviours");
    qDebug() << "MainWindow::loadBehaviourList():" << count << "behaviours found in settings";
    if (count > 0)
    {
        for (int i = 0; i < count; ++i)
        {
            settings.setArrayIndex(i);
            behaviourNames.append(settings.value("name").toString());
        }
    }
    settings.endArray();

    // This allows the function to be called again should we want to reload all
    // the model names from scratch. In practice we generally add them one at a
    // time (except when starting up).
    qDebug() << "MainWindow::loadBehaviourList(): clearing behaviourList";
    behaviourList->clear();

    qDebug() << "MainWindow::loadBehaviourList(): adding new items";
    behaviourList->addItems(behaviourNames);

    return behaviourList->count();
}

int MainWindow::loadDomainList()
{
    qDebug() << "MainWindow::loadDomainList(): opening Settings";
    QSettings settings;
    QStringList domainNames;

    // Read the model names from settings

    /*
     * This is now done when creating dock windows
     *
    int count = settings.beginReadArray("Domains");
    qDebug() << "MainWindow::loadDomainList():" << count << "domains found in settings";
    if (count > 0)
    {
        for (int i = 0; i < count; ++i)
        {
            settings.setArrayIndex(i);


            domainNames.append(settings.value("name").toString());
        }
    }
    settings.endArray();
    */

}

bool MainWindow::isBehaviourSelected()
{
    return selectedBehaviourItem != nullptr;
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
    // ***
    // We are going to remove the chart altogether and replace it with a new
    // one to make sure we get a clean slate. However if we don't remove the
    // objects owned by the old chart the program eventually crashes. So far,
    // the following lines seem to fix that problem. This may all be overkill,
    // but I haven't found an alternative way of keeping the axes up to data.
    // ***

    QList<QAbstractSeries*> current_series = chart->series();

    // We remove the series one by one because for some reason removeAllSeries
    // causes the program to crash. I think it's after restart() is called, so
    // perhaps it doesn't like it if there are no series to remove.
    /*
    chart->removeAllSeries();        // causes crash
    */

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
        _currentBehaviour->run(randomised);
        statsDialog->hide();
        if (property_selected) {
            updateStatsDialog(propertyList->currentItem());
        }
    }

    chart->legend()->show();
    chart->setTitle("<h2 style=\"text-align:center;\">"
                    + _currentBehaviour->name()
                    + "</h2><p style=\"text-align:center;\">"
                    + currentProfile
                    + "</p>");

    QLineSeries *anySeries = nullptr;

    int y_max = -INT_MAX, y_min = INT_MAX;
    qDebug() << "MainWindow::drawChart(): resetting range y_min = " << y_min << "y_max" << y_max << "***";

    for (int i = 0, n = propertyColours.count(); i < propertyList->count(); i++)
    {
        QListWidgetItem *item;
        item = propertyList->item(i);
        bool selected = item->checkState();
        if (selected)
        {
            QString series_name = item->text();
            Behaviour::Property prop = propertyMap[series_name];
            QLineSeries *ser = _currentBehaviour->series[prop];
            ser->setName(series_name);
            chart->addSeries(ser);

            anySeries = ser;

            // Set the line colour and type for this series
            switch(prop)
            {
            case Behaviour::Property::zero:
            case Behaviour::Property::hundred:
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


            // Set values for y axis range

            // TODO: prop is an enum but max_value() and min_value() expect
            // ints, so we have to do a static cast. Perhaps this should really
            // be done in the functions themselves.
            int ix = static_cast<int>(prop);

            y_max = std::max(y_max, _currentBehaviour->max_value(ix));
            y_min = std::min(y_min,_currentBehaviour->min_value(ix));
            qDebug() << "MainWindow::drawChart(): series name" << series_name
                     << "series max" << _currentBehaviour->max_value(ix)
                     << "y_max" << y_max
                     << "series min" << _currentBehaviour->min_value(ix)
                     << "y_min" << y_min;
        }

    }

    int scale = magnitude(std::max(std::abs(y_max), std::abs(y_min)));

    qDebug() << "MainWindow::drawChart(): min" << y_min << "max" << y_max << "scale" << scale;

    // Format the axis numbers to whole integers. This needs a series to have
    // been selected, so avoid otherwise
    if (anySeries != nullptr)
    {
        chart->createDefaultAxes();
        QValueAxis *x_axis = static_cast<QValueAxis*>(chart->axisX(anySeries));
        x_axis->setLabelFormat("%d");
        QValueAxis *y_axis = static_cast<QValueAxis*>(chart->axisY(anySeries));
        y_axis->setLabelFormat("%d");

        int temp;
        if (y_max > 0 && y_min >= 0) {
            // Both positive: range from zero to y_max rounded up to power of 10
            temp = std::pow(10, (scale + 1));
            y_max = (temp >= y_max * 2 ? (temp >= y_max * 4 ? temp / 4 : temp / 2) : temp);
            y_min = 0;
        } else if (y_min < 0 && y_max <= 0) {
            // Both negative: range y_min rounded down to power of 10, to zero
            y_max = 0;
            temp = -std::pow(10, (scale + 1));
            y_min = (temp <= y_min * 2 ? (temp <= y_min * 4 ? temp / 4 : temp / 2) : temp);
        } else {
            // TODO: It would be nicer if the intervals were equally reflected
            // about the x-axis but this isn't really very important
            temp = std::pow(10, (scale + 1));
            y_max = (temp >= y_max * 2 ? (temp >= y_max * 4 ? temp / 4: temp / 2) : temp);
            temp = -std::pow(10, (scale + 1));
            y_min = (temp <= y_min * 2 ? (temp <= y_min * 4 ? temp / 4 : temp / 2) : temp);
        }

        qDebug() << "MainWindow::drawChart(): Setting range from" << y_min << "to" << y_max;
        y_axis->setRange(y_min, y_max);
    }

    double gini = _currentBehaviour->getGini();
    double prod = _currentBehaviour->getProductivity();

    inequalityLabel->setText(tr("Inequality: ") + QString::number(round(gini * 100)) + "%");
    productivityLabel->setText(tr("Productivity: ") + QString::number(round(prod + 0.5)) + "%");

    // emit drawingCompleted();
}

int MainWindow::magnitude(double y)
{
    int x = (y == 0.0 ? -INT_MAX : (static_cast<int>(log10(abs(y)))));
    qDebug() << "MainWindow::magnitude(): magnitude of" << y << "is" << x;
    return x;
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
    Behaviour::Property prop = propertyMap[key];
    int ix = static_cast<int>(prop);
    int min = _currentBehaviour->min_value(ix);
    int max = _currentBehaviour->max_value(ix);
    int total = _currentBehaviour->total(ix);
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
    currentProfile = item->text();
    qDebug() << "New profile is" << currentProfile;

    profile_changed = false;    // i.e. _this_ profile hasn't been changed

    // Load settings for this profile and redraw the chart
    QSettings settings;
    settings.setValue("current-profile", currentProfile);
    settings.beginGroup("Profiles");
    settings.beginGroup(currentProfile);

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

// NEXT: COMPLETE THIS FUNCTION (changeDomain())
void MainWindow::changeDomain(QListWidgetItem *item)
{
    QString domainName = item->text();

    qDebug() << "MainWindow::changeDomain(): changing to"
             << domainName;

    QSettings settings;

    settings.setValue("current_domain", domainName);
    settings.beginGroup("Domains");
    int count = settings.beginReadArray(domainName);

    /* sort this out later
     *
    if (count > 0)
    {
        for (int i = 0; i < count; ++i)
        {
            settings.setArrayIndex(i);
            ui->comboBox->addItem(settings.value("name").toString());
        }
    } else {
        // TODO:
    }
    */

    settings.endArray();


    /*
     * We need to use most of the functionality of the changeBehaviour function
     * but without reference to the QListWidgetItem corresponding to the list
     * of behaviours. since the latter will eventually be removed we may as
     * well just copy the relevant code into changeDomain.
     */

    // Get the behaviour associated with the required domain and set it as current.
    Domain *dom = getDomain(domainName);

    if (dom == nullptr) {
        errorMessage("Cannot find the requested domain (" + item->text() + ")");    // exits

    }
    qDebug() << "MainWindow::changeDomain(): domain"
             << item->text()
             << "found";

    _currentBehaviour = dom->getBehaviour();

    qDebug() << "MainWindow::changeDomain(): _currentBehaviour set";

    // Check that it worked -- this can be removed after testing
    if (_currentBehaviour == nullptr) {
        errorMessage("Cannot find the requested behaviour (" + item->text() + ")");    // exits
    }

    // Not sure why we need to do this -- investigate...
    changeAction->setDisabled(false);
    saveCSVAction->setDisabled(false);

    qDebug() << "MainWindow::changeDomain(): opening Settings";

    // Get settings to display the status bar. This should really be done globally, not here...
    double nominal_population = settings.value("nominal-population", 1000).toDouble();
    double scale = nominal_population / 1000;
    int startups = scale * settings.value("startups", 0).toInt();

    qDebug() << "MainWindow::changeDomain(): setting status bar";

    infoLabel->setText(  tr("  Total population: ") + QString::number(nominal_population)
                             + tr("  Government employees: ") + settings.value("government-employees", 200).toString()
                             + tr("  Standard wage: ") + settings.value("unit-wage", 500).toString()
                             + tr("  Private businesses at start: ") + QString::number(startups)
                            );

    qDebug() << "MainWindow::changeDomain(): status bar set";

    QString behaviourName = _currentBehaviour->name();

    qDebug() << "MainWindow::changeDomain(): behaviourName =" << behaviourName;


    settings.setValue("current_model", behaviourName);
    settings.beginGroup(behaviourName);
    settings.endGroup();

    qDebug() << "MainWindow::changeDomain(): redrawing chart";

    drawChart(true, false);
}

void MainWindow::changeBehaviour(QListWidgetItem *item)
{
    if (reloading) {
        return;
    }

    selectedBehaviourItem = item;
    changeAction->setDisabled(false);
    saveCSVAction->setDisabled(false);
    _currentBehaviour = Behaviour::getBehaviour(item->text());

    if (_currentBehaviour == nullptr) {
        errorMessage("Cannot find the requested behaviour (" + item->text() + ")");    // exits
    }

    QSettings settings;

    double nominal_population = settings.value("nominal-population", 1000).toDouble();
    double scale = nominal_population / 1000;
    int startups = scale * settings.value("startups", 0).toInt();

    QString behaviourName = item->text();
    infoLabel->setText(  tr("  Total population: ") + QString::number(nominal_population)
                             + tr("  Government employees: ") + settings.value("government-employees", 200).toString()
                             + tr("  Standard wage: ") + settings.value("unit-wage", 500).toString()
                             + tr("  Private businesses at start: ") + QString::number(startups)
                            );

    settings.setValue("current_model", behaviourName);
    settings.beginGroup(behaviourName);
    settings.endGroup();

    drawChart(true, false);
}

