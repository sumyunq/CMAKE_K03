#include "LoadLib.h"
#include <tchar.h>
#include <QCoreApplication>
#include <QTimer>
#include <windows.h>


#include <QDebug>
unsigned char buf[64] = {0};
unsigned char Rbuf[64] = {0};

int HidDevStatus = -1;


LoadLib::LoadLib()
{
    TCHAR  szModule[512] = { 0 };
    HINSTANCE m_hAct,m_hAct2;
    wcscat_s(szModule, L"ActHID.dll");

    m_hAct = LoadLibrary(szModule);
    if(m_hAct == NULL)
    {
        qDebug("1LoadLibrary failed:%d\n",GetLastError());
    }else
    {
        //qDebug("1LoadLibrary success\n");
    }

    //导出库里面的函数地址
    pfn_ActHID_IniDev = (ActHID_IniDev)GetProcAddress(m_hAct, "ActHID_IniDev");
    pfn_ActHID_ReleaseDev = (ActHID_ReleaseDev)GetProcAddress(m_hAct, "ActHID_ReleaseDev");
    pfn_ActHID_DownFW = (ActHID_DownFW)GetProcAddress(m_hAct, "ActHID_DownFW");
    pfn_ActHID_Write = (ActHID_Write)GetProcAddress(m_hAct, "ActHID_Write");
    pfn_ActHID_Read = (ActHID_Read)GetProcAddress(m_hAct, "ActHID_Read");

    memset(szModule,0,sizeof(szModule));
    wcscat_s(szModule, L"hidapi.dll");
    m_hAct2 = LoadLibrary(szModule);
    if(m_hAct2 == NULL)
    {
        qDebug("LoadLibrary failed:%d\n",GetLastError());
    }

    //导出库里面的函数地址
    pfn_hid_init = (hid_init_func)GetProcAddress(m_hAct2, "hid_init");
    pfn_hid_enumerate = (hid_enumerate_func)GetProcAddress(m_hAct2, "hid_enumerate");
    pfn_hid_free_enumeration = (hid_free_enumeration_func)GetProcAddress(m_hAct2, "hid_free_enumeration");
    pfn_hid_exit = (hid_exit_func)GetProcAddress(m_hAct2, "hid_exit");
}

QString extractLocationIdentifier(const QString& devicePath)
{
    // 查找 "#8&" 的位置
    int startPos = devicePath.indexOf("#8&");
    if (startPos == -1) {
        return "unknown";
    }

    // 跳过 "#8&"，找到标识符的起始位置
    startPos += 3;

    // 查找标识符的结束位置（下一个 '&' 或字符串结尾）
    int endPos = devicePath.indexOf('&', startPos);
    if (endPos == -1) {
        endPos = devicePath.length();
    }

    return devicePath.mid(startPos, endPos - startPos);
}

//枚举所有包含“XIBERIA”的设备名称与VID、PID
QHash<QString, QHash<QString, QString>> LoadLib::EnumeDev()
{
    const QString targetDevice = "XIBERIA";  // 目标设备名称关键词
    //const QString targetDevice = "MCHOSE";  // 目标设备名称关键词
    int foundCount = 0;

    QHash<QString, QHash<QString, QString>> dev;
    QSet<QString> uniqueKeys;//使用哈希集合确保唯一性，因为同一个设备会出现多个路径，Col0等不同，所以该设备会出现多次

    // 检查函数指针是否获取成功
    if(pfn_hid_init == nullptr || pfn_hid_enumerate == nullptr) {
        qDebug("GetProcAddress failed to get required functions\n");
    }

    //初始化
    int initResult = pfn_hid_init();
    if(initResult != 0) {
        qDebug("hid_init failed with code: %d\n", initResult);
    }
    //读取所有HID设备
    hid_device_info* devices = (hid_device_info*)pfn_hid_enumerate(0,0);
    for(;devices != nullptr;devices = devices->next)
    {
        QString productName = devices->product_string ?QString::fromWCharArray(devices->product_string) : "";

        // 检查是否包含目标关键词（不区分大小写）
        if (productName.contains(targetDevice, Qt::CaseInsensitive))
        {
            // 格式化VID/PID为十六进制
            QString vidHex = QString("%1").arg(devices->vendor_id, 4, 16, QChar('0')).toUpper();
            QString pidHex = QString("%1").arg(devices->product_id, 4, 16, QChar('0')).toUpper();

            //提取"#8&"后面的位置标识符部分
            QString locationId = extractLocationIdentifier(devices->path);

            QString uniqueKey = QString("%1_%2_%3").arg(vidHex).arg(pidHex).arg(locationId);
            // 使用哈希集合确保唯一性（若相同的设备插了多个，需全显示出来）
            if(!uniqueKeys.contains(uniqueKey))
            {
                uniqueKeys.insert(uniqueKey);

                QHash<QString, QString> ids;

                ids["VID"] = vidHex;
                ids["PID"] = pidHex;
                ids["Name"] = productName;

                dev[QString::number(foundCount)] = ids;//若foundCount相同则覆盖掉

                qDebug().nospace()
                    << "\n设备 #" << foundCount
                    << "\n名称: " << productName
                    << "\nVID: 0x" << vidHex
                    << "\nPID: 0x" << pidHex
                    << "\n路径: " << devices->path
                    << "\n序列号: " << devices->serial_number
                    << "\n接口: " << devices->interface_number
                    << "\nrelease_number: " << devices->release_number;

                foundCount++;
            }
        }
    }
    return dev;

}

