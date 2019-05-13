#include "dwintest.h"
#include "ui_dwintest.h"
#include <QMessageBox>
#include <QDebug>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QTableView>
#include <QHeaderView>
#include <QStandardItemModel>

//保存表格数据,初始化为全0
float UData[DIMENSION][ARRAYCOLS];
float IData[DIMENSION][ARRAYCOLS];

DwinTest::DwinTest(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::DwinTest)
{
    ui->setupUi(this);

    setWidgetStatus();

    for(int i=0; i<DIMENSION; ++i)
    {
        for(int j=0; j<ARRAYCOLS; ++j)
        {
            UData[i][j] = 0.000;
            IData[i][j] = 0.000;
        }
    }

}

DwinTest::~DwinTest()
{
    if(myCom)
    {
        delete myCom;
        myCom = NULL;
    }
    if(spinDelegate)
    {
        delete spinDelegate;
        spinDelegate = NULL;
    }
    delete ui;
}

void DwinTest::setWidgetStatus()
{
    myCom = NULL;
    tableModel = new myTable(this);
    tableModel->setIsOpenComPort(false);
    ui->tableView->setModel(tableModel);
    ui->tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->tableView->horizontalHeader()->setSectionResizeMode(0,QHeaderView::ResizeToContents);
    ui->tableView->horizontalHeader()->setSectionResizeMode(4,QHeaderView::ResizeToContents);
    QVector<int> colVec{1,2,3,5,6,7};
    for(int i=0; i<colVec.size(); ++i)
    {
        ui->tableView->horizontalHeader()->setSectionResizeMode(colVec[i],QHeaderView::Fixed);
        ui->tableView->setColumnWidth(colVec[i],60);
    }

    spinDelegate = new SpinDelegate;
    ui->tableView->setItemDelegate(spinDelegate);
    connect(tableModel,SIGNAL(signal_editCompleted(float,int)),this,SLOT(slot_editCompleted(float,int)));

    ui->iconLabel->setScaledContents(true);
    ui->iconLabel->setPixmap(QPixmap(":/image/image/com_OFF.jpg"));
    ui->portNameComboBox->setEnabled(true);
    ui->updateComPortBtn->setEnabled(true);
    ui->BaudRateComboBox->setEnabled(true);
    ui->dataBitsComboBox->setEnabled(true);
    ui->parityComboBox->setEnabled(true);
    ui->stopBitsComboBox->setEnabled(true);
    ui->openComPortBtn->setEnabled(true);
    ui->closeComPortBtn->setEnabled(false);
}

void DwinTest::synDwinData()
{
    for(int i=0; i<DIMENSION; ++i)
    {
        for(int j=0; j<ARRAYCOLS; ++j)
        {
            int Uaddr = tableModel->index(i,j+1).data(Qt::UserRole).toInt();
            int Iaddr = tableModel->index(i,j+5).data(Qt::UserRole).toInt();
            slot_editCompleted(UData[i][j],Uaddr);
            slot_editCompleted(IData[i][j],Iaddr);
        }
    }
}

void DwinTest::updateComPort()
{
    QStringList comPortList = getEnableComPort();
    ui->portNameComboBox->clear();
    ui->portNameComboBox->addItems(comPortList);
}

QStringList DwinTest::getEnableComPort()
{
    QStringList comPortList;

    foreach(const QSerialPortInfo &info, QSerialPortInfo::availablePorts())
    {
        QSerialPort serial;
        serial.setPort(info);
        if(serial.open(QIODevice::ReadWrite))
        {
            comPortList.append(serial.portName());
            serial.close();
        }
    }

    return comPortList;
}

QByteArray DwinTest::stringToHex(QString sendStr)
{
    for(int i=0; i<sendStr.length(); ++i)
    {
        if(QString(sendStr[i]) == " ")
            sendStr.remove(i,1);
    }
    QByteArray buf;
    if(sendStr.length()%2 != 0)
    {
        QMessageBox::warning(this,"warning","发送数据长度错误!",QMessageBox::Ok);
        return buf;     //此时buf为空，可用于调用函数判断
    }

    QString data;
    for(int i=0; i<sendStr.length()-1; i+=2)
    {
        data = sendStr.mid(i,2);
        buf.append(data.toInt(0,16));
    }
    return buf;
}

