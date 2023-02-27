// 用以测试FastMath.h速度的测试框架


#include "assert.h"

#include "FastMath.h"
// #include "DiscardedImpl.h"

// #include<bits/stdc++.h>
#include <iostream>
#include <random>
#include <cstdlib>
#include <assert.h>
#include <algorithm>
#include <vector>

template <typename T>
class my_rd_real_eng{
    private:
    std::default_random_engine e;
    std::uniform_real_distribution<T> u;

    public:

    my_rd_real_eng(T l=-1000.0,T r=1000.0,
            int sd=time(NULL)):e(sd),u(l,r){}
    T get_rd_val(){return u(e);}
};
#define MAX_DATA_N 15000005
#define MAX_DIFF_N 20


// #define float double

template <typename T>
class test_framework_unary{
    static T data[MAX_DATA_N],ans[MAX_DATA_N],gen[MAX_DATA_N];
    
    int round,n;
    my_rd_real_eng<T> &rd_eng;
    T(*func[MAX_DIFF_N])(T); //func[0]:err_standard
    int funcN;
    void gendata(){
        for(int i=1;i<=n;++i) data[i]=rd_eng.get_rd_val();
    }
    void get_rd_perm(int *p,int n){
        for(int i=0;i<n;++i) p[i]=i+1;
        for(int i=n-1;i>0;--i) {
            std::swap(p[i], p[std::rand() % (i+1)]);
        }
    }
    double calc_rerr(){
        double rt=0;
        for(int i=1;i<=n;++i)
        // if(fabs(data[i]-1)>10)
        // if(fabs(ans[i])>1e-2)
        {
            double cur_err =std::min(fabs((double)(ans[i]-gen[i])/ans[i]),fabs((double)(ans[i]-gen[i])));
            // printf("%.10f\n",cur_err);
            rt=std::max(rt,cur_err);
            // if(cur_err>0.0005){
            //     static int ct=0;
            //     static float mi=30,pj=0;
            //     mi=std::min(mi,(float)data[i]);
            //     ct++;
            //     pj+=cur_err/(data[i]*data[i]);
            //     if(ct==100000){
            //         printf("para:  %.10lf %.10lf\n",mi,pj/ct);
            //     }
            //     static int a=0;
            //     a++;
            //     if(a<=10){
            //     printf("[%.10lf]%.10lf %.10lf\n",cur_err,data[i],cur_err/(data[i]*data[i]));
            //     }
            //     // assert(0);
            // }
        }
        return rt;
    }


    public:
    test_framework_unary(my_rd_real_eng<T>&re,
                        int round_i=20,
                        int n_i=MAX_DATA_N-5):
        round(round_i),n(n_i),rd_eng(re){
            func[0]=NULL;
            funcN=0;
            srand(time(0));
        }
    void add_func(T(*func_i)(T)){
        funcN++;
        func[funcN]=func_i;
    }
    void set_err_standard(T(*func_i)(T)){
        func[0]=func_i;
    }
    int discard_round=2;
    #define apply_func(f,out) for(int oo=1;oo<=n;++oo) out[oo]=(f)(data[oo]);
    void runtest(int need_err=1){
        assert(need_err==0||func[0]!=NULL);
        double tm[MAX_DIFF_N],terr[MAX_DIFF_N];
        for(int i=0;i<=funcN;++i) tm[i]=0,terr[i]=0;
        for(int rd=1;rd<=round;++rd){
            double tuse[MAX_DIFF_N],rerr[MAX_DIFF_N];
            gendata();
            if(need_err) apply_func(func[0],ans);
            int p[MAX_DIFF_N];
            get_rd_perm(p,funcN);
            for(int i=0;i<funcN;++i){
                double tbg=(double)clock()/CLOCKS_PER_SEC;
                apply_func(func[p[i]],gen);
                tuse[p[i]]=(double)clock()/CLOCKS_PER_SEC-tbg;
                if(rd>discard_round){
                    tm[p[i]]+=tuse[p[i]]/(round-discard_round);
                }
                if(need_err){
                    rerr[p[i]]=calc_rerr();
                    terr[p[i]]=std::max(rerr[p[i]],terr[p[i]]);
                }
            }
            printf("[##%3d] Time: ",rd);
            for(int i=1;i<=funcN;++i)
                printf("%10.6lf",tuse[i]);
            if(need_err){
                printf("  Err:  ");
                for(int i=1;i<=funcN;++i)
                    printf("%.6lf ",rerr[i]);
            }
            printf("\n");
        }
        printf("\033[31m");
        printf("[TOTAL] Time: ");
        for(int i=1;i<=funcN;++i)
            printf("%10.6lf",tm[i]);
        printf(" {%.6lf %.6lf}",tm[1]/tm[2],1-tm[2]/tm[1]);
        printf(" {%.6lf %.6lf}",tm[1]/tm[3],1-tm[3]/tm[1]);
        printf(" {%.6lf %.6lf}",tm[2]/tm[3],1-tm[3]/tm[2]);

        if(need_err){
            printf("  Err:  ");
            for(int i=1;i<=funcN;++i)
                printf("%.6lf ",terr[i]);
        }
        printf("\033[0m\n");        
    }
    #undef apply_func
};
template<typename T>
T test_framework_unary<T>::data[MAX_DATA_N];
template<typename T>
T test_framework_unary<T>::ans[MAX_DATA_N];
template<typename T>
T test_framework_unary<T>::gen[MAX_DATA_N];

