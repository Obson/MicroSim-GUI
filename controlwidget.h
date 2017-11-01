#ifndef CONTROLWIDGET_H
#define CONTROLWIDGET_H

#include <QWidget>

namespace Ui {
class ControlWidget;
}

class ControlWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ControlWidget(QWidget *parent = 0);
    ~ControlWidget();

    void setStats(QString caption, int min_val, int max_val, int mean);
    void setNotes(QString);
    void updateStatus(QString);
    void setGini(double gini, double prod);

public slots:
    void statusChanged(QString status);
    void chartDrawn();

private slots:
    void on_btn_setup_clicked();
    void on_btn_redraw_clicked();
    void on_btn_close_clicked();

    void on_le_start_textEdited(const QString);
    void on_le_iters_textEdited(const QString);

    void on_btn_edit_model_clicked();

    void on_btnNewModel_clicked();

    void on_btn_random_clicked();

    void on_btn_profile_clicked();

    void on_lineEdit_textChanged(const QString &arg1);

signals:
    void newModelRequest();
    void setupModel();
    void redrawChart(bool rerun, bool randomised);
    void randomise();
    void closeDown();
    void newProfile(QString name);

private:
    Ui::ControlWidget *ui;

    int start_period;
    int iters;
};

#endif // CONTROLWIDGET_H
