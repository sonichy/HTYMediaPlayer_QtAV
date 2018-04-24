#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFileDialog>
#include <QDesktopWidget>
#include <QShortcut>
#include <QMessageBox>
#include <QKeyEvent>
#include <QTime>
#include <QTimer>
#include <QDebug>
#include <math.h>
#include <QInputDialog>
#include <QGraphicsView>
#include <QTableWidget>
#include <QDesktopServices>
#include <QStandardPaths>
#include <QTextBrowser>
#include <QMimeData>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkReply>
#include <QtNetwork/QNetworkRequest>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <stdio.h>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    m_bPressed = false;
    mode = "";
    ui->setupUi(this);

    QPalette pal(palette());
    pal.setColor(QPalette::Background, Qt::black);
    setPalette(pal);
    if (!m_preview)
        m_preview = new VideoPreviewWidget();

    qApp->installEventFilter(this);
    CP = new ControlPanel(this);
    CP->move(0,height()-CP->height());
    CP->resize(width()-ui->tableWidget->width(),CP->height());
    sr = 1;
    QActionGroup *scaleGroup = new QActionGroup(this);
    scaleGroup->addAction(ui->action_scale0_5);
    scaleGroup->addAction(ui->action_scale1);
    scaleGroup->addAction(ui->action_scale1_5);
    scaleGroup->addAction(ui->action_scale2);

    connect(new QShortcut(QKeySequence(Qt::Key_O),this), SIGNAL(activated()),this, SLOT(on_action_open_triggered()));
    connect(new QShortcut(QKeySequence(Qt::Key_U),this), SIGNAL(activated()),this, SLOT(on_action_openURL_triggered()));
    connect(new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_I),this), SIGNAL(activated()),this, SLOT(on_action_info_triggered()));
    connect(new QShortcut(QKeySequence(Qt::Key_Escape),this), SIGNAL(activated()),this, SLOT(exitFullscreen()));
    connect(new QShortcut(QKeySequence(Qt::Key_Return),this), SIGNAL(activated()),this, SLOT(EEFullscreen()));
    connect(new QShortcut(QKeySequence(Qt::Key_Enter),this), SIGNAL(activated()),this, SLOT(EEFullscreen()));
    connect(new QShortcut(QKeySequence(Qt::Key_Space),this), SIGNAL(activated()),this, SLOT(playPause()));
    connect(new QShortcut(QKeySequence(Qt::Key_Left),this), SIGNAL(activated()),this, SLOT(seekBackward()));
    connect(new QShortcut(QKeySequence(Qt::Key_Right),this), SIGNAL(activated()),this, SLOT(seekForward()));
    connect(new QShortcut(QKeySequence(Qt::Key_Up),this), SIGNAL(activated()),this, SLOT(on_action_volumeUp_triggered()));
    connect(new QShortcut(QKeySequence(Qt::Key_Down),this), SIGNAL(activated()),this, SLOT(on_action_volumeDown_triggered()));
    connect(new QShortcut(QKeySequence(Qt::Key_M),this), SIGNAL(activated()),this, SLOT(on_action_volumeMute_triggered()));
    connect(new QShortcut(QKeySequence(Qt::Key_P),this), SIGNAL(activated()),this, SLOT(on_action_capture_triggered()));
    connect(new QShortcut(QKeySequence(Qt::Key_R),this), SIGNAL(activated()),this, SLOT(on_action_rotateCW_triggered()));
    connect(new QShortcut(QKeySequence(Qt::Key_L),this), SIGNAL(activated()),this, SLOT(on_action_rotateCCW_triggered()));
    connect(new QShortcut(QKeySequence(Qt::Key_1),this), SIGNAL(activated()),this, SLOT(on_action_scale1_triggered()));
    connect(new QShortcut(QKeySequence(Qt::Key_2),this), SIGNAL(activated()),this, SLOT(on_action_scale2_triggered()));
    connect(new QShortcut(QKeySequence(Qt::Key_3),this), SIGNAL(activated()),this, SLOT(on_action_scale1_5_triggered()));
    connect(new QShortcut(QKeySequence(Qt::Key_4),this), SIGNAL(activated()),this, SLOT(on_action_scale0_5_triggered()));
    connect(new QShortcut(QKeySequence(Qt::Key_5),this), SIGNAL(activated()),this, SLOT(fitDesktop()));
    connect(new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_Left),this), SIGNAL(activated()),this, SLOT(skipBackward()));
    connect(new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_Right),this), SIGNAL(activated()),this, SLOT(skipForward()));
    connect(new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_Up),this), SIGNAL(activated()),this, SLOT(speedUp()));
    connect(new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_Down),this), SIGNAL(activated()),this, SLOT(speedDown()));
    connect(CP->ui->pushButtonSkipB, SIGNAL(pressed()), this, SLOT(skipBackward()));
    connect(CP->ui->pushButtonSeekB, SIGNAL(pressed()), this, SLOT(seekBackward()));
    connect(CP->ui->pushButtonPlay, SIGNAL(pressed()), this, SLOT(playPause()));
    connect(CP->ui->pushButtonStop, SIGNAL(pressed()), this, SLOT(stop()));
    connect(CP->ui->pushButtonSeekF, SIGNAL(pressed()), this, SLOT(seekForward()));
    connect(CP->ui->pushButtonSkipF, SIGNAL(pressed()), this, SLOT(skipForward()));
    connect(CP->ui->pushButtonMute, SIGNAL(pressed()), this, SLOT(on_action_volumeMute_triggered()));
    connect(CP->ui->pushButtonFullscreen, SIGNAL(pressed()), this, SLOT(EEFullscreen()));
    connect(CP->ui->pushButtonList, SIGNAL(pressed()), this, SLOT(showHideList()));

    player = new AVPlayer(this);
    videoItem = new GraphicsItemRenderer;
    QGraphicsScene *scene = new QGraphicsScene(this);
    scene->addItem(videoItem);
    ui->graphicsView->setScene(scene);
    player->setRenderer(videoItem);

    connect(player, SIGNAL(started()), SLOT(durationChange()));
    connect(player,SIGNAL(positionChanged(qint64)),this,SLOT(positionChange(qint64)));
    connect(player->audio(), SIGNAL(volumeChanged(qreal)), this, SLOT(volumeChange(qreal)));
    connect(player->videoCapture(), SIGNAL(imageCaptured(QImage)), SLOT(saveImage(QImage)));
    connect(player, SIGNAL(error(QtAV::AVError)), this, SLOT(handleError(QtAV::AVError)));
    connect(player,SIGNAL(stateChanged(QtAV::AVPlayer::State)),this,SLOT(stateChange(QtAV::AVPlayer::State)));

    labelTL=new QLabel(this);
    labelTL->setStyleSheet("color:white;font:20px;background:transparent;");
    labelTL->move(50,50);

    desktop = QApplication::desktop();
    move((desktop->width() - width())/2, (desktop->height() - height())/2);

    connect(CP->ui->sliderProgress,SIGNAL(sliderPressed()),this,SLOT(sliderProgressPressed()));
    connect(CP->ui->sliderProgress,SIGNAL(sliderReleased()),this,SLOT(sliderProgressReleased()));
    //connect(CP->ui->sliderProgress,SIGNAL(sliderMoved(int)),this,SLOT(sliderProgressMoved(int)));
    connect(CP->ui->sliderVolume,SIGNAL(sliderMoved(int)),this,SLOT(sliderVolumeMoved(int)));

    createPopmenu();

    ui->tableWidget->setColumnHidden(1,true);
    ui->tableWidget->setStyleSheet("QTableWidget{color:white; background-color:black;} QTableWidget::item:selected{background-color:#222222;}");    
    connect(ui->tableWidget,SIGNAL(cellClicked(int,int)),this,SLOT(playTV(int,int)));
    fillTable("tv.txt");

    QStringList Largs = QApplication::arguments();
    qDebug() << Largs;
    if(Largs.length()>1){
        if(!Largs.at(1).contains("chrome-extension://")){
            //QString path = Largs.at(1);
            //open(path.replace("file://",""));
            qDebug() << "args:" << Largs.at(1);
            open(Largs.at(1));
        }else{
            // 下面这段如果在没有参数时使用，会导致调试时窗口出不来和从外部程序启动打开文件中断
            // 接收Chrome扩展传来的数据
            int length = 0;
            //read the first four bytes (=> Length)
            //getwchar: receive char from stdin
            //putwchar: write char to stdout
            for ( int i = 0; i < 4; i++) {
                length += getwchar();
            }
            //read the json-message
            QString url = "";
            for ( int i = 0; i < length; i++) {
                url += getwchar();
            }
            //浏览器端传来的数据会有一个双引号引在两端
            url = url.mid(1, url.length()-2);
            qDebug() << "chrome-extension://" << url;
            if(url!=""){
                ui->tableWidget->hide();
                player->play(url);
                addHistory(url);
                setWindowTitle(url);
            }
        }
    }

    dialogUrl = new DialogURL(this);
    connect(dialogUrl->ui->pushButtonAnalyze,SIGNAL(pressed()),this,SLOT(analyze()));
    connect(dialogUrl->ui->tableWidget,SIGNAL(cellClicked(int,int)),this,SLOT(playURL(int,int)));    

}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_action_open_triggered()
{
    if(filename==""){
        filename = QFileDialog::getOpenFileName(this, "打开媒体文件", ".");
    }else{
        filename = QFileDialog::getOpenFileName(this, "打开媒体文件", filename);
    }
    if(!filename.isEmpty()){
        open(filename);
    }
}

