#include <iostream>
using namespace std;
#include <algorithm>
#include <cmath>
#include <fstream>
#include <map>
#include <queue>
#include <sstream>
#include <string>
#include <unistd.h>
#include <vector>

struct Process {
  int ID;
  float _time_;
  string name;
  int PhyRead;
  int LogRead;
  int PhyWrite;
  int buffersize; 
  bool operator<(const Process &other) const { return _time_ > other._time_; }
};
float needTime = 0; 
bool CPUempty = true;
bool SSDempty = true;
float clocktime = 0;
queue<Process> ssd;
queue<Process> Ready;
priority_queue<Process> Mainqueue;

struct Input {
  int index;
  string command;
  float time;
};

struct Processtable {
  int PID;
  int startline;
  int endline;
  int curline;
  int state;
};

void coreReq(Process &process, vector<Processtable> &processTable,
             const vector<Input> &inputTable) {
  if (CPUempty) {
    CPUempty = false;
    processTable[process.ID].state = 1;
    float compTime = clocktime + process._time_;
    process._time_ = compTime;
    Mainqueue.push(process);
  } else {
    processTable[process.ID].state = 2;
    Ready.push(process);
  }
}

void ssdReq(Process &process, vector<Processtable> &processTable,
            const vector<Input> &inputTable, queue<Process> &Ready,
            queue<Process> &ssd, priority_queue<Process> &Mainqueue) {
  string command = inputTable[processTable[process.ID].curline].command;
  int reqBit = inputTable[processTable[process.ID].curline].time;
  int curBit = process.buffersize;
  reqBit = reqBit + reqBit; 
  if (command == "READ") {
    if (process.PhyRead == 0){
      process.PhyRead++;
      if(SSDempty){
        process._time_ = clocktime + .1;
        processTable[process.ID].state = 3;
        Mainqueue.push(process);
        SSDempty = true;
      } else {
        SSDempty = false;
        ssd.push(process);
      }
    } else {
      if (reqBit >= curBit){
        process.PhyRead++;
        if(SSDempty){
          process._time_ = clocktime + .1;
          processTable[process.ID].state = 3;
          Mainqueue.push(process);
          SSDempty = true;
        } else {
          ssd.push(process);
          SSDempty = false; 
        }
      } else{
        process.LogRead++;
        processTable[process.ID].curline++;
        process.name = inputTable[processTable[process.ID].curline].command;
        process._time_ = inputTable[processTable[process.ID].curline].time;
        coreReq(process, processTable, inputTable);
      }
    }
  } else if (command == "WRITE"){
    process.PhyWrite++;
    if (SSDempty){
      process._time_ = clocktime + .1;
      processTable[process.ID].state = 3; 
      Mainqueue.push(process);
      SSDempty = true;
    } else {
      ssd.push(process);
      SSDempty = false;
    }
  }
}
void ssdCompletion(Process& process,const vector<Input>& inputTable, vector<Processtable>& processTable,
queue<Process> &Ready, queue<Process> &ssd,
priority_queue<Process> &Mainqueue){
  SSDempty = true;
  if (!ssd.empty()){
    Process nextProcess = ssd.front();
    ssd.pop();
    //cout << nextProcess._time_ << endl;
    ssdReq(nextProcess, processTable,inputTable, Ready, ssd, Mainqueue);
  } else {
    processTable[process.ID].curline++;
    process.name = inputTable[processTable[process.ID].curline].command;
    process._time_ = inputTable[processTable[process.ID].curline].time; 
    //cout << process._time_ << endl; 
    coreReq(process, processTable, inputTable);
  }
} 
void coreComp(vector<Processtable> &processTable, vector<Input> &inputTable,
              queue<Process> &Ready, queue<Process> &ssd,
              priority_queue<Process> &Mainqueue, Process &process){
  CPUempty = true;
  if (!Ready.empty()) {
    Process top = Ready.front();
    needTime = top._time_;
    Ready.pop();
    coreReq(top, processTable, inputTable);
  }
    if (processTable[process.ID].curline == processTable[process.ID].endline) {
      processTable[process.ID].state = -1;
      cout << "Process " << process.ID << " terminates at " << process._time_ << "ms."
           << endl;
      cout << "It performed " << process.PhyRead << " physical read(s), "
           << process.LogRead << " logical read(s), " << process.PhyWrite
           << " physical write(s), and " << endl;
      cout << endl;
      cout << "Process states:" << endl;
      cout << "--------------" << endl;
      cout << process.ID << " TERMINATED" << endl;
      for (int i = 0; i < processTable.size(); i++){
        if (processTable[i].state == 1){
          cout << processTable[i].PID << " RUNNING" << endl;
        }else if (processTable[i].state == 2){
          cout << processTable[i].PID << " READY" << endl;
        } else if (processTable[i].state == 3){
          cout << processTable[i].PID << " BLOCKED" << endl;
        }
      }
      cout << endl; 
    } else {
      processTable[process.ID].curline++;
      process.name = inputTable[processTable[process.ID].curline].command;
      process._time_ = clocktime + needTime;
      //cout << process.name << " h "<< process._time_ << endl;
      needTime = 0; 
      if (process.name == "READ" || process.name == "WRITE") {
        ssdReq(process, processTable, inputTable, Ready, ssd, Mainqueue);
      } else if (process.name == "INPUT" || process.name == "DISPLAY"){
          int comp = inputTable[processTable[process.ID].curline].time;
          process._time_ = comp + clocktime; 
          Mainqueue.push(process);
        }
    }
}
void arrivalFunc(Process &process, vector<Processtable> &processTable,
 vector<Input> &inputTable, queue<Process> &Ready, 
 queue<Process> &ssd, priority_queue<Process> &Mainqueue) {
  processTable[process.ID].curline = processTable[process.ID].startline + 1;
  process.name = inputTable[processTable[process.ID].curline].command;
  process._time_ = inputTable[processTable[process.ID].curline].time;
  coreReq(process, processTable, inputTable);
}

