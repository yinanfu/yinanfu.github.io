#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>
#include <map>
#include <string.h>
#include <iostream>
#include <cstring>
#include <vector>
#include <iostream>
#include <set>
#include <sstream>

#define SERVER_A_PORT "21278"
#define SERVER_B_PORT "22278"
#define SERVER_M_UDP_PORT "23278"
#define SERVER_M_TCP_PORT "24278"

#define IP_ADDR "127.0.0.1"
#define BACKLOG 10 // how many pending connections queue will hold

using namespace std;

set<string> *allNameAP;
set<string> allNameA;
set<string> allNameB;
int errNamFlag = 0; //=0:no error !=0:error
vector<string> errNames;
vector<string> valNames;
string errNamStr;
string valNamStr;
vector<string> inputNameA;
vector<string> inputNameB;
string inputNameAStr;
string inputNameBStr;
vector<vector<int> > timeA;
vector<vector<int> > timeB;
vector<vector<int> > timeRes;

// TCP variables
struct addrinfo hints_TCP;
struct addrinfo *servinfo_TCP, *serA, *serB;
int s_TCP;
char recvTcpMess[2000];
char *namePoint = NULL;
vector<string> inputNames;

int tcpstatus;
bool flag; // track if there is at least 1 hospital available.
// char *outMess = new char[2000];

struct sockaddr_storage client_addr;
socklen_t client_addr_size;
int new_fd;

// UDP variables
int status;
int nameCnt = 0;

struct addrinfo hints_UDP;
struct addrinfo *servinfo_UDP;
int s_UDP;
char endFlag[4] = "END";

void init(){
    errNamFlag=0;
    errNames.clear();
    valNames.clear();
    errNamStr.clear();
    valNamStr.clear();
    inputNameA.clear();
    inputNameB.clear();
    inputNameAStr.clear();
    inputNameBStr.clear();
    timeA.clear();
    timeB.clear();
    timeRes.clear();
    inputNames.clear();
    nameCnt=0;
}

char *timeLotOutput(char *RevMessage)
{
    // trans from message format to output format
    // i.e. 5 6,7 8 -> [5,6],[7,8]
    char *outputMessage = new char[2000];
    char *tmpMessage = new char[2000];
    char *mesP;
    strcpy(tmpMessage, RevMessage);

    char delims[] = ",";
    mesP = strtok(tmpMessage, delims);
    strcat(outputMessage, "[");

    while (mesP)
    {
        // cout<<"mesP: "<<mesP<<endl;
        strcat(outputMessage, "[");
        string time1;
        string time2;

        string tim1tim2str(mesP);
        // cout<<"tim1tim2str: "<<tim1tim2str<<endl;
        int p = 0;
        int spaceflag = 0;

        while (p < tim1tim2str.size())
        {
            if (tim1tim2str[p] == ' ')
            {
                spaceflag = p;
                time1 = tim1tim2str.substr(0, p);
            }
            p++;
        }
        // cout<<"outwhile: "<<endl;

        time2 = tim1tim2str.substr(spaceflag, tim1tim2str.size() - 1);
        // cout<<"time2: "<<time2<<endl;

        strcat(outputMessage, time1.data()); //);
        strcat(outputMessage, ",");

        // time2 = strtok(NULL, " ");
        strcat(outputMessage, time2.data());
        strcat(outputMessage, "]");

        mesP = strtok(NULL, delims);
        if (mesP)
            strcat(outputMessage, ", ");
    }
    strcat(outputMessage, "]");

    // cout<<"get return!!!!"<<endl;
    return outputMessage;
}

