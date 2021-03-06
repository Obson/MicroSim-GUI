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

#include "model.h"

class ExtraPage;

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
    void setProperties(QMap<QString,Model::Property> map);

    void done(int result);
    int nextId() const;

private:
    enum PageType {
        default_page,
        extra_page
    };

protected:
    QSettings settings;
    QStringList prop_names;
    QStringList rels;
    //QList<Model::Property> properties;

    QMap<QString,Model::Property> property_map;

    QStringList models;         // list of defined model names
    QString current_model;
    QString import_model;       // model to import settings from or empty

    int pid;                    // This is the CURRENT page id -- i.e. the id of
                                // the page currently being processed. Qt's
                                // currentId gives the next page id when changing
                                // pages, which isn't much help if we want to
                                // validate the page we are just leaving!

    int next_id = -1;           // We set this to the id of a new page just before
                                // calling next. It's then picked up by our
                                // reimplemented nextid() to ensure that that's
                                // the page we go to. Iy must then be immediately
                                // reset to -1.

public slots:
    void currentIdChanged(int id);
    ExtraPage *createNewPage();
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
    QSpinBox *sb_recoup;        // time (periods) to recoup capex
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
    void setPageNumber(int page_num);
    bool validatePage();
    bool isChecked(QString model, QString attrib);
    void readSettings(QString model);
    QString getPropertyName(int prop);

    void showAssocCtrl(int);
    QCheckBox *createCheckBox();

private:
    QString readCondSetting(QString model, QString key);

    ParameterWizard *wiz;

    int pnum;

    QComboBox *cb_property;
    QComboBox *cb_rel;
    QLineEdit *le_value;

    QCheckBox *cbx_dir_exp_rate;
    QCheckBox *cbx_thresh;

    QCheckBox *cbx_prop_con;
    QCheckBox *cbx_dedns;
    QCheckBox *cbx_inc_tax;
    QCheckBox *cbx_sales_tax;
    QCheckBox *cbx_bcr;           // business creation rate
    QCheckBox *cbx_recoup;        // time (periods) to recoup capex
    QCheckBox *cbx_distrib;
    QCheckBox *cbx_prop_inv;
    QCheckBox *cbx_ubr;           // unempl benefit rate
    QCheckBox *cbx_boe_loan_int;
    QCheckBox *cbx_bus_loan_int;

    QCheckBox *cbx_loan_prob;

    QLineEdit *le_dir_exp_rate;
    QLineEdit *le_thresh;

    QSpinBox *sb_prop_con;
    QSpinBox *sb_dedns;
    QSpinBox *sb_inc_tax;
    QSpinBox *sb_sales_tax;
    QSpinBox *sb_bcr;           // business creation rate
    QSpinBox *sb_recoup;        // time (periods) to recoup capex
    QSpinBox *sb_distrib;
    QSpinBox *sb_prop_inv;
    QSpinBox *sb_ubr;           // unempl benefit rate
    QSpinBox *sb_boe_loan_int;
    QSpinBox *sb_bus_loan_int;

    QComboBox *cb_loan_prob;

};

#endif // PARAMETERWIZARD_H