void MainWindow::open(QString path)
{
    player->play(path);
    addHistory(path);
    CP->ui->pushButtonPlay->setIcon(style()->standardIcon(QStyle::SP_MediaPause));    
    setWindowTitle(QFileInfo(path).fileName());
    ui->tableWidget->hide();
}

void MainWindow::on_action_openURL_triggered()
{
    dialogUrl->show();
}

void MainWindow::on_action_liveList_triggered()
{
    showHideList();
}

void MainWindow::showHideList()
{
    if(ui->tableWidget->isVisible()){
        ui->tableWidget->setVisible(false);
        //resize(QSize(width()-ui->tableWidget->width(),height()));
        CP->resize(this->width(),CP->height());
    }else{
        ui->tableWidget->setVisible(true);
        //resize(QSize(width()+ui->tableWidget->width(),height()));
        CP->resize(this->width()-ui->tableWidget->width(),CP->height());
    }
}

void MainWindow::on_action_liveImport_triggered()
{
    if(filename==""){
        filename = QFileDialog::getOpenFileName(this, "打开文本", "tv.txt", "文本文件(*.txt)");
    }else{
        filename = QFileDialog::getOpenFileName(this, "打开文本", filename, "文本文件(*.txt)");
    }
    if(!filename.isEmpty()){
        fillTable(filename);
    }
}

