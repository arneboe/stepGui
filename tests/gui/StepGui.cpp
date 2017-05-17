#include "StepGui.hpp"

#include <QtGui/QLabel>
#include <QtGui/QMenu>
#include <QtGui/QMenuBar>
#include <QtGui/QAction>
#include <QtGui/QVBoxLayout>
#include <QtGui/QHBoxLayout>
#include <QtGui/QSplitter>
#include <QtGui/QTableView>
#include <QtGui/QSpinBox>
#include <QtGui/QScrollArea>
#include <QtGui/QGridLayout>
#include <QtGui/QTabWidget>
#include <QtGui/QPushButton>
#include <QApplication>
#include <QtGui/QFileDialog>
#include <iostream>
#include <fstream>
#include "qcustomplot.h"
#include "TrajectoryModel.hpp"


StepGui::StepGui(std::function<void(const Step&)> sendCallback) : sendCallback(sendCallback)
{
    QSplitter* splitter = new QSplitter(Qt::Vertical);
    
    QWidget* middleWidget = new QWidget();
    QHBoxLayout* middleLayout = new QHBoxLayout();
    
    QLabel* stepLenLabel = new QLabel(this);
    stepLenLabel->setText("Step Length (in ms):");
    stepLenLabel->setAlignment(Qt::AlignRight);
    middleLayout->addWidget(stepLenLabel);
    
    stepLenBox = new QSpinBox();
    stepLenBox->setMinimum(1);
    stepLenBox->setMaximum(99999999);
    
    connect(stepLenBox, SIGNAL(valueChanged(int)), this, SLOT(setStepLength(int)));
    middleLayout->addWidget(stepLenBox);
    
    QPushButton* sendButton = new QPushButton(this);
    sendButton->setText("Send");
    middleLayout->addWidget(sendButton);
    
    QPushButton* saveButton = new QPushButton(this);
    saveButton->setText("Save");
    middleLayout->addWidget(saveButton);
    
    QPushButton* loadButton = new QPushButton(this);
    loadButton->setText("Load");
    middleLayout->addWidget(loadButton);
    
    connect(sendButton, SIGNAL(released()), this, SLOT(sendStep()));
    connect(saveButton, SIGNAL(released()), this, SLOT(save()));
    connect(loadButton, SIGNAL(released()), this, SLOT(load()));
    
    
    middleWidget->setLayout(middleLayout);
    middleWidget->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    middleWidget->setMaximumHeight(stepLenBox->height());
    splitter->addWidget(middleWidget);
    splitter->setStretchFactor(0, 0);

    //FIXME needs to be in the same order as enum Servos. 
    servoNames = {"SERVO_FR_HIP",
                  "SERVO_FR_KNEE",
                  "SERVO_BR_HIP",
                  "SERVO_BR_KNEE",
                  "SERVO_BL_HIP",
                  "SERVO_BL_KNEE",
                  "SERVO_FL_HIP",
                  "SERVO_FL_KNEE"};
                                                 
    QTabWidget* plotTabWidget = new QTabWidget(this);
    splitter->addWidget(plotTabWidget);
    splitter->setStretchFactor(1, 1);
                                                 
    QWidget* plotWidget = new QWidget(this);                                                     
    QGridLayout* plotLayout = new QGridLayout(this);
    plotWidget->setLayout(plotLayout);
    plotTabWidget->addTab(plotWidget, "Plots");
    
    for(size_t i = 0; i < servoNames.size(); ++i)
    {
        const std::string& servoName = servoNames[i];
        QCustomPlot* plot = new QCustomPlot(this);
        plots[servoName] = plot;
        plot->addGraph();
        QCPTextElement *title = new QCPTextElement(plot);
        title->setText(QString::fromStdString(servoName));
        plot->plotLayout()->insertRow(0);
        plot->plotLayout()->addElement(0, 0, title);
        plot->xAxis->setLabel("Time");
        plot->yAxis->setLabel("Angle");
        
        plotLayout->addWidget(plot, i/3, i%3);
    }
                                                 
    combinedPlot = new QCustomPlot(this);
    combinedPlot->legend->setVisible(true);
    combinedPlot->xAxis->setLabel("Time");
    combinedPlot->yAxis->setLabel("Angle");
    
    
    std::vector<QPen> colors = {QPen(Qt::red), QPen(Qt::blue), QPen(Qt::green),
                                QPen(Qt::magenta), QPen(Qt::cyan), QPen(Qt::yellow),
                                QPen(Qt::black), QPen(Qt::gray)};
    
    for(size_t i = 0; i < servoNames.size(); ++i)
    {
        const std::string& servoName = servoNames[i];
        combinedPlot->addGraph()->setName(QString::fromStdString(servoName));
        graphs[servoName] = combinedPlot->graph(i);
        graphs[servoName]->setPen(colors[i]);
    }
    
    
    plotTabWidget->addTab(combinedPlot, "Combined Plot");
    
    
    QScrollArea* tableScroll = new QScrollArea(this);
    QWidget* scrollWidget = new QWidget(this);
    QVBoxLayout* tableLayout = new QVBoxLayout(this);
    tableLayout->setSizeConstraint(QLayout::SetMinAndMaxSize);
    scrollWidget->setLayout(tableLayout);
    
    //to get good alignment of the tables :)
    //FIXME we use stepLenLabel here to get the default font, that is stupid 
    //HACK dirty stupid hack :/
    QFont servoNameFont(QApplication::font());
    servoNameFont.setBold(true);
    QFontMetrics fm(servoNameFont);
    int infoAreaWidth = 0;
    for(const std::string& name : servoNames)
    {
        const int width = fm.width(QString::fromStdString(name));
        infoAreaWidth = std::max(width, infoAreaWidth);
    }
    
    tableScroll->setWidget(scrollWidget);
    for(size_t i = 0; i < servoNames.size(); ++i)
    {
        const std::string& servoName = servoNames[i];
        QHBoxLayout* trajLayout = new QHBoxLayout(this);
        QVBoxLayout* infolayout = new QVBoxLayout(this);
        
        QLabel* servoNameLabel = new QLabel(this);
        servoNameLabel->setFont(servoNameFont);
        servoNameLabel->setText(QString::fromStdString(servoName));
        infolayout->addWidget(servoNameLabel);
        trajLayout->addLayout(infolayout);
        
        QLabel* setSizeLabel = new QLabel(this);
        setSizeLabel->setText("Size:");
        setSizeLabel->setMinimumWidth(infoAreaWidth);
        infolayout->addWidget(setSizeLabel);
        QSpinBox* sizeSpinbox = new QSpinBox(this);
        sizes[servoName] = sizeSpinbox;
        sizeSpinbox->setObjectName(QString::fromStdString(servoName)); //HACK to be able to distinguish senders in slot
        sizeSpinbox->setMinimum(1);
        sizeSpinbox->setMaximum(MAX_TRAJECTORY_SIZE);
        connect(sizeSpinbox, SIGNAL(valueChanged(int)), this, SLOT(setTrajectorySize(int)));
        infolayout->addWidget(sizeSpinbox);
        
        trajModels[servoName] = new TrajectoryModel(servoName, step.servoTrajectory[i]);
        
        connect(trajModels[servoName], SIGNAL(trajectoryChanged(const std::string&, const Trajectory&)),
                this, SLOT(trajectoryChanged(const std::string&, const Trajectory&)));
        
        QTableView* table = new QTableView(this);
        table->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        table->setModel(trajModels[servoName]);
        table->setMinimumHeight(table->rowHeight(0) * 3);
        table->setMinimumWidth(table->columnWidth(0) * (MAX_TRAJECTORY_SIZE + 1));
        
        trajLayout->addWidget(table);
        
        tableLayout->addLayout(trajLayout);
    }
    tableScroll->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    splitter->addWidget(tableScroll);
    splitter->setStretchFactor(2, 1);
    
    setCentralWidget(splitter);
    stepLenBox->setValue(200); //this causes a signal which updates the plots
}