#define MAX_DATA_N_B (((MAX_DATA_N)-5)/2 + 5)
template <typename T>
class test_framework_binary{
    static T data1[MAX_DATA_N_B],data2[MAX_DATA_N_B],ans[MAX_DATA_N_B],gen[MAX_DATA_N_B];
    
    int round,n;
    my_rd_real_eng<T> &rd_eng1;
    my_rd_real_eng<T> &rd_eng2;
    T(*func[MAX_DIFF_N])(T,T); //func[0]:err_standard
    int funcN;
    void gendata(){
        for(int i=1;i<=n;++i) data1[i]=rd_eng1.get_rd_val();
        for(int i=1;i<=n;++i) data2[i]=rd_eng2.get_rd_val();
    }
    void get_rd_perm(int *p,int n){
        for(int i=0;i<n;++i) p[i]=i+1;
        for(int i=n-1;i>0;--i) {
            std::swap(p[i], p[std::rand() % (i+1)]);
        }
    }
    double calc_rerr(){
        double rt=0;
        for(int i=1;i<=n;++i)
        // if(fabs(data[i]-1)>10)
        // if(fabs(ans[i])>1e-2)
        {
            // printf("%.10lf \n",(double)(ans[i]-gen[i])/ans[i]);
            double cur_err =std::min(fabs((double)(ans[i]-gen[i])/ans[i]),fabs((double)(ans[i]-gen[i])));
            rt=std::max(rt,cur_err);
            // rt+=cur_err/n;
            if(cur_err>10){
                // printf("%.10lf %.10lf %.10lf %.10lf\n",data1[i],data2[i],gen[i],ans[i]);
                // assert(0);
            }
        }
        return rt;
    }