void MainWindow::on_action_quit_triggered()
{
    player->stop();
    qApp->quit();
}

void MainWindow::on_action_fullscreen_triggered()
{
    EEFullscreen();
}

void MainWindow::on_action_info_triggered()
{
    QString SInfo = "媒体地址：" + player->statistics().url + "\n视频解码：" + player->statistics().video.decoder_detail + "\n音频解码：" + player->statistics().audio.decoder_detail +"\n分辨率：" + QString::number(player->statistics().video_only.width) + " X " + QString::number(player->statistics().video_only.height) + "\n文件大小：" + SB(QFileInfo(player->statistics().url).size());
    QMessageBox aboutMB(QMessageBox::NoIcon, "视频信息", SInfo);
    aboutMB.exec();
}

void MainWindow::on_action_capture_triggered()
{
    cn=1;
    player->videoCapture()->setAutoSave(true);
    //自动保存在 /home/用户名/Pictures ，不需要手动保存。
    player->videoCapture()->capture();
#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
    labelTL->setText("截图 " + QDesktopServices::storageLocation(QDesktopServices::PicturesLocation));
#else
    labelTL->setText("截图 " + QStandardPaths::writableLocation(QStandardPaths::PicturesLocation));
#endif
    labelTL->adjustSize();
    labelTL->show();
    QTimer::singleShot(3000,this,SLOT(timeoutTL()));
}

QString MainWindow::SB(qint64 b)
{
    QString s="";
    if(b>999999999){
        s=QString::number(b/(1024*1024*1024.0),'f',2)+"GB";
    }else{
        if(b>999999){
            s=QString::number(b/(1024*1024.0),'f',2)+"MB";
        }else{
            if(b>999){
                s=QString::number(b/1024.0,'f',2)+"KB";
            }else{
                s=QString::number(b/1.0,'f',2)+"B";
            }
        }
    }
    return s;
}

void MainWindow::on_action_capture16_triggered()
{
    cn = 16;
    cnn = 0;
    player->pause(false);
    player->audio()->setMute(true);
    player->videoCapture()->setAutoSave(false);
    QImage imageDS = QImage(1280+50,960+80+40,QImage::Format_RGB32);
    imageDS.fill(Qt::white);
    int k = 0;
    QPainter painter(&imageDS);
    painter.drawPixmap(20,10,QPixmap("icon.png").scaled(60,60));
    painter.drawText(300,30,"文件名称：" + QFileInfo(filename).fileName());
    painter.drawText(300,60,"文件大小：" + SB(QFileInfo(filename).size()));
    painter.drawText(500,60,"视频尺寸：" + QString::number(player->statistics().video_only.width) + " X " + QString::number(player->statistics().video_only.height));
    QTime t(0,0,0);
    t = t.addMSecs(player->duration());
    QString STimeTotal = t.toString("hh:mm:ss");
    painter.drawText(700,60,"视频时长：" + STimeTotal);
    for( qint64 i = player->duration()/16; i < player->duration(); i += player->duration()/16 ) {
        player->setPosition(i);
        player->videoCapture()->capture();
        QEventLoop eventloop;
        QTimer::singleShot(1000, &eventloop, SLOT(quit()));
        eventloop.exec();
    }
    for(int y=0; y<4; y++ ) {
        for(int x=0; x<4; x++){
            //QImage imgtemp=imagec[k].scaled(320,240);
            painter.drawImage(x*(320+10)+10,y*(240+10)+80,imagec[k]);
            k++;
        }
    }
    QFont font("Arial",20,QFont::Normal,false);
    painter.setFont(font);
    painter.drawText(100,50,"海天鹰播放器");
#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
    QString filepath = QDesktopServices::openUrl(QDesktopServices::storageLocation(QDesktopServices::PicturesLocation))+ "/Summary_"+QFileInfo(filename).baseName()+".jpg";
#else
    QString filepath = QStandardPaths::writableLocation(QStandardPaths::DesktopLocation) + "/Summary_" + QFileInfo(filename).baseName()+".jpg";
#endif
    imageDS.save(filepath,0,100);
    player->audio()->setMute(false);
    labelTL->setText("剧情连拍 " + filepath);
    labelTL->adjustSize();
    labelTL->show();
    QTimer::singleShot(3000,this,SLOT(timeoutTL()));
}

void MainWindow::on_action_captureDirectory_triggered(){
#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
    QDesktopServices::openUrl(QDesktopServices::storageLocation(QDesktopServices::PicturesLocation));
#else
    QDesktopServices::openUrl(QStandardPaths::writableLocation(QStandardPaths::PicturesLocation));
#endif
}

void MainWindow::on_action_rotateCW_triggered()
{
    videoItem->setOrientation(videoItem->orientation()+90);
}

void MainWindow::on_action_rotateCCW_triggered(){
    videoItem->setOrientation(videoItem->orientation()-90);
}

void MainWindow::on_action_volumeUp_triggered()
{
    qreal vol=player->audio()->volume();
    if(vol<2){
        vol+=0.01;
        player->audio()->setVolume(vol);        
        CP->ui->sliderVolume->setValue(vol*100);
        qDebug() << vol;
    }else{
        labelTL->show();
        QTimer::singleShot(3000,this,SLOT(timeoutTL()));
    }
}