void DwinTest::on_openComPortBtn_clicked()
{
    if(ui->portNameComboBox->count() == 0)
    {
        QMessageBox::warning(this,"warning","串口号错误!\r\n请检查设备或点击“刷新串口”按钮!",QMessageBox::Ok);
        return;
    }
    QString portName = ui->portNameComboBox->currentText(); //获取串口名
    myCom = new Win_QextSerialPort(portName,QextSerialBase::EventDriven);//创建串口对象，使用事件驱动

    if(!myCom->open(QIODevice::ReadWrite))                  //以可读写方式打开串口
    {
        QMessageBox::warning(this,"warning","open serial failed!",QMessageBox::Ok);
        return;
    }
    //根据组合框内容对串口进行设置
    if(ui->BaudRateComboBox->currentText() == tr("9600"))   //设置波特率
        myCom->setBaudRate(BAUD9600);
    else if(ui->BaudRateComboBox->currentText() == tr("115200"))
        myCom->setBaudRate(BAUD115200);

    QString dataStr = ui->dataBitsComboBox->currentText();  //设置数据位
    if(dataStr == tr("8"))
        myCom->setDataBits(DATA_8);
    else if(dataStr == tr("7"))
        myCom->setDataBits(DATA_7);

    QString parStr = ui->parityComboBox->currentText();     //设置奇偶校验
    if(parStr == tr("无"))
        myCom->setParity(PAR_NONE);
    else if(parStr == tr("奇"))
        myCom->setParity(PAR_ODD);
    else if(parStr == tr("偶"))
        myCom->setParity(PAR_EVEN);

    QString stopStr = ui->stopBitsComboBox->currentText();  //设置停止位
    if(stopStr == tr("1"))
        myCom->setStopBits(STOP_1);
    else if(stopStr == tr("2"))
        myCom->setStopBits(STOP_2);

    myCom->setFlowControl(FLOW_OFF);              //设置数据流控制，使用无数据流控制的默认设置
    myCom->setTimeout(500);                       //设置延时
    tableModel->setIsOpenComPort(true);
    synDwinData();                                //同步显示数据

    //信号和槽函数关联，当串口缓冲区有数据时，进行读串口操作
    connect(myCom,SIGNAL(readyRead()),this,SLOT(slot_readMyCom()));

    //设置打开串口图标
    ui->iconLabel->setPixmap(QPixmap(":/image/image/com_ON.jpg"));

    //设置串口属性部件状态
    ui->portNameComboBox->setEnabled(false);
    ui->updateComPortBtn->setEnabled(false);
    ui->BaudRateComboBox->setEnabled(false);
    ui->dataBitsComboBox->setEnabled(false);
    ui->parityComboBox->setEnabled(false);
    ui->stopBitsComboBox->setEnabled(false);
    ui->openComPortBtn->setEnabled(false);
    ui->closeComPortBtn->setEnabled(true);
}

void DwinTest::slot_readMyCom()
{
    QByteArray received = myCom->readAll();
    received = received.toHex().toUpper();
    ui->receivedTextBrowser->insertPlainText(received);
    ui->receivedTextBrowser->insertPlainText("\r\n");
}

void DwinTest::slot_editCompleted(float value,int addr)
{
    if(!myCom || !myCom->isOpen())
    {
        QMessageBox::information(this,"提示","请打开串口以同步显示数据！",QMessageBox::Ok);
        return ;
    }

    //QString PKTHeadStr = "5AA5 05 82";
    QString PKTHeadStr = "5AA5 07 82";
    QString addrStr = QString("%1").arg(addr,4,16,QChar('0')).toUpper();
    QString valueStr;
    int *fptr = (int*)&value;
    if(value < 0)   //负数则转补码发送
    {
        valueStr = QString("%1").arg((quint64)pow(2,32)-abs(*fptr),8,16,QChar('0')).toUpper();
    }
    else
    {
        valueStr = QString("%1").arg(*fptr,8,16,QChar('0')).toUpper();
    }
    QString sendStr = PKTHeadStr + addrStr + valueStr;

    QByteArray buf = stringToHex(sendStr);
    if(buf.isEmpty())
    {
        return ;
    }

    //以16进制发送
    myCom->write(buf);

    //将发送数据显示到“已发送数据”显示区
    ui->sendTextBrowser->insertPlainText(sendStr);
    ui->sendTextBrowser->insertPlainText("\r\n");
}

void DwinTest::on_closeComPortBtn_clicked()
{
    myCom->close();
    ui->iconLabel->setPixmap(QPixmap(":/image/image/com_OFF.jpg"));
    ui->portNameComboBox->setEnabled(true);
    ui->updateComPortBtn->setEnabled(true);
    ui->BaudRateComboBox->setEnabled(true);
    ui->dataBitsComboBox->setEnabled(true);
    ui->parityComboBox->setEnabled(true);
    ui->stopBitsComboBox->setEnabled(true);
    ui->openComPortBtn->setEnabled(true);
    ui->closeComPortBtn->setEnabled(false);
}

