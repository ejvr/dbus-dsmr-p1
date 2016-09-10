#include <QCoreApplication>
#include <QRegularExpression>
#include <QTimer>
#include <velib/qt/ve_qitem.hpp>
#include "dsmr_p1_acquisitor.h"
#include "dsmr_p1_parser.h"

static const QString ServiceName = "com.victronenergy.gridmeter";
static const int UsbDeviceInstance = 288;

DsmrP1Acquisitor::DsmrP1Acquisitor(const QString &portName, VeQItem *root, QObject *parent):
    DsmrP1Acquisitor(root, parent)
{
    Q_ASSERT(!portName.isEmpty());
    mPortName = portName;
    /// @todo EV We assume we use ttyUSBx to create the device instance.
    mDeviceInstance = UsbDeviceInstance + portName.rightRef(1).toInt();
    QSerialPort *serialPort = new QSerialPort(this);
    mIoDevice = serialPort;
    serialPort->setBaudRate(115200);
    serialPort->setPortName(portName);
    serialPort->setDataBits(QSerialPort::Data8);
    serialPort->setParity(QSerialPort::NoParity);
    serialPort->setStopBits(QSerialPort::OneStop);
    connect(serialPort, SIGNAL(readyRead()), this, SLOT(onReadyRead()));
    connect(serialPort, SIGNAL(error(QSerialPort::SerialPortError)),
            this, SLOT(onError(QSerialPort::SerialPortError)));
}

DsmrP1Acquisitor::DsmrP1Acquisitor(QIODevice *ioDevice, VeQItem *root, QObject *parent):
    DsmrP1Acquisitor(root, parent)
{
    Q_ASSERT(ioDevice != 0);
    mIoDevice = ioDevice;
    mPortName = "p1";
    connect(ioDevice, SIGNAL(readyRead()), this, SLOT(onReadyRead()));
}

DsmrP1Acquisitor::DsmrP1Acquisitor(VeQItem *root, QObject *parent):
    QObject(parent),
    mTimer(new QTimer(this)),
    mParser(new DsmrP1Parser),
    mLineRegex(new QRegularExpression("(\\d+)-(\\d+):(\\d+)\\.(\\d+)\\.(\\d+)\\(([^)]+)\\)")),
    mDeviceInstance(0),
    mRoot(root),
    mProductName(0),
    mSerial(0),
    mPower(0),
    mEnergyForward(0),
    mEnergyReverse(0),
    mL1Power(0),
    mL1EnergyForward(0),
    mL1EnergyReverse(0),
    mL1Current(0)
{
    Q_ASSERT(root != 0);
    mTimer->setInterval(12000);
    connect(mTimer, SIGNAL(timeout()), this, SLOT(onTimeout()));
}

DsmrP1Acquisitor::~DsmrP1Acquisitor()
{
}

bool DsmrP1Acquisitor::start()
{
    if (!mIoDevice->open(QIODevice::ReadOnly)) {
        qDebug() << "Could not open port";
        return false;
    }
    mTimer->start();
    return true;
}

void DsmrP1Acquisitor::handleMessage(const DsmrP1Message &msg)
{
    mTimer->start();
    // From http://www.dlms.com/faqanswers/questionsonthedlmscosemspecification/obisnameswhatarethey.php
    // OBIS uses six value groups in a hierarchical structure.
    // * Value group A identifies the energy type measured. A=0 identifies abstract codes.
    //   Value group A has reserved space for future extensions.
    // * Value group B identifies the measuring channels.
    // * Value group C identifies the physical quantity measured,
    // * Value group D identifies the processing methods and country specific codes.
    // * Value group E is used for identifying rates or can be used for further
    //   classification.
    // * Value group F is used for identifying historical values or can be used for further
    //   classification.
    // Groups B to D have code space for manufacturer specific identifiers.
    double power = qQNaN();
    double current = qQNaN();
    double energyForward = qQNaN();
    double energyReverse  = qQNaN();
    if (mProductName == 0) {
        QString s = mPortName;
        s.replace("/dev/", "").replace('/', '_');
        initService(QString("%1.%2").arg(ServiceName).arg(s));
    }
    produce(mProductName, msg.type);
    for (QString line: msg.lines) {
        QRegularExpressionMatch m = mLineRegex->match(line);
        if (m.hasMatch()) {
            int obisType = m.captured(1).toInt();
            int obisChannel = m.captured(2).toInt();
            int obisQuantity = m.captured(3).toInt();
            int obisProcessing = m.captured(4).toInt();
            int obisMisc = m.captured(5).toInt();
            QString payload = m.captured(6);
            switch (obisType)
            {
            case 0:
                switch (obisQuantity)
                {
                case 24:
                    switch (obisProcessing)
                    {
                    case 2:
                        // Gass volume
                        break;
                    }
                    break;
                case 96:
                    switch (obisChannel)
                    {
                    case 0:
                        switch (obisProcessing)
                        {
                        case 1:
                            produce(mSerial, payload);
                            break;
                        case 14:
                            // Tariff, 1 = Low, 2 = Normal (?)
                            break;
                        }
                        break;
                    case 1:
                        switch (obisProcessing)
                        {
                        case 1:
                            // Gass meter serial?
                            break;
                        }
                        break;
                    }
                    break;
                }
                break;
            case 1:
                switch (obisQuantity)
                {
                case 1:
                    switch (obisProcessing)
                    {
                    case 7:
                        // Power in
                        power = safeAdd(power, getDouble(payload));
                        break;
                    case 8:
                        switch (obisMisc)
                        {
                        case 1:
                            // Low tariff energy in
                            energyForward = safeAdd(energyForward, getDouble(payload));
                            break;
                        case 2:
                            // Normal tariff energy in
                            energyForward = safeAdd(energyForward, getDouble(payload));
                            break;
                        }
                        break;
                    }
                    break;
                case 2:
                    switch (obisProcessing)
                    {
                    case 7:
                        // Power out
                        power = safeAdd(power, -getDouble(payload));
                        break;
                    case 8:
                        switch (obisMisc)
                        {
                        case 1:
                            // Low tariff energy out
                            energyReverse = safeAdd(energyReverse, getDouble(payload));
                            break;
                        case 2:
                            // Normal tariff energy out
                            energyReverse = safeAdd(energyReverse, getDouble(payload));
                            break;
                        }
                        break;
                    }
                    break;
                case 31:
                    switch (obisProcessing)
                    {
                    case 7:
                        // AC current?
                        current = getDouble(payload);
                        break;
                    }
                }
                break;
            }
        }
    }
    power *= 1000;
    if (!qIsFinite(power))
        current = qQNaN();
    if (power < 0)
        current = -current;
    produce(mPower, power, "W", 1);
    produce(mL1Power, power, "W", 1);
    produce(mL1Current, current, "A", 2);
    produce(mEnergyForward, energyForward, "kWh", 2);
    produce(mL1EnergyForward, energyForward, "kWh", 2);
    produce(mEnergyReverse, energyReverse, "kWh", 2);
    produce(mL1EnergyReverse, energyReverse, "kWh", 2);
}