//打开设备
int LoadLib::openCard()
{
    //HidDevStatus = pfn_ActHID_IniDev(SelDev_VID, SelDev_PID, reportID, &reportIDnum);
    HidDevStatus = pfn_ActHID_IniDev(SelDev_VID, SelDev_PID, reportID, &reportIDnum);//初始化，得到HID的report ID数组与HID的report ID有效个数。函数执行成功返回1
    if (HidDevStatus == 1) {
        qDebug("dev connect suc\n");
        return 1;
    }
    else {
        qDebug("dev connect fail\n");
        return 0;
    }
}
//打开设备并切换为HID（切换HID后需等待8s，再次打开设备）
int LoadLib::openANDswitchCard()
{
    unsigned char CBuff[64] = {0};
    int res = -1;

    HidDevStatus = pfn_ActHID_IniDev(SelDev_VID, SelDev_PID, reportID, &reportIDnum);//初始化，得到HID的report ID数组与HID的report ID有效个数。函数执行成功返回1
    if (HidDevStatus != 1)
    {

        qDebug("dev connect fail\n");
        return HidDevStatus;
    }
    // 构造数据包,发送控制命令，让下位机反馈HID信息给上位机
    CBuff[0] = 0x55;
    CBuff[1] = 0xaa;
    CBuff[2] = 0x55;
    CBuff[3] = 0xaa;
    CBuff[4] = 0x55;

    res = pfn_ActHID_Write(CBuff, sizeof(CBuff));//成功返回写入长度，失败返回-1
    if (res < 0) {
        pfn_ActHID_ReleaseDev();//释放设备
        return res;
    }
    //切换HID后还需等8秒
    pfn_ActHID_ReleaseDev();//释放设备
    //Sleep(8000);

    //休眠一秒
    QEventLoop loop;
    QTimer::singleShot(8000, &loop, SLOT(quit()));//一秒后触发quit()，终止事件循环
    loop.exec();//启动事件循环并阻塞当前线程

    HidDevStatus = pfn_ActHID_IniDev(SelDev_VID, SelDev_PID, reportID, &reportIDnum);//初始化，得到HID的report ID数组与HID的report ID有效个数。函数执行成功返回1
    if (HidDevStatus != 1) {

        qDebug("dev connect fail\n");
        return HidDevStatus;
    }
    return 1;
}
//关闭设备
int LoadLib::closeCard()
{
    if(HidDevStatus)
    {
        pfn_ActHID_ReleaseDev();//释放设备
    }
    return 1;
}
//写
int LoadLib::write(unsigned char* buf,int len)
{
    int res = 0;
    res = pfn_ActHID_Write(buf, len);//写64字节数据，数据的第一个字节为report ID,大于等于0发送成功
    // if (res < 0) {
    //      errorMsg = "写失败， " + QString::fromWCharArray(hid_error(Hhandle));
    //      QMessageBox::critical(this,"失败","写失败");

    // }
    return  res;//-1应该代表设备不存在等

}
//读
int LoadLib::read(unsigned char* buf, int len, int timeout, int cnt)
{
    // int ret = -1;
    // ret = pfn_ActHID_Read(reportID[2], (unsigned char*)buf, len, timeout, cnt);//读数据，读取三次

    // return ret;//-1代表设备不存在等

    int ret = -1;
    ret = pfn_ActHID_Read(reportID[2], (unsigned char*)buf, len, timeout, cnt);//读数据，读取三次

    if(ret == 0)
    {
        // 遍历整个数组，跳过 0 值（即未使用的槽位），因为不是所有电脑都是默认reportID[2]的
        for (int i = 0; i < 20; ++i) {
            if (reportID[i] == 0)
                continue;           // 无效 ID，跳过

            ret = pfn_ActHID_Read(reportID[i], (unsigned char*)buf, len, timeout, cnt);
            if (ret != 0 && (ret != (-1))) break;  // 找到了一个能成功读取的 ID
        }
    }
    //ret = pfn_ActHID_Read(reportID[4], (unsigned char*)buf, len, timeout, cnt);//读数据，读取三次   有线连接时，用的reportID[4]


    return ret;//-1代表设备不存在等
}