char *vector2string(vector<vector<int> > &timeRes)
{
    char *resOutMess = new char[2000];
    char *timeLot1 = new char[2000];
    char *timeLot2 = new char[2000];
    strcat(resOutMess, "[");
    // cout << timeRes.size() << endl;
    for (int i = 0; i < timeRes.size(); i++)
    {
        // fprintf(stdout, "[%d,%d]", timeRes[i][0], timeRes[i][1]);

        strcat(resOutMess, "[");

        snprintf(timeLot1, sizeof(timeLot1), "%d", timeRes[i][0]);
        strcat(resOutMess, timeLot1);
        strcat(resOutMess, ",");
        snprintf(timeLot2, sizeof(timeLot2), "%d", timeRes[i][1]);

        strcat(resOutMess, timeLot2);
        strcat(resOutMess, "]");
        // strcat(resOutMess, timeLot);
        if ((i + 1) < timeRes.size())
        {
            // fprintf(stdout, ", ");
            strcat(resOutMess, ", ");
        }
        // strcat(&timeLot, ' ');
    }
    strcat(resOutMess, "]");

    return resOutMess;
}

void tcpSetup()
{
    memset(&hints_TCP, 0, sizeof(hints_TCP));
    hints_TCP.ai_family = AF_INET;
    hints_TCP.ai_socktype = SOCK_STREAM;
    hints_TCP.ai_flags = AI_PASSIVE;
    // setting up SERVERM TCP port
    if ((tcpstatus = getaddrinfo(IP_ADDR, SERVER_M_TCP_PORT, &hints_TCP, &servinfo_TCP)) != 0)
    {
        fprintf(stderr, "SERVERM TCP set up incorrectly: %s\n", gai_strerror(tcpstatus));
        exit(1);
    }
    s_TCP = socket(servinfo_TCP->ai_family, servinfo_TCP->ai_socktype, servinfo_TCP->ai_protocol);
    bind(s_TCP, servinfo_TCP->ai_addr, servinfo_TCP->ai_addrlen);
    listen(s_TCP, BACKLOG);
}

void clientNameTrans()
{
    char *mess = new char[2000];
    strcpy(mess, recvTcpMess);
    char delims[] = " ";
    namePoint = strtok(mess, delims);
    while (namePoint)
    {
        inputNames.push_back(namePoint);
        namePoint = strtok(NULL, delims);
    }

    // test if all names are stored in inputnames
    // cout << "test if all names are stored in inputnames" << endl;
    // vector<string>::iterator iter;
    // iter = inputNames.begin();
    // while (iter != inputNames.end())
    // {
    //     cout << *iter << endl;
    //     iter++;
    // }
}

void errNamesTrans()
{
    vector<string>::iterator errIter;
    errIter = errNames.begin();
    // serverM output message
    while (errIter != errNames.end())
    {
        errNamStr = errNamStr + *errIter;
        if ((errIter + 1) != errNames.end())
            errNamStr = errNamStr + ", ";
        errIter++;
    }
}

void valNamesTrans()
{
    vector<string>::iterator valIter;
    valIter = valNames.begin();
    // serverM output message
    while (valIter != valNames.end())
    {
        valNamStr = valNamStr + *valIter;
        if ((valIter + 1) != valNames.end())
            valNamStr = valNamStr + ", ";
        valIter++;
    }
}

void tcpErrName()
{
    send(new_fd, errNamStr.c_str(), 2000, 0);
}

// void tcpSendResMess(){
//     cout<<"\nOutmessage: " << outMess;
//     send(new_fd, outMess, 2000, 0);

// }

void udpSetup()
{
    memset(&hints_UDP, 0, sizeof(hints_UDP));
    hints_UDP.ai_family = AF_INET;
    hints_UDP.ai_socktype = SOCK_DGRAM;
    hints_UDP.ai_flags = AI_PASSIVE;

    if ((status = getaddrinfo(IP_ADDR, SERVER_M_UDP_PORT, &hints_UDP, &servinfo_UDP)) != 0)
    {
        fprintf(stderr, "SERVERM UDP set up incorrectly: %s\n", gai_strerror(status));
        exit(1);
    }
    s_UDP = socket(servinfo_UDP->ai_family, servinfo_UDP->ai_socktype, servinfo_UDP->ai_protocol);
    bind(s_UDP, servinfo_UDP->ai_addr, servinfo_UDP->ai_addrlen);
}

