
#include "mainwindow.h"
#include "account.h"
#include "domainparametersdialog.h"
//#include "newbehaviourldlg.h"

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

#include "optionsdialog.h"
#include "removemodeldlg.h"
#include "version.h"
#include "saveprofiledialog.h"
#include "statsdialog.h"
#include "removeprofiledialog.h"
#include "createdomaindlg.h"

MainWindow::MainWindow()
{
    QSettings settings;

    qDebug() << "MainWindow()";

    first_time_shown = true;

    setWindowTitle(tr("Stock-Flow Consistent Economic Model"));
    setWindowIcon(QIcon(":/obson.icns"));
    setUnifiedTitleAndToolBarOnMac(true);
    setMinimumSize(1024, 768);
    resize(1280, 800);

    statsDialog = new StatsDialog(this);
    statsDialog->setWindowFlags(Qt::Tool);

    qDebug() << "Settings are in" << settings.fileName();

    /*
     * Make sure preferences exist. We look for 'iterations' because 'General'
     * seems to be a reserved word and a search for it always returns false.
     * Perhaps I'm missing something...
     */
    if (!settings.contains("iterations"))
    {
        qDebug() << "No general settings -- saving defaults";
        settings.setValue("iterations", 100);
        settings.setValue("start-period", 0);
        settings.setValue("startups", 10);
        settings.setValue("nominal-population", 1000);
        settings.setValue("unit-wage", 100);
        settings.setValue("government-employees", 200); // approx tot pop / 5
    }

    /*
     * It makes sense to compare the same profile on different domains, so
     * the current chart profile applies regardless of the domain. However it
     * is not necessary to save a profile, and initially current-profile will
     * be empty. The chart profile determines the propertyList settings.
     */
    chartProfile = settings.value("current-profile", "").toString();

    /*
     * Allocate signals to menus
     */
    createActions();

    /*
     * Create the menus and the status bar
     */
    createMenus();
    createStatusBar();

    /*
     * Create and populate the dock windows, creating the propertyList and
     * instantiating domains from the values in settings
     */
    createDockWindows();

    /*
     * Create the subwindows that will contain the charts and the MdiArea they
     * will be contained by
     */
    createSubWindows();

    /*
     * Set the MDI area as central widget
     */
    setCentralWidget(&mdi);

    /*
     * Call drawCharts(propertyList), which does all the real work.
     */
    qDebug() << "calling Domain::drawCharts() with" << propertyList->count()
             << "propertyList items";
    Domain::drawCharts(propertyList);

    qDebug() << "********* returned from drawCharts **********";
}

MainWindow::~MainWindow()
{
    // Nothing to do...
}

