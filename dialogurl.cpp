#include "dialogurl.h"
#include "ui_dialogurl.h"

DialogURL::DialogURL(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogURL)
{
    ui->setupUi(this);
    ui->tableWidget->setColumnHidden(1,true);
}

DialogURL::~DialogURL()
{
    delete ui;
}

void DialogURL::closeEvent(QCloseEvent *event)
{
    hide();
    event->ignore();
}
