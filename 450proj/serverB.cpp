#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <fstream>
#include <map>
#include <string.h>
#include <iostream>
#include <cstring>
#include <vector>
#include <iostream>
#include <set>
#include <stdlib.h>
#include <stdio.h>
#define SERVER_A_PORT "21278"
#define SERVER_B_PORT "22278"
#define SERVER_M_UDP_PORT "23278"
#define SERVER_M_TCP_PORT "24278"

#define IP_ADDR "127.0.0.1"
#define INTMAX 2147483647
// #define BACKLOG 10	 // how many pending connections queue will hold

using namespace std;

// string is the name, the value is the pointer to the meeting time.
map<string, vector<vector<int> > > mapB;
int nameCnt = 0;
set<string> allName;
set<string> *allNameP;
vector<string> inputNames;

vector<string> notExiName;
int notExiNameCnt=0;

vector<int> transToVector(int a, int b)
{
    vector<int> result;
    result.push_back(a);
    result.push_back(b);
    return result;
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

vector<vector<int> > allPartMeeting(vector<string> inputName)
{
    vector<vector<int> >tmpRes;
    vector<vector<int> >res;
    vector<string>::iterator iter;
    vector<string>::iterator lastIter;
    iter = inputName.begin();
    lastIter=iter;
    // if the map is empty
    if (iter == inputName.end())
        return res;

    while (iter != inputName.end())
    {
        if (mapB.count(*iter) == 0)
        {
            notExiName.push_back(*iter);
            notExiNameCnt++;
        }
        iter++;
    }
    //
    if (notExiNameCnt != 0)
        return res;
    iter = inputName.begin();
    iter++;
    if(iter!=inputName.end()){
        // get the common time for the first two elements
        // if(mapB.count(*lastIter)!=0)
        tmpRes=twoPartMeeting(mapB[*lastIter],mapB[*iter]);
        // else{
        //     //error
        // }
        iter++;
    }
    else{
        // if map only has one participant
        return mapB[*lastIter];
    }
    // if map only has two participant
    if(iter==inputName.end())return tmpRes;
    while (iter != inputName.end())
    {
        tmpRes=twoPartMeeting(tmpRes,mapB[*iter]);
        iter++;
    }
    res=tmpRes;
    return res;
}

void readFile()
{

    ifstream f;
    string line(1000, ' ');
    char *cstr = new char[1000];
    f.open("b.txt", ios::in);

    int index = 0;
    int numCnt = 0;

    // reindex the locations and find the count
    if (!f)
    {
        // error
        fprintf(stderr, "failed to read a.txt\n");
        exit(1);
    }
    while (!f.eof())
    {
        // check later,看看如果有不合法的名字是直接跳过 还是报错，然后namecnt++放前面还是后面
        nameCnt++;
        // traverse every line
        int lineTime[1000];
        vector<vector<int> >lineTim;
        for (int i = 0; i < 1000; i++)
        {
            lineTime[i] = 0; // 设置元素 i 为 i + 100
        }
        int lineTimeInd = 0;

        numCnt = 0;
        f >> line;
        // delet all spaces
        while ((index = line.find(' ', index)) != string::npos)
        {
            line.erase(index, 1);
        }
        // find the first coma, get the name
        std::strcpy(cstr, line.c_str());
        char *name;
        name = strtok(cstr, ";");
        int numInd = INTMAX;

        for (size_t i = 0; i < line.size(); i++)
        {
            if (line[i] == ';')
                numInd = i + 1;
        }
        if (numInd == INTMAX)
        {
            // error
        }

        // find num pairs, and store in int[]
        int tmpTime1 = 0;
        int tmpTime2 = 0;
        int timeCnt = 0;
        int timeLotCnt = 0;
        size_t i = numInd;
        while (i < line.size())
        {
            char tmpchar = line[i];
            if (line[i] == '[')
            {
                i += 1;
                while (i < line.size() && isdigit(line[i]))
                {
                    tmpTime1 = tmpTime1 * 10;
                    tmpTime1 += line[i] - '0';
                    i++;
                }
                if (i < line.size() && isdigit(line[i - 1]))
                    timeCnt++;
            }
            else if (line[i] == ',' && isdigit(line[i - 1]))
            {
                i += 1;
                while (i < line.size() && isdigit(line[i]))
                {
                    tmpTime2 = tmpTime2 * 10;
                    tmpTime2 += line[i] - '0';
                    i++;
                }
                if (i < line.size() && isdigit(line[i - 1]))
                    timeCnt++;
            }
            else if (line[i] == ']' && isdigit(line[i - 1]))
            {
                if (timeCnt != 2)
                {
                    std::cout << "error: unexpected" << endl;
                }
                else
                {
                    timeCnt = 0;
                    // lineTim.push_back(tmpTime1);
                    lineTime[lineTimeInd] = tmpTime1;
                    timeLotCnt++;
                    lineTimeInd++;
                    lineTim.push_back(transToVector(tmpTime1,tmpTime2));

                    lineTime[lineTimeInd] = tmpTime2;
                    lineTimeInd++;
                    timeLotCnt++;
                    tmpTime1 = 0;
                    tmpTime2 = 0;
                }
                i++;
            }
            else
                i++;
        }

        int *timelot = new int[timeLotCnt];
        for (int i = 0; i < timeLotCnt; i++)
        {
            timelot[i] = lineTime[i];
        }
        mapB[name] = lineTim;
    }
    // initializing the map, len with specified size
    f.close();
}

// tarverse the map to store all names in set `allName`
void creatNameMess()
{
    map<string, vector<vector<int> > >::iterator iter;
    iter = mapB.begin();
    while (iter != mapB.end())
    {
        allName.insert(iter->first);
        std::cout << iter->first << endl;
        iter++;
    }
}

int main(int argc, char const *argv[])
{
    int status;
    struct addrinfo hints;
    struct addrinfo *servinfo, *scheduler, *p;
    int s;
    int sockfd;
    set<string>::iterator it;

    // tmp store message sent and received by UDP
    char sendMess[2000];
    char receMess[2000];
    readFile();
    creatNameMess();

    // create UDP socket
    // It sets up a UDP socket for communication with the scheduler.

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_flags = AI_PASSIVE;

    if ((status = getaddrinfo(IP_ADDR, SERVER_B_PORT, &hints, &servinfo)) != 0)
    {
        fprintf(stderr, "SERVER B set up incorrectly: %s\n", gai_strerror(status));
        exit(1);
    }

    if (servinfo == NULL)
    {
        fprintf(stderr, "listener: failed to bind socket\n");
        exit(1);
        return 2;
    }
    s = socket(servinfo->ai_family, servinfo->ai_socktype, servinfo->ai_protocol);
    if (s == -1)
    {
        perror("listener: socket");
    }
    // check later--have a bug with ==
    if (bind(s, servinfo->ai_addr, servinfo->ai_addrlen) == -1)
    {
        close(s);
        perror("listener: bind");
    }

    fprintf(stdout, "SERVER B is up and running using UDP on port %s.\n", SERVER_B_PORT);
    // fprintf(stdout, "SERVER B has total capactity %d and initial occupancy %d\n", total_cap, cur_occup);

    getaddrinfo(IP_ADDR, SERVER_M_UDP_PORT, &hints, &scheduler);
    // send names to serverM
    allNameP = &allName;
    cout << allNameP << endl;
    sendto(s, &nameCnt, allName.size(), 0, scheduler->ai_addr, scheduler->ai_addrlen);

    for (it = allName.begin(); it != allName.end(); it++)
    {
        strcpy(sendMess, (*it).c_str());
        sendto(s, sendMess, allName.size(), 0, scheduler->ai_addr, scheduler->ai_addrlen);
    }
    fprintf(stderr, "Server B finished sending a list of usernames to Main Server.\n");

    // test start
    // vector<string> testNames;
    // testNames.push_back("yinan");
    // testNames.push_back("fu");
    // testNames.push_back("eliana");
    // testNames.push_back("zecchinos");
    // // khloe;[[5,6],[11,12],[15,18],[21,22],[24,29],[30,35],[39,42]]
    // // eli;[[5,10],[12,16],[21,25],[29,31],[35,37],[39,42],[46,48],[53,56],[57,62],[64,68]]
    // // eliana;[[4,5],[7,8],[10,15],[18,19],[23,26],[30,32],[34,38]]
    // // zecchinos;[[3,4],[8,11],[16,18],[21,26]]
    // // addison;[[3,7]]
    // // theodore;[[4,5],[7,8],[11,16],[20,22],[26,27],[28,30]]
    // // callie;[[2,4]]
    // // zein;[[4,8],[11,12],[13,17],[20,22]]
    // // graham;[[1,6],[9,14],[18,21],[25,30],[34,38],[40,41],[44,48]]
    // // jameson;[[1,5],[8,12],[13,18],[19,23],[27,30]]
    // vector<vector<int> > test1=twoPartMeeting(mapB["khloe"],mapB["eli"]);
    // vector<vector<int> > testRes=allPartMeeting(testNames);
    // // for (int i = 0; i < test1.size(); i++)
    // //     cout << test1[i][0] << ' ' << test1[i][1] << endl;
    // for (int i = 0; i < testRes.size(); i++)
    //     cout << testRes[i][0] << ' ' << testRes[i][1] << endl;
    // for (int i = 0; i < notExiName.size(); i++)
    //     cout << notExiName[i]  << endl;
    // // test end

    // an infinite loop, where it receives messages from the scheduler
    while (1)
    {
        // receive messages from the scheduler
        recvfrom(s, receMess, 2000, 0, scheduler->ai_addr, &(scheduler->ai_addrlen));
        // cout<<receMess<<endl;

        // not end, add it to the vector inputnames
        if (strcmp(receMess, "END")){
            string tmpName=receMess;
            inputNames.push_back(receMess);

        }
        // input ends:
        // run the alg and return time to serverM
        // print all names
        // init vector inputnames
        else{
            fprintf(stdout, "Server B received the usernames from Main Server using UDP over port %s.\n", IP_ADDR);
            vector<vector<int> > timeRes = allPartMeeting(inputNames);
            // test
            char timeLot[2000];
            char timeLot2[2000];
            char sendTimeMess[2000]="";
            fprintf(stdout, "Found the intersection result: [");
            for (int i = 0; i < timeRes.size(); i++)
            {
                fprintf(stdout, "[%d,%d]", timeRes[i][0], timeRes[i][1]);
                if ((i + 1) < timeRes.size())
                {
                    fprintf(stdout, ", ");
                }

                snprintf(timeLot, sizeof(timeLot), "%d", timeRes[i][0]);
                strcat(timeLot, " ");
                snprintf(timeLot2, sizeof(timeLot), "%d", timeRes[i][1]);

                strcat(timeLot, timeLot2);
                strcat(sendTimeMess,timeLot);
                strcat(sendTimeMess,",");
                // strcat(&timeLot, ' ');
            }
            fprintf(stdout, "]\n");

            cout<<"!!!!!"<<sendTimeMess<<endl;

            sendto(s, sendTimeMess, 2000, 0, scheduler->ai_addr, scheduler->ai_addrlen);
            fprintf(stdout,"Server B finished sending the response to Main Server.\n");
            //     cout << timeRes[i][0] << ' ' << timeRes[i][1] << endl;

            // after getting the time lot, clear inputNames
            inputNames.clear();
        }

    }

}