void MainWindow::show()
{
    /*
     * NOTE: I no longer remember why we have to override QMainWindow::show(),
     * but if this is removed the window remains invisible. Investigate...
     */

    qDebug() << "MainWindow::show()";

    QMainWindow::show();
    QApplication::processEvents();

    // This isn't connected to anything and seems to be redundant
    // emit windowShown();

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

QChartView *MainWindow::createChart()
{
    QChart *chart = new QChart();
    chart->legend()->setAlignment(Qt::AlignBottom);
    chartView = new QChartView(chart);
    chartView->setRenderHint(QPainter::Antialiasing);
    chartView->setMinimumWidth(1000);

    return chartView;
}


void MainWindow::createActions()
{
    // Save as CVS file
    const QIcon csvIcon = QIcon(":/chart-2.icns");
    saveCSVAction = new QAction(csvIcon, tr("&Save as CSV file..."), this);
    saveCSVAction->setStatusTip(tr("Save current chart as a CSV file"));
    // saveCSVAction->setDisabled(!isBehaviourSelected());
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

    // Edit domain parameters
    const QIcon setupIcon = QIcon(":/settings.icns");
    changeAction = new QAction(setupIcon, tr("Edit domain &parameters..."));
    // changeAction->setDisabled(!isBehaviourSelected());
    changeAction->setStatusTip(tr("Modify the behaviour of this domain"));
    connect(changeAction, &QAction::triggered, this, &MainWindow::editParameters);

    // New domain
    const QIcon domainListIcon(":/world.icns");
    domainAction = new QAction(domainListIcon, tr("New &domain..."), this);
    domainAction->setStatusTip(tr("Create a new domain"));
    connect(domainAction, &QAction::triggered, this, &MainWindow::createDomain);

    // Options
    setOptionsAction = new QAction(tr("&Options"));
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
    runAction = new QAction(runIcon, tr("&Update the chart"), this);
    runAction->setStatusTip(tr("Update chart"));
    connect(runAction, &QAction::triggered, this, &MainWindow::drawChartNormal);

    // Close
    const QIcon closeIcon = QIcon(":/exit.icns");
    closeAction = new QAction(closeIcon, tr("&Quit..."), this);
    closeAction->setStatusTip(tr("Quit Obson"));
    connect(closeAction, &QAction::triggered, this, &MainWindow::close);
}


void MainWindow::createMenus()
{
    myMenuBar = new QMenuBar(nullptr);

    applicationMenu = myMenuBar->addMenu(tr("&Obson"));

    qDebug() << "Adding File menu";
    fileMenu = myMenuBar->addMenu(tr("&File"));
    fileMenu->addAction(runAction);
    fileMenu->addSeparator();
    fileMenu->addAction(domainAction);
    fileMenu->addSeparator();
    //fileMenu->addAction(removeAction);
    fileMenu->addSeparator();
    fileMenu->addAction(saveCSVAction);
    fileMenu->addAction(saveProfileAction);

    qDebug() << "Adding Edit menu";
    editMenu = myMenuBar->addMenu(tr("&Edit"));
    editMenu->addAction(changeAction);
    setOptionsAction->setMenuRole(QAction::ApplicationSpecificRole);
    editMenu->addAction(setOptionsAction);
    //editMenu->addAction(notesAction);

    qDebug() << "Adding View menu";
    viewMenu = myMenuBar->addMenu(tr("&View")); // add zoom here eventually
    //viewMenu->addAction(coloursAction);

    qDebug() << "Adding Help menu";
    helpMenu = myMenuBar->addMenu(tr("&Help"));
    applicationMenu->addAction(aboutAction);
    helpMenu->addAction(aboutQtAction);
    helpMenu->addAction(helpAction);

    qDebug() << "Setting menu bar";
    setMenuBar(myMenuBar);

    QToolBar *myToolBar = new QToolBar(this);
    addToolBar(Qt::LeftToolBarArea, myToolBar);

    //myToolBar->addAction(newAction);
    myToolBar->addAction(domainAction);
    //myToolBar->addAction(removeAction);
    myToolBar->addAction(changeAction);
    //myToolBar->addAction(notesAction);
    myToolBar->addAction(saveCSVAction);
    myToolBar->addAction(saveProfileAction);
    myToolBar->addAction(removeProfileAction);
    myToolBar->addAction(runAction);
    //myToolBar->addAction(coloursAction);
    myToolBar->addAction(statsAction);
    myToolBar->addAction(helpAction);
    myToolBar->addAction(closeAction);

    qDebug() << "Menus and tools created";
}

void MainWindow::reassignColours()  // redundant
{
    qDebug() << "MainWindow::reassignColours() called";
//    propertyColours.clear();
//    drawChartNormal();
}

void MainWindow::saveCSV()
{
#if 0   // this needs rewriting or removing
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
    int iters = getIters(), n = 0;
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

    int start = getStartPeriod();
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
#endif
}

void MainWindow::nyi()
{
    QMessageBox msgBox(this);
    msgBox.setText(tr("Not Yet Implemented"));
    msgBox.setInformativeText(tr("You have requested a service that is not yet implemented."));
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

    /*
     * Save current config to settings
     */

    QSettings settings;

    if (profile_changed)    // this doesn't seem to work -- investigate
    {
        /*
         * Check whether to save the new settings, either as the current
         * profile or under a (possibly) new profile name
         */
        SaveProfileDialog dlg(this);
        if (QDialog::Accepted == dlg.exec())
        {
            saveSettingsAsProfile(dlg.profileName());
        }
    }

    /*
     * Save the current profile (name) anyway
     */
    settings.setValue("current-profile", chartProfile);

    /*
     * Now safe to close
     */
    event->accept();
}

// #include "domain.h"

//QList<Domain*> domains;
//Domain *currentDomain = nullptr;
//Domain *defaultDomain = nullptr;


void MainWindow::saveSettingsAsProfile(QString name)
{
    if (name.isEmpty()) {
        if (chartProfile.isEmpty()) {
            // TODO: There should be an error message here, unless this case is
            // filtered out -- check.
            return;
        } else {
            name = chartProfile;
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
    chartProfile = name;
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
    QList<QListWidgetItem*> items = profileList->findItems(chartProfile, Qt::MatchExactly);
    if (items.count() == 1) {
        profileList->setCurrentRow(profileList->row(items[0]), QItemSelectionModel::SelectCurrent);
    }
    updatingProfileList = false;
}


/*
 * Call the static function in Domain to create a domain.
 */
void MainWindow::createDomain()
{
    qDebug() << "MainWindow::createDomain(): calling CreateDomainDlg";
    CreateDomainDlg dlg(this);

    if (dlg.exec() == QDialog::Accepted)
    {
        Domain::createDomain(
                    dlg.getDomainName(),
                    dlg.getCurrency(),
                    dlg.getCurrencyAbbrev()
                    );

    }
}

/*
 * Remove one or more chart profiles (dialog)
 */
void MainWindow::remove()
{
    RemoveModelDlg dlg(this);
    dlg.exec();
}

/*
 * Call the dialog to review or edit the parameters for a domain
 */
void MainWindow::editParameters()
{
    DomainParametersDialog dlg(this);

    QString domainName = dlg.getDomain();
    Domain *dom = Domain::getDomain(domainName);

    int val;

    if (dlg.exec() == QDialog::Accepted)
    {
        QSettings settings;
        /*
         * Write the parameters back to settings and to the params list in
         * Domain
         */
        settings.beginGroup("Domains");
        settings.beginGroup(domainName);

        val = dlg.getProcurement();
        settings.setValue("govt-procurement", val);
        dom->params[ParamType::procurement] = val;

        /* TODO: propensity to consume out of ...
         *
         * At present we only have a generic  propensity to consume. It would
         * be better to have both propensity to consume out of income and out
         * of savings.
         */
        val = dlg.getPropConsumeInc();
        settings.setValue("propensity-to-consume", val);
        dom->params[ParamType::prop_con] = val;

        val = dlg.getIncTaxThresh();
        settings.setValue("income-threshold", val);
        dom->params[ParamType::inc_thresh] = val;

        val = dlg.getDedns();
        settings.setValue("pre-tax-dedns-rate", val);
        dom->params[ParamType::dedns] = val;

        val = dlg.getIncTaxRate();
        settings.setValue("income-tax-rate", val);
        dom->params[ParamType::inc_tax_rate] = val;

        val = dlg.getSalesTaxRate();
        settings.setValue("sales-tax-rate", val);
        dom->params[ParamType::sales_tax_rate] = val;

        val = dlg.getStartupProb();
        settings.setValue("firm-creation-prop", val);
        dom->params[ParamType::firm_creation_prob] = val;

        val = dlg.getRecoupPeriods();
        settings.setValue("capex-recoup-periods", val);
        dom->params[ParamType::recoup] = val;

        val = dlg.getPropInvest();
        settings.setValue("prop-invest", val);
        dom->params[ParamType::prop_inv] = val;

        val = dlg.getUnempBen();
        settings.setValue("unempl-benefit-rate", val);
        dom->params[ParamType::unemp_ben_rate] = val;

        val = dlg.getCBInterest();
        settings.setValue("boe-interest", val);
        dom->params[ParamType::boe_int] = val;

        val = dlg.getClearingBankInterest();
        settings.setValue("bus-interest", val);
        dom->params[ParamType::bus_int] = val;

        val = dlg.getLoanProb();
        settings.setValue("loan-prob", val);
        dom->params[ParamType::loan_prob] = val;

        val = dlg.getStdWage();
        settings.setValue("standard-wage", val);
        dom->params[ParamType::std_wage] = val;

        /*
         * TODO: Add missing values to Params dlg...
         *
         * reserve-rate (dlg.getDistrib())
         * Propensity to consume out of savings
         *
         * Employment rate discontinued
         */

        settings.endGroup();
        settings.endGroup();

        Domain::drawCharts(propertyList);
    }
}

/*
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
*/

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
    if (dlg.exec() == QDialog::Accepted && !Domain::domains.isEmpty())
    {
        Domain::drawCharts(propertyList);
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

    infoLabel->setText(tr("Obson economic modelling"));
}

/*
 * This function is called (via a signal fromQListWidget propertyList)
 * whenever a property is activated or deactivated
 */
void MainWindow::propertyChanged()
{
    Domain::drawCharts(propertyList);
    profile_changed = true;
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
    /*
     * Initialise the PropertyMap in the Domain class. We can't leave this
     * until an instance is created as we need to read back (e.g.) the
     * PropertyMap entries
     */
    Domain::initialisePropertyMap();

    /*
     * Create a dockable widget for the property list and allow it to be placed
     * either side of the window
     */
    QDockWidget *dock = new QDockWidget(tr("Properties"), this);
    dock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);

    /*
     * Create an unpolulated list widget in the dock widget
     */
    propertyList = new QListWidget(dock);
    propertyList->setFixedWidth(295);

    /*
     * When the  property is selected signal the propertyChanged function. When
     * a property is clicked show its stats.
     */
    connect(propertyList, &QListWidget::itemChanged, this, &MainWindow::propertyChanged);
    //connect(propertyList, &QListWidget::itemClicked, this, &MainWindow::updateStatsDialog);


    /*
     * Populate the property list, setting the text for each item according to
     * property key with the same index, and its state to unchecked, selectable,
     * checkeble and enabled.
     */
    qDebug() << "Reading propertyMap, Domain::propertyMap has"
             << Domain::propertyMap.count() << "entries";
    QMap<QString,Property>::iterator i;
    for (i = Domain::propertyMap.begin(); i != Domain::propertyMap.end(); ++i)
    {
        QListWidgetItem *item = new QListWidgetItem;

        item->setText(i.key());
        item->setCheckState(Qt::Unchecked);
        item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsUserCheckable | Qt::ItemIsEnabled);

        propertyList->addItem(item);
    }

    /*
     * Add the property list widget to the dock and put the dock on the right
     * of the window
     */
    dock->setWidget(propertyList);
    addDockWidget(Qt::RightDockWidgetArea, dock);

    /*
     * Create the profile list
     */
    dock = new QDockWidget(tr("Chart Profiles"), this);
    dock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    profileList = new QListWidget(dock);

    /*
     * Populate the profile list and add it to the dock
     */
    QSettings settings;
    settings.beginGroup("Profiles");
    profileList->addItems(settings.childGroups());
    settings.endGroup();
    dock->setWidget(profileList);
    addDockWidget(Qt::RightDockWidgetArea, dock);

    /*
     * Select the  current profile
     */
    if (!chartProfile.isEmpty())
    {
        QList<QListWidgetItem*> items = profileList->findItems(chartProfile, Qt::MatchExactly);
        if (items.count() == 1)
        {
            changeProfile(items[0]);
            profileList->setItemSelected(items[0], true);
        }
        else
        {
            qDebug() << "chartProfile from settings (" << chartProfile << ") not found";
        }
    }
    else
    {
        qDebug() << "chartProfile is empty";
    }
}

int MainWindow::createSubWindows()
{
    /*
     * Populate the list of domains from settings
     */
    QSettings settings;
    qDebug() << "Creating list of domains";
    settings.beginGroup("Domains");
    domainNameList.append(settings.childGroups());
    settings.endGroup();

    /*
     * Create the domains and a window for each one, inserting a chartview into
     * each window and linking it to the associated domain.
     */
    if (domainNameList.count() > 0)
    {
        Domain::restoreDomains(domainNameList);
        Q_ASSERT(Domain::domains.count() == domainNameList.count());  // just checking

        /*
         * Create an MDI (sub)window for each domain listed, Add the subwindow
         * to the MDI area, create a QChart and assign it to a new QChartView,
         * and set the chartview as the chartview for each domain
         */
        qDebug() << "Creating MDI subwindows";
        foreach(Domain *dom, Domain::domains){
            QString title = dom->getName();
            qDebug() << "setting up domain" << dom->getName();
            qDebug() << "Creating an MDI subwindow";
            QMdiSubWindow *w = new QMdiSubWindow();

            w->setWindowTitle(title);
            w->resize(470, 370);
            mdi.addSubWindow(w);
            QChartView *chartView = createChart();
            w->setWidget(chartView);
            dom->setChartView(chartView);
        }
    }
    else
    {
        QMessageBox msgBox;
        msgBox.setText("No domains found");
        msgBox.exec();
    }

    /*
     * Connect signals for changing selection and double-click
     */
    connect(profileList, &QListWidget::currentItemChanged, this, &MainWindow::changeProfile);

    return domainNameList.count();
}

void MainWindow::showStatistics()
{
    if (!property_selected) {
        QMessageBox msgBox;
        msgBox.setText("Please select a property to display");
        msgBox.exec();
    }
    else
    {
        statsDialog->show();
    }
}

int MainWindow::loadProfileList()
{
    return 0;
}

int MainWindow::getPeriod()
{
    return _period;
}

#if 0
void MainWindow::drawChartRandomised()
{
    int seed = QTime(0,0,0).secsTo(QTime::currentTime());
    qDebug() << "MainWindow::drawChartRandomised(): reseeding with" << seed;
    qsrand(seed);
    drawChart(true, true);
}
#endif

void MainWindow::drawChartNormal()
{
    qDebug() << "Calling Domain::drawCharts() from drawChartNormal";
    Domain::drawCharts(propertyList);
}

int MainWindow::magnitude(double y)
{
    int x = (y == 0.0 ? -INT_MAX : (static_cast<int>(log10(abs(y)))));
    qDebug() << "MainWindow::magnitude(): magnitude of" << y << "is" << x;
    return x;
}

void MainWindow::updateStatsDialog(QListWidgetItem *current)
{
//    if (current == nullptr) {
//        return;
//    }

//    statsAction->setEnabled(true);

//    QSettings settings;
//    int range = settings.value("iterations", 100).toInt() + 1;

//    QListWidgetItem *it = current; //propertyList->currentItem();
//    QString key = it->text();
//    Behaviour::Property prop = propertyMap[key];
//    int ix = static_cast<int>(prop);
//    int min = _currentBehaviour->min_value(ix);
//    int max = _currentBehaviour->max_value(ix);
//    int total = _currentBehaviour->total(ix);
//    int mean = total / range;

//    statsDialog->setLimits(key, min, max, mean);

//    property_selected = true;
}

void MainWindow::changeProfile(QListWidgetItem *item)
{
    if (updatingProfileList) {
        return;
    }
    qDebug() << "MainWindow::changeProfile(QListWidgetItem *item) called: profile_changed ="
             << profile_changed;
    reloading = true;

    // Allow old profile to be saved if changed
    if (profile_changed) {
        createProfile();
        profile_changed = false;
        qDebug()  << "profile_changed now set to false";
    }

    qDebug() << "Changing profile";
    chartProfile = item->text();
    qDebug() << "New profile is" << chartProfile;

    // Load settings for this profile and redraw the chart
    QSettings settings;
    settings.setValue("current-profile", chartProfile);
    settings.beginGroup("Profiles");
    settings.beginGroup(chartProfile);

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

    Domain::drawCharts(propertyList);

    /*
     * A spin-off from drawCharts is that profile_changed gets set. We only
     * want it to be set in respnse to a user-initiated change, so we set it
     * back to false
     */
    profile_changed = false;
}

