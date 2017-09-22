#ifndef DIALOGURL_H
#define DIALOGURL_H

#include <QDialog>
#include <QCloseEvent>

namespace Ui {
class DialogURL;
}

class DialogURL : public QDialog
{
    Q_OBJECT

public:
    explicit DialogURL(QWidget *parent = 0);
    ~DialogURL();
    Ui::DialogURL *ui;

private:
    void closeEvent(QCloseEvent*);
};

#endif // DIALOGURL_H
