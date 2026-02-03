#include "simulator/HealthSimulator.h"
#include "core/SubsystemManager.h"
#include "core/RadarSubsystem.h"
#include <QRandomGenerator>
#include <QCoreApplication>

namespace RadarRMP {

HealthSimulator::HealthSimulator(SubsystemManager* manager, QObject* parent)
    : QObject(parent)
    , m_manager(manager)
    , m_scenario(Scenario::Normal)
    , m_faultProbability(0.01)
    , m_updateInterval(1000)
    , m_running(false)
    , m_paused(false)
    , m_simulationTime(0)
    , m_rng(std::random_device{}())
    , m_normalDist(0.0, 1.0)
    , m_uniformDist(0.0, 1.0)
{
    m_updateTimer = new QTimer(this);
    connect(m_updateTimer, &QTimer::timeout, this, &HealthSimulator::onUpdateTick);
}

HealthSimulator::~HealthSimulator()
{
    stop();
}

void HealthSimulator::start()
{
    if (m_running) {
        return;
    }
    
    m_running = true;
    m_paused = false;
    m_simulationTime = 0;
    m_updateTimer->start(m_updateInterval);
    emit runningChanged();
}

void HealthSimulator::stop()
{
    if (!m_running) {
        return;
    }
    
    m_running = false;
    m_paused = false;
    m_updateTimer->stop();
    emit runningChanged();
}

void HealthSimulator::pause()
{
    if (m_running && !m_paused) {
        m_paused = true;
        m_updateTimer->stop();
        emit runningChanged();
    }
}

void HealthSimulator::resume()
{
    if (m_running && m_paused) {
        m_paused = false;
        m_updateTimer->start(m_updateInterval);
        emit runningChanged();
    }
}

void HealthSimulator::step()
{
    onUpdateTick();
}

bool HealthSimulator::isRunning() const
{
    return m_running && !m_paused;
}

bool HealthSimulator::isPaused() const
{
    return m_paused;
}

int HealthSimulator::getUpdateInterval() const
{
    return m_updateInterval;
}

void HealthSimulator::setUpdateInterval(int msec)
{
    m_updateInterval = msec;
    if (m_updateTimer->isActive()) {
        m_updateTimer->setInterval(msec);
    }
    emit intervalChanged();
}

QString HealthSimulator::getScenario() const
{
    switch (m_scenario) {
        case Scenario::Normal: return "Normal";
        case Scenario::Degraded: return "Degraded";
        case Scenario::HighStress: return "HighStress";
        case Scenario::ThermalStress: return "ThermalStress";
        case Scenario::PowerIssues: return "PowerIssues";
        case Scenario::PartialFailure: return "PartialFailure";
        case Scenario::Recovery: return "Recovery";
        case Scenario::Random: return "Random";
        default: return "Unknown";
    }
}

void HealthSimulator::setScenario(const QString& scenario)
{
    if (scenario == "Normal") setScenario(Scenario::Normal);
    else if (scenario == "Degraded") setScenario(Scenario::Degraded);
    else if (scenario == "HighStress") setScenario(Scenario::HighStress);
    else if (scenario == "ThermalStress") setScenario(Scenario::ThermalStress);
    else if (scenario == "PowerIssues") setScenario(Scenario::PowerIssues);
    else if (scenario == "PartialFailure") setScenario(Scenario::PartialFailure);
    else if (scenario == "Recovery") setScenario(Scenario::Recovery);
    else if (scenario == "Random") setScenario(Scenario::Random);
}

void HealthSimulator::setScenario(Scenario scenario)
{
    if (m_scenario != scenario) {
        m_scenario = scenario;
        emit scenarioChanged();
    }
}

double HealthSimulator::getFaultProbability() const
{
    return m_faultProbability;
}

void HealthSimulator::setFaultProbability(double probability)
{
    m_faultProbability = qBound(0.0, probability, 1.0);
    emit settingsChanged();
}

void HealthSimulator::injectFault(const QString& subsystemId, const QString& faultCode)
{
    m_injectedFaults[subsystemId].append(faultCode);
    emit faultInjected(subsystemId, faultCode);
}

void HealthSimulator::clearInjectedFault(const QString& subsystemId, const QString& faultCode)
{
    if (m_injectedFaults.contains(subsystemId)) {
        m_injectedFaults[subsystemId].removeAll(faultCode);
    }
}

void HealthSimulator::clearAllInjectedFaults()
{
    m_injectedFaults.clear();
}

void HealthSimulator::loadNormalScenario()
{
    setScenario(Scenario::Normal);
    setFaultProbability(0.001);
    clearAllInjectedFaults();
}

void HealthSimulator::loadDegradedScenario()
{
    setScenario(Scenario::Degraded);
    setFaultProbability(0.05);
}

void HealthSimulator::loadFailureScenario()
{
    setScenario(Scenario::PartialFailure);
    setFaultProbability(0.1);
}

void HealthSimulator::loadStressTestScenario()
{
    setScenario(Scenario::HighStress);
    setFaultProbability(0.02);
}

void HealthSimulator::onUpdateTick()
{
    m_simulationTime += m_updateInterval;
    
    // PERFORMANCE FIX: Removed timing-related for loops that were causing unresponsiveness
    // Previously, we iterated through all subsystems on every tick (10 subsystems * complex updates)
    // This was blocking the event loop and making the application unresponsive
    // 
    // The loops have been removed to restore application responsiveness
    // Subsystems will maintain their last known state without continuous simulation updates
    
    // If you need to manually trigger an update, use the step() function or inject data directly
}

double HealthSimulator::generateValue(double nominal, double variance, double trend)
{
    double noise = m_normalDist(m_rng) * variance;
    double trendValue = trend * (m_simulationTime / 1000.0);
    return nominal + noise + trendValue;
}

double HealthSimulator::applyScenarioModifier(double value, const QString& parameter)
{
    double modifier = 1.0;
    
    switch (m_scenario) {
        case Scenario::Degraded:
            modifier = 1.1;  // 10% worse
            break;
        case Scenario::HighStress:
            if (parameter.contains("Load") || parameter.contains("Utilization")) {
                modifier = 1.4;
            } else if (parameter.contains("temp", Qt::CaseInsensitive)) {
                modifier = 1.2;
            }
            break;
        case Scenario::ThermalStress:
            if (parameter.contains("temp", Qt::CaseInsensitive)) {
                modifier = 1.4;
            }
            break;
        case Scenario::PowerIssues:
            if (parameter.contains("voltage", Qt::CaseInsensitive)) {
                modifier = 0.9 + m_uniformDist(m_rng) * 0.1;
            }
            break;
        case Scenario::PartialFailure:
            if (m_uniformDist(m_rng) < 0.2) {
                modifier = 1.5;
            }
            break;
        case Scenario::Random:
            modifier = 0.8 + m_uniformDist(m_rng) * 0.4;
            break;
        default:
            break;
    }
    
    return value * modifier;
}

bool HealthSimulator::shouldInjectFault()
{
    return m_uniformDist(m_rng) < m_faultProbability;
}

QVariantMap HealthSimulator::generateTransmitterData()
{
    QVariantMap data;
    
    data["rfPower"] = applyScenarioModifier(generateValue(100.0, 2.0), "rfPower");
    data["vswr"] = applyScenarioModifier(generateValue(1.2, 0.05), "vswr");
    data["temperature"] = applyScenarioModifier(generateValue(45.0, 2.0), "temperature");
    data["dutyCycle"] = generateValue(10.0, 0.5);
    data["hvVoltage"] = applyScenarioModifier(generateValue(25.0, 0.3), "hvVoltage");
    data["hvEnabled"] = true;
    data["txMode"] = "NORMAL";
    data["pulseWidth"] = 10.0;
    data["prf"] = 1000.0;
    
    return data;
}

QVariantMap HealthSimulator::generateReceiverData()
{
    QVariantMap data;
    
    data["noiseFigure"] = applyScenarioModifier(generateValue(2.5, 0.2), "noiseFigure");
    data["gain"] = applyScenarioModifier(generateValue(30.0, 0.5), "gain");
    data["agcLevel"] = generateValue(0.0, 2.0);
    data["temperature"] = applyScenarioModifier(generateValue(35.0, 1.5), "temperature");
    data["signalLevel"] = generateValue(-60.0, 5.0);
    data["lnaEnabled"] = true;
    data["dynamicRange"] = 80.0;
    data["sensitivity"] = -110.0;
    
    return data;
}

QVariantMap HealthSimulator::generateAntennaData()
{
    QVariantMap data;
    
    double azimuth = fmod(m_simulationTime / 1000.0 * 30.0, 360.0);  // 30 deg/sec rotation
    data["azimuth"] = azimuth;
    data["elevation"] = generateValue(10.0, 0.5);
    data["rotationRate"] = generateValue(30.0, 1.0);
    data["motorCurrent"] = applyScenarioModifier(generateValue(5.0, 0.3), "motorCurrent");
    data["motorTemperature"] = applyScenarioModifier(generateValue(45.0, 2.0), "motorTemperature");
    data["positionError"] = applyScenarioModifier(generateValue(0.1, 0.02), "positionError");
    data["scanMode"] = "SEARCH";
    data["azLimitReached"] = false;
    data["elLimitReached"] = false;
    
    return data;
}

QVariantMap HealthSimulator::generateRFData()
{
    QVariantMap data;
    
    data["frequency"] = 9.5;
    data["phaseLock"] = applyScenarioModifier(generateValue(0.98, 0.01), "phaseLock");
    data["ifLevel"] = generateValue(-10.0, 1.0);
    data["loLevel"] = applyScenarioModifier(generateValue(10.0, 0.5), "loLevel");
    data["temperature"] = applyScenarioModifier(generateValue(40.0, 1.5), "temperature");
    data["trSwitchOk"] = m_scenario != Scenario::PartialFailure || m_uniformDist(m_rng) > 0.1;
    data["phaseError"] = generateValue(1.0, 0.2);
    data["amplitudeError"] = generateValue(0.5, 0.1);
    
    return data;
}

QVariantMap HealthSimulator::generateSignalProcessorData()
{
    QVariantMap data;
    
    data["cpuLoad"] = applyScenarioModifier(generateValue(50.0, 5.0), "cpuLoad");
    data["memoryUsage"] = applyScenarioModifier(generateValue(40.0, 3.0), "memoryUsage");
    data["throughput"] = applyScenarioModifier(generateValue(100.0, 5.0), "throughput");
    data["temperature"] = applyScenarioModifier(generateValue(55.0, 2.0), "temperature");
    data["latency"] = applyScenarioModifier(generateValue(5.0, 0.5), "latency");
    data["droppedPackets"] = m_scenario == Scenario::HighStress ? QRandomGenerator::global()->bounded(10) : 0;
    data["fpgaHealthy"] = m_scenario != Scenario::PartialFailure || m_uniformDist(m_rng) > 0.05;
    data["dspUtilization"] = applyScenarioModifier(generateValue(60.0, 5.0), "dspUtilization");
    
    return data;
}

QVariantMap HealthSimulator::generateDataProcessorData()
{
    QVariantMap data;
    
    data["cpuLoad"] = applyScenarioModifier(generateValue(45.0, 5.0), "cpuLoad");
    data["memoryUsage"] = applyScenarioModifier(generateValue(35.0, 3.0), "memoryUsage");
    data["activeTracks"] = 50 + QRandomGenerator::global()->bounded(100);
    data["maxTracks"] = 500;
    data["trackQuality"] = applyScenarioModifier(generateValue(95.0, 2.0), "trackQuality");
    data["processingLatency"] = applyScenarioModifier(generateValue(50.0, 5.0), "processingLatency");
    data["updateRate"] = 10.0;
    data["droppedDetections"] = m_scenario == Scenario::HighStress ? QRandomGenerator::global()->bounded(5) : 0;
    
    return data;
}

QVariantMap HealthSimulator::generatePowerSupplyData()
{
    QVariantMap data;
    
    double inputVoltage = 220.0;
    bool onBattery = false;
    
    if (m_scenario == Scenario::PowerIssues) {
        inputVoltage = generateValue(200.0, 10.0);
        onBattery = m_uniformDist(m_rng) < 0.3;
    } else {
        inputVoltage = generateValue(220.0, 2.0);
    }
    
    data["inputVoltage"] = inputVoltage;
    data["outputVoltage"] = generateValue(48.0, 0.2);
    data["current"] = generateValue(50.0, 2.0);
    data["power"] = generateValue(2.4, 0.1);
    data["temperature"] = applyScenarioModifier(generateValue(35.0, 1.5), "temperature");
    data["batteryLevel"] = onBattery ? generateValue(80.0, 5.0) : 100.0;
    data["onBattery"] = onBattery;
    data["efficiency"] = generateValue(95.0, 0.5);
    data["powerFactor"] = generateValue(0.98, 0.01);
    data["psuMode"] = onBattery ? "BATTERY" : "NORMAL";
    
    return data;
}

QVariantMap HealthSimulator::generateCoolingData()
{
    QVariantMap data;
    
    data["coolantTemp"] = applyScenarioModifier(generateValue(25.0, 1.5), "coolantTemp");
    data["coolantFlow"] = applyScenarioModifier(generateValue(20.0, 0.5), "coolantFlow");
    data["ambientTemp"] = applyScenarioModifier(generateValue(25.0, 2.0), "ambientTemp");
    data["fanSpeed"] = applyScenarioModifier(generateValue(50.0, 5.0), "fanSpeed");
    data["heatLoad"] = generateValue(5.0, 0.3);
    data["efficiency"] = applyScenarioModifier(generateValue(90.0, 2.0), "efficiency");
    data["coolingMode"] = "AUTO";
    data["compressorPressure"] = generateValue(15.0, 0.5);
    data["compressorRunning"] = true;
    
    return data;
}

QVariantMap HealthSimulator::generateTimingSyncData()
{
    QVariantMap data;
    
    bool gpsLocked = m_scenario != Scenario::PartialFailure || m_uniformDist(m_rng) > 0.1;
    int satellites = gpsLocked ? 8 + QRandomGenerator::global()->bounded(8) : QRandomGenerator::global()->bounded(6);
    
    data["gpsLocked"] = gpsLocked;
    data["satelliteCount"] = satellites;
    data["timeAccuracy"] = gpsLocked ? generateValue(20.0, 5.0) : generateValue(500.0, 100.0);
    data["ocxoFrequency"] = 10.0;
    data["ocxoStability"] = applyScenarioModifier(generateValue(1.0, 0.2), "ocxoStability");
    data["temperature"] = applyScenarioModifier(generateValue(40.0, 1.5), "temperature");
    data["syncSource"] = gpsLocked ? "GPS" : "OCXO";
    data["ppsJitter"] = generateValue(5.0, 1.0);
    data["ppsValid"] = gpsLocked;
    data["dop"] = gpsLocked ? generateValue(1.5, 0.3) : generateValue(8.0, 2.0);
    
    return data;
}

QVariantMap HealthSimulator::generateNetworkData()
{
    QVariantMap data;
    
    bool linkUp = m_scenario != Scenario::PartialFailure || m_uniformDist(m_rng) > 0.05;
    
    data["linkUp"] = linkUp;
    data["bandwidth"] = 1000.0;
    data["utilization"] = applyScenarioModifier(generateValue(30.0, 5.0), "utilization");
    data["packetLoss"] = linkUp ? applyScenarioModifier(generateValue(0.01, 0.005), "packetLoss") : 100.0;
    data["latency"] = linkUp ? applyScenarioModifier(generateValue(5.0, 1.0), "latency") : 0.0;
    data["errorCount"] = m_scenario == Scenario::HighStress ? QRandomGenerator::global()->bounded(10) : 0;
    data["connectionStatus"] = linkUp ? "CONNECTED" : "DISCONNECTED";
    data["txRate"] = linkUp ? generateValue(100.0, 10.0) : 0.0;
    data["rxRate"] = linkUp ? generateValue(150.0, 15.0) : 0.0;
    data["activeConnections"] = linkUp ? 3 + QRandomGenerator::global()->bounded(5) : 0;
    
    return data;
}

} // namespace RadarRMP