void udpGetnames()
{
    char udpReceMess[2000];
    set<string>::iterator it;

    // getting port info for the serverA and serverB
    getaddrinfo(IP_ADDR, SERVER_A_PORT, &hints_UDP, &serA);
    getaddrinfo(IP_ADDR, SERVER_B_PORT, &hints_UDP, &serB);

    // get name from serverA
    if (recvfrom(s_UDP, &nameCnt, 2000, 0, serA->ai_addr, &(serA->ai_addrlen))){};
        // cout << nameCnt << endl;
    for (int i = 0; i < nameCnt; i++)
    {
        if (recvfrom(s_UDP, udpReceMess, 2000, 0, serA->ai_addr, &(serA->ai_addrlen)))
        {
            allNameA.insert(udpReceMess);
            // cout << udpReceMess << " ";
        }
    }
    // // test allnameA
    // for (it = allNameA.begin(); it != allNameA.end(); it++)
    // {
    //     cout << *it << endl;
    // }

    fprintf(stdout, "Main Server received the username list from server A using UDP over port %s. \n", SERVER_M_UDP_PORT);
    // get name from serverB
    if (recvfrom(s_UDP, &nameCnt, 2000, 0, serB->ai_addr, &(serB->ai_addrlen))){};
        // cout << nameCnt << endl;
    for (int i = 0; i < nameCnt; i++)
    {
        if (recvfrom(s_UDP, udpReceMess, 2000, 0, serB->ai_addr, &(serB->ai_addrlen)))
        {
            allNameB.insert(udpReceMess);
        }
    }
    fprintf(stdout, "Main Server received the username list from server B using UDP over port %s. \n", SERVER_M_UDP_PORT);
    // // test allnameB
    // for (it = allNameB.begin(); it != allNameB.end(); it++)
    // {
    //     cout << *it << endl;
    // }
}

void udpSendNames()
{
    vector<string>::iterator nameIter;

    // send inputnameA to server A
    nameIter = inputNameA.begin();

    while (nameIter != inputNameA.end())
    {
        inputNameAStr = inputNameAStr + *nameIter;
        if ((nameIter + 1) != inputNameA.end())
            inputNameAStr = inputNameAStr + ", ";
        nameIter++;
    }

    nameIter = inputNameA.begin();
    while (nameIter != inputNameA.end())
    {
        sendto(s_UDP, (*nameIter).c_str(), 2000, 0, serA->ai_addr, serA->ai_addrlen);
        nameIter++;
    }
    sendto(s_UDP, endFlag, 2000, 0, serA->ai_addr, serA->ai_addrlen);

    fprintf(stdout, "Found %s located at Server A. Send to Server A.\n", (inputNameAStr).c_str());

    // send inputnameA to server B
    nameIter = inputNameB.begin();

    while (nameIter != inputNameB.end())
    {
        inputNameBStr = inputNameBStr + *nameIter;
        if ((nameIter + 1) != inputNameB.end())
            inputNameBStr = inputNameBStr + ", ";
        nameIter++;
    }

    nameIter = inputNameB.begin();
    while (nameIter != inputNameB.end())
    {
        sendto(s_UDP, (*nameIter).c_str(), 2000, 0, serB->ai_addr, serB->ai_addrlen);
        nameIter++;
    }
    sendto(s_UDP, endFlag, 2000, 0, serB->ai_addr, serB->ai_addrlen);

    fprintf(stdout, "Found %s located at Server B. Send to Server B.\n ", (inputNameBStr).c_str());
}