int main() {
  int totalP = 0;
  vector<Input> inputs;
  vector<Processtable> Ptable;
  cout << "Kalyan Gautam" << endl;
  cout << "First assignment" << endl;
  cout << "How to compile (Please use repilt https://replit.com): " << endl;
  cout << "You need to downalod the .ccp file and you can run it in VS code or repilt (works best here). If you choose Visual Studio Code then open the .cpp file then hit the run code using g++. Then in the VS terminal you type ./a.out <input#.txt (I dont know if it works on other OS but it works on mac) or input#.txt" << endl;
  cout << "If you choose repilt the best method then open this file in c++ template and after pressing the green run button type any of the following things I listed for Visual Studio Code in the Console(not the shell) or you can just type in the file name in the console as well." << endl;
  string line;
  string inputname;
  getline(cin, inputname);
  inputname.erase(0, inputname.find_first_not_of(" \t\f\v\n\r")); //stack overflow
  inputname.erase(inputname.find_last_not_of(" \t\f\v\n\r") + 1); //stack overflow 
  size_t found = inputname.find("<");
  if (found != string::npos) { // stack overflow
    inputname = inputname.substr(found + 1);
  }
  ifstream inputfile(inputname);
  int row = 0;
  int buff;
  if (getline(inputfile, line)){
    istringstream of(line);
    string command;
    of >> command >> buff;
  }
  while (getline(inputfile, line)) {
    istringstream iss(line);
    string com;
    float time;
    if (iss >> com >> time) {
      Input input;
      input.index = row;
      input.command = com;
      if (com == "START") {
        totalP++;
      }
      input.time = time;
      inputs.push_back(input);
      row += 1;
    }
  }
  int curPD = 0;
  for (size_t i = 0; i < inputs.size(); i++) {
    if (inputs[i].command == "START") {
      Processtable PD;
      PD.PID = curPD++;
      PD.startline = inputs[i].index;
      size_t j;
      for (j = i + 1; j < inputs.size(); ++j) {
        if (inputs[j].command == "START") {
          break;
        }
      }
      PD.endline = inputs[j - 1].index;
      PD.curline = -1;
      PD.state = 0;
      Ptable.push_back(PD);
    }
  }
  for (int i = 0; i < totalP; i++) {
    int start = Ptable[i].startline;
    string comand = inputs[start].command;
    int tie = inputs[start].time;
    Process p;
    p.ID = Ptable[i].PID;
    p._time_ = static_cast<float>(tie);
    p.name = comand;
    p.PhyRead = 0;
    p.LogRead = 0;
    p.PhyWrite = 0;
    p.buffersize = buff;
    Mainqueue.push(p);
  }
  while (!Mainqueue.empty()) {
    Process top = Mainqueue.top();
   //cout << "Processing process " << top.ID << " with command " << top.name << " at time " << top._time_ << endl;
    Mainqueue.pop();
    clocktime = top._time_;
    if (top.name == "START") {
      //cout << "main got here start" << endl;
      arrivalFunc(top, Ptable, inputs, Ready, ssd, Mainqueue);
    }else if (top.name == "INPUT" || top.name == "DISPLAY"){
      Ptable[top.ID].curline++;
      top.name = inputs[Ptable[top.ID].curline].command;
      top._time_ = inputs[Ptable[top.ID].curline].time;
      coreReq(top, Ptable, inputs);
      //ssdCompletion(top, inputs, Ptable, Ready, ssd, Mainqueue);
    }else if (top.name == "CORE"){
      //cout << "main got here comp" << endl;
      coreComp(Ptable, inputs, Ready, ssd, Mainqueue, top);
    } else if (top.name == "READ" || top.name == "WRITE"){
      //cout << "main got here ssdcomp" << endl; 
      ssdCompletion(top, inputs, Ptable, Ready, ssd, Mainqueue);
    }
  }
  return 0;
}