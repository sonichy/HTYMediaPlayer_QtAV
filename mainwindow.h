#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QtAVWidgets>
#include <QtAV/AVPlayer.h>
#include <QtAV>
#include <QLabel>
#include <QDesktopWidget>
#include "controlpanel.h"
#include "ui_controlpanel.h"
#include "dialogurl.h"
#include "ui_dialogurl.h"

using namespace QtAV;
namespace Ui {
    class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private:
    Ui::MainWindow *ui;
    QString filename, STimeET, mode;
    QMenu *popmenu, *menu_soundTrack;
    QAction *PMAFullscreen, *PMACapture;
    int volume, cn, cnn;
    float sr;
    QLabel *labelTL;
    AVPlayer *player;
    GraphicsItemRenderer *videoItem;
    QDesktopWidget *desktop;
    QPoint posw;
    bool m_bPressed;
    QPoint m_point;
    QImage imagec[16];
    bool isListShow;
    void createPopmenu();
    void closeEvent(QCloseEvent*);
    void mouseDoubleClickEvent(QMouseEvent*);
    void mousePressEvent(QMouseEvent*);
    void mouseMoveEvent(QMouseEvent*);
    void mouseReleaseEvent(QMouseEvent*);
    void open(QString);
    ControlPanel *CP;
    bool eventFilter(QObject*, QEvent*);
    void setSTime(int v);
    void dragEnterEvent(QDragEnterEvent*);
    void dropEvent(QDropEvent*);
    DialogURL *dialogUrl;
    VideoPreviewWidget *m_preview;
    void preview(int value);

private slots:
    void on_action_open_triggered();
    void on_action_openURL_triggered();
    void on_action_liveList_triggered();
    void on_action_liveImport_triggered();
    void on_action_quit_triggered();
    void on_action_fullscreen_triggered();    
    void on_action_info_triggered();
    void on_action_capture_triggered();
    void on_action_capture16_triggered();
    void on_action_captureDirectory_triggered();
    void on_action_rotateCW_triggered();
    void on_action_rotateCCW_triggered();
    void on_action_scale0_5_triggered();
    void on_action_scale1_triggered();
    void on_action_scale1_5_triggered();
    void on_action_scale2_triggered();
    void on_action_volumeUp_triggered();
    void on_action_volumeDown_triggered();
    void on_action_volumeMute_triggered();
    void on_action_help_triggered();
    void on_action_changelog_triggered();
    void on_action_aboutQtAV_triggered();
    void on_action_about_triggered();
    void seekBackward();
    void seekForward();
    void skipBackward();
    void skipForward();
    void stop();
    void durationChange();
    void positionChange(qint64);
    void volumeChange(qreal);
    void setMPPosition(int);    
    void setVolume(int);    
    void enterFullscreen();
    void exitFullscreen();
    void EEFullscreen();
    void fitDesktop();
    void playPause();    
    void contextMenuEvent(QContextMenuEvent*);
    void timeoutTL();
    void hideCP();    
    void saveImage(QImage);
    void scale(float s);
    void playTV(int,int);
    void fillTable(QString);
    void showHideList();
    void analyze();
    void playURL(int,int);    
    void handleError(const QtAV::AVError& e);
    void speedUp();
    void speedDown();
    void changeAudioTrack(bool);
    void stateChange(QtAV::AVPlayer::State);
    void addHistory(QString);
    void openHistory(bool);
};

#endif // MAINWINDOW_H
