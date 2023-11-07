#ifndef MAINWINDOW_H
#define MAINWINDOW_H


#include <string.h>

#include <QMainWindow>
#include "parameterwizard.h"
#include <QList>
#include <QListWidgetItem>
#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>
#include <QValueAxis>
#include <QMenuBar>
#include <QAction>
#include <QLabel>
#include <QMdiArea>

//#include "account.h"
//#include "behaviour.h"
//#include "controlwidget.h"

#include "statsdialog.h"

namespace Ui {
class MainWindow;
}

/*
 * UPGRADE
 * -------
 * Parameters are used by Behaviours, Properties by Domains. Previously there
 * was no distinction as a Model had both and only one Model
 * could be active at any time.
 */


// This makes references to QCharts and related classes simpler.
QT_CHARTS_USE_NAMESPACE

//static QList<Domain*> domains;
//static Domain *currentDomain = nullptr;
//static Domain *defaultDomain = nullptr;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow();
    ~MainWindow() override;

    void show();


    static int magnitude(double);

signals:

    void clockTick(int period);

    void modelChanged(QString);
    void windowShown();
    void windowLoaded();
    void drawingCompleted();

protected:

    // These functions have been moved from the Behaviour class as they should
    // be global


    void run(bool randomised = false);
    void restart();
    int getStartPeriod();

    // Much of this should be transferred to Domain

    int _period;
    int _iterations;
    int _first_period;

    // Domain *getDomain(QString);

    int loadDomains(QListWidget *domainList);
    // int loadDomainList();
    int loadProfileList();

    int getIters();         // number of iterations (periods)
    int getPeriod();

    void saveCSV();
    void editParameters();
    void editModelDescription();
    //void createDefaultBehaviour();
    void createNewBehaviour();
    void createDomain();
    void createProfile();
    void removeProfile();
    void saveSettingsAsProfile(QString name);
    void reassignColours();
    void remove();
    void about();
    void aboutQt();
    void nyi();

    // errorMessage terminates the program
    void errorMessage [[noreturn]] (QString);

    void setOptions();
    void showWiki();
    void showStatistics();  // will replace showStats()
    //void showStats(QListWidgetItem *current, QListWidgetItem *prev);
    void updateStatsDialog(QListWidgetItem *current/*, QListWidgetItem *previous*/);

    void closeEvent(QCloseEvent *event) override;
    //void restoreState();

    void propertyChanged();

    QMenuBar *myMenuBar;

    QMenu *applicationMenu;
    QMenu *fileMenu;
    QMenu *editMenu;
    QMenu *viewMenu;
    QMenu *helpMenu;

    QAction *saveCSVAction;
    QAction *saveProfileAction;
    QAction *removeProfileAction;
    QAction *changeAction;
    QAction *coloursAction;
    QAction *newAction;
    QAction *domainAction;
    QAction *removeAction;
    QAction *notesAction;
    QAction *aboutAction;
    QAction *aboutQtAction;
    QAction *setOptionsAction;
    QAction *helpAction;
    QAction *statsAction;
    QAction *runAction;
    QAction *randomAction;
    QAction *closeAction;

    bool isBehaviourSelected();

private:

    Ui::MainWindow *ui;
    ParameterWizard *wiz;

    QMdiArea mdi;   // *****
    QList<QMdiSubWindow*> mdiWindowList;

    QList<Domain*> domains;

    QString chartProfile;

    StatsDialog *statsDialog;

    bool first_time_shown;
    bool first_time_loaded;

    bool profile_changed = false;
    bool reloading = false;
    bool updatingProfileList = false;
    bool property_selected = false;

    QMap<QString,Domain::Property> propertyMap;

    QChartView *createChart();

    void createActions();
    void createMenus();
    void createStatusBar();
    void createDockWindows();
    void drawChart(bool rerun, bool randomised = true);
    void drawChartRandomised();
    void drawChartNormal();

    QColor nextColour(int n);
    void selectProfile(QString text);

    void changeBehaviour(QListWidgetItem*);
    void changeProfile(QListWidgetItem*);
    void changeDomain(QListWidgetItem*);

    QList<QColor> colours;
    QMap<Domain::Property,QColor> propertyColours;

    QListWidgetItem *selectedBehaviourItem;

    QMenu *mainMenu;

    QChartView *chartView;
    QChart *chart;
    QListWidget *behaviourList;
    QListWidget *profileList;
    QListWidget *propertyList;

    QList<QString> domainNameList;

    QtCharts::QLineSeries period;
    QtCharts::QLineSeries gov_exp;
    QtCharts::QLineSeries bens_paid;
    QtCharts::QLineSeries gov_recpts;
    QtCharts::QLineSeries deficit;
    QtCharts::QLineSeries gov_bal;
    QtCharts::QLineSeries num_firms;
    QtCharts::QLineSeries num_emps;
    QtCharts::QLineSeries num_unemps;
    QtCharts::QLineSeries num_gov_emps;
    QtCharts::QLineSeries num_hired;
    QtCharts::QLineSeries num_fired;
    QtCharts::QLineSeries prod_bal;
    QtCharts::QLineSeries wages;
    QtCharts::QLineSeries consumption;
    QtCharts::QLineSeries bonuses;
    QtCharts::QLineSeries dedns;
    QtCharts::QLineSeries inc_tax;
    QtCharts::QLineSeries sales_tax;
    QtCharts::QLineSeries dom_bal;

    QValueAxis *axisX = nullptr;
    QValueAxis *axisY = nullptr;

    QLabel *productivityLabel;
    QLabel *inequalityLabel;
    QLabel *infoLabel;

    enum Opr
    {
        invalid_op,
        eq,
        neq,
        lt,
        gt,
        leq,
        geq
    };

    struct Condition
    {
        Domain::Property property = Domain::Property::zero;
        Opr opr;
        int val;            // possibly extend to allow expressions later
    };

    struct Pair
    {
        bool is_set = false;
        int val;
    };

    struct Params
    {
        Params();

        Condition condition;
        Pair invalid;
        Pair iters;
        Pair count;
        Pair emp_rate;
        Pair prop_con;
        Pair inc_tax_rate;
        Pair sales_tax_rate;
        Pair firm_creation_prob;
        Pair dedns;
        Pair unemp_ben_rate;
        Pair active_pop;
        Pair reserve;
        Pair prop_inv;
    };

    QList<Params*> paramList;
};

#endif // MAINWINDOW_H
