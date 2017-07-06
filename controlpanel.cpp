#include "controlpanel.h"
#include "ui_controlpanel.h"

ControlPanel::ControlPanel(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ControlPanel)
{
    ui->setupUi(this);
    QPalette pal(palette());
    pal.setColor(QPalette::Background,QColor(0,0,0,0));
    setPalette(pal);
    ui->pushButtonSkipB->setIcon(style()->standardIcon(QStyle::SP_MediaSkipBackward));
    ui->pushButtonSeekB->setIcon(style()->standardIcon(QStyle::SP_MediaSeekBackward));
    ui->pushButtonPlay->setIcon(style()->standardIcon(QStyle::SP_MediaPlay));
    ui->pushButtonSkipF->setIcon(style()->standardIcon(QStyle::SP_MediaSkipForward));
    ui->pushButtonSeekF->setIcon(style()->standardIcon(QStyle::SP_MediaSeekForward));
    ui->pushButtonStop->setIcon(style()->standardIcon(QStyle::SP_MediaStop));
    ui->pushButtonMute->setIcon(style()->standardIcon(QStyle::SP_MediaVolume));
}

ControlPanel::~ControlPanel()
{
    delete ui;
}