void MainWindow::on_action_volumeDown_triggered()
{
    qreal vol=player->audio()->volume()-0.01;
    player->audio()->setVolume(vol);    
    CP->ui->sliderVolume->setValue(vol*100);
    qDebug() << vol;
    if(vol<0){
        labelTL->show();
        QTimer::singleShot(3000,this,SLOT(timeoutTL()));
    }
}

void MainWindow::on_action_volumeMute_triggered()
{
    if(player->audio()->isMute()){
        player->audio()->setMute(false);        
        CP->ui->sliderVolume->setValue(volume);
        CP->ui->pushButtonMute->setIcon(style()->standardIcon(QStyle::SP_MediaVolume));
        labelTL->setText("音量："+QString::number(volume));
        labelTL->adjustSize();
        labelTL->show();
        QTimer::singleShot(3000,this,SLOT(timeoutTL()));
    }else{
        volume=player->audio()->volume()*100;
        player->audio()->setMute(true);        
        CP->ui->sliderVolume->setValue(0);
        CP->ui->pushButtonMute->setIcon(style()->standardIcon(QStyle::SP_MediaVolumeMuted));
        labelTL->setText("静音");
        labelTL->adjustSize();
        labelTL->show();
        QTimer::singleShot(3000,this,SLOT(timeoutTL()));
    }
}

void MainWindow::on_action_help_triggered()
{
    QMessageBox MB(QMessageBox::NoIcon, "帮助", "快捷键：\n空格\t播放、暂停\n回车、双击\t全屏、退出全屏\nEsc\t退出全屏\n上\t增加音量\n下\t减小音量\n左 \t快退\n右\t快进\nM\t静音\nP\t截图\nL\t左转90°\nR\t右转90°\n1\t原始视频大小\n2\t2倍视频大小\n3\t1.5倍视频大小\n4\t0.5倍视频大小\n5\t视频铺满全屏\nT\t直播列表\nI\t导入直播文件\nCtrl + ←\t上一个\nCtrl + →\t下一个\nCtrl + ↑\t加速播放\nCtrl + ↓\t减速播放\nCtrl + I\t媒体信息");
    MB.exec();
}

void MainWindow::on_action_changelog_triggered()
{
    QString s = "1.7\n(2018-04)\n修复：视频分辨率为0X0时，仍然缩放会出错。\n\n1.6\n(2018-03)\n修复拖动进度条时，进度条被拉回的问题。\n修复：增加从Chrome扩展打开后，调试时窗口无法启动和从外部程序启动打开文件中断。\n\n1.5\n(2017-10)\n支持接收Chrome扩展传来的数据。\n修复放大适合桌面不居中、最大化缩放失效。\n增加解析直播API。\n增加音轨选择。\n增加加速播放、减速播放。\n增加显示错误信息。\n解析分号间隔的视频片段到列表。\n(2017-09-14)\n修复时间栏样式使用rgb没有使用rgba引起的闪烁。\n(2017-08-20)\n增加拖放打开文件(不知为何视频区域无效)。\n\n1.4 (2017-07)\n更新日志太长，消息框改成带滚动条的文本框。\n设计新的浮动透明控制栏，延时自动隐藏，鼠标移动显示。\n全屏缩放背景设置为黑色，视频居中。\n\n1.3 (2017-05)\n记忆全屏前直播列表的显示状态，退出全屏时按状态决定是否显示直播列表。\n直播列表并入窗体。\n\n1.2 (2017-03)\n增加打开方式打开文件。\n右键增加截图菜单。\n快进、快退在左上角显示时间。\n使用动态路径代替绝对路径，由于Qt4和Qt5获取路径的方法不同，使用预处理指令#if选择版本。\n增加剧情连拍。\n\n1.1 (2017-03)\n窗口标题增加台号。\n增加打开截图目录。\n2017-02\n增加导入直播列表功能。\n上一个、下一个按钮换台。\n增加直播列表。\n\n1.0 (2017-02)\n缩放菜单改成单选样式。\n增加香港卫视、北京时间直播源。\n增加缩放。\n解决 GraphicsItemRenderer 大部分全屏问题。\nVideoOutput 改成 GraphicsItemRenderer，支持视频旋转。\n增加截图。\n增加视频信息。\n使用第三方库QtAV代替QMultimedia库，解决快捷键无效的问题。\n解决停止后不能播放的问题。\n静音图标切换和拖动条。\n增加快进、快退。\n增加时间。\n修复拖动进度条卡顿BUG。\n全屏修改进度条样式。\n实现全屏。\n增加视频控件。\n增加控制栏。";
    QDialog *dialog=new QDialog;
    dialog->setWindowTitle("更新历史");
    dialog->setFixedSize(400,300);
    QVBoxLayout *vbox=new QVBoxLayout;
    QTextBrowser *textBrowser=new QTextBrowser;
    textBrowser->setText(s);
    textBrowser->zoomIn();
    vbox->addWidget(textBrowser);
    QHBoxLayout *hbox=new QHBoxLayout;
    QPushButton *btnConfirm=new QPushButton("确定");
    hbox->addStretch();
    hbox->addWidget(btnConfirm);
    hbox->addStretch();
    vbox->addLayout(hbox);
    dialog->setLayout(vbox);
    dialog->show();
    connect(btnConfirm, SIGNAL(clicked()), dialog, SLOT(accept()));
    if(dialog->exec()==QDialog::Accepted){
        dialog->close();
    }
}