void StepGui::setTrajectorySize(int size)
{
    const QString servoName(sender()->objectName());
    setTrajectorySize(size, servoName.toStdString());
}

void StepGui::setTrajectorySize(int size, const std::string& servoName)
{
    trajModels[servoName]->setSize(size);
    sizes[servoName]->blockSignals(true); //to avoid endless signal loop
    sizes[servoName]->setValue(size);
    sizes[servoName]->blockSignals(false);
}



void StepGui::setStepLength(int ms)
{
    step.length = ms;
    for(auto& pair : plots)
    {
        QCustomPlot* plot = pair.second;
        plot->xAxis->setRange(0, ms + 2);
        plot->replot();
    }
    combinedPlot->xAxis->setRange(0, ms + 2);
    combinedPlot->replot();
}

void StepGui::trajectoryChanged(const std::string& servoName, const Trajectory& traj)
{
    if(plots.find(servoName) == plots.end())
    {
        std::cerr << "unknown servo name" << std::endl;
        return;
    }
    if(graphs.find(servoName) == graphs.end())
    {
        std::cerr << "unknown servo name" << std::endl;
        return;
    }
    
    QCustomPlot* plot = plots[servoName];
    setGraphData(plot->graph(0), traj);
    setGraphData(graphs[servoName], traj);

    plot->yAxis->rescale();
    plot->replot();
    combinedPlot->yAxis->rescale();
    combinedPlot->replot();
}