void DwinTest::on_updateComPortBtn_clicked()
{
    updateComPort();
}

void DwinTest::on_clearSendDataBtn_clicked()
{
    ui->sendTextBrowser->clear();
}

void DwinTest::on_clearReceivedDataBtn_clicked()
{
    ui->receivedTextBrowser->clear();
}
void DwinTest::on_SwitchPageComboBox_currentIndexChanged(int index)
{
    if(index == -1)
        return ;
    if(!myCom || !myCom->isOpen())
    {
        QMessageBox::information(this,"提示","请打开串口再选择页面！",QMessageBox::Ok);
        return ;
    }

    QString PKTHeadStr = "5AA5 07 82 0084 5A01";
    QString pageIdStr = ui->SwitchPageComboBox->currentText();
    pageIdStr = QString("%1").arg(pageIdStr.toInt(),4,16,QChar('0'));
    QString sendStr = PKTHeadStr + pageIdStr;

    QByteArray buf = stringToHex(sendStr);
    if(buf.isEmpty())
    {
        return ;
    }
    myCom->write(buf);

    //将发送数据显示到“已发送数据”显示区
    ui->sendTextBrowser->insertPlainText(sendStr);
    ui->sendTextBrowser->insertPlainText("\r\n");
}

myTable::myTable(QObject *parent)
    : QAbstractTableModel(parent)
{
    UList <<"Ua"<<"Ub"<<"Uc"<<"Ux"<<"Uy"<<"Uz";
    IList <<"Ia"<<"Ib"<<"Ic"<<"Ix"<<"Iy"<<"Iz";
    headerList <<" "<<"幅值"<<"相位"<<"频率"<<" "<<"幅值"<<"相位"<<"频率";

    isOpenComPort = false;
}

myTable::~myTable()
{
    UList.clear();
    IList.clear();
    headerList.clear();
}

int myTable::rowCount(const QModelIndex &/*parent*/) const
{
    return TABLEROWS;
}

int myTable::columnCount(const QModelIndex &/*parent*/) const
{
    return TABLECOLS;
}

QVariant myTable::data(const QModelIndex &index, int role) const
{
    decltype(index.row()) row = index.row();
    decltype(index.column()) col = index.column();

    if(role == Qt::DisplayRole)
    {
        if(col == 0)
        {
            return UList[row];
        }
        else if(col == 4)
        {
            return IList[row];
        }
        else if(col >0 && col < 4)
        {
            switch(col)
            {
            case 2:
                return QString("%1").arg(UData[row][col-1],0,'f',2,QChar('0')); break;
            default:
                return QString("%1").arg(UData[row][col-1],0,'f',3,QChar('0')); break;
            }
        }
        else if(col > 4 && col < 8)
        {
            switch(col)
            {
            case 6:
                return QString("%1").arg(IData[row][col-5],0,'f',2,QChar('0')); break;
            default:
                return QString("%1").arg(IData[row][col-5],0,'f',3,QChar('0')); break;
            }

        }
    }
    else if(role == Qt::UserRole)
    {
        if(col == 0 || col == 4)
            return QVariant();

        if(col < 4)
        {
            return (UBASEADDR+row*6+(col-1)*0x0002);    //变量地址
        }
        else if(col > 4)
        {
            return (IBASEADDR+row*6+(col-5)*0x0002);
        }
    }
    return QVariant();
}

QVariant myTable::headerData(int section, Qt::Orientation orientation, int role) const
{
    if(role == Qt::DisplayRole && orientation == Qt::Horizontal)
    {
        return headerList[section];
    }
    return QVariant();
}

bool myTable::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if(!isOpenComPort)
    {
        QMessageBox::information(0,"提示","请打开串口！",QMessageBox::Ok);
        return false;
    }

    if (role == Qt::EditRole)
    {
        decltype(index.row()) row = index.row();
        decltype(index.column()) col = index.column();
        if(col == 0 || col == 4)
            return false;
        if(col > 0 && col < 4)
        {
            //UData[row][col-1] = value.toInt();
            UData[row][col-1] = value.toFloat();
        }
        else if(col > 4 && col <8)
        {
            //IData[row][col-5] = value.toInt();
            IData[row][col-5] = value.toFloat();
        }
        emit signal_editCompleted(value.toFloat(),index.data(Qt::UserRole).toInt());

    }
    return true;
}

Qt::ItemFlags myTable::flags(const QModelIndex &index) const
{
    return Qt::ItemIsSelectable | Qt::ItemIsEnabled |
            Qt::ItemIsEditable | QAbstractTableModel::flags(index);
}

void myTable::setIsOpenComPort(bool flag)
{
    isOpenComPort = flag;
}




