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

signals:
    void newModelRequest();
    void setupModel();
    void redrawChart();
    void closeDown();

private:
    Ui::ControlWidget *ui;

    int start_period;
    int iters;
};

#endif // CONTROLWIDGET_H
