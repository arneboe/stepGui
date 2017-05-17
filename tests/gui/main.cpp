#include <QtGui/QApplication>
#include "StepGui.hpp"
#include "../../engine.hpp"
#include <iostream>


/** Is called when the user clicks "Send" in the gui */
void stepCallback(const Step& step)
{
    std::cout << "stepcallback called\n";
    
    std::cout << "Step Length: " << step.length << std::endl;
    
    for(int i = 0; i < NUM_SERVOS; ++i)
    {
        std::cout << "--------------------------------------------------\n";
        std::cout << "Time:  ";
        for(int j = 0; j < step.servoTrajectory[i].size; ++j)
        {
            std::cout << step.servoTrajectory[i].data[j].time << "\t";
        }    
        std::cout << "\nAngle: ";
        for(int j = 0; j < step.servoTrajectory[i].size; ++j)
        {
            std::cout << step.servoTrajectory[i].data[j].angle << "\t";
        }
        std::cout << "\n";
    }
}

int main(int argc, char** argv)
{
    QApplication app(argc, argv);
    StepGui gui(stepCallback);
    gui.show();
    return app.exec();
}