//握手上报耳机状态信息
int LoadLib::GetDevStatus(DevStatus &sta)
{
    //QMutexLocker locker(&mutex);  // 构造函数加锁，析构时自动解锁
    // mutex.lock();
    int res = 0;
    unsigned char buf[64] = {0};
    unsigned char Rbuf[64] = {0};
    memset(buf,0,sizeof(buf));
    memset(Rbuf,0,sizeof(Rbuf));

    buf[0] = 0x55; // Report ID
    buf[1] = 0x10;//0x20;
    buf[2] = 0x01;

    res = write(buf,sizeof(buf));
    if (res < 0) {
        return res;
    }
    res = read((unsigned char*)Rbuf, 64, 100, 3);//读数据，读取三次
    if (res < 0) {
        return res;
    }else if(res > 0)
    {
        // if(Rbuf[1] == 0x20 && Rbuf[2] == 0x01)
        if(Rbuf[1] == 0x10 && Rbuf[2] == 0x01)
        {
            //耳机连接状态(0：2.4G 模式，耳机未连接; 1：2.4G 模式，耳机已连接; 2：有线模式)
            sta.ConnectSta = Rbuf[4];
            //音量等级(0：未知; 16：16级; 50：50级; 100：100级 )
            sta.VolumeLevel = Rbuf[5];
            //音量值(音量等级非0，这个值才有效 )
            sta.Volume = Rbuf[6];
            //电池电量(0~100)
            sta.electricity = Rbuf[7] & 0x7F;
            //EQ 模式(0：未知; 1：自定义EQ; 2：音乐; 3：游戏; 4：电影)
            sta.EQMode = Rbuf[8];
            //麦克风状态(0：未知; 1：麦克风开启; 2：麦克风关闭)
            sta.MicEn = Rbuf[9];
            //麦克风侦听状态(0：未知; 1：麦克风侦听开启; 2：麦克风侦听关闭)
            sta.MicListening = Rbuf[10];
            //麦克风增强状态(0：未知; 1：麦克风恢复; 2：麦克风增强)
            sta.MicSta = Rbuf[11];
        }
    }
    // mutex.unlock();
    return res;
}

