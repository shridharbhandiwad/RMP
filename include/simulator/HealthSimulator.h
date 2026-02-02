#ifndef HEALTHSIMULATOR_H
#define HEALTHSIMULATOR_H

#include <QObject>
#include <QTimer>
#include <QMap>
#include <random>
#include "core/HealthStatus.h"

namespace RadarRMP {

class SubsystemManager;
class RadarSubsystem;

/**
 * @brief Health data simulator for testing and demonstration
 * 
 * Generates realistic health telemetry data for all subsystems.
 * Supports various simulation scenarios including normal operation,
 * degraded states, and failure conditions.
 */
class HealthSimulator : public QObject {
    Q_OBJECT
    Q_PROPERTY(bool running READ isRunning NOTIFY runningChanged)
    Q_PROPERTY(int updateInterval READ getUpdateInterval WRITE setUpdateInterval NOTIFY intervalChanged)
    Q_PROPERTY(QString scenario READ getScenario WRITE setScenario NOTIFY scenarioChanged)
    Q_PROPERTY(double faultProbability READ getFaultProbability WRITE setFaultProbability NOTIFY settingsChanged)
    
public:
    /**
     * @brief Simulation scenarios
     */
    enum class Scenario {
        Normal,           // All systems operating normally
        Degraded,         // Some systems showing degradation
        HighStress,       // Systems under high load
        ThermalStress,    // Elevated temperatures
        PowerIssues,      // Power supply problems
        PartialFailure,   // Some components failed
        Recovery,         // Recovering from failure
        Random            // Random state changes
    };
    Q_ENUM(Scenario)
    
    explicit HealthSimulator(SubsystemManager* manager, QObject* parent = nullptr);
    ~HealthSimulator() override;
    
    // Simulation control
    Q_INVOKABLE void start();
    Q_INVOKABLE void stop();
    Q_INVOKABLE void pause();
    Q_INVOKABLE void resume();
    Q_INVOKABLE void step();  // Single update step
    
    bool isRunning() const;
    bool isPaused() const;
    
    // Configuration
    int getUpdateInterval() const;
    void setUpdateInterval(int msec);
    
    QString getScenario() const;
    void setScenario(const QString& scenario);
    void setScenario(Scenario scenario);
    
    double getFaultProbability() const;
    void setFaultProbability(double probability);
    
    // Manual fault injection
    Q_INVOKABLE void injectFault(const QString& subsystemId, const QString& faultCode);
    Q_INVOKABLE void clearInjectedFault(const QString& subsystemId, const QString& faultCode);
    Q_INVOKABLE void clearAllInjectedFaults();
    
    // Scenario presets
    Q_INVOKABLE void loadNormalScenario();
    Q_INVOKABLE void loadDegradedScenario();
    Q_INVOKABLE void loadFailureScenario();
    Q_INVOKABLE void loadStressTestScenario();
    
signals:
    void runningChanged();
    void intervalChanged();
    void scenarioChanged();
    void settingsChanged();
    void dataGenerated(const QString& subsystemId, const QVariantMap& data);
    void faultInjected(const QString& subsystemId, const QString& faultCode);
    
private slots:
    void onUpdateTick();
    
private:
    // Data generation for each subsystem type
    QVariantMap generateTransmitterData();
    QVariantMap generateReceiverData();
    QVariantMap generateAntennaData();
    QVariantMap generateRFData();
    QVariantMap generateSignalProcessorData();
    QVariantMap generateDataProcessorData();
    QVariantMap generatePowerSupplyData();
    QVariantMap generateCoolingData();
    QVariantMap generateTimingSyncData();
    QVariantMap generateNetworkData();
    
    // Helper methods
    double generateValue(double nominal, double variance, double trend = 0.0);
    double applyScenarioModifier(double value, const QString& parameter);
    bool shouldInjectFault();
    QString selectRandomFault(SubsystemType type);
    
    SubsystemManager* m_manager;
    QTimer* m_updateTimer;
    
    Scenario m_scenario;
    double m_faultProbability;
    int m_updateInterval;
    bool m_running;
    bool m_paused;
    
    // State tracking for realistic simulation
    QMap<QString, double> m_trendValues;
    QMap<QString, QStringList> m_injectedFaults;
    
    // Random number generation
    std::mt19937 m_rng;
    std::normal_distribution<double> m_normalDist;
    std::uniform_real_distribution<double> m_uniformDist;
    
    qint64 m_simulationTime;  // Simulated elapsed time in ms
};

} // namespace RadarRMP

#endif // HEALTHSIMULATOR_H
