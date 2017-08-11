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

private slots:
    void on_btn_setup_clicked();

    void on_btn_redraw_clicked();

    void on_btn_close_clicked();

signals:
    void setupModel();
    void redrawChart();
    void closeDown();

private:
    Ui::ControlWidget *ui;
};

#endif // CONTROLWIDGET_H