//设置默认十个频点的EQ值(设置自定义EQ)
int LoadLib::SetEQ(int eq30,int eq60,int eq120,int eq250,int eq500,int eq1k,int eq2k,int eq4k,int eq8k,int eq16k)
{
    int res = 0;
    unsigned char buf[64] = {0};
    memset(buf,0,sizeof(buf));

    buf[0] = 0x55; // Report ID
    buf[1] = 0x10;//0x20;//CLX 自定义命令
    buf[2] = 0x02;//Command
    buf[3] = 10;
    buf[4] = 5*(12+eq30);
    buf[5] = 5*(12+eq60);
    buf[6] = 5*(12+eq120);
    buf[7] = 5*(12+eq250);
    buf[8] = 5*(12+eq500);
    buf[8] = 5*(12+eq1k);
    buf[10] = 5*(12+eq2k);
    buf[11] = 5*(12+eq4k);
    buf[12] = 5*(12+eq8k);
    buf[13] = 5*(12+eq16k);

    res = write(buf, sizeof(buf));
    return res;
}
//设置下行(扬声器) 自定义10 个频点的 EQ 参数
int LoadLib::NewSetEQ(int *Freq,int *Gain,int *q)
{
    int res = 0;
    unsigned char buf[64] = {0};
    memset(buf,0,sizeof(buf));

    buf[0] = 0x55; // Report ID
    buf[1] = 0x10;//0x20;//CLX 自定义命令
    buf[2] = 0x0d;//Command
    buf[3] = 60;
    for(int i = 0; i < 10; i++)
    {
        buf[4 + i * 6] = (*Freq >> 8) & 0xFF;//fc高8位
        buf[5 + i * 6] = *Freq & 0xFF;//fc低8位
        Freq++;
        buf[6 + i * 6] = (5*(12 + *Gain)) & 0xFF;//Gain
        Gain++;
        buf[7 + i * 6] = ((*q * 10) >> 8) & 0xFF;//q值高8位
        buf[8 + i * 6] = (*q * 10) & 0xFF;//q值低8位
        q++;
        buf[9 + i * 6] = 1;//EQ频点类型（0:按钮 1：Peaking 2:High pass 3:Low pass 4:Low self 5:High shelf）
    }
    res = write(buf, sizeof(buf));
    return res;
}
//上行 MIC 自定义 4 个频点的 EQ 参数
int LoadLib::NewSetMicEQ(int *Freq,int *Gain,int *q)
{
    int res = 0;
    unsigned char buf[64] = {0};
    memset(buf,0,sizeof(buf));

    buf[0] = 0x55; // Report ID
    buf[1] = 0x10;//0x20;//CLX 自定义命令
    buf[2] = 0x0e;//Command
    buf[3] = 40;
    for(int i = 0; i < 4; i++)
    {
        buf[4 + i * 6] = (*Freq >> 8) & 0xFF;//fc高8位
        buf[5 + i * 6] = *Freq & 0xFF;//fc低8位
        Freq++;
        buf[6 + i * 6] = (5*(12 + *Gain)) & 0xFF;//Gain
        Gain++;
        buf[7 + i * 6] = ((*q * 10) >> 8) & 0xFF;//q值高8位
        buf[8 + i * 6] = (*q * 10) & 0xFF;//q值低8位
        q++;
        buf[9 + i * 6] = 1;//EQ频点类型（0:按钮 1：Peaking 2:High pass 3:Low pass 4:Low self 5:High shelf）
    }
    res = write(buf, sizeof(buf));
    return res;
}
//切换 EQ 模式
int LoadLib::SetEQMode(int Mode)
{
    int res = 0;
    unsigned char buf[64] = {0};
    memset(buf,0,sizeof(buf));

    buf[0] = 0x55; // Report ID
    buf[1] = 0x10;//0x20;//CLX 自定义命令
    buf[2] = 0x03;//Command
    buf[3] = 2;
    buf[4] = 2;//1:获取，2:设置
    buf[5] = Mode;//1：自定义 EQ; 2：音乐; 3：游戏; 4：电影

    res = write(buf, sizeof(buf));
    return res;

}
//获取当前EQ模式
int LoadLib::GetEQMode(int *Mode)
{
    //mutex.lock();
    int res = 0;
    unsigned char buf[64] = {0};
    unsigned char Rbuf[64] = {0};
    memset(buf,0,sizeof(buf));
    memset(Rbuf,0,sizeof(Rbuf));

    buf[0] = 0x55; // Report ID
    buf[1] = 0x10;//0x20;//CLX 自定义命令
    buf[2] = 0x03;//Command
    buf[3] = 2;
    buf[4] = 1;//1:获取，2:设置
    res = write(buf, sizeof(buf));
    if(res < 0)
        return res;

    res = read((unsigned char*)Rbuf, 64, 100, 3);//读数据，读取三次
    if(res < 0)
        return res;

    if(Rbuf[2] == 0x3 && Rbuf[4] == 1)
    {
        *Mode = Rbuf[5];
    }

    return res;
    //mutex.unlock();
}