void MainWindow::on_action_aboutQtAV_triggered()
{    
    QtAV::about();
}

void MainWindow::on_action_about_triggered()
{
    QMessageBox aboutMB(QMessageBox::NoIcon, "关于", "海天鹰媒体播放器 1.7\n一款基于Qt和QtAV库的媒体播放器。\n作者：黄颖\nE-mail: sonichy@163.com\n主页：sonichy.96.lt\n参考：\nhttps://github.com/wang-bin/QtAV");
    aboutMB.setIconPixmap(QPixmap(":/icon.png"));
    aboutMB.exec();
}


void MainWindow::seekBackward()
{
    player->seekBackward();
    setSTime(player->position());
    labelTL->setText(CP->ui->labelTimeVideo->text());
    labelTL->adjustSize();
    labelTL->show();
    QTimer::singleShot(3000,this,SLOT(timeoutTL()));
}

void MainWindow::seekForward()
{
    player->seekForward();
    setSTime(player->position());
    labelTL->setText(CP->ui->labelTimeVideo->text());
    labelTL->adjustSize();
    labelTL->show();
    QTimer::singleShot(3000,this,SLOT(timeoutTL()));
}

void MainWindow::skipBackward()
{
    if( mode == "live" ){
        if( ui->tableWidget->currentRow() > 0 ){
            ui->tableWidget->setCurrentCell(ui->tableWidget->currentRow()-1,0);
            playTV(ui->tableWidget->currentRow(),1);
        }
    }
    if(mode == "urllist"){
        if( dialogUrl->ui->tableWidget->currentRow() >0 ){
            dialogUrl->ui->tableWidget->setCurrentCell(dialogUrl->ui->tableWidget->currentRow()-1,0);
            playURL(dialogUrl->ui->tableWidget->currentRow(),0);
        }
    }
}

void MainWindow::skipForward()
{
    qDebug() << mode;
    if(mode=="live"){
        if( ui->tableWidget->currentRow() < ui->tableWidget->rowCount()-1 ){
            ui->tableWidget->setCurrentCell(ui->tableWidget->currentRow()+1,0);
            playTV(ui->tableWidget->currentRow(),0);
        }
    }
    if(mode == "urllist"){
        if( dialogUrl->ui->tableWidget->currentRow() < dialogUrl->ui->tableWidget->rowCount()-1 ){
            dialogUrl->ui->tableWidget->setCurrentCell(dialogUrl->ui->tableWidget->currentRow()+1,0);
            playURL(dialogUrl->ui->tableWidget->currentRow(),0);
        }
    }
}

void MainWindow::enterFullscreen()
{
    isListShow = ui->tableWidget->isVisible();
    posw = pos();
    showFullScreen();
    ui->menuBar->hide();
    videoItem->resizeRenderer(desktop->width(),desktop->height());
    PMAFullscreen->setText("退出全屏");
    ui->tableWidget->setVisible(false);
    CP->move(0,desktop->height()-CP->height());
    CP->resize(desktop->width(),CP->height());
}

void MainWindow::exitFullscreen()
{    
    showNormal();
    ui->menuBar->show();
    //videoItem->resizeRenderer(player->statistics().video_only.width*sr, player->statistics().video_only.height*sr);
    ui->tableWidget->setVisible(isListShow);
    int tww;
    if (ui->tableWidget->isVisible()) {
        tww = ui->tableWidget->width();
    } else {
        tww = 0;
    }
    //resize(player->statistics().video_only.width*sr + tww, player->statistics().video_only.height*sr + ui->menuBar->height());
    //move(posw);
    PMAFullscreen->setText("全屏");
    CP->move(0,height()-CP->height());
    CP->resize(width()-tww,CP->height());

}

void MainWindow::EEFullscreen()
{
    if (isFullScreen()) {
        exitFullscreen();
    } else {
        enterFullscreen();
    }
}

void MainWindow::durationChange()
{    
    CP->ui->sliderProgress->setRange(0,player->duration());
    qDebug() << player->statistics().video_only.width << "X" << player->statistics().video_only.height;    
    if(player->statistics().video_only.width != 0 || player->statistics().video_only.height != 0){
        videoItem->resizeRenderer(player->statistics().video_only.width, player->statistics().video_only.height);
//        int tww;
//        if(ui->tableWidget->isVisible()){
//            tww=ui->tableWidget->width();
//        }else{
//            tww=0;
//        }
        resize(player->statistics().video_only.width , player->statistics().video_only.height + ui->menuBar->height());
//      move((desktop->width() - width())/2, (desktop->height() - height())/2);
        CP->move(0,height()-CP->height());
        CP->resize(ui->graphicsView->width(),CP->height());
    }

    // 设置音轨菜单
    ui->menu_soundTrack->clear();
    for(int i=0; i<player->audioStreamCount(); i++){
        QAction *action = new QAction("音轨" + QString::number(i), this);
        action->setData(i);
        action->setCheckable(true);
        ui->menu_soundTrack->addAction(action);
        connect(action,SIGNAL(triggered(bool)),this,SLOT(changeAudioTrack(bool)));
    }
    QActionGroup *AG_soundTrack = new QActionGroup(this);
    for(int i=0; i<ui->menu_soundTrack->actions().size(); i++){
        AG_soundTrack->addAction(ui->menu_soundTrack->actions().at(i));
    }
    ui->menu_soundTrack->actions().at(0)->setChecked(true);
}

