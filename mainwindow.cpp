#include "mainwindow.h"
#include "newmodeldlg.h"
#include <QtWidgets>
#include <QMessageBox>
#include <QDebug>
#include <QtCharts/QChart>
#include <QDockWidget>
#include <QValueAxis>
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

MainWindow::MainWindow()
{
    _current_model = nullptr;
    first_time_shown = true;

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
    property_map[tr("Business sector balance")] = Model::Property::prod_bal;
    property_map[tr("Wages paid")] = Model::Property::wages;
    property_map[tr("Consumption")] = Model::Property::consumption;
    property_map[tr("Bonuses paid")] = Model::Property::bonuses;
    property_map[tr("Pre-tax deductions")] = Model::Property::dedns;
    property_map[tr("Income tax paid")] = Model::Property::inc_tax;
    property_map[tr("Sales tax paid")] = Model::Property::sales_tax;
    property_map[tr("Domestic sector balance")] = Model::Property::dom_bal;
    property_map[tr("Bank loans")] = Model::Property::amount_owed;
    property_map[tr("Average business size")] = Model::Property::bus_size;
    property_map[tr("100 reference line")] = Model::Property::hundred;
    property_map[tr("Procurement expenditure")] = Model::Property::procurement;
    property_map[tr("Productivity")] = Model::Property::productivity;
    property_map[tr("Productivity (relative)")] = Model::Property::rel_productivity;
    property_map[tr("Govt direct support")] = Model::Property::unbudgeted;
    property_map[tr("Zero reference line")] = Model::Property::zero;

    // If non-zero, points to currently selected listwidget item
    selectedModelItem = 0;

    // Make sure preferences exist
    QSettings settings;
    qDebug() << "Settings are in" << settings.fileName();

    if (!settings.contains("iterations"))
    {
        settings.setValue("iterations", 99);
        settings.setValue("start-period", 1);
        settings.setValue("startups", 10);
        settings.setValue("nominal-population", 1000);
        settings.setValue("unit-wage", 100);
        settings.setValue("government-employees", 200); // approx tot pop / 5
        //settings.setValue("sample-size", 10); // for moving averages -- adjust as necessary
    }

    createChart();
    createActions();
    createMenus();
    createStatusBar();
    createDockWindows();

    setWindowTitle(tr("MicroSim"));
    setWindowIcon(QIcon(":/microsim.icns"));
    setUnifiedTitleAndToolBarOnMac(true);
    setMinimumSize(1280, 800);
    resize(1280, 800);

    // The left margin is to prevent the control being right against the side
    // of the window. May not really be a good idea. The border and padding
    // definitely improve things though.
    setStyleSheet("QListWidget{margin-left: 2px; border: 0px; padding:12px;}");
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

    colours << Qt::red << Qt::blue << Qt::darkRed << Qt::darkGreen
            << Qt::darkBlue << Qt::darkMagenta << Qt::darkYellow
            << Qt::darkCyan;

    setCentralWidget(chartView);
}

void MainWindow::createActions()
{
    saveCSVAction = new QAction(tr("&Save as CSV file..."), this);
    saveCSVAction->setDisabled(!isModelSelected());
    connect(saveCSVAction, &QAction::triggered, this,
            &MainWindow::saveCSV);

    changeAction = new QAction(tr("Edit &parameters..."));
    changeAction->setDisabled(!isModelSelected());
    connect(changeAction, &QAction::triggered, this,
            &MainWindow::editParameters);

    newAction = new QAction(tr("&New model..."), this);
    newAction->setStatusTip(tr("Create a new model"));
    connect(newAction, &QAction::triggered, this, &MainWindow::createNewModel);

    removeAction = new QAction(tr("&Remove models..."));
    connect(removeAction, &QAction::triggered, this, &MainWindow::remove);

    setOptionsAction = new QAction(tr("&Preferences"));
    connect(setOptionsAction, &QAction::triggered, this, &MainWindow::setOptions);

    aboutAction = new QAction(tr("&About"), this);
    connect(aboutAction, &QAction::triggered, this, &MainWindow::about);

    aboutQtAction = new QAction(tr("A&bout Qt"), this);
    connect(aboutQtAction, &QAction::triggered, this, &MainWindow::aboutQt);

    helpAction = new QAction(tr("Open &wiki in browser"), this);
    connect(helpAction, &QAction::triggered, this, &MainWindow::showWiki);

    connect(this, &MainWindow::windowShown, this, &MainWindow::createFirstModel);
    connect(this, &MainWindow::windowLoaded, this, &MainWindow::restoreState);
}

void MainWindow::createMenus()
{
    myMenuBar = new QMenuBar(0);

    applicationMenu = myMenuBar->addMenu(tr("&MicroSim"));

    fileMenu = myMenuBar->addMenu(tr("&File"));
    fileMenu->addAction(newAction);
    fileMenu->addAction(removeAction);
    fileMenu->addSeparator();
    fileMenu->addAction(saveCSVAction);

    editMenu = myMenuBar->addMenu(tr("&Edit"));
    editMenu->addAction(changeAction);
    setOptionsAction->setMenuRole(QAction::ApplicationSpecificRole);
    editMenu->addAction(setOptionsAction);

    helpMenu = myMenuBar->addMenu(tr("&Help"));
    applicationMenu->addAction(aboutAction);
    helpMenu->addAction(aboutQtAction);
    helpMenu->addAction(helpAction);

    setMenuBar(myMenuBar);
}