void udpRevTime()
{
    char buf[2000];
    if (recvfrom(s_UDP, buf, 2000, 0, serA->ai_addr, &(serA->ai_addrlen)))
    {
        char *mess = new char[2000];
        char *mesP = new char[2000];
        strcpy(mess, buf);
        char delims[] = ",";
        mesP = strtok(mess, delims);

        while (mesP)
        {
            // cout << "mesP: " << mesP << endl;

            // cout << "test:" << endl;
            char tmpTimeLot[2000];
            strcpy(tmpTimeLot, mesP);
            //     cout<<"mesP: "<<mesP<<endl;
            if (mesP)
            {
                string tim1tim2(mesP);
                int i = 0;
                int tim2Flag = 0;
                int tmpTime1 = 0;
                int tmpTime2 = 0;

                while (i < tim1tim2.size())
                {
                    if (tim1tim2[i] == ' ')
                        tim2Flag = 1;
                    else
                    {
                        if (!tim2Flag)
                        {
                            tmpTime1 = tmpTime1 * 10;
                            tmpTime1 += tim1tim2[i] - '0';
                        }
                        else
                        {
                            tmpTime2 = tmpTime2 * 10;
                            tmpTime2 += tim1tim2[i] - '0';
                        }
                    }
                    i++;
                }
                vector<int> onetimelot;
                onetimelot.push_back(tmpTime1);
                onetimelot.push_back(tmpTime2);
                timeA.push_back(onetimelot);
            mesP = strtok(NULL, delims);

                // onetimelot.clear();
                // fprintf(stdout, "tim1,tim2:%d , %d\n", tmpTime1, tmpTime2);

            }
        }
                // test vector

        // for (int i = 0; i < timeA.size(); i++) cout << timeA[i][0] << ' ' << timeA[i][1] << endl;
        fprintf(stdout, "Main Server received from server A the intersection result using UDP over port %s: ", SERVER_M_UDP_PORT);
        fprintf(stdout, "%s.\n", timeLotOutput(buf));
    }
    // char * buf2=new char[2000];
    if (recvfrom(s_UDP, buf, 2000, 0, serB->ai_addr, &(serB->ai_addrlen)))
    {
        // cout << buf << "\n"
        //      << endl;
        char *messB = new char[2000];
        char *mesPB = new char[2000];

        strcpy(messB, buf);
        char delims[] = ",";
        mesPB = strtok(messB, delims);
        // cout << endl;
        // cout << endl;
        // cout << endl;

        while (mesPB)
        {
            // cout << "mesP: " << mesPB << endl;

            // cout << "test:" << endl;
            char tmpTimeLot[2000];
            strcpy(tmpTimeLot, mesPB);
            // cout<<"mesP: "<<mesPB<<endl;
            if (mesPB)
            {
                string tim1tim2(mesPB);
                int i = 0;
                int tim2Flag = 0;
                int tmpTime1 = 0;
                int tmpTime2 = 0;

                while (i < tim1tim2.size())
                {
                    if (tim1tim2[i] == ' ')
                        tim2Flag = 1;
                    else
                    {
                        if (!tim2Flag)
                        {
                            tmpTime1 = tmpTime1 * 10;
                            tmpTime1 += tim1tim2[i] - '0';
                        }
                        else
                        {
                            tmpTime2 = tmpTime2 * 10;
                            tmpTime2 += tim1tim2[i] - '0';
                        }
                    }
                    i++;
                }
                vector<int> onetimelot;

                onetimelot.push_back(tmpTime1);
                onetimelot.push_back(tmpTime2);
                // cout<<"onetimelot:"<<onetimelot[0]<<" "<<onetimelot[1]<<endl;
                timeB.push_back(onetimelot);
                mesPB = strtok(NULL, delims);

                // cout<<"onetimcheckelot:"<<onetimelot[0]<<" "<<onetimelot[1]<<endl;
                // onetimelot.clear();
                // fprintf(stdout, "tim1,tim2:%d , %d\n", tmpTime1, tmpTime2);
            }
        }
        // for (int i = 0; i < timeB.size(); i++) cout << timeB[i][0] << ' ' << timeB[i][1] << endl;

        fprintf(stdout, "Main Server received from server B the intersection result using UDP over port %s: ", SERVER_M_UDP_PORT);
        fprintf(stdout, "%s.\n", timeLotOutput(buf));
    }
}