void MainWindow::positionChange(qint64 p)
{    
    if(!CP->ui->sliderProgress->isSliderDown())CP->ui->sliderProgress->setValue(p);
    setSTime(p);
}

void MainWindow::setSTime(qint64 v)
{
    QTime t(0,0,0);
    t = t.addMSecs(v);
    QString STimeElapse = t.toString("hh:mm:ss");
    t.setHMS(0,0,0);
    t = t.addMSecs(player->duration());
    QString STimeTotal = t.toString("hh:mm:ss");
    STimeET = STimeElapse + "/" + STimeTotal;
    CP->ui->labelTimeVideo->setText(STimeET);
    CP->ui->sliderProgress->setToolTip(STimeElapse);
}

void MainWindow::timeoutTL()
{
    labelTL->hide();
}

void MainWindow::hideCP()
{
    CP->hide();
    if (isFullScreen()) setCursor(QCursor(Qt::BlankCursor));
}

void MainWindow::volumeChange(qreal v)
{    
    CP->ui->sliderVolume->setToolTip(QString::number(floor(v*100)));
    labelTL->setText("音量：" + QString::number(floor(v*100)));
    labelTL->adjustSize();
    labelTL->show();
    QTimer::singleShot(3000,this,SLOT(timeoutTL()));
}

void MainWindow::playPause()
{
    //qDebug() << "state=" << player->state();
    if(player->state() == player->PlayingState){
        player->pause(true);
    }else if(player->state() == player->PausedState){
        player->pause(false);
    }else if(player->state() == player->StoppedState){
        player->play();
    }
}

void MainWindow::createPopmenu()
{
    popmenu = new QMenu();
    PMAFullscreen = new QAction("全屏",this);
    PMACapture = new QAction("截图",this);
    popmenu->addAction(PMAFullscreen);
    popmenu->addAction(PMACapture);
    connect(PMAFullscreen, SIGNAL(triggered()), this, SLOT(EEFullscreen()));
    connect(PMACapture, SIGNAL(triggered()), this, SLOT(on_action_capture_triggered()));
}

void MainWindow::contextMenuEvent(QContextMenuEvent *event)
{
    //pop_menu->clear();
    popmenu->exec(QCursor::pos());
    event->accept();
}

void MainWindow::sliderVolumeMoved(int v)
{
    player->audio()->setVolume(v/100.0);
}

void MainWindow::closeEvent(QCloseEvent *e)
{
    Q_UNUSED(e);
    //if(player)player->stop();
    //qApp->quit();
}

void MainWindow::scale(float s)
{    
    if (isMaximized()) showNormal();
    if (player->statistics().video_only.width != 0 || player->statistics().video_only.height != 0) {
        videoItem->resizeRenderer(player->statistics().video_only.width * s, player->statistics().video_only.height * s);
        if (isFullScreen()) {
            ui->graphicsView->move((desktop->width() - player->statistics().video_only.width * s)/2, (desktop->height() - player->statistics().video_only.height * s)/2);
        } else {
            resize(player->statistics().video_only.width * s, player->statistics().video_only.height * s + ui->menuBar->height());
            CP->move(0, height() - CP->height());
            CP->resize(ui->graphicsView->width(), CP->height());
        }
    }
}

void MainWindow::on_action_scale0_5_triggered()
{
    sr = 0.5;
    scale(sr);
    ui->action_scale0_5->setChecked(true);
}

void MainWindow::on_action_scale1_triggered()
{
    sr = 1;
    scale(sr);
    ui->action_scale1->setChecked(true);
}

void MainWindow::on_action_scale1_5_triggered()
{
    sr = 1.5;
    scale(sr);
    ui->action_scale1_5->setChecked(true);
}

void MainWindow::on_action_scale2_triggered()
{
    sr = 2;
    scale(sr);
    ui->action_scale2->setChecked(true);
}

void MainWindow::mouseDoubleClickEvent(QMouseEvent* event)
{
    Q_UNUSED(event);
    if (player) {
        EEFullscreen();
    }
}

void MainWindow::fitDesktop()
{
    qDebug() << ui->graphicsView->x() << ui->graphicsView->y();
    ui->graphicsView->move(0,0);
    videoItem->resizeRenderer(desktop->width(),desktop->height());
}