//设置音量与增益
int LoadLib::SetVolumeGain(int GainVal)
{
    int res = 0;
    unsigned char buf[64] = {0};
    memset(buf,0,sizeof(buf));

    buf[0] = 0x55; // Report ID
    buf[1] = 0x10;//0x20;//CLX 自定义命令
    buf[2] = 0x04;//Command
    buf[3] = 3;
    buf[4] = 2;//1:获取，2:设置
    buf[5] = 0;
    buf[6] = 5*(12+GainVal);

    res = write(buf, sizeof(buf));
    return res;
}
//获取音量与增益
int LoadLib::GetVolumeGain(int *GainVal)
{
    //mutex.lock();
    int res = 0;
    unsigned char buf[64] = {0};
    unsigned char Rbuf[64] = {0};
    memset(buf,0,sizeof(buf));
    memset(Rbuf,0,sizeof(Rbuf));

    buf[0] = 0x55; // Report ID
    buf[1] = 0x10;//0x20;//CLX 自定义命令
    buf[2] = 0x04;//Command
    buf[3] = 3;
    buf[4] = 1;//1:获取，2:设置
    res = write(buf, sizeof(buf));
    if(res < 0)
        return res;

    res = read((unsigned char*)Rbuf, 64, 100, 3);//读数据，读取三次
    if(res < 0)
        return res;

    if(Rbuf[2] == 0x4 && Rbuf[4] == 1)
    {
        *GainVal = Rbuf[6];
    }

    return res;
    //mutex.unlock();
}

