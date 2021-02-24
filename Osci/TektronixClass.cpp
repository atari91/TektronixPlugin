/******************************************************************************
**                                                                           **
**  Tektronix Plugin for LabAnlyser, control&visualize data of Tex Scopes.   **
**  Copyright (C) 2015-2021 Andreas Hoffmann                                 **
**                                                                           **
**  Tektronix Plugin is free software: you can redistribute it and/or modify **
**  it under the terms of the GNU General Public License as published by     **
**  the Free Software Foundation, either version 3 of the License, or        **
**  (at your option) any later version.                                      **
**                                                                           **
**  This program is distributed in the hope that it will be useful,          **
**  but WITHOUT ANY WARRANTY; without even the implied warranty of           **
**  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the            **
**  GNU General Public License for more details.                             **
**                                                                           **
**  You should have received a copy of the GNU General Public License        **
**  along with this program.  If not, see http://www.gnu.org/licenses/.      **
**                                                                           **
*******************************************************************************
*******************************************************************************/

#include "TektronixClass.h"
#include "visa.h"
#include "math.h"
#include <string>
#include <QDebug>

/****************************************************************************************/
// Constructor
/****************************************************************************************/
Tektronix::Tektronix()
{

    this->defaultRM = &defaultRMS;
    int state = viOpenDefaultRM(this->defaultRM);
    this->session = NULL;

}


int Tektronix::Connect(QString IP)
{
    CloseConnection();
    QString geraet = "TCPIP0::" + IP + "::inst0::INSTR";


    status = viOpen(*defaultRM, (ViRsrc)geraet.toStdString().c_str(), VI_NULL, 500, &session);
    if (status < VI_SUCCESS) {
        session = NULL;
        return status;
    }

    status = viSetAttribute(session, VI_ATTR_TMO_VALUE, 500);
    if (status < VI_SUCCESS) {
        session = NULL;
        return status;
    }
    else {
        return status;
    }
}

int Tektronix::Calibrate()
{

    status = viSetAttribute(session, VI_ATTR_TMO_VALUE, 15000);
    if (status < VI_SUCCESS) {
        std::cout << "Setting time out failed";
    }
    bool Error = 0;
    QString command = "*CAL?";
    QString logMsg = "Calibrating oscilloscope";
    read(command, logMsg, Error);

    std::cout << "Setting time out to 0.5 seconds";
    status = viSetAttribute(session, VI_ATTR_TMO_VALUE, 500);
    if (status < VI_SUCCESS) {
        std::cout << "Setting time out failed";
    }

    return 0;
}

bool Tektronix::CloseConnection()
{
    if (session!=NULL){
        status = viClose(session);
        if (status < VI_SUCCESS){
            return false;
        }
        else{
            return true;
        }
        session = NULL;
    }
    else{
        return  true;
    }

}


/****************************************************************************************/
// Destructor
/****************************************************************************************/
Tektronix::~Tektronix()
{

}

QStringList Tektronix::read(QString command, QString logMsg, bool &Error)
{
    // creating log message for debugging


    status = viWrite(session, (ViBuf)command.toStdString().c_str(), command.length(), &retCount);
    if (status < VI_SUCCESS)
    {
        qDebug() << "Sending command failed";
        qDebug()  << "VISA error code:" << status;
    }


    unsigned char buffer[1024*4];
    for (int i =0; i< sizeof(buffer); i++) buffer[i] = ' ';

    status = viRead(session, (ViPBuf)buffer, (ViUInt32)sizeof(buffer), &retCount);
    if (status < VI_SUCCESS) {
        qDebug()  << logMsg << "failed";
        qDebug()  << "VISA error code:" << status;
        Error = 1;
    }
    QString Answer(QString::fromLocal8Bit((const char*) buffer,retCount));
    // remove some strange stuff at the end of buffer

    auto AnswerParts = Answer.simplified().split(";");

    return AnswerParts;
}

QStringList Tektronix::CheckStates(QStringList CommandList)
{
    QString  logMsg = "";
    QStringList ErrorCommands;

    for(auto itt : CommandList)
    {
        bool Error = 0;
        QStringList Answers = read(itt,logMsg, Error);
        if(Error)
        {
            ErrorCommands.push_back(itt);
        }
    }

    return ErrorCommands;
}

QStringList Tektronix::ReadState(QStringList CommandList)
{
    QString  command;
    for(int i = 0; i < CommandList.size();i++)
    {
        command.append(CommandList[i].trimmed() + ";:");
    }

    QString  logMsg = "";
    bool Error = 0;
    QStringList Answers = read(command,logMsg, Error);

    if(Answers.size()>2)
    {
        for(int i = 0; i < Answers.size(); i++)
        {
            if(Answers[i].split(" ").size()>1)
                Answers[i] = Answers[i].split(" ").at(1).simplified();
        }
    }

    return Answers;
}

void Tektronix::write(QString command, QString logMsg)
{
    // creating log message for debugging
    status = viWrite(session, (ViBuf)QString(command+ "\r\n").toStdString().c_str(), command.length(), &retCount);
    if (status < VI_SUCCESS) {
        qDebug()  << logMsg << "failed";
        qDebug()  << "VISA error code:" << status;
    }


}

std::vector<unsigned char> Tektronix::readbin(QString command, int size)
{
    status = viWrite(session, (ViBuf)command.toStdString().c_str(), command.length(), &retCount);
    if (status < VI_SUCCESS)
    {
        qDebug() << "Sending command failed";
        qDebug()  << "VISA error code:" << status;
    }


    std::vector<unsigned char> buffer;
    buffer.resize(220e6);
    //unsigned char buffer[1024];
    for (int i =0; i< buffer.size(); i++) buffer[i] = ' ';

    status = viRead(session, (ViPBuf)&(buffer[0]), (ViUInt32)buffer.size(), &retCount);
    if (status < VI_SUCCESS) {

        qDebug()  << "VISA error code:" << status;
    }

    buffer.erase(buffer.begin()+retCount,buffer.end());
    return buffer;
}


QStringList Tektronix::readOutPre(void){

    QString command = "WFMOutpre?";

    status = viWrite(session, (ViBuf)command.toStdString().c_str(), command.length(), &retCount);
    if (status < VI_SUCCESS)
    {
        qDebug() << "Sending command failed";
        qDebug()  << "VISA error code:" << status;
    }


    unsigned char buffer[1024];
    for (int i =0; i< sizeof(buffer); i++) buffer[i] = ' ';

    status = viRead(session, (ViPBuf)buffer, (ViUInt32)sizeof(buffer), &retCount);
    if (status < VI_SUCCESS) {
        qDebug()  << command << "failed";
        qDebug()  << "VISA error code:" << status;
    }
    QString Answer(QString::fromLocal8Bit((const char*) buffer,retCount));
    auto AnswerParts = Answer.simplified().split(";");

    return AnswerParts;
}
