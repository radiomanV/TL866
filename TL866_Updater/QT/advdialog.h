#ifndef ADVDIALOG_H
#define ADVDIALOG_H

#include <QDialog>

namespace Ui {
class AdvDialog;
}

class AdvDialog : public QDialog
{
    Q_OBJECT

public:
    explicit AdvDialog(QWidget *parent = 0);
    ~AdvDialog();
    void SetSerial(QString devcode, QString serial);
    void SetInfo(QString info);
    void SetUi(bool cp, int type);

private slots:
    void on_btnEdit_clicked();
    void on_btnDefault_clicked();
    void on_btnClone_clicked();
    void on_btnWriteBootloader_clicked();
    void on_btnWriteConfig_clicked();
    void on_btnWriteInfo_clicked();

private:
    Ui::AdvDialog *ui;
    QString device_code;
    QString serial_number;

};

#endif // DIALOG2_H