void DsmrP1Acquisitor::onReadyRead()
{
    QByteArray r = mIoDevice->read(mIoDevice->bytesAvailable());
    DsmrP1Message msg;
    for (char c: r)
    {
        if (mParser->process(c, msg) == DsmrP1State::Ok)
            handleMessage(msg);
    }
}

void DsmrP1Acquisitor::onError(QSerialPort::SerialPortError error)
{
    qDebug() << __func__ << error;
    if (error == QSerialPort::ResourceError) {
        qDebug() << "Device unplugged";
        emit connectionLost();
    }
}

void DsmrP1Acquisitor::onTimeout()
{
    qDebug() << "Communication timeout";
    emit connectionLost();
}

void DsmrP1Acquisitor::initService(const QString &serviceName)
{
    Q_ASSERT(mProductName == 0);
    Q_ASSERT(mRoot != 0);
    VeQItem *service = mRoot->itemGetOrCreate(serviceName, false);
    service->produceValue(QVariant(), VeQItem::Synchronized);
    produce(service->itemGetOrCreate("ProductId"), 0x0000); /// @todo EV Reserve product ID
    produce(service->itemGetOrCreate("DeviceInstance"), mDeviceInstance);
    produce(service->itemGetOrCreate("Connected"), 1);
    produce(service->itemGetOrCreate("Mgmt/Connection"), mPortName);
    produce(service->itemGetOrCreate("Mgmt/ProcessName"), QCoreApplication::applicationName());
    produce(service->itemGetOrCreate("Mgmt/ProcessVersion"), QCoreApplication::applicationVersion());
    mProductName = service->itemGetOrCreate("ProductName");
    mSerial = service->itemGetOrCreate("Serial");
    mPower = service->itemGetOrCreate("Ac/Power");
    mEnergyForward = service->itemGetOrCreate("Ac/EnergyForward");
    mEnergyReverse = service->itemGetOrCreate("Ac/EnergyReverse");
    mL1Power = service->itemGetOrCreate("Ac/L1/Power");
    mL1Current = service->itemGetOrCreate("Ac/L1/Current");
    mL1EnergyForward = service->itemGetOrCreate("Ac/L1/EnergyForward");
    mL1EnergyReverse = service->itemGetOrCreate("Ac/L1/EnergyReverse");
}

void DsmrP1Acquisitor::produce(VeQItem *item, const QVariant &value, const QString &text)
{
    item->produceValue(value);
    item->produceText(text);
}

void DsmrP1Acquisitor::produce(VeQItem *item, const QVariant &value)
{
    produce(item, value, value.toString());
}

void DsmrP1Acquisitor::produce(VeQItem *item, double value, const QString &unit, int precision)
{
    if (!qIsFinite(value)) {
        produce(item, QVariant(), "");
        return;
    }
    QString text = QString::number(value, 'f', precision);
    text += unit;
    produce(item, value, text);
}

double DsmrP1Acquisitor::safeAdd(double d0, double d1)
{
    if (!qIsFinite(d0))
        return d1;
    if (!qIsFinite(d1))
        return d0;
    return d0 + d1;
}

double DsmrP1Acquisitor::getDouble(const QString &payLoad)
{
    int i = payLoad.indexOf("*");
    if (i == -1)
        return payLoad.toDouble();
    return payLoad.leftRef(i).toDouble();
}
