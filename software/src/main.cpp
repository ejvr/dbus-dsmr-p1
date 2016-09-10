#include <QCoreApplication>
#include <QCommandLineParser>
#include <QTextStream>
#include <velib/qt/ve_qitem.hpp>
#include <velib/qt/ve_qitem_dbus_publisher.hpp>
#include "dsmr_p1_acquisitor.h"

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    app.setApplicationVersion(VERSION);
    app.setApplicationName(argv[0]);

    QCommandLineParser parser;
    parser.setApplicationDescription("Retrieves data from a P1 grid meter and publishes it to the D-Bus");
    parser.addPositionalArgument("port", "Name of the port where the P1 meter is connected", "port");
    QCommandLineOption helpOption = parser.addHelpOption();
    QCommandLineOption versionOption = parser.addVersionOption();
    QCommandLineOption dbusAddressOption{"dbus", "D-Bus address (system, session, etc...)",
                                         "D-Bus address", "system" };
    parser.addOption(dbusAddressOption);

    if (!parser.parse(QCoreApplication::arguments()) || parser.positionalArguments().isEmpty()) {
        QTextStream out(stdout);
        out << parser.errorText() << parser.helpText();
        return -1;
    }

    if (parser.isSet(versionOption)) {
        QTextStream out(stdout);
        out << VERSION << '\n';
        return -2;
    }

    if (parser.isSet(helpOption)) {
        QTextStream out(stdout);
        out << parser.helpText();
        return -3;
    }

    QStringList posArgs = parser.positionalArguments();
    QString portName = posArgs.first();
    QString dbusAddress = parser.value(dbusAddressOption);

    VeQItemProducer itemProducer(VeQItems::getRoot(), "dbus");
    itemProducer.open();

    VeQItemDbusPublisher dbusPublisher(itemProducer.services());
    dbusPublisher.open(dbusAddress);

    DsmrP1Acquisitor acquisitor(portName, itemProducer.services());
    app.connect(&acquisitor, SIGNAL(connectionLost()), &app, SLOT(quit()));

    if (!acquisitor.start())
        return -4;

    return app.exec();
}