void StepGui::setGraphData(QCPGraph* graph, const Trajectory& data)
{
    QVector<double> x, y;
    for(int i = 0; i < data.size; ++i)
    {
        x.append(data.data[i].time);
        y.append(data.data[i].angle);
    }
    graph->data()->clear();
    graph->setData(x, y);
}


void StepGui::sendStep()
{
    sendCallback(step);
}


void StepGui::save()
{
    
    QString fileName = QFileDialog::getSaveFileName(this, tr("Save File"));
    if(fileName.size() <= 0)
        return;
    
    std::ofstream of;
    of.open(fileName.toStdString());
    
    of << step.length << std::endl;
    
    for(size_t i = 0; i < NUM_SERVOS; ++i)
    {
        const int size = step.servoTrajectory[i].size;
        std::vector<int> angles, times;
        for(int j = 0; j < size; ++j)
        {
            const int angle = step.servoTrajectory[i].data[j].angle;
            const int time = step.servoTrajectory[i].data[j].time;
            angles.push_back(angle);
            times.push_back(time);
        }
        for(size_t j = 0; j < times.size(); ++j)
        {
            if(j < times.size() - 1)
                of << times[j] << ", ";
            else
                of << times[j] << "\n";
        }
        for(size_t j = 0; j < angles.size(); ++j)
        {
            if(j < angles.size() - 1)
                of << angles[j] << ", ";
            else
                of << angles[j] << "\n";
        }
    }
    of.close();
}

void StepGui::load()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open File"));
    if(fileName.size() <= 0)
        return;
    
    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly)) {
        std::cerr << file.errorString().toStdString() << std::endl;
        return;
    }
    
    QByteArray line = file.readLine();
    bool stepLenOk = false;
    const int stepLen = QString(line).toInt(&stepLenOk);
    if(!stepLenOk)
    {
        std::cerr << "steplen parsing failed\n";
        return;
    }
    stepLenBox->setValue(stepLen);
    
    int trajectoryId = 0;
    while (!file.atEnd()) 
    {
        QByteArray times = file.readLine();
        QByteArray angles = file.readLine();
        
        std::cout << "read times: " << QString(times).toStdString() << std::endl;
        std::cout << "read angles: " << QString(angles).toStdString() << std::endl;
        
        
        if(times.length() == 0 || angles.length() == 0)
        {
            std::cerr << "error while reading line\n";
            return;
        }
        QList<QByteArray> timeList = times.split(',');
        QList<QByteArray> angleList = angles.split(',');
        
        if(timeList.length() != angleList.length() || timeList.length() == 0 || angleList.length() == 0)
        {
            std::cerr << "error while reading\n";
            return;           
        }
        
        step.servoTrajectory[trajectoryId].size = timeList.size();
        setTrajectorySize(timeList.size(), servoNames[trajectoryId]);
        
        for(int i = 0; i < timeList.size(); ++i)
        {
            bool timeOk = false;
            const int time = QString(timeList[i]).toInt(&timeOk);
            bool angleOk = false;
            const int angle = QString(angleList[i]).toInt(&angleOk);
            if(!timeOk || !angleOk)
            {
                std::cerr << "parse error\n";
                return;
            }
            step.servoTrajectory[trajectoryId].data[i].time = time;
            step.servoTrajectory[trajectoryId].data[i].angle = angle;
        }
        ++trajectoryId;
    }
    
    //tell models that data has changed.
    //this will cause a cascade of signals and also update all plots etc.
    for(auto& pair : trajModels)
    {
        pair.second->trajectoryUpdated();
    }
    
}


StepGui::~StepGui()
{
    for(auto& pair : trajModels)
        delete pair.second;
    trajModels.clear();
    
    //FIXME destruct correctly :D
}
