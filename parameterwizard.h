#ifndef PARAMETERWIZARD_H
#define PARAMETERWIZARD_H

#include <QWizard>
#include <QWizardPage>
#include <QSpinBox>
#include <QSettings>
#include <QTextEdit>
#include <QLineEdit>
#include <QListWidgetItem>

#include <QComboBox>
#include <QCheckBox>

class ParameterWizard : public QWizard
{
    //friend class StartPage;
    friend class DefaultPage;
    friend class ExtraPage;

public:
    ParameterWizard(QWidget *parent = Q_NULLPTR);
    QSpinBox *getSpinBox(int min, int max);

    void setCurrentModel(QString model_name);
    void importFrom(QString model_name);

    void done(int result);

private:
    enum PageType {
        default_page,
        extra_page
    };

protected:
    QSettings settings;
    QStringList props;
    QStringList rels;

    QStringList models;         // list of defined model names
    QString current_model;
    QString import_model;       // model to import settings from or empty

    int pid;                    // This is the CURRENT page id -- i.e. the id of
                                // the page currently being processed. Qt's
                                // currentId gives the next page id when changing
                                // pages, which isn't much help if we want to
                                // validate the page we are just leaving!

public slots:
    void currentIdChanged(int id);
    void createNewPage();
};

class DefaultPage : public QWizardPage
{
public:
    DefaultPage(ParameterWizard *w);
    void initializePage();
    bool validatePage();

private:
    void readSettings(QString model);

    ParameterWizard *wiz;

    QLineEdit *le_dir_exp_rate;
    QLineEdit *le_thresh;

    QSpinBox *sb_emp_rate;
    QSpinBox *sb_prop_con;
    QSpinBox *sb_dedns;
    QSpinBox *sb_inc_tax;
    QSpinBox *sb_sales_tax;
    QSpinBox *sb_bcr;           // business creation rate
    QSpinBox *sb_distrib;
    QSpinBox *sb_prop_inv;
    QSpinBox *sb_ubr;           // unempl benefit rate
    QSpinBox *sb_boe_loan_int;
    QSpinBox *sb_bus_loan_int;

    QComboBox *cb_loan_prob;

    QCheckBox *xb_locked;

    void toggleLock();
};

class ExtraPage : public QWizardPage
{
public:
    ExtraPage(ParameterWizard *w);

private:
    ParameterWizard *wiz;

};

#endif // PARAMETERWIZARD_H