//设置麦克风开关
int LoadLib::SetMicEn(bool MicOpenEn)
{
    int res = 0;
    unsigned char buf[64] = {0};
    memset(buf,0,sizeof(buf));

    buf[0] = 0x55; // Report ID
    buf[1] = 0x10;//0x20;//CLX 自定义命令
    buf[2] = 0x05;//Command
    buf[3] = 0x5;
    buf[4] = 2;//1:获取，2：设置麦克风使能  3：设置模拟/数字增益值  4：设置麦克风强度状态
    if(MicOpenEn)
    {
        buf[5] =1;//麦克风开启
    }else
    {
        buf[5] =2;//麦克风关闭
    }
    res = write(buf, sizeof(buf));
    return res;
}
//设置麦克风增益值
int LoadLib::SetMicGain(int AnalogGain, int DigitalGain)
{
    int res = 0;
    unsigned char buf[64] = {0};
    memset(buf,0,sizeof(buf));

    buf[0] = 0x55; // Report ID
    buf[1] = 0x10;//0x20;//CLX 自定义命令
    buf[2] = 0x05;//Command
    buf[3] = 0x5;
    buf[4] = 3;//1:获取，2：设置麦克风使能  3：设置模拟/数字增益值  4：设置麦克风强度状态
    buf[6] = AnalogGain;//模拟增益值()
    buf[7] = DigitalGain;//数字增益值(0-49db)
    res = write(buf, sizeof(buf));
    return res;
}
//设置麦克风增益使能
int LoadLib::SetMicGainEn(bool GainEn)
{
    int res = 0;
    unsigned char buf[64] = {0};
    memset(buf,0,sizeof(buf));

    buf[0] = 0x55; // Report ID
    buf[1] = 0x10;//0x20;//CLX 自定义命令
    buf[2] = 0x05;//Command
    buf[3] = 0x5;
    buf[4] = 4;//1:获取，2：设置麦克风使能  3：设置模拟/数字增益值  4：设置麦克风强度状态
    if(GainEn)
    {
        buf[8] = 2;//麦克风增强
    }else
    {
        buf[8] = 1;//麦克风恢复
    }

    res = write(buf, sizeof(buf));
    return res;
}
//获取麦克风开关使能、增益值、增益使能
int LoadLib::GetMicMsg(bool *MicOpenEn,int *AnalogGain, int *DigitalGain,bool *BoostEn)
{
    //mutex.lock();
    int res = 0;
    unsigned char buf[64] = {0};
    unsigned char Rbuf[64] = {0};
    memset(buf,0,sizeof(buf));
    memset(Rbuf,0,sizeof(Rbuf));

    buf[0] = 0x55; // Report ID
    buf[1] = 0x10;//0x20;//CLX 自定义命令
    buf[2] = 0x05;//Command
    buf[3] = 5;
    buf[4] = 1;//1:获取
    res = write(buf, sizeof(buf));
    if(res < 0)
        return res;

    res = read((unsigned char*)Rbuf, 64, 100, 3);//读数据，读取三次
    if(res < 0)
        return res;

    if(Rbuf[2] == 0x5 && Rbuf[4] == 1)
    {
        if(Rbuf[5] == 1)
        {
            *MicOpenEn = true;
        }else if(Rbuf[5] == 2)
        {
            *MicOpenEn = false;
        }
        *AnalogGain = Rbuf[6];
        *DigitalGain = Rbuf[7];
        if(Rbuf[8] == 2)
        {
            *BoostEn = 1;
        }else
        {
            *BoostEn = 0;
        }
    }

    return res;
    //mutex.unlock();
}
//设置麦克风侦听使能
int LoadLib::SetMicListening(bool en)
{
    int res = 0;
    unsigned char buf[64] = {0};
    memset(buf,0,sizeof(buf));

    buf[0] = 0x55; // Report ID
    buf[1] = 0x10;//0x20;//CLX 自定义命令
    buf[2] = 0x06;//Command
    buf[3] = 0x2;
    buf[4] = 2;//1:获取，2:设置
    if(en)
    {
        buf[5] = 1;//1，麦克风侦听开启
    }else
    {
        buf[5] = 2;//2，麦克风侦听关闭
    }
    res = write(buf, sizeof(buf));
    return res;
}
//获取麦克风侦听使能
int LoadLib::GetMicListening(bool *en)
{
    //mutex.lock();
    int res = 0;
    unsigned char buf[64] = {0};
    unsigned char Rbuf[64] = {0};
    memset(buf,0,sizeof(buf));
    memset(Rbuf,0,sizeof(Rbuf));

    buf[0] = 0x55; // Report ID
    buf[1] = 0x10;//0x20;//CLX 自定义命令
    buf[2] = 0x06;//Command
    buf[3] = 2;
    buf[4] = 1;//1:获取，2:设置
    res = write(buf, sizeof(buf));
    if(res < 0)
        return res;

    res = read((unsigned char*)Rbuf, 64, 100, 3);//读数据，读取三次
    if(res < 0)
        return res;

    if(Rbuf[2] == 0x6 && Rbuf[4] == 1)
    {
        if(Rbuf[5] == 1)
        {
            *en = true;
        }else if(Rbuf[5] == 2)
        {
            *en = false;
        }
    }

    return res;
    //mutex.unlock();
}
//设置麦克风降噪使能
int LoadLib::SetMicNoise(bool en)
{
    int res = 0;
    unsigned char buf[64] = {0};
    memset(buf,0,sizeof(buf));

    buf[0] = 0x55; // Report ID
    buf[1] = 0x10;//0x20;//CLX 自定义命令
    buf[2] = 0x07;//Command
    buf[3] = 0x2;
    buf[4] = 2;//1:获取，2:设置
    if(en)
    {
        buf[5] = 1;//1，麦克风降噪开启
    }else
    {
        buf[5] = 2;//2，麦克风降噪关闭
    }
    res = write(buf, sizeof(buf));
    return res;
}
//获取麦克风降噪使能
int LoadLib::GetMicNoise(bool *en)
{
    //mutex.lock();
    int res = 0;
    unsigned char buf[64] = {0};
    unsigned char Rbuf[64] = {0};
    memset(buf,0,sizeof(buf));
    memset(Rbuf,0,sizeof(Rbuf));

    buf[0] = 0x55; // Report ID
    buf[1] = 0x10;//0x20;//CLX 自定义命令
    buf[2] = 0x07;//Command
    buf[3] = 2;
    buf[4] = 1;//1:获取，2:设置
    res = write(buf, sizeof(buf));
    if(res < 0)
        return res;

    res = read((unsigned char*)Rbuf, 64, 100, 3);//读数据，读取三次
    if(res < 0)
        return res;

    if(Rbuf[2] == 0x7 && Rbuf[4] == 1)
    {
        if(Rbuf[5] == 1)
        {
            *en = true;
        }else if(Rbuf[5] == 2)
        {
            *en = false;
        }
    }

    return res;
    //mutex.unlock();
}

