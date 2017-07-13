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
    explicit EditDialog(QWidget *parent = 0);
    ~EditDialog();
    void GetResult(QString* devcode, QString* serial);
    void SetText(QString devcode, QString serial);

public slots:

private slots:
    void on_btnRndDev_clicked();
    void on_btnRndSer_clicked();
    void okButton_clicked();

private:
    Ui::EditDialog *ui;
};

#endif // EDITDIALOG_H