    public:
    test_framework_binary(my_rd_real_eng<T>&re1,my_rd_real_eng<T>&re2,
                        int round_i=20,
                        int n_i=MAX_DATA_N_B-5):
        round(round_i),n(n_i),rd_eng1(re1),rd_eng2(re2){
            func[0]=NULL;
            funcN=0;
            srand(time(0));
        }
    void add_func(T(*func_i)(T,T)){
        funcN++;
        func[funcN]=func_i;
    }
    void set_err_standard(T(*func_i)(T,T)){
        func[0]=func_i;
    }
    int discard_round=2;
    #define apply_func(f,out) for(int oo=1;oo<=n;++oo) out[oo]=(f)(data1[oo],data2[oo]);
    void runtest(int need_err=1){
        assert(need_err==0||func[0]!=NULL);
        double tm[MAX_DIFF_N],terr[MAX_DIFF_N];
        for(int i=0;i<=funcN;++i) tm[i]=0,terr[i]=0;
        for(int rd=1;rd<=round;++rd){
            double tuse[MAX_DIFF_N],rerr[MAX_DIFF_N];
            gendata();
            if(need_err) apply_func(func[0],ans);
            int p[MAX_DIFF_N];
            get_rd_perm(p,funcN);
            for(int i=0;i<funcN;++i){
                double tbg=(double)clock()/CLOCKS_PER_SEC;
                apply_func(func[p[i]],gen);
                tuse[p[i]]=(double)clock()/CLOCKS_PER_SEC-tbg;
                if(rd>discard_round){
                    tm[p[i]]+=tuse[p[i]]/(round-discard_round);
                }
                if(need_err){
                    rerr[p[i]]=calc_rerr();
                    terr[p[i]]=std::max(rerr[p[i]],terr[p[i]]);
                }
            }
            printf("[##%3d] Time: ",rd);
            for(int i=1;i<=funcN;++i)
                printf("%10.6lf",tuse[i]);
            if(need_err){
                printf("  Err:  ");
                for(int i=1;i<=funcN;++i)
                    printf("%.6lf ",rerr[i]);
            }
            printf("\n");
        }
        printf("\033[31m");
        printf("[TOTAL] Time: ");
        for(int i=1;i<=funcN;++i)
            printf("%10.6lf",tm[i]);
        printf(" {%.6lf %.6lf}",tm[1]/tm[2],1-tm[2]/tm[1]);
        printf(" {%.6lf %.6lf}",tm[1]/tm[3],1-tm[3]/tm[1]);
        printf(" {%.6lf %.6lf}",tm[2]/tm[3],1-tm[3]/tm[2]);
        if(need_err){
            printf("  Err:  ");
            for(int i=1;i<=funcN;++i)
                printf("%.6lf ",terr[i]);
        }
        printf("\033[0m\n");        
    }
};
template<typename T>
T test_framework_binary<T>::data1[MAX_DATA_N_B];
template<typename T>
T test_framework_binary<T>::data2[MAX_DATA_N_B];
template<typename T>
T test_framework_binary<T>::ans[MAX_DATA_N_B];
template<typename T>
T test_framework_binary<T>::gen[MAX_DATA_N_B];



using fm::pi_f;

int main(){
    // float a,b;
    // std::cin>>a>>b;
    // fm::init();
    // exit(0);
    #define totst atan2
    // float x = fm::log(10.0f);
    
    // unary
    // #define float double
    // my_rd_real_eng<float> myeg(-20,20);
    // test_framework_unary<float> tp(myeg,20,MAX_DATA_N-5);
    // tp.add_func([](float x)->float{return fm::totst(x,fm::ESpeedStd);});
    // tp.add_func([](float x)->float{return fm::totst(x,fm::ESpeedNormal);});    
    // // tp.add_func([](float x)->float{return fm::totst(x,fm::ESpeedFast3);});    
    // tp.set_err_standard([](float x)->float{return std::totst(x);});
    // tp.runtest(1);

    // printf("%.6lf %.6lf %.6lf %.6lf\n",
        // fm::atan2(1,0),fm::atan2(-1,0),fm::atan2(0,1),fm::atan2(0,-1));

    // binary
    // #define float double
    my_rd_real_eng<float> myeg1(-10,10),myeg2(-10,10);
    test_framework_binary<float> tp2(myeg1,myeg2,20,MAX_DATA_N_B-5);
    tp2.add_func([](float x,float y)->float{return fm::totst(x,y,fm::ESpeedStd);});
    tp2.add_func([](float x,float y)->float{return fm::totst(x,y,fm::ESpeedNormal);});  
    tp2.add_func([](float x,float y)->float{return fm::totst(x,y,fm::ESpeedFast3);});      
    tp2.set_err_standard([](float x,float y)->float{return std::totst(x,y);});
    tp2.runtest(1);

    // printf("%.10lf\n",noinlinewp(a)+noinlinewp2(b));
}