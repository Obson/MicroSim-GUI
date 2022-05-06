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

#include "behaviour.h"
//#include "controlwidget.h"
#include "statsdialog.h"

namespace Ui {
class MainWindow;
}

// This makes references to QCharts and related classes simpler.
QT_CHARTS_USE_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow();
    ~MainWindow() override;

    void show();

    static int magnitude(double);

signals:
    void modelChanged(QString);
    void windowShown();
    void windowLoaded();
    void drawingCompleted();

protected:

    Behaviour *currentBehaviour();

    int loadBehaviourList();
    int loadProfileList();

    int getIters();         // number of iterations (periods)
    int getPopSize();       // max available population size
    int getActivePop();     // target size of economically active population
    int getGovExpRate();    // government expenditure (currency units per period)
    int getStdWage();       // standard wage (currency units per employee per period)
    int getPropCon();       // propensity to consume (%)
    int getIncTaxRate();    // income tax rate (%)
    int getSalesTaxRate();  // sales tax rate (%)
    int getFCP();           // firm creaton probability (%)
    int getUBR();           // unemployment benefit rate (% of std wage)
    int getPropInv();       // propensity to invest
    int getReserve();       // funds kept in reserve for next period (%)

    void saveCSV();
    void editParameters();
    void editModelDescription();
    void createDefaultBehaviour();
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
    void restoreState();

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
    Behaviour *_currentBehaviour;

    QString currentProfile;

    StatsDialog *statsDialog;

    bool first_time_shown;
    bool first_time_loaded;

    bool profile_changed = false;
    bool reloading = false;
    bool updatingProfileList = false;
    bool property_selected = false;

    QMap<QString,Behaviour::Property> propertyMap;

    void createChart();
    void createActions();
    void createMenus();
    void createStatusBar();
    void createDockWindows();
    void drawChart(bool rerun, bool randomised = true);
    void drawChartRandomised();
    void drawChartNormal();

    QColor nextColour(int n);
    void selectProfile(QString text);

    void changeModel(QListWidgetItem*);
    void changeProfile(QListWidgetItem*);

    QList<QColor> colours;
    QMap<Behaviour::Property,QColor> propertyColours;

    QListWidgetItem *selectedBehaviourItem;

    QMenu *mainMenu;

    QChartView *chartView;
    QChart *chart;
    QListWidget *behaviourList;
    QListWidget *profileList;
    QListWidget *propertyList;

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
        Behaviour::Property property = Behaviour::Property::zero;
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
    QList<Domain*> domainList;
};

#endif // MAINWINDOW_H
