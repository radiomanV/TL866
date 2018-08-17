#ifndef EDITDIALOG_H
#define EDITDIALOG_H

#include <QDialog>

namespace Ui {
class EditDialog;
}

class EditDialog : public QDialog
{
    Q_OBJECT

public:
    explicit EditDialog(QWidget *parent = nullptr, QString devcode = "", QString serial = "");
    ~EditDialog();
    void GetResult(QString &devcode, QString &serial);

public slots:

private slots:
    void on_btnRndDev_clicked();
    void on_btnRndSer_clicked();
    void okButton_clicked();

    void on_txtDevcode_textChanged(const QString &arg1);

private:
    Ui::EditDialog *ui;
    ushort get_dev_crc();
};

#endif // EDITDIALOG_H