void nameCheck()
{
    vector<string>::iterator iter;
    iter = inputNames.begin();
    while (iter != inputNames.end())
    {
        if ((allNameA.count(*iter) == 0) && (allNameB.count(*iter) == 0))
        {
            errNames.push_back(*iter);
            errNamFlag++;
        }
        else if (allNameA.count(*iter) != 0)
        {
            valNames.push_back(*iter);
            inputNameA.push_back(*iter);
        }
        else if (allNameB.count(*iter) != 0)
        {
            valNames.push_back(*iter);
            inputNameB.push_back(*iter);
        }
        iter++;
    }
    // test errNames
    if (errNamFlag)
    {
        iter = errNames.begin();
        while (iter != errNames.end())
        {
            cout << *iter << endl;
            iter++;
        }
    }
}

vector<vector<int> > twoPartMeeting(vector<vector<int> > &slots1, vector<vector<int> > &slots2)
{
    vector<vector<int> > result;
    sort(begin(slots1), end(slots1));
    sort(begin(slots2), end(slots2));

    int i = 0; // slots1's index
    int j = 0; // slots2's index

    while (i < slots1.size() && j < slots2.size())
    {

        const int start = max(slots1[i][0], slots2[j][0]);
        const int end = min(slots1[i][1], slots2[j][1]);
        if (start + 1 <= end)
        {
            vector<int> resPair;
            resPair.push_back(start);
            resPair.push_back(end);
            result.push_back(resPair);
            // cout<<"!!!!"<<resPair[0]<<" "<<resPair[1]<<endl;
            ++i;
            ++j;
        }
        else
        {
            // return {start, s};
            if (slots1[i][1] < slots2[j][1])
                ++i;
            else
                ++j;
        }
    }

    if (result.size() == 0)
        return vector<vector<int> >();
    else
        return result;
}

int main(int argc, char const *argv[])
{
    // Boot-up
    tcpSetup();
    udpSetup();
    fprintf(stdout, "Main Server is up and running.\n");
    // udp-receive name list from server A and B
    udpGetnames();

    // tcp-get names from client
    while (1)
    {
        // initialize all variables
        init();
        client_addr_size = sizeof(client_addr);
        // getting connected with the client and receive location of the client
        new_fd = accept(s_TCP, (struct sockaddr *)&client_addr, &client_addr_size);
        recv(new_fd, recvTcpMess, 2000, 0);
        fprintf(stdout, "Main Server received the request from client using TCP over port %s.\n", SERVER_M_TCP_PORT);

        // trans client input names to names vectors
        clientNameTrans();

        // check if names are valid
        nameCheck();
        valNamesTrans();

        // if some names are invalid
        if (errNamFlag)
        {
            errNamesTrans();
            cout << errNamStr << " do not exist. Send a reply to the client." << endl;

            // send error names to client
            tcpErrName();
        }
        // send names to server A and B
        udpSendNames();
        // receive time lot from server A and B
        udpRevTime();

        timeRes = twoPartMeeting(timeA, timeB);
        char *outMess=new char[2000];
        outMess = vector2string(timeRes);
        // fprintf(stdout, "res!!!!!");
        // cout<<timeRes.size()<<endl;
        // for (int i = 0; i < timeA.size(); i++) cout << timeA[i][0] << ' ' << timeA[i][1] << endl;
        // for (int i = 0; i < timeB.size(); i++) cout << timeB[i][0] << ' ' << timeB[i][1] << endl;
        // for (int i = 0; i < timeRes.size(); i++) cout << timeRes[i][0] << ' ' << timeRes[i][1] << endl;

        fprintf(stdout, "Found the intersection between the results from server A and B: ");
        fprintf(stdout, "%s\n", outMess);

        send(new_fd, outMess, 2000, 0);
        send(new_fd, valNamStr.c_str(), 2000, 0);
        fprintf(stdout, "Main Server sent the result to the client. \n");

    }
}