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

#include "readoscichannel.h"

ReadOsciChannel::ReadOsciChannel( QString DeviceName_, MessengerClass &messenger_, Tektronix &Osci_ ,  std::map<QString, DataStorage>& data):
    m_data(data),
    DeviceName(DeviceName_),
    Osci(Osci_),
    messenger(messenger_)
{

}

void ReadOsciChannel::ReadChannel(int Channel)
{
    QString Chan = "C" + QString::number(Channel);
    QString ChanT = "Ch" + QString::number(Channel);

    if(this->m_data[DeviceName + "::Channel::"+Chan+"::State"].GetBool())
    {
        auto ID = DeviceName + "::Data::Source";
        this->Osci.write("DATA:SOURCE "+ ChanT,ID);
        this->Osci.write("HEADer 1","");
        QStringList answer = this->Osci.readOutPre();
        this->Osci.write("HEADer 0","");

        std::map<QString, QString> AnswerMap;
        for(auto itt : answer)
        {
            QStringList splitted = itt.split(" ");
            if(splitted.size() == 2)
                AnswerMap[splitted[0]] = splitted[1];
        }

        double XZero, YZero, Ymult, Xincr, Yoff;

        if(AnswerMap.find("NR_PT") == AnswerMap.end())
        {
            messenger.Info("NR_PT is missing in answer of WFMOutpre?");
            return;
        }
        int transferredNum = AnswerMap["NR_PT"].toInt();


        if(AnswerMap.find(":WFMOUTPRE:XZERO") == AnswerMap.end() && AnswerMap.find("XZERO") == AnswerMap.end())
        {
            messenger.Info(":WFMOUTPRE:XZERO or XZERO is missing in answer of WFMOutpre?");
            return;
        }
        if(AnswerMap.find(":WFMOUTPRE:XZERO") != AnswerMap.end())
             XZero =AnswerMap[":WFMOUTPRE:XZERO"].toDouble();
        else
            XZero =AnswerMap["XZERO"].toDouble();


        if(AnswerMap.find("XIN") == AnswerMap.end() && AnswerMap.find("XINCR") == AnswerMap.end())
        {
            messenger.Info("XIN of XINCR is missing in answer of WFMOutpre?");
            return;
        }
        if(AnswerMap.find("XIN") != AnswerMap.end())
             Xincr =AnswerMap["XIN"].toDouble();
        else
            Xincr =AnswerMap["XINCR"].toDouble();

        if(AnswerMap.find("YOFF") == AnswerMap.end() && AnswerMap.find(":WFMOUTPRE:YOFF") == AnswerMap.end())
        {
            messenger.Info(":WFMOUTPRE:YOFF of YOFF is missing in answer of WFMOutpre?");
            return;
        }
        if(AnswerMap.find("YOFF") != AnswerMap.end())
             Yoff =AnswerMap["YOFF"].toDouble();
        else
             Yoff =AnswerMap[":WFMOUTPRE:YOFF"].toDouble();

        if(AnswerMap.find("YMULT") == AnswerMap.end() && AnswerMap.find("YMU") == AnswerMap.end())
        {
            messenger.Info("YMU of YMULT is missing in answer of WFMOutpre?");
            return;
        }
        if(AnswerMap.find("YMULT") != AnswerMap.end())
             Ymult =AnswerMap["YMULT"].toDouble();
        else
             Ymult =AnswerMap["YMU"].toDouble();

        if(AnswerMap.find("YZERO") == AnswerMap.end() && AnswerMap.find("YZE") == AnswerMap.end())
        {
            messenger.Info("YZERO of YZE is missing in answer of WFMOutpre?");
            return;
        }
        if(AnswerMap.find("YZERO") != AnswerMap.end())
             YZero =AnswerMap["YZERO"].toDouble();
        else
             YZero =AnswerMap["YZE"].toDouble();


        std::vector<unsigned char> buffer = this->Osci.readbin("CURVE?",(int)(2*transferredNum + 500));
        int header = QString(buffer[1]).toInt()+3; // information ueber anzahl gesendete bytes
        int fp = header;
        int lp =2*transferredNum+header-2;
        //Set new buffered data :)
        ID = DeviceName + + "::Buffered::" + Chan;
        InterfaceData _Data;
        _Data.SetDataType("vector<double>");
        _Data.SetType("Data");

        std::vector<double> T;
        std::vector<double> Y;

        for(int i = fp ;i<lp && i < buffer.size();i=i+2)
        {
             int16_t tmp2 = *((int16_t*)&buffer[i]);
             Y.push_back(YZero+Ymult*((double) tmp2-Yoff));
             double tmp1 = XZero+Xincr*(((double)i)/2-Yoff);
             T.push_back(tmp1);
       }
       _Data.SetData(DataPair( boost::shared_ptr<std::vector<double>>(new std::vector<double>(T)), boost::shared_ptr<std::vector<double>>(new std::vector<double>(Y))));
        messenger.MessageSender("set", ID,  _Data);;
    }
}
