#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "parameterwizard.h"
#include <QList>
#include <QListWidgetItem>
#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>
#include <QValueAxis>

#include "model.h"

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
    ~MainWindow();

signals:
    void modelChanged(QString);

protected:

    Model *current_model();

    int loadModelList();

    int getIters();         // number of iterations (periods)
    int getPopSize();       // max available population size
    int getActivePop();     // target size of economically active population
    int getGovExpRate();    // government expenditure (currency units per period)
    int getStdWage();       // standard wage (currency units per employee per period)
    int getPropCon();       // propensity to consume (%)
    int getIncTaxRate();    // income tax rate (%)
    int getSalesTaxRate();  // sales tax rate (%)
    int getPreTaxDedns();   // pre-tax deductions (%)
    int getFCP();           // firm creaton probability (%)
    int getUBR();           // unemployment benefit rate (% of std wage)
    int getPropInv();       // propensity to invest
    int getReserve();       // funds kept in reserve for next period (%)

    void clearModels();
    void saveCVS();
    void copy();
    void editParameters();
    void createNewModel();
    void remove();
    void about();
    void aboutQt();
    void nyi();
    void errorMessage(QString);

    void propertyChanged(QListWidgetItem *item);

    QMenu *fileMenu;
    QMenu *editMenu;
    QMenu *helpMenu;

    QAction *clearModelsAction;
    QAction *saveCVSAction;
    QAction *copyAction;
    QAction *changeAction;
    QAction *newAction;
    QAction *removeAction;
    QAction *aboutAction;
    QAction *aboutQtAction;

    bool isModelSelected();

private:
    Ui::MainWindow *ui;
    ParameterWizard *wiz;
    Model *_current_model;

    QStringList prop_names;

    QMap<QString,Model::Property> property_map;

    void createActions();
    void createMenus();
    void createStatusBar();
    void createDockWindows();
    void drawChart();
    QColor nextColour(int n);

    void changeModel(QListWidgetItem*);

    QList<QColor> colours;

    QListWidgetItem *selectedModelItem;

    QMenu *mainMenu;

    QChartView *chartView;
    QChart *chart;
    QListWidget *modelList;
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

    enum Property
    {
        pr_dflt,
        pr_current_period,
        pr_gov_exp,
        pr_bens_paid,
        pr_gov_recpts,
        pr_deficit,
        pr_gov_bal,
        pr_num_firms,
        pr_num_emps,
        pr_num_unemps,
        pr_num_gov_emps,
        pr_num_hired,
        pr_num_fired,
        pr_prod_bal,
        pr_wages,
        pr_consumption,
        pr_bonuses,
        pr_dedns,
        pr_inc_tax,
        pr_sales_tax,
        pr_dom_bal
    };

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
        Property property = pr_dflt;
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
        Pair std_wage;
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