void MainWindow::playTV(int row,int column)
{
    Q_UNUSED(column);
    labelTL->hide();
    mode = "live";
    QString surl = ui->tableWidget->item(row,1)->text();
    QString realurl = "";
    if(surl.contains("http://vdn.live.cntv.cn/api2/live.do?channel=")){
        qDebug() << "analyze(" << surl << ")";
        QUrl url = QString(surl);
        QNetworkAccessManager *NAM = new QNetworkAccessManager;
        QNetworkRequest request(url);
        QNetworkReply *reply = NAM->get(request);
        QEventLoop loop;
        connect(reply, SIGNAL(finished()), &loop, SLOT(quit()));
        loop.exec();
        QByteArray responseText = reply->readAll();
        QJsonDocument json = QJsonDocument::fromJson(responseText);
        QString hls_url = json.object().value("hls_url").toObject().value("hls4").toString();
        //qDebug() << "play(" << hls_url << ")";
        //player->play(hls_url);
        realurl = hls_url;
    }else if(surl.contains("http://apiv1.starschina.com/cms/v1.0/stream?")){
        qDebug() << "analyze(" << surl << ")";
        QUrl url = QString(surl);
        QNetworkAccessManager *NAM = new QNetworkAccessManager;
        QNetworkRequest request(url);
        QNetworkReply *reply = NAM->get(request);
        QEventLoop loop;
        connect(reply, SIGNAL(finished()), &loop, SLOT(quit()));
        loop.exec();
        QByteArray responseText = reply->readAll();
        QJsonDocument json = QJsonDocument::fromJson(responseText);
        QJsonArray play_urls = json.object().value("data").toObject().value("play_urls").toArray();
        QString urls = play_urls.last().toObject().value("urls").toArray()[0].toString();
        //qDebug() << "play(" << urls << ")";
        //player->play(urls);
        realurl = urls;
    }else if(surl.contains("http://live.api.hunantv.com/pc/getById?")){
        qDebug() << "analyze(" << surl << ")";
        QUrl url = QString(surl);
        QNetworkAccessManager *NAM = new QNetworkAccessManager;
        QNetworkRequest request(url);
        QNetworkReply *reply = NAM->get(request);
        QEventLoop loop;
        connect(reply, SIGNAL(finished()), &loop, SLOT(quit()));
        loop.exec();
        QByteArray responseText = reply->readAll();
        QJsonDocument json = QJsonDocument::fromJson(responseText);
        QJsonArray html5Sources = json.object().value("data").toObject().value("html5Sources").toArray();
        QString urls = html5Sources.last().toObject().value("url").toString();
        //qDebug() << "play(" << urls << ")";
        //player->play(urls);
        realurl = urls;
    }else{
        //qDebug() << "play(" << surl << ")";
        //player->play(surl);
        realurl = surl;
    }
    qDebug() << "play(" << realurl << ")";
    player->play(realurl);
    addHistory(realurl);
    setWindowTitle(ui->tableWidget->item(row,0)->text());
    setFocus();
}

void MainWindow::fillTable(QString filename)
{
    QFile *file=new QFile(filename);
    if(file->open(QIODevice::ReadOnly | QIODevice::Text)){
        ui->tableWidget->setRowCount(0);
        QTextStream ts(file);
        QString s=ts.readAll();
        file->close();
        QStringList line=s.split("\n");
        for(int i=0;i<line.size();i++){
            ui->tableWidget->insertRow(i);
            if(line.at(i).contains(",")){
                QStringList strlist=line.at(i).split(",");
                ui->tableWidget->setItem(i,0,new QTableWidgetItem(strlist.at(0)));
                ui->tableWidget->setItem(i,1,new QTableWidgetItem(strlist.at(1).split("#").at(0)));
            }else{
                ui->tableWidget->setItem(i,0,new QTableWidgetItem("【"+line.at(i)+"】"));
                ui->tableWidget->setItem(i,1,new QTableWidgetItem(""));
            }
        }
        ui->tableWidget->resizeColumnsToContents();        
    }
}

void MainWindow::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
    {
        m_bPressed = true;
        m_point = event->pos();
    }
    CP->show();
    setCursor(Qt::ArrowCursor);
    QTimer::singleShot(5000,this,SLOT(hideCP()));
}

void MainWindow::mouseMoveEvent(QMouseEvent *event)
{
    if (m_bPressed){
        move(event->pos() - m_point + pos());
        setCursor(Qt::ClosedHandCursor);
    }
}

void MainWindow::mouseReleaseEvent(QMouseEvent *event)
{
    Q_UNUSED(event);
    m_bPressed = false;
    setCursor(Qt::ArrowCursor);
}

void MainWindow::saveImage(QImage image)
{
    if (cn==16) {
        imagec[cnn]=image.scaled(320,240);
        QTime t(0,0,0);
        t=t.addMSecs(player->position());
        QString STimeElapse=t.toString("hh:mm:ss");
        QPainter painter(&imagec[cnn]);
        painter.setPen(QPen(Qt::white));
        painter.drawText(270,235,STimeElapse);

        /*// 描边文字
        QPainterPath path;
        QPen pen(Qt::black);
        pen.setStyle(Qt::SolidLine);
        pen.setWidth(2);
        QFont font;
        font.setBold(true);
        font.setPixelSize(15);
        path.addText(250, 230, font,STimeElapse);
        painter.strokePath(path, pen);
        painter.fillPath(path, QBrush(Qt::white));
        */

        cnn++;
    }
}

bool MainWindow::eventFilter(QObject *obj, QEvent *event)
{
    Q_UNUSED(obj);
    if (event->type() == QEvent::MouseMove) {
        QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
        Q_UNUSED(mouseEvent);
        //statusBar()->showMessage(QString("Mouse (%1,%2)").arg(mouseEvent->pos().x()).arg(mouseEvent->pos().y()));
        //qDebug() << "(" << mouseEvent->pos().x() << mouseEvent->pos().y() << ")";
        if (CP->isHidden()) {
            CP->show();
            setCursor(QCursor(Qt::ArrowCursor));
            QTimer::singleShot(5000,this,SLOT(hideCP()));
        }
    }
    return false;
}


void MainWindow::dragEnterEvent(QDragEnterEvent *e)
{
    //if(e->mimeData()->hasFormat("text/uri-list")) //只能打开文本文件
    e->acceptProposedAction(); //可以在这个窗口部件上拖放对象
}

