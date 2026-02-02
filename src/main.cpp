#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQuickStyle>
#include <QtQml>

#include "core/SubsystemManager.h"
#include "core/HealthDataPipeline.h"
#include "core/FaultManager.h"

#include "subsystems/TransmitterSubsystem.h"
#include "subsystems/ReceiverSubsystem.h"
#include "subsystems/AntennaServoSubsystem.h"
#include "subsystems/RFFrontEndSubsystem.h"
#include "subsystems/SignalProcessorSubsystem.h"
#include "subsystems/DataProcessorSubsystem.h"
#include "subsystems/PowerSupplySubsystem.h"
#include "subsystems/CoolingSubsystem.h"
#include "subsystems/TimingSyncSubsystem.h"
#include "subsystems/NetworkInterfaceSubsystem.h"

#include "simulator/HealthSimulator.h"
#include "simulator/FaultInjector.h"

#include "analytics/HealthAnalytics.h"
#include "analytics/TrendAnalyzer.h"
#include "analytics/UptimeTracker.h"

using namespace RadarRMP;

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);
    
    app.setApplicationName("Radar Maintenance Processor");
    app.setApplicationVersion("1.0.0");
    app.setOrganizationName("RadarRMP");
    
    // Set the Quick Controls style
    QQuickStyle::setStyle("Universal");
    
    // Register meta types
    qRegisterMetaType<RadarRMP::HealthState>("RadarRMP::HealthState");
    qRegisterMetaType<RadarRMP::FaultSeverity>("RadarRMP::FaultSeverity");
    qRegisterMetaType<RadarRMP::SubsystemType>("RadarRMP::SubsystemType");
    qRegisterMetaType<RadarRMP::FaultCode>("RadarRMP::FaultCode");
    
    // Create subsystem manager
    SubsystemManager* subsystemManager = new SubsystemManager();
    
    // Create and register all subsystems
    TransmitterSubsystem* tx = new TransmitterSubsystem("TX-001", "Main Transmitter");
    ReceiverSubsystem* rx = new ReceiverSubsystem("RX-001", "Main Receiver");
    AntennaServoSubsystem* ant = new AntennaServoSubsystem("ANT-001", "Antenna & Servo");
    RFFrontEndSubsystem* rf = new RFFrontEndSubsystem("RF-001", "RF Front-End");
    SignalProcessorSubsystem* sp = new SignalProcessorSubsystem("SP-001", "Signal Processor");
    DataProcessorSubsystem* dp = new DataProcessorSubsystem("DP-001", "Data Processor");
    PowerSupplySubsystem* psu = new PowerSupplySubsystem("PSU-001", "Power Supply");
    CoolingSubsystem* cool = new CoolingSubsystem("COOL-001", "Cooling System");
    TimingSyncSubsystem* timing = new TimingSyncSubsystem("TIME-001", "Timing & Sync");
    NetworkInterfaceSubsystem* net = new NetworkInterfaceSubsystem("NET-001", "Network Interface");
    
    subsystemManager->registerSubsystem(tx);
    subsystemManager->registerSubsystem(rx);
    subsystemManager->registerSubsystem(ant);
    subsystemManager->registerSubsystem(rf);
    subsystemManager->registerSubsystem(sp);
    subsystemManager->registerSubsystem(dp);
    subsystemManager->registerSubsystem(psu);
    subsystemManager->registerSubsystem(cool);
    subsystemManager->registerSubsystem(timing);
    subsystemManager->registerSubsystem(net);
    
    // Add default subsystems to canvas
    subsystemManager->addToCanvas("TX-001");
    subsystemManager->addToCanvas("RX-001");
    subsystemManager->addToCanvas("ANT-001");
    subsystemManager->addToCanvas("PSU-001");
    subsystemManager->addToCanvas("COOL-001");
    
    // Create health data pipeline
    HealthDataPipeline* pipeline = new HealthDataPipeline();
    
    // Create simulator
    HealthSimulator* simulator = new HealthSimulator(subsystemManager);
    
    // Create fault injector
    FaultInjector* faultInjector = new FaultInjector(subsystemManager);
    
    // Create analytics
    HealthAnalytics* analytics = new HealthAnalytics(subsystemManager);
    TrendAnalyzer* trendAnalyzer = new TrendAnalyzer();
    UptimeTracker* uptimeTracker = new UptimeTracker();
    
    // Register subsystems with uptime tracker
    for (auto* sub : subsystemManager->getAllSubsystems()) {
        uptimeTracker->registerSubsystem(sub->getId());
        
        // Connect health changes to uptime tracker
        QObject::connect(sub, &RadarSubsystem::healthChanged,
                        [uptimeTracker, sub]() {
                            uptimeTracker->updateState(sub->getId(), sub->getHealthState());
                        });
        
        // Connect to trend analyzer
        QObject::connect(sub, &RadarSubsystem::telemetryChanged,
                        [trendAnalyzer, sub]() {
                            trendAnalyzer->addDataPoints(sub->getId(), sub->getTelemetry());
                        });
    }
    
    // Create QML engine
    QQmlApplicationEngine engine;
    
    // Expose C++ objects to QML
    engine.rootContext()->setContextProperty("subsystemManager", subsystemManager);
    engine.rootContext()->setContextProperty("healthPipeline", pipeline);
    engine.rootContext()->setContextProperty("healthSimulator", simulator);
    engine.rootContext()->setContextProperty("faultInjector", faultInjector);
    engine.rootContext()->setContextProperty("healthAnalytics", analytics);
    engine.rootContext()->setContextProperty("trendAnalyzer", trendAnalyzer);
    engine.rootContext()->setContextProperty("uptimeTracker", uptimeTracker);
    
    // Load QML
    const QUrl url(QStringLiteral("qrc:/qml/Main.qml"));
    
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreated,
                     &app, [url](QObject *obj, const QUrl &objUrl) {
        if (!obj && url == objUrl)
            QCoreApplication::exit(-1);
    }, Qt::QueuedConnection);
    
    engine.load(url);
    
    // Start the simulator
    simulator->start();
    
    // Start subsystem manager updates
    subsystemManager->startUpdates();
    
    return app.exec();
}
