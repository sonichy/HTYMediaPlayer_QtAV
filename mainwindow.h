#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

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
    void createPopmenu();
    void closeEvent(QCloseEvent *e);
    void mouseDoubleClickEvent(QMouseEvent* event);
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void open(QString path);

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
    void on_btnPlay_clicked();    
    void on_btnStop_clicked();
    void on_btnSeekB_clicked();
    void on_btnSeekF_clicked();
    void on_btnSkipB_clicked();
    void on_btnSkipF_clicked();
    void on_btnMute_clicked();
    void on_btnFullscreen_clicked();
    void on_btnList_clicked();
    void durationChange();
    void positionChange(qint64 d);    
    void volumeChange(qreal v);
    void setMPPosition(int);
    void setMPPosition();
    void setVolume(int);
    void setVolume();
    void enterFullscreen();
    void exitFullscreen();
    void EEFullscreen();
    void fitDesktop();
    void playPause();    
    void contextMenuEvent(QContextMenuEvent *event);
    void setSTime(int v);
    void timeoutTL();
    void timeoutMouseMove();
    void saveImage(QImage image);
    void scale(float s);
    void playTV(int row, int column);
    void fillTable(QString filename);
    void showHideList();
};

#endif // MAINWINDOW_H
