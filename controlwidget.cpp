#include "controlwidget.h"
#include "ui_controlwidget.h"
#include <QDebug>
#include <QSettings>
#include "newmodeldlg.h"

ControlWidget::ControlWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ControlWidget)
{
    ui->setupUi(this);

    QSettings settings;
    start_period = settings.value("start-period", 1).toInt();
    iters = settings.value("iterations", 100).toInt();
    ui->le_start->setText(QString::number(start_period));
    ui->le_iters->setText(QString::number(iters));

    if (settings.value("current_model", "").toString().isEmpty()) {
        ui->label_status->setText("No model selected");
    }
}

ControlWidget::~ControlWidget()
{
    delete ui;
}

void ControlWidget::on_btn_setup_clicked()
{
    emit setupModel();
}

void ControlWidget::updateStatus(QString new_status)
{
    ui->label_status->setText("<h2>" + new_status + "</h2>");

    // Force the label to redraw before processing continues
    this->repaint();
    qApp->processEvents();
}

void ControlWidget::statusChanged(QString status)
{
    updateStatus(status);
}

void ControlWidget::on_btn_redraw_clicked()
{
    qDebug() << "ControlWidget::on_btn_redraw_clicked()";

    ui->btn_redraw->setEnabled(false);
    ui->btn_redraw->setDefault(false);

    bool ok;
    int _start_period = ui->le_start->text().toInt(&ok);

    if (ok)
    {
        int _iters = ui->le_iters->text().toInt(&ok);
        if (ok && start_period > 0 && iters >= 10 && iters + start_period <= 1000)
        {
            updateStatus(tr("Loading..."));

            QSettings settings;
            start_period = _start_period;
            iters = _iters;
            settings.setValue("start-period", start_period);
            settings.setValue("iterations", iters);
            emit redrawChart();
            return;
        }
    }

    // Not valid, so restore input fields
    ui->le_start->setText(QString::number(start_period));
    ui->le_iters->setText(QString::number(iters));
}

void ControlWidget::chartDrawn()
{
    QSettings settings;
    updateStatus(settings.value("current_model").toString());
}

void ControlWidget::on_btn_close_clicked()
{
    emit closeDown();
}

void ControlWidget::setStats(QString caption, int min_val, int max_val, int mean)
{
    ui->lab_property->setText(caption);
    ui->lab_minimum->setText(QString::number(min_val));
    ui->lab_maximum->setText(QString::number(max_val));
    ui->lab_mean->setText(QString::number(mean));
}

void ControlWidget::setNotes(QString s)
{
    ui->label_notes->setText(s);
}

void ControlWidget::on_le_start_textEdited(const QString)
{
    ui->btn_redraw->setEnabled(true);
    ui->btn_redraw->setDefault(true);
}

void ControlWidget::on_le_iters_textEdited(const QString)
{
    ui->btn_redraw->setEnabled(true);
    ui->btn_redraw->setDefault(true);
}

void ControlWidget::on_btn_edit_model_clicked()
{
    NewModelDlg *dlg = new NewModelDlg(this);
    dlg->setPreexisting();
    if (dlg->exec() == QDialog::Accepted)
    {
        // Update the notes
        ui->label_notes->setText(dlg->getNotes());
    }
}
