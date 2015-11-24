#ifndef SIP_TIMERS_H
#define SIP_TIMERS_H

#include "sobject.h"

class Sip_Timers : public SObject
{
public:
    Sip_Timers();

    std::string get_mem_object_type() const {return "SipTimers";}
    void setT1(int t){A=E=G=T1=t; B=F=H=J=t*64; }
    void setT2(int t){T2 = t;}
    void setT4(int t){I=K=T4=t;}
    void setA(int t){A = t;}
    void setB(int t){B = t;}
    void setC(int t){C = t;}
    void setD(int t){D = t;}
    void setE(int t){E = t;}
    void setF(int t){F = t;}
    void setG(int t){G = t;}
    void setH(int t){H = t;}
    void setI(int t){I = t;}
    void setJ(int t){J = t;}
    void setK(int t){K = t;}
    int getT1(){return T1;}
    int getT2(){return T2;}
    int getT4(){return T4;}
    int getA(){return A;}
    int getB(){return B;}
    int getC(){return C;}
    int getD(){return D;}
    int getE(){return E;}
    int getF(){return F;}
    int getG(){return G;}
    int getH(){return H;}
    int getI(){return I;}
    int getJ(){return J;}
    int getK(){return K;}
private:
    int T1;
    int T2;
    int T4;
    int A;
    int B;
    int C;
    int D;
    int E;
    int F;
    int G;
    int H;
    int I;
    int J;
    int K;
};

#endif // SIP_TIMERS_H
