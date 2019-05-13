#ifndef DWINTEST_H
#define DWINTEST_H

#include <QMainWindow>
#include "win_qextserialport.h"
#include <QMap>
#include "spindelegate.h"

#define UBASEADDR 0x1000
#define IBASEADDR 0x1024
#define TABLEROWS 6
#define TABLECOLS 8
#define DIMENSION 6
#define ARRAYCOLS 3

class myTable;

namespace Ui {
class DwinTest;
}

class DwinTest : public QMainWindow
{
    Q_OBJECT

public:
    explicit DwinTest(QWidget *parent = 0);
    ~DwinTest();
signals:
private slots:
    void slot_readMyCom();
    void slot_editCompleted(float value,int addr);

    void on_openComPortBtn_clicked();

    void on_closeComPortBtn_clicked();

    void on_updateComPortBtn_clicked();

    void on_clearSendDataBtn_clicked();

    void on_clearReceivedDataBtn_clicked();

    void on_SwitchPageComboBox_currentIndexChanged(int index);

private:
    void setWidgetStatus();
    void synDwinData();
    void updateComPort();

    QStringList getEnableComPort();
    QByteArray stringToHex(QString sendStr);
private:
    Ui::DwinTest *ui;

    Win_QextSerialPort *myCom;
    myTable *tableModel;
    SpinDelegate *spinDelegate;
};

//自定义表格
class myTable : public QAbstractTableModel
{
    Q_OBJECT
public:
    myTable(QObject *parent = 0);
    ~myTable();

    int rowCount(const QModelIndex &parent) const;
    int columnCount(const QModelIndex &parent) const;
    QVariant data(const QModelIndex &index, int role) const;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const;
    //实现支持修改数据的虚函数
    bool setData(const QModelIndex &index, const QVariant &value, int role);
    Qt::ItemFlags flags(const QModelIndex &index) const;

    void setIsOpenComPort(bool flag);
signals:
    void signal_editCompleted(float value,int addr);
private slots:
private:
    QStringList UList;
    QStringList IList;
    QStringList headerList;

    bool isOpenComPort;
};

#endif // DWINTEST_H
