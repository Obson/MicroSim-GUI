#include "controlwidget.h"
#include "ui_controlwidget.h"
#include <QDebug>

ControlWidget::ControlWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ControlWidget)
{
    ui->setupUi(this);
}

ControlWidget::~ControlWidget()
{
    delete ui;
}

void ControlWidget::on_btn_setup_clicked()
{
    qDebug() << "ControlWidget::on_btn_setup_clicked()";
    emit setupModel();
}

void ControlWidget::on_btn_redraw_clicked()
{
    qDebug() << "ControlWidget::on_btn_redraw_clicked()";
    emit redrawChart();
}

void ControlWidget::on_btn_close_clicked()
{
    qDebug() << "ControlWidget::on_btn_close_clicked()";
    emit closeDown();
}

void ControlWidget::setStats(QString caption, int min_val, int max_val, int mean)
{
    ui->lab_property->setText(caption);
    ui->lab_minimum->setText(QString::number(min_val));
    ui->lab_maximum->setText(QString::number(max_val));
    ui->lab_mean->setText(QString::number(mean));
}