Model *MainWindow::current_model()
{
    return _current_model;
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
    RemoveModelDlg dlg;
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
        ctrl->setGini(mod->getGini(), mod->getProductivity());
    }
    propertyChanged();
}

void MainWindow::about()
{
    QMessageBox::about(
                this,
                "About MicroSim",
                "MicroSim version " + QString(VERSION)
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
    QString message = tr("No model has been selected");
    statusBar()->showMessage(message);
}

void MainWindow::propertyChanged()
{
    qDebug() << "MainWindow::propertyChanged";
    // Allow the property to be changed even when there's no model selected.
    if (_current_model != nullptr)
    {
        drawChart(false);   // no need to rerun
    }
}

void MainWindow::createDockWindows()
{
    // Create property list
    QDockWidget *dock = new QDockWidget(tr("Properties"), this);
    dock->setAllowedAreas(Qt::RightDockWidgetArea);
    propertyList = new QListWidget(dock);

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
    connect(propertyList, &QListWidget::currentItemChanged, this, &MainWindow::showStats);

    // Add to dock
    dock->setWidget(propertyList);
    dock->setFeatures(QDockWidget::NoDockWidgetFeatures);
    addDockWidget(Qt::RightDockWidgetArea, dock);

    // Create the model list
    dock = new QDockWidget(tr("Models"), this);
    dock->setAllowedAreas(Qt::LeftDockWidgetArea);
    modelList = new QListWidget(dock);

    // Populate the model list
    loadModelList();

    // Add to dock
    dock->setWidget(modelList);
    dock->setFeatures(QDockWidget::NoDockWidgetFeatures);
    addDockWidget(Qt::LeftDockWidgetArea, dock);

    // Create the bottom area
    dock = new QDockWidget("Controls and Statistics", this);
    dock->setFeatures(QDockWidget::NoDockWidgetFeatures);
    ctrl = new ControlWidget(this);
    ctrl->setFixedHeight(120);
    dock->setWidget(ctrl);
    dock->setAllowedAreas(Qt::BottomDockWidgetArea);
    addDockWidget(Qt::BottomDockWidgetArea, dock);

    // Create the parameter wizard
    wiz = new ParameterWizard(this);
    wiz->setProperties(property_map);
    wiz->setModal(true);

    // Connect signals for changing selection and double-click
    connect(modelList, &QListWidget::currentItemChanged, this, &MainWindow::changeModel);
    connect(modelList, &QListWidget::itemDoubleClicked, this, &MainWindow::editParameters);

    // Signals from bottom-area buttons
    connect(ctrl, &ControlWidget::setupModel, this, &MainWindow::editParameters);
    connect(ctrl, &ControlWidget::closeDown, this, &MainWindow::close);
    connect(ctrl, &ControlWidget::redrawChart, this, &MainWindow::drawChart);
    connect(ctrl, &ControlWidget::randomise, this, &MainWindow::drawChartRandomised);
    connect(ctrl, &ControlWidget::newModelRequest, this, &MainWindow::createNewModel);

    // Signal to bottom-area
    connect(this, &MainWindow::drawingCompleted, ctrl, &ControlWidget::chartDrawn);
}

void MainWindow::showStats(QListWidgetItem *current, QListWidgetItem *prev)
{
    // NOTE: This function seems to be triggered at the start even if the user
    // hasn't selected an item. In which case prev has a zero value so it seems
    // to be safe to use it to indicate this condition, and as no item has been
    // selected we cannot show any stats. This isn't quite right as it prevents
    // stats for the first selection being displayed if it's the top item in the
    // list, but at least it doesn't display nonsense.
    if (prev == nullptr) {
        return;
    }

    QSettings settings;
    int range = settings.value("iterations", 100).toInt();

    QListWidgetItem *it = current; // propertyList->currentItem();
    QString key = it->text();
    Model::Property prop = property_map[key];
    int ix = static_cast<int>(prop);
    int min = _current_model->min_value(ix);
    int max = _current_model->max_value(ix);
    int total = _current_model->total(ix);
    int mean = total / range;

    ctrl->setStats("<b>" + key + "</b>", min, max, mean);
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

void MainWindow::drawChart(bool rerun, bool randomised)    // uses _current_model
{
    ctrl->updateStatus("Loading");

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
        ctrl->setGini(_current_model->getGini(), _current_model->getProductivity());
    }

    chart->legend()->show();
    chart->setTitle("<h2>" + _current_model->name() + "</h2>");

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

    emit drawingCompleted();
}

void MainWindow::changeModel(QListWidgetItem *item)
{
    if (reloading)
    {
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
    statusBar()->showMessage(  tr("  Total population: ") + QString::number(nominal_population)
                             + tr("  Government employees: ") + settings.value("government-employees", 200).toString()
                             + tr("  Standard wage: ") + settings.value("unit-wage", 500).toString()
                             + tr("  Number of businesses at start: ") + QString::number(startups)
                            );

    settings.setValue("current_model", model_name);
    settings.beginGroup(model_name);
    ctrl->setNotes(settings.value("notes", "No notes entered for this model").toString());
    settings.endGroup();

    drawChart(true, false);
}

