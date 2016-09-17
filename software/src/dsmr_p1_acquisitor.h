#ifndef DSMRP1ACQUISITOR_H
#define DSMRP1ACQUISITOR_H

#include <QObject>
#include <QScopedPointer>
#include <QSerialPort>

class DsmrP1Parser;
class DsmrP1Message;
class QIODevice;
class QRegularExpression;
class QTimer;
class VeQItem;

class DsmrP1Acquisitor : public QObject
{
    Q_OBJECT
public:
    DsmrP1Acquisitor(const QString &portName, VeQItem *root, QObject *parent = 0);

    DsmrP1Acquisitor(QIODevice *ioDevice, VeQItem *root, QObject *parent = 0);

    virtual ~DsmrP1Acquisitor();

    bool start();

    void handleMessage(const DsmrP1Message &msg);

signals:
    void connectionLost();

private slots:
    void onReadyRead();

    void onError(QSerialPort::SerialPortError error);

    void onTimeout();

private:
    DsmrP1Acquisitor(VeQItem *root, QObject *parent = 0);

    void initService(const QString &serviceName);

    static void produce(VeQItem *item, const QVariant &value, const QString &text);

    static void produce(VeQItem *item, const QVariant &value);

    static void produce(VeQItem *item, double value, const QString &unit, int precision);

    static double safeAdd(double d0, double d1);

    static double getDouble(const QString &payLoad);

    static QString fromHexString(const QString &hexString);

    QIODevice *mIoDevice;
    QTimer *mTimer;
    QScopedPointer<DsmrP1Parser> mParser;
    QScopedPointer<QRegularExpression> mLineRegex;
    QString mPortName;
    int mDeviceInstance;

    VeQItem *mRoot;
    VeQItem *mProductName;
    VeQItem *mSerial;
    VeQItem *mPower;
    VeQItem *mEnergyForward;
    VeQItem *mEnergyReverse;
    VeQItem *mL1Power;
    VeQItem *mL1EnergyForward;
    VeQItem *mL1EnergyReverse;
    VeQItem *mL1Current;
};

#endif // DSMRP1ACQUISITOR_H
