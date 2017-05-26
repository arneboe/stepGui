#pragma once
#include <QtGui/QMainWindow>
#include <QtGui/QLineEdit>
#include <QtGui/QProgressBar>
#include <unordered_map>
#include <functional>
#include "../../engine.hpp"

class QSpinBox;
class QCustomPlot;
class TrajectoryModel;
class QCPGraph;

class StepGui : public QMainWindow
{
    Q_OBJECT

public:
    
    /** @param sendCallback A callback that is invoked when the user clicks on "Send".*/
    StepGui(std::function<void(const Step&, unsigned short index)> sendCallback, std::function<void(const StepSequence&)> seqCallback);
    
    virtual ~StepGui();
    
private slots:
    /**Is called when the user changes the global step length */
    void setStepLength(int ms);
    
    /**Is called when the user changes the step size of a trajectory
     * HACK sender()->objectName needs to be the name of the corresponding servo*/
    void setTrajectorySize(int size);
    
    /**Is called when the user changes trajectory data */
    void trajectoryChanged(const std::string& servoName, const Trajectory& traj);
    
    /**Is called when the user clicks "Send" */
    void sendStep();
    /**save data to file */
    void save();
    /**load data from file */
    void load();

    /** Is called when user clicks "Send sequence" */
    void sendSeq();

    void loadPreLoad();

    void sendPreLoad();
    
private:
    
    /**Change size of trajectory @p servoName */
    void setTrajectorySize(int size, const std::string& servoName);
    
    /**Update @p graph from @p data */
    void setGraphData(QCPGraph* graph, const Trajectory& data);

    void loadFile(QString fileName);
    
    Step step;
    StepSequence seq;
    /**mappings from servo names to ui elements */
    std::unordered_map<std::string, TrajectoryModel*> trajModels;
    std::unordered_map<std::string, QCustomPlot*> plots;
    std::unordered_map<std::string, QCPGraph*> graphs;
    std::unordered_map<std::string, QSpinBox*> sizes;
    
    QCustomPlot* combinedPlot;
    std::vector<std::string> servoNames;
    std::function<void(const Step&, unsigned short index)> sendCallback;
    std::function<void(const StepSequence&)> seqCallback;
    QSpinBox* stepLenBox;
    QSpinBox* stepIndex;
    QLineEdit* stepSeqEdit;
    QLineEdit* preloadPath;
    QProgressBar* bar;
};

