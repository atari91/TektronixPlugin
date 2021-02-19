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

#include "TektronixPlugin.h"
#include "qtimer.h"
#include <QThread>

Tektronix_Class::Tektronix_Class(QObject* messenger_): messenger(messenger_)
{

    //Register Data Types
    qRegisterMetaType<DataPair>("std::pair<boost::shared_ptr<std::vector<double>>, boost::shared_ptr<std::vector<double>>>");
    qRegisterMetaType<InterfaceData>("InterfaceData");
    connect((this), SIGNAL(objectNameChanged(QString)), this, SLOT(Initialize()));

}


void Tektronix_Class::ThreadFinished()
{
    ThreadFinishedSignal = true;
}

Tektronix_Class::~Tektronix_Class()
{
    if(Worker->isRunning())
    {
        Work->Stop();
        while(!Work->IsFinished())
            QThread::msleep(1);
        Worker->quit();
        Worker->wait();
    }
}

void Tektronix_Class::Initialize()
{
    Worker = new QThread(this);

    //Create Workerthread
    Work = new WorkClass(this->objectName());
    Work->moveToThread(Worker);
    Work->setObjectName(this->objectName());

    //Connect
    connect((Work), SIGNAL(MessageSender(QString,QString,InterfaceData)), messenger, SLOT(MessageReceiver(QString,QString,InterfaceData)));
    connect((messenger), SIGNAL(MessageSender(QString,QString,InterfaceData)), Work, SLOT(MessageReceiver(QString,QString,InterfaceData)));
    connect(Worker,SIGNAL (started()), Work, SLOT (process()));
    connect((Work->GetMessenger()), SIGNAL(MessageSender(QString,QString,InterfaceData)), messenger, SLOT(MessageReceiver(QString,QString,InterfaceData)));

    connect(Work, SIGNAL(ThreadFinished()), Worker, SLOT(quit()));
    connect(Work, SIGNAL(ThreadFinished()), Worker, SLOT(deleteLater()));

    Worker->start();
    Info("Tektronix Plugin Loaded");

}


void Tektronix_Class::MessageReceiver(const QString &Command, const QString &ID, InterfaceData Data)
{
    return;
}

QObject* Tektronix_Class::GetObject()
{
    return this;
}

InterfaceData* Tektronix_Class::GetSymbol(const QString &ID)
{
    while(!(Worker))
        QThread::msleep(1);
    while(!(Work->HasPublishedSymbols())  && Worker->isRunning() && !Work->IsError())
        QThread::msleep(1);

    if(Work->IsError())
        return nullptr;


    if(!Worker->isRunning())
        return nullptr;

    return Work->GetContainer(ID);
}


void Tektronix_Class::Info(QString text)
{
    InterfaceData Data;
    Data.SetData(text);
    emit MessageSender("info", this->objectName(), Data);
}

void Tektronix_Class::StatusMessage(QString text)
{
    InterfaceData Data;
    Data.SetData(text);
    emit MessageSender("StatusMessage", this->objectName(), Data);
}

void Tektronix_Class::Error(QString text)
{
    InterfaceData Data;
    Data.SetData(text);
    emit MessageSender("error", this->objectName(), Data);
}

