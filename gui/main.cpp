#include <QtGui/QApplication>
#include "StepGui.hpp"
#include "../../engine.hpp"
#include "../ComProtocol.hpp"
#include "../UARTCommunication.hpp"
#include "../CommunicationStub.hpp"

#include <iostream>

Communication* com;
ComProtocol *proto;

/** Is called when the user clicks "Send" in the gui */
void stepCallback(const Step& step, unsigned short index)
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

    std::vector<uint8_t> data;
    data.resize(sizeof(StepPacket));

    StepPacket *packet = reinterpret_cast<StepPacket *>(data.data());

    packet->index = index;
    packet->step = step;

    std::cout << "Sending Step" <<std::endl;
    proto->sendData(SET_STEP,  data);
}

void seqCallback(const StepSequence& seq)
{
    std::vector<uint8_t> data;
    data.resize(sizeof(StepSequence));
    StepSequence *se = reinterpret_cast<StepSequence *>(data.data());
    se->length = seq.length;
    for(unsigned short i = 0; i < se->length; ++i)
    {
        se->sequence[i] = seq.sequence[i];
    }
    proto->sendData(SET_SEQUENCE,data);
}

int main(int argc, char** argv)
{
    if(argc < 1 || argc > 3)
    {
        std::cerr << "call stepgui [/dev/ttyUSBx]" << std::endl;
        return 0;
    }

    if(argc == 1)
    {
        IPCSender *sender = new IPCSender("to_robot_queueU2", "from_robot_queueU2");
        com = new CommunicationStub(sender);
    } else
    {
        com = new UARTCommunication(argv[1]);
    }

    proto = ComProtocol::getInstance(com);

    QApplication app(argc, argv);
    StepGui gui(stepCallback,seqCallback);
    gui.show();
    return app.exec();
}