void MainWindow::dropEvent(QDropEvent *e) //释放对方时，执行的操作
{
    QList<QUrl> urls = e->mimeData()->urls();
    if(urls.isEmpty())
        return ;

    QString fileName = urls.first().toLocalFile();

    foreach (QUrl u, urls) {
        qDebug() << u.toString();
    }
    qDebug() << urls.size();

    if(fileName.isEmpty())
        return;

    open(fileName);
}

void MainWindow::analyze()
{
    QString surl = dialogUrl->ui->lineEdit->text();
    if(!surl.isEmpty()){
        if(surl.contains(";")){
            mode = "urllist";
            dialogUrl->ui->tableWidget->setRowCount(0);
            QStringList clip = surl.split(";");
            for(int i=0; i<clip.size(); i++){
                dialogUrl->ui->tableWidget->insertRow(i);
                dialogUrl->ui->tableWidget->setItem(i,0,new QTableWidgetItem(QFileInfo(clip.at(i)).fileName()));
                dialogUrl->ui->tableWidget->setItem(i,1,new QTableWidgetItem(clip.at(i)));
            }
            dialogUrl->ui->tableWidget->resizeColumnsToContents();
            dialogUrl->ui->tableWidget->setCurrentCell(0,0);
            playURL(0,0);
        }else{            
            player->play(surl);
            ui->tableWidget->hide();
            addHistory(surl);
            setWindowTitle(QFileInfo(surl).fileName());
        }
    }
}

void MainWindow::playURL(int row,int column)
{
    Q_UNUSED(column);
    ui->tableWidget->hide();
    QString surl = dialogUrl->ui->tableWidget->item(row,1)->text();
    qDebug() << "play(" << surl << ")";
    //if(surl!=""){
    player->play(surl);
    addHistory(surl);
    if(isFullScreen())fitDesktop();
    //dialogUrl->hide();
    setWindowTitle(QString::number(row+1) + ":" + dialogUrl->ui->tableWidget->item(row,0)->text());    
    CP->ui->pushButtonPlay->setIcon(style()->standardIcon(QStyle::SP_MediaPause));
    //}
}

void MainWindow::handleError(const AVError &e)
{
    qDebug() << e.string();
    labelTL->setText(e.string());
    labelTL->adjustSize();
    labelTL->show();
    QTimer::singleShot(5000,[=]{labelTL->hide();});
}

void MainWindow::speedUp()
{
    player->setSpeed(player->speed() + 0.1);
    labelTL->setText(QString("%1 X").arg(player->speed()));
    if( player->speed() == 1.0 ){
        labelTL->hide();
    }else{
        labelTL->show();
    }
}

void MainWindow::speedDown()
{
    if( player->speed() > 0.11 ){
        player->setSpeed( player->speed() - 0.1 );
        qDebug() << player->speed();
        labelTL->setText( QString("%1 X").arg(player->speed()) );
        if( player->speed() == 1.0 ){
            labelTL->hide();
        }else{
            labelTL->show();
        }
    }
}

// 预览图
void MainWindow::preview(int value)
{    
    int w = 100;
    int h = 70;
    m_preview->setFile(player->file());
    m_preview->setTimestamp(value);
    m_preview->preview();
    //m_preview->setWindowFlags(m_preview->windowFlags() | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
    m_preview->setWindowFlags(m_preview->windowFlags() | Qt::WindowStaysOnTopHint);
    m_preview->resize(w,h);
    m_preview->move(cursor().pos().x()-w/2 , cursor().pos().y()-h-20);
    m_preview->show();
}

void MainWindow::changeAudioTrack(bool b)
{
    Q_UNUSED(b);
    QAction *action = qobject_cast<QAction*>(sender());
    player->setAudioStream(action->data().toInt());
    action->setChecked(true);
}

void MainWindow::stop()
{
    player->stop();
}

void MainWindow::stateChange(QtAV::AVPlayer::State state)
{
    if ( state == player->PlayingState ) {
        CP->ui->pushButtonPlay->setIcon(style()->standardIcon(QStyle::SP_MediaPause));
        CP->ui->pushButtonPlay->setToolTip("暂停");
    }
    if ( state == player->PausedState ) {
        CP->ui->pushButtonPlay->setIcon(style()->standardIcon(QStyle::SP_MediaPlay));
        CP->ui->pushButtonPlay->setToolTip("播放");
    }
    if ( state == player->StoppedState ) {
        CP->ui->pushButtonPlay->setIcon(style()->standardIcon(QStyle::SP_MediaPlay));
        CP->ui->pushButtonPlay->setToolTip("播放");
    }
}

void MainWindow::addHistory(QString url)
{
    QAction *action = new QAction(url, this);
    action->setData(url);
    ui->menu_history->addAction(action);
    connect(action,SIGNAL(triggered(bool)),this,SLOT(openHistory(bool)));
}

void MainWindow::openHistory(bool b)
{
    Q_UNUSED(b);
    QAction *action = qobject_cast<QAction*>(sender());
    player->play(action->data().toString());
}

void MainWindow::sliderProgressPressed()
{
}

void MainWindow::sliderProgressMoved(int v)
{
    player->setPosition(v);
    labelTL->setText(CP->ui->labelTimeVideo->text());
    labelTL->adjustSize();
    labelTL->show();
}

void MainWindow::sliderProgressReleased()
{
    player->setPosition(CP->ui->sliderProgress->value());
    labelTL->setText(CP->ui->labelTimeVideo->text());
    labelTL->adjustSize();
    labelTL->show();
    QTimer::singleShot(3000,this,SLOT(timeoutTL()));
}