//设置提示音使能
int LoadLib::SetBeepEn(bool en)
{
    int res = 0;
    unsigned char buf[64] = {0};
    memset(buf,0,sizeof(buf));

    buf[0] = 0x55; // Report ID
    buf[1] = 0x10;//0x20;//CLX 自定义命令
    buf[2] = 0x08;//Command
    buf[3] = 0x3;
    buf[4] = 2;//1:获取，2:设置提示音开关, 3：设置提示音音量
    if(en)
    {
        buf[5] = 1;//1，提示音开启
    }else
    {
        buf[5] = 2;//2，提示音关闭
    }
    res = write(buf, sizeof(buf));
    return res;
}
//设置提示音音量
int LoadLib::SetBeepVolume(int level)
{
    int res = 0;
    unsigned char buf[64] = {0};
    memset(buf,0,sizeof(buf));

    buf[0] = 0x55; // Report ID
    buf[1] = 0x10;//0x20;//CLX 自定义命令
    buf[2] = 0x08;//Command
    buf[3] = 0x3;
    buf[4] = 3;//1:获取，2:设置提示音开关, 3：设置提示音音量
    buf[6] = level;//0-16
    res = write(buf, sizeof(buf));
    return res;
}
//获取提示音使能、音量
int LoadLib::GetBeepMsg(bool *en,int *level)
{
    //mutex.lock();
    int res = 0;
    unsigned char buf[64] = {0};
    unsigned char Rbuf[64] = {0};
    memset(buf,0,sizeof(buf));
    memset(Rbuf,0,sizeof(Rbuf));

    buf[0] = 0x55; // Report ID
    buf[1] = 0x10;//0x20;//CLX 自定义命令
    buf[2] = 0x08;//Command
    buf[3] = 3;
    buf[4] = 1;//1:获取
    res = write(buf, sizeof(buf));
    if(res < 0)
        return res;

    res = read((unsigned char*)Rbuf, 64, 100, 3);//读数据，读取三次
    if(res < 0)
        return res;

    if(Rbuf[2] == 0x8 && Rbuf[4] == 1)
    {
        if(Rbuf[5] == 1)
        {
            *en = true;
        }else if(Rbuf[5] == 2)
        {
            *en = false;
        }
        *level = Rbuf[6];
    }

    return res;
    //mutex.unlock();
}

//获取耳机电量
int LoadLib::GetElectricity(int *level)
{
    //mutex.lock();
    int res = 0;
    unsigned char buf[64] = {0};
    unsigned char Rbuf[64] = {0};
    memset(buf,0,sizeof(buf));
    memset(Rbuf,0,sizeof(Rbuf));

    buf[0] = 0x55; // Report ID
    buf[1] = 0x10;//0x20;//CLX 自定义命令
    buf[2] = 0x09;//Command
    buf[3] = 1;
    buf[4] = 1;//1:获取，2:设置
    res = write(buf, sizeof(buf));
    if(res < 0)
        return res;

    res = read((unsigned char*)Rbuf, 64, 100, 3);//读数据，读取三次
    if(res < 0)
        return res;

    if(Rbuf[2] == 0x9)
    {
        *level = Rbuf[4];
    }

    return res;
    //mutex.unlock();
}

// 获取版本信息
int LoadLib::GetVersion(char *DongleVer, char *EarVer)
{
    //mutex.lock();
    // 第4~33字节:Dongle 版本信息  提取第34~63字节:耳机版本信息
    int res = 0;
    unsigned char buf[64] = {0};
    memset(buf,0,sizeof(buf));
    unsigned char Rbuf[64] = {0};
    memset(Rbuf,0,sizeof(Rbuf));

    buf[0] = 0x55; // Report ID
    buf[1] = 0x5e;
    // buf[1] = 0x10;//0x20;//CLX 自定义命令
    // buf[2] = 0x0A;//Command
    // buf[3] = 0x1;
    // buf[4] = 1;

    res = write(buf, sizeof(buf));
    if (res < 0) {
        return res;
    }

    res = read((unsigned char*)Rbuf, 64, 100, 3);//读数据，读取三次
    if (res < 0) {
        return res;
    }
    // if(Rbuf[1] == 0x10 && Rbuf[2] == 0x0A)
    {
        // 直接拷贝，不做“跳过前导零”处理（因为版本字符串通常不会以 \0 开头）
        strncpy(DongleVer, (char*)(Rbuf + 4), 30);
        DongleVer[30] = '\0';

        strncpy(EarVer, (char*)(Rbuf + 34), 30);
        EarVer[30] = '\0';
    }

    return res;
    //mutex.unlock();
}

//按键功能自定义
int LoadLib::SetKey(int MuteKey,int MuteAct,int PlayKey,int PlayAct,int EqKey,int EqAct)
{
    int res = 0;
    unsigned char buf[64] = {0};
    memset(buf,0,sizeof(buf));

    buf[0] = 0x55; // Report ID
    buf[1] = 0x10;//0x20;//CLX 自定义命令
    buf[2] = 0x0B;//Command
    buf[3] = 9;
    buf[4] = 1;//1：播放/暂停
    buf[5] = PlayKey;//1：按键 1  2：按键 2  3：按键 3
    buf[6] = PlayAct;//1：单击 2：双击 3：三击 4：四击 5：五击 6：长按 7：超长按 8：极长按
    buf[7] = 2;//2：麦克风开关（静音）
    buf[8] = MuteKey;//1：按键 1  2：按键 2  3：按键 3
    buf[9] = MuteAct;//1：单击 2：双击 3：三击 4：四击 5：五击 6：长按 7：超长按 8：极长按
    buf[10] = 3;//3：EQ 切换
    buf[11] = EqKey;//1：按键 1  2：按键 2  3：按键 3
    buf[12] = EqAct;//1：单击 2：双击 3：三击 4：四击 5：五击 6：长按 7：超长按 8：极长按

    res = write(buf, sizeof(buf));
    return res;
}
//播放/暂停按键自定义
int LoadLib::SetPlayKey(int PlayKey,int PlayAct)
{
    int res = 0;
    unsigned char buf[64] = {0};
    memset(buf,0,sizeof(buf));

    buf[0] = 0x55; // Report ID
    buf[1] = 0x10;//0x20;//CLX 自定义命令
    buf[2] = 0x0B;//Command
    buf[3] = 3;
    buf[4] = 1;//1：播放/暂停
    buf[5] = PlayKey;//1：按键 1  2：按键 2  3：按键 3
    buf[6] = PlayAct;//1：单击 2：双击 3：三击 4：四击 5：五击 6：长按 7：超长按 8：极长按

    res = write(buf, sizeof(buf));
    return res;
}
//麦克风开关（静音）按键自定义
int LoadLib::SetMuteKey(int MuteKey,int MuteAct)
{
    int res = 0;
    unsigned char buf[64] = {0};
    memset(buf,0,sizeof(buf));

    buf[0] = 0x55; // Report ID
    buf[1] = 0x10;//0x20;//CLX 自定义命令
    buf[2] = 0x0B;//Command
    buf[3] = 3;
    buf[4] = 2;//2：麦克风开关（静音）
    buf[5] = MuteKey;//1：按键 1  2：按键 2  3：按键 3
    buf[6] = MuteAct;//1：单击 2：双击 3：三击 4：四击 5：五击 6：长按 7：超长按 8：极长按

    res = write(buf, sizeof(buf));
    return res;
}
//EQ 切换按键自定义
int LoadLib::SetEqKey(int EqKey,int EqAct)
{
    int res = 0;
    unsigned char buf[64] = {0};
    memset(buf,0,sizeof(buf));

    buf[0] = 0x55; // Report ID
    buf[1] = 0x10;//0x20;//CLX 自定义命令
    buf[2] = 0x0B;//Command
    buf[3] = 3;
    buf[4] = 3;//3：EQ 切换
    buf[5] = EqKey;//1：按键 1  2：按键 2  3：按键 3
    buf[6] = EqAct;//1：单击 2：双击 3：三击 4：四击 5：五击 6：长按 7：超长按 8：极长按

    res = write(buf, sizeof(buf));
    return res;
}
//按键恢复默认设置
int LoadLib::KeyReset()
{
    int res = 0;
    unsigned char buf[64] = {0};
    memset(buf,0,sizeof(buf));

    buf[0] = 0x55; // Report ID
    buf[1] = 0x10;//0x20;//CLX 自定义命令
    buf[2] = 0x0C;//Command
    buf[3] = 0x1;
    buf[4] = 1;

    res = write(buf, sizeof(buf));
    return res;
}
