#pragma once


// const double	one	= 1.0, tiny=1.0e-300;
// typedef union 
// {
//   double value;
//   struct 
//   {
//     __uint32_t lsw;
//     __uint32_t msw;
//   } parts;
// } ieee_double_shape_type;

#include <cmath>
#include <cstdlib>
#include <stdint.h>
#include <limits>


//所有优化时间幅度都是指O3下相对ESpeedStd的数据，一般来说O1，O2幅度会更大
//误差一般指 max{所有输入}(min(相对误差，绝对误差))
//注意：O1及以上优化下，
//下列所有数学函数中以常量方式传速度开关参数，
//以及对应ifelse语句不会在汇编中体现，
//即多包装的一层“传参+选择判断”不会影响效率。
namespace fm{

    // 目的是防止*2 /2 导致的精度误差
    const float  hpi_f = 1.5707963267948966192313f;
    const double hpi_d = 1.5707963267948966192313;
    const float  pi_f  = 3.1415926535897932384626f;
    const double pi_d  = 3.1415926535897932384626;
    const float  tpi_f = 6.2831853071795864769252f;
    const float  tpi_d = 6.2831853071795864769252;


    #ifndef FM_SPEED_DEFAULT
        #define FM_SPEED_DEFAULT ESpeedNormal
    #endif


    enum speed_option{
        ESpeedStd=0, //直接调用std
        ESpeedNormal=1, //保证float最后一位级别精度
        ESpeedFast1=2, //保证所有输入min(绝对误差,相对误差)不超过1e-4，不保证nan,inf的特殊处理
        ESpeedFast2=3, //保证所有输入min(绝对误差,相对误差)不超过2e-3，不保证nan,inf的特殊处理        
        ESpeedFast3=4, //保证所有输入min(绝对误差,相对误差)不超过4e-2，不保证nan,inf的特殊处理
    };
    

    //这个确实没法优化
    template <typename T>
    inline T abs(T x,speed_option speed=FM_SPEED_DEFAULT){
        return std::abs(x);
    }

    //cmath中abs本身提供float等泛型支持，fabs只是pure C的历史遗留问题
    template <typename T>
    inline T fabs(T x,speed_option speed=FM_SPEED_DEFAULT){
        return std::fabs(x);
    }

    //这个确实没法优化
    template <typename T>
    inline T ceil(T x,speed_option speed=FM_SPEED_DEFAULT){
        return std::ceil(x);
    }    

    //这个确实没法优化
    template <typename T>
    inline T floor(T x,speed_option speed=FM_SPEED_DEFAULT){
        return std::floor(x);
    }    

    //这个确实没法优化
    template <typename T>
    inline T round(T x,speed_option speed=FM_SPEED_DEFAULT){
        return std::round(x);
    }    

    // fast1/fast2/fast3 用时少37%，误差不超过8e-5
    inline float log2(float x,speed_option speed=FM_SPEED_DEFAULT){
        if(speed==ESpeedStd||speed==ESpeedNormal){
            return std::log2(x);
        }
        else{//ESpeedFast1 ESpeedFast2 ESpeedFast3
            union {float f; uint32_t i;} vx;
            union {uint32_t i; float f;} mx;
            vx.f = x;
            mx.i = (vx.i & 0x007FFFFF) | 0x3F000000;
            float y = vx.i;
            y *= 1.1920928955078125e-7f;
            return y - 124.22544637f - 1.498030302f * mx.f -
                1.72587999f / (0.3520887068f + mx.f);
        }
    }


    //经过测试，能够找到的其他实现均不如标准库（这些实现可见于DiscardedImpl.h）
    template <typename T>
    inline T sqrt(T x, speed_option speed=FM_SPEED_DEFAULT){
        return std::sqrt(x);
    }

    // fast2 用时少8%，误差不超过2e-3
    // fast3 用时少55%，误差不超过4e-2
    inline float exp(float x, speed_option speed=FM_SPEED_DEFAULT){
        if(speed==ESpeedStd||speed==ESpeedNormal||speed==ESpeedFast1){
            return std::exp(x);
        }
        else if(speed==ESpeedFast2){          
            float t = 1.0f + x/4096;
            t *= t;  t *= t; t *= t; t *= t;
            t *= t;  t *= t; t *= t; t *= t;
            t *= t;  t *= t; t *= t; t *= t;
            t = t * (1+ 0.0001271152f * x * x);
            
            return t;
        }
        else{//ESpeedFast3
            union
            {
            uint32_t i;
            float f;
            }v;
            v.i=(1<<23)*(1.4426950409*x+126.94201519f);
            return v.f;        
        }
    }    

    // fast2 用时少3%，误差不超过2e-3
    // fast3 用时少46%，误差不超过4e-2
    inline float exp2(float x, speed_option speed=FM_SPEED_DEFAULT){
        if(speed==ESpeedStd||speed==ESpeedNormal||speed==ESpeedFast1){        
            return std::exp2(x);
        }
        else{//ESpeedFast2 ESpeedFast3
            return exp(x*0.6931471805599453f,speed);//ln2
        }
    }

    // fast1/fast2/fast3 用时少45%，误差不超过6e-5
    inline float log(float x, speed_option speed=FM_SPEED_DEFAULT){
        if(speed==ESpeedStd||speed==ESpeedNormal){
            return std::log(x);
        }
        else{//ESpeedFast1 ESpeedFast2 ESpeedFast3
            return log2(x,speed)*0.6931471805599453f; //ln2
        }    
    }

    const int32_t _bk=1024;
    const float _sin_lut[_bk+2]={
0,0.00613588,0.0122715,0.0184067,0.0245412,0.0306748,0.0368072,0.0429383,0.0490677,0.0551952,
0.0613207,0.0674439,0.0735646,0.0796824,0.0857973,0.091909,0.0980171,0.104122,0.110222,0.116319,0.122411,0.128498,0.134581,0.140658,0.14673,0.152797,0.158858,0.164913,0.170962,0.177004,0.18304,0.189069,0.19509,0.201105,0.207111,0.21311,0.219101,0.225084,0.231058,0.237024,0.24298,0.248928,0.254866,0.260794,0.266713,0.272621,0.27852,0.284408,0.290285,0.296151,0.302006,0.30785,0.313682,0.319502,0.32531,0.331106,0.33689,0.342661,0.348419,0.354164,0.359895,0.365613,0.371317,0.377007,0.382683,0.388345,0.393992,0.399624,0.405241,0.410843,0.41643,0.422,0.427555,0.433094,0.438616,0.444122,0.449611,0.455084,0.460539,0.465977,0.471397,0.476799,0.482184,0.48755,0.492898,0.498228,0.503538,0.50883,0.514103,0.519356,0.52459,0.529804,0.534998,0.540172,0.545325,0.550458,0.55557,0.560662,0.565732,0.570781,0.575808,0.580814,0.585798,0.59076,0.595699,0.600617,0.605511,0.610383,0.615232,0.620057,
0.62486,0.629638,0.634393,0.639124,0.643832,0.648514,0.653173,0.657807,0.662416,0.667,0.671559,0.676093,0.680601,0.685084,0.689541,0.693971,0.698376,0.702755,0.707107,0.711432,0.715731,0.720003,0.724247,0.728464,0.732654,0.736817,0.740951,0.745058,0.749136,0.753187,0.757209,0.761202,0.765167,0.769103,0.77301,0.776888,0.780737,0.784557,0.788346,0.792107,0.795837,0.799537,0.803208,0.806848,0.810457,0.814036,0.817585,0.821103,0.824589,0.828045,0.83147,0.834863,0.838225,0.841555,0.844854,0.84812,0.851355,0.854558,0.857729,0.860867,0.863973,0.867046,0.870087,0.873095,0.87607,0.879012,0.881921,0.884797,0.88764,0.890449,0.893224,0.895966,0.898674,0.901349,0.903989,0.906596,0.909168,0.911706,0.91421,0.916679,0.919114,0.921514,0.92388,0.92621,0.928506,0.930767,0.932993,0.935184,0.937339,0.939459,0.941544,0.943593,0.945607,0.947586,0.949528,0.951435,0.953306,0.955141,0.95694,0.958704,
0.960431,0.962121,0.963776,0.965394,0.966976,0.968522,0.970031,0.971504,0.97294,0.974339,0.975702,0.977028,0.978317,0.97957,0.980785,0.981964,0.983105,0.98421,0.985278,0.986308,0.987301,0.988258,0.989177,0.990058,0.990903,0.99171,0.99248,0.993212,0.993907,0.994565,0.995185,0.995767,0.996313,0.99682,0.99729,0.997723,0.998118,0.998476,0.998795,0.999078,0.999322,0.999529,0.999699,0.999831,0.999925,0.999981,1,0.999981,0.999925,0.999831,0.999699,0.999529,0.999322,0.999078,0.998795,0.998476,0.998118,0.997723,0.99729,0.99682,0.996313,0.995767,0.995185,0.994565,0.993907,0.993212,0.99248,0.99171,0.990903,0.990058,0.989177,0.988258,0.987301,0.986308,0.985278,0.98421,0.983105,0.981964,0.980785,0.97957,0.978317,0.977028,0.975702,0.974339,0.97294,0.971504,0.970031,0.968522,0.966976,0.965394,0.963776,0.962121,0.960431,0.958703,0.95694,0.955141,0.953306,0.951435,0.949528,0.947586,
0.945607,0.943593,0.941544,0.939459,0.937339,0.935184,0.932993,0.930767,0.928506,0.92621,0.92388,0.921514,0.919114,0.916679,0.91421,0.911706,0.909168,0.906596,0.903989,0.901349,0.898674,0.895966,0.893224,0.890449,0.88764,0.884797,0.881921,0.879012,0.87607,0.873095,0.870087,0.867046,0.863973,0.860867,0.857729,0.854558,0.851355,0.84812,0.844854,0.841555,0.838225,0.834863,0.83147,0.828045,0.824589,0.821102,0.817585,0.814036,0.810457,0.806848,0.803208,0.799537,0.795837,0.792107,0.788346,0.784557,0.780737,0.776888,0.77301,0.769103,0.765167,0.761202,0.757209,0.753187,0.749136,0.745058,0.740951,0.736816,0.732654,0.728464,0.724247,0.720003,0.715731,0.711432,0.707107,0.702755,0.698376,0.693971,0.689541,0.685084,0.680601,0.676093,0.671559,0.667,0.662416,0.657807,0.653173,0.648514,0.643832,0.639124,0.634393,0.629638,0.624859,0.620057,0.615232,0.610383,0.605511,0.600616,0.595699,0.59076,
0.585798,0.580814,0.575808,0.570781,0.565732,0.560661,0.55557,0.550458,0.545325,0.540171,0.534997,0.529803,0.52459,0.519356,0.514103,0.50883,0.503538,0.498228,0.492898,0.48755,0.482184,0.476799,0.471397,0.465976,0.460539,0.455083,0.449611,0.444122,0.438616,0.433094,0.427555,0.422,0.416429,0.410843,0.405241,0.399624,0.393992,0.388345,0.382683,0.377007,0.371317,0.365613,0.359895,0.354164,0.348419,0.342661,0.33689,0.331106,0.32531,0.319502,0.313682,0.307849,0.302006,0.296151,0.290285,0.284408,0.27852,0.272621,0.266713,0.260794,0.254866,0.248928,0.24298,0.237023,0.231058,0.225084,0.219101,0.21311,0.207111,0.201105,0.19509,0.189069,0.18304,0.177004,0.170962,0.164913,0.158858,0.152797,0.14673,0.140658,0.134581,0.128498,0.122411,0.116319,0.110222,0.104122,0.0980171,0.0919089,0.0857972,0.0796823,0.0735644,0.0674438,0.0613206,0.0551951,0.0490675,0.0429381,0.0368072,0.0306748,0.0245412,0.0184067,
0.0122715,0.00613581,-8.74228e-08,-0.00613599,-0.0122717,-0.0184069,-0.0245414,-0.030675,-0.0368074,-0.0429382,-0.0490677,-0.0551952,-0.0613208,-0.067444,-0.0735646,-0.0796825,-0.0857974,-0.0919091,-0.0980173,-0.104122,-0.110222,-0.116319,-0.122411,-0.128498,-0.134581,-0.140658,-0.14673,-0.152797,-0.158858,-0.164913,-0.170962,-0.177004,-0.18304,-0.189069,-0.19509,-0.201105,-0.207112,-0.21311,-0.219101,-0.225084,-0.231058,-0.237024,-0.24298,-0.248928,-0.254866,-0.260794,-0.266713,-0.272622,-0.27852,-0.284408,-0.290285,-0.296151,-0.302006,-0.30785,-0.313682,-0.319502,-0.32531,-0.331106,-0.33689,-0.342661,-0.348419,-0.354164,-0.359895,-0.365613,-0.371317,-0.377007,-0.382683,-0.388345,-0.393992,-0.399624,-0.405241,-0.410843,-0.41643,-0.422,-0.427555,-0.433094,-0.438616,-0.444122,-0.449612,-0.455084,-0.460539,-0.465977,-0.471397,-0.476799,-0.482184,-0.48755,-0.492898,-0.498228,-0.503538,-0.50883,-0.514103,-0.519356,-0.52459,-0.529804,-0.534998,-0.540172,-0.545325,-0.550458,-0.55557,-0.560662,
-0.565732,-0.570781,-0.575808,-0.580814,-0.585798,-0.59076,-0.595699,-0.600617,-0.605511,-0.610383,-0.615232,-0.620057,-0.62486,-0.629638,-0.634393,-0.639125,-0.643832,-0.648515,-0.653173,-0.657807,-0.662416,-0.667,-0.671559,-0.676093,-0.680601,-0.685084,-0.689541,-0.693972,-0.698376,-0.702755,-0.707107,-0.711432,-0.715731,-0.720003,-0.724247,-0.728464,-0.732654,-0.736817,-0.740951,-0.745058,-0.749136,-0.753187,-0.757209,-0.761202,-0.765167,-0.769103,-0.77301,-0.776888,-0.780737,-0.784557,-0.788346,-0.792107,-0.795837,-0.799537,-0.803208,-0.806848,-0.810457,-0.814036,-0.817585,-0.821103,-0.824589,-0.828045,-0.83147,-0.834863,-0.838225,-0.841555,-0.844854,-0.848121,-0.851355,-0.854558,-0.857729,-0.860867,-0.863973,-0.867046,-0.870087,-0.873095,-0.87607,-0.879012,-0.881921,-0.884797,-0.88764,-0.890449,-0.893224,-0.895966,-0.898675,-0.901349,-0.903989,-0.906596,-0.909168,-0.911706,-0.91421,-0.916679,-0.919114,-0.921514,-0.92388,-0.92621,-0.928506,-0.930767,-0.932993,-0.935183,
-0.937339,-0.939459,-0.941544,-0.943593,-0.945607,-0.947586,-0.949528,-0.951435,-0.953306,-0.955141,-0.95694,-0.958704,-0.960431,-0.962121,-0.963776,-0.965394,-0.966977,-0.968522,-0.970031,-0.971504,-0.97294,-0.974339,-0.975702,-0.977028,-0.978317,-0.97957,-0.980785,-0.981964,-0.983105,-0.98421,-0.985278,-0.986308,-0.987301,-0.988258,-0.989177,-0.990058,-0.990903,-0.99171,-0.99248,-0.993212,-0.993907,-0.994565,-0.995185,-0.995767,-0.996313,-0.99682,-0.99729,-0.997723,-0.998118,-0.998476,-0.998795,-0.999078,-0.999322,-0.999529,-0.999699,-0.999831,-0.999925,-0.999981,-1,-0.999981,-0.999925,-0.999831,-0.999699,-0.999529,-0.999322,-0.999078,-0.998795,-0.998476,-0.998118,-0.997723,-0.99729,-0.99682,-0.996313,-0.995767,-0.995185,-0.994565,-0.993907,-0.993212,-0.99248,-0.99171,-0.990903,-0.990058,-0.989177,-0.988258,-0.987301,-0.986308,-0.985278,-0.98421,-0.983105,-0.981964,-0.980785,-0.97957,-0.978317,-0.977028,-0.975702,-0.974339,-0.97294,-0.971504,-0.970031,-0.968522,
-0.966976,-0.965394,-0.963776,-0.962121,-0.96043,-0.958703,-0.95694,-0.955141,-0.953306,-0.951435,-0.949528,-0.947586,-0.945607,-0.943593,-0.941544,-0.939459,-0.937339,-0.935183,-0.932993,-0.930767,-0.928506,-0.92621,-0.923879,-0.921514,-0.919114,-0.916679,-0.91421,-0.911706,-0.909168,-0.906596,-0.903989,-0.901349,-0.898674,-0.895966,-0.893224,-0.890449,-0.88764,-0.884797,-0.881921,-0.879012,-0.87607,-0.873095,-0.870087,-0.867046,-0.863973,-0.860867,-0.857729,-0.854558,-0.851355,-0.84812,-0.844853,-0.841555,-0.838225,-0.834863,-0.831469,-0.828045,-0.824589,-0.821102,-0.817585,-0.814036,-0.810457,-0.806847,-0.803208,-0.799537,-0.795837,-0.792107,-0.788346,-0.784557,-0.780737,-0.776888,-0.77301,-0.769103,-0.765167,-0.761202,-0.757209,-0.753187,-0.749136,-0.745058,-0.740951,-0.736816,-0.732654,-0.728464,-0.724247,-0.720002,-0.715731,-0.711432,-0.707107,-0.702754,-0.698376,-0.693971,-0.689541,-0.685084,-0.680601,-0.676093,-0.671559,-0.667,-0.662416,-0.657807,-0.653173,-0.648514,
-0.643831,-0.639124,-0.634393,-0.629638,-0.624859,-0.620057,-0.615231,-0.610383,-0.605511,-0.600616,-0.595699,-0.590759,-0.585798,-0.580814,-0.575808,-0.57078,-0.565732,-0.560661,-0.55557,-0.550458,-0.545325,-0.540172,-0.534998,-0.529804,-0.52459,-0.519356,-0.514103,-0.50883,-0.503538,-0.498228,-0.492898,-0.48755,-0.482184,-0.476799,-0.471397,-0.465976,-0.460538,-0.455083,-0.449611,-0.444122,-0.438616,-0.433094,-0.427555,-0.422,-0.416429,-0.410843,-0.405241,-0.399624,-0.393992,-0.388345,-0.382683,-0.377007,-0.371317,-0.365613,-0.359895,-0.354163,-0.348419,-0.342661,-0.33689,-0.331106,-0.32531,-0.319502,-0.313682,-0.307849,-0.302006,-0.296151,-0.290284,-0.284407,-0.278519,-0.272621,-0.266712,-0.260794,-0.254865,-0.248928,-0.24298,-0.237024,-0.231058,-0.225084,-0.219101,-0.21311,-0.207111,-0.201105,-0.19509,-0.189069,-0.18304,-0.177004,-0.170962,-0.164913,-0.158858,-0.152797,-0.14673,-0.140658,-0.13458,-0.128498,-0.12241,-0.116318,-0.110222,-0.104121,-0.0980168,-0.0919086,
-0.0857969,-0.0796825,-0.0735646,-0.0674439,-0.0613207,-0.0551952,-0.0490676,-0.0429382,-0.0368072,-0.0306747,-0.0245411,-0.0184066,-0.0122714,-0.00613573,1.74846e-07,0        
    };

    // fast1/fast2/fast3 用时少75%，误差不超过6e-6
    inline float sin(float x, speed_option speed=FM_SPEED_DEFAULT){        
        if(speed==ESpeedStd||speed==ESpeedNormal){
            return std::sin(x);
        }
        else{//ESpeedFast1 ESpeedFast2 ESpeedFast3       

            //fmod比sin慢10倍多，不可能使用，好在一般sin不会有奇怪过大输入
            if(abs(x)>(1e9f/_bk)) {
                // x=std::fmod(x,tpi_f);
                return std::sin(x);
            }

            //查表法
            x = x * (_bk/(tpi_f));
            int32_t id = (int32_t)x;
            if(x<0) id--;
            x -= id;
            id = ((id & 1023) + 1024) & 1023;
            return (1-x)*_sin_lut[id]+x*_sin_lut[id+1];

        }       
    }

    // fast1/fast2/fast3  用时少75%，误差不超过6e-6
    inline float cos(float x, speed_option speed=FM_SPEED_DEFAULT){
        if(speed==ESpeedStd||speed==ESpeedNormal){
            return std::cos(x);
        }
        else{ //ESpeedFast1 ESpeedFast2 ESpeedFast3
            return sin(x+hpi_f,speed);
        }
    }

    // fast1/fast2/fast3 用时少65%，误差不超过2e-5(除奇异点附近的极端值外)
    inline float tan(float x, speed_option speed=FM_SPEED_DEFAULT){
        if(speed==ESpeedStd||speed==ESpeedNormal){
            return std::tan(x);
        }
        else{  //ESpeedFast1 ESpeedFast2 ESpeedFast3
            return sin(x,speed)/sin(x+hpi_f,speed);
        }
    }

    const float _asin_lut[_bk+2]={
-hpi_f,-1.50829,-1.48238,-1.46249,-1.44571,-1.43093,-1.41755,-1.40525,-1.39379,-1.38302,
-1.37283,-1.36313,-1.35386,-1.34497,-1.33641,-1.32814,-1.32014,-1.31238,-1.30485,-1.29752,-1.29037,-1.2834,-1.27659,-1.26992,-1.2634,-1.25701,-1.25074,-1.24459,-1.23855,-1.23261,-1.22678,-1.22103,-1.21538,-1.2098,-1.20431,-1.1989,-1.19356,-1.1883,-1.1831,-1.17796,-1.17289,-1.16788,-1.16293,-1.15803,-1.15319,-1.1484,-1.14366,-1.13897,-1.13433,-1.12973,-1.12518,-1.12067,-1.1162,-1.11177,-1.10738,-1.10303,-1.09872,-1.09444,-1.0902,-1.08599,-1.08182,-1.07767,-1.07356,-1.06948,-1.06544,-1.06142,-1.05743,-1.05346,-1.04953,-1.04562,-1.04174,-1.03788,-1.03405,-1.03024,-1.02646,-1.0227,-1.01896,-1.01524,-1.01155,-1.00788,-1.00423,-1.0006,-0.996995,-0.993407,-0.989839,-0.98629,-0.982759,-0.979248,-0.975754,-0.972279,-0.968821,-0.965381,-0.961957,-0.95855,-0.95516,-0.951786,-0.948428,-0.945085,-0.941758,-0.938446,-0.935149,-0.931866,-0.928598,-0.925345,-0.922105,-0.918879,-0.915666,-0.912467,-0.909281,-0.906108,
-0.902948,-0.899801,-0.896666,-0.893543,-0.890432,-0.887333,-0.884246,-0.881171,-0.878107,-0.875054,-0.872012,-0.868982,-0.865962,-0.862953,-0.859954,-0.856966,-0.853988,-0.85102,-0.848062,-0.845114,-0.842176,-0.839247,-0.836328,-0.833419,-0.830519,-0.827627,-0.824745,-0.821872,-0.819008,-0.816153,-0.813306,-0.810467,-0.807638,-0.804816,-0.802003,-0.799198,-0.796401,-0.793612,-0.79083,-0.788057,-0.785291,-0.782533,-0.779783,-0.77704,-0.774304,-0.771576,-0.768855,-0.766141,-0.763434,-0.760734,-0.758041,-0.755355,-0.752675,-0.750003,-0.747337,-0.744677,-0.742024,-0.739378,-0.736737,-0.734104,-0.731476,-0.728855,-0.726239,-0.72363,-0.721027,-0.718429,-0.715838,-0.713252,-0.710673,-0.708099,-0.70553,-0.702967,-0.70041,-0.697858,-0.695312,-0.692771,-0.690235,-0.687705,-0.68518,-0.68266,-0.680146,-0.677636,-0.675132,-0.672632,-0.670138,-0.667648,-0.665163,-0.662683,-0.660208,-0.657738,-0.655272,-0.652811,-0.650355,-0.647903,-0.645456,-0.643013,-0.640575,-0.638141,-0.635711,-0.633286,
-0.630865,-0.628449,-0.626036,-0.623628,-0.621224,-0.618825,-0.616429,-0.614037,-0.61165,-0.609266,-0.606886,-0.60451,-0.602139,-0.599771,-0.597406,-0.595046,-0.592689,-0.590337,-0.587987,-0.585642,-0.5833,-0.580962,-0.578627,-0.576296,-0.573968,-0.571644,-0.569324,-0.567006,-0.564693,-0.562382,-0.560075,-0.557772,-0.555471,-0.553174,-0.55088,-0.54859,-0.546302,-0.544018,-0.541737,-0.539459,-0.537184,-0.534912,-0.532644,-0.530378,-0.528115,-0.525856,-0.523599,-0.521345,-0.519094,-0.516846,-0.514601,-0.512359,-0.510119,-0.507883,-0.505649,-0.503417,-0.501189,-0.498963,-0.49674,-0.49452,-0.492303,-0.490088,-0.487875,-0.485665,-0.483458,-0.481253,-0.479051,-0.476852,-0.474655,-0.47246,-0.470268,-0.468078,-0.465891,-0.463706,-0.461523,-0.459343,-0.457165,-0.45499,-0.452817,-0.450646,-0.448477,-0.446311,-0.444147,-0.441985,-0.439825,-0.437668,-0.435512,-0.433359,-0.431208,-0.429059,-0.426913,-0.424768,-0.422625,-0.420485,-0.418346,-0.41621,-0.414076,-0.411943,-0.409813,-0.407684,
-0.405558,-0.403433,-0.40131,-0.39919,-0.397071,-0.394954,-0.392839,-0.390726,-0.388614,-0.386505,-0.384397,-0.382291,-0.380187,-0.378084,-0.375984,-0.373885,-0.371787,-0.369692,-0.367598,-0.365506,-0.363416,-0.361327,-0.35924,-0.357154,-0.35507,-0.352988,-0.350907,-0.348828,-0.346751,-0.344675,-0.3426,-0.340528,-0.338456,-0.336386,-0.334318,-0.332251,-0.330186,-0.328122,-0.326059,-0.323998,-0.321939,-0.31988,-0.317824,-0.315768,-0.313714,-0.311662,-0.30961,-0.30756,-0.305512,-0.303464,-0.301418,-0.299374,-0.29733,-0.295288,-0.293247,-0.291208,-0.289169,-0.287132,-0.285096,-0.283062,-0.281028,-0.278996,-0.276965,-0.274935,-0.272906,-0.270879,-0.268852,-0.266827,-0.264803,-0.262779,-0.260757,-0.258737,-0.256717,-0.254698,-0.25268,-0.250664,-0.248648,-0.246633,-0.24462,-0.242607,-0.240596,-0.238585,-0.236576,-0.234567,-0.232559,-0.230553,-0.228547,-0.226542,-0.224538,-0.222535,-0.220533,-0.218532,-0.216532,-0.214532,-0.212534,-0.210536,-0.208539,-0.206544,-0.204548,-0.202554,
-0.200561,-0.198568,-0.196576,-0.194585,-0.192595,-0.190605,-0.188616,-0.186628,-0.184641,-0.182655,-0.180669,-0.178684,-0.176699,-0.174716,-0.172733,-0.17075,-0.168769,-0.166788,-0.164808,-0.162828,-0.160849,-0.158871,-0.156893,-0.154916,-0.152939,-0.150963,-0.148988,-0.147013,-0.145039,-0.143066,-0.141093,-0.13912,-0.137148,-0.135177,-0.133206,-0.131236,-0.129266,-0.127297,-0.125328,-0.12336,-0.121392,-0.119424,-0.117457,-0.115491,-0.113525,-0.111559,-0.109594,-0.10763,-0.105665,-0.103701,-0.101738,-0.0997748,-0.0978121,-0.0958498,-0.0938879,-0.0919263,-0.0899651,-0.0880042,-0.0860436,-0.0840834,-0.0821235,-0.080164,-0.0782047,-0.0762457,-0.0742871,-0.0723287,-0.0703706,-0.0684127,-0.0664552,-0.0644978,-0.0625408,-0.0605839,-0.0586273,-0.056671,-0.0547148,-0.0527588,-0.0508031,-0.0488475,-0.0468922,-0.044937,-0.042982,-0.0410271,-0.0390724,-0.0371179,-0.0351635,-0.0332092,-0.0312551,-0.0293011,-0.0273472,-0.0253934,-0.0234396,-0.021486,-0.0195325,-0.017579,-0.0156256,-0.0136723,-0.011719,-0.00976578,-0.00781258,-0.00585941,
-0.00390626,-0.00195313,0,0.00195313,0.00390626,0.00585941,0.00781258,0.00976578,0.011719,0.0136723,0.0156256,0.017579,0.0195325,0.021486,0.0234396,0.0253934,0.0273472,0.0293011,0.0312551,0.0332092,0.0351635,0.0371179,0.0390724,0.0410271,0.042982,0.044937,0.0468922,0.0488475,0.0508031,0.0527588,0.0547148,0.056671,0.0586273,0.0605839,0.0625408,0.0644978,0.0664552,0.0684127,0.0703706,0.0723287,0.0742871,0.0762457,0.0782047,0.080164,0.0821235,0.0840834,0.0860436,0.0880042,0.0899651,0.0919263,0.0938879,0.0958498,0.0978121,0.0997748,0.101738,0.103701,0.105665,0.10763,0.109594,0.111559,0.113525,0.115491,0.117457,0.119424,0.121392,0.12336,0.125328,0.127297,0.129266,0.131236,0.133206,0.135177,0.137148,0.13912,0.141093,0.143066,0.145039,0.147013,0.148988,0.150963,0.152939,0.154916,0.156893,0.158871,0.160849,0.162828,0.164808,0.166788,0.168769,0.17075,0.172733,0.174716,0.176699,0.178684,0.180669,0.182655,0.184641,0.186628,0.188616,0.190605,
0.192595,0.194585,0.196576,0.198568,0.200561,0.202554,0.204548,0.206544,0.208539,0.210536,0.212534,0.214532,0.216532,0.218532,0.220533,0.222535,0.224538,0.226542,0.228547,0.230553,0.232559,0.234567,0.236576,0.238585,0.240596,0.242607,0.24462,0.246633,0.248648,0.250664,0.25268,0.254698,0.256717,0.258737,0.260757,0.262779,0.264803,0.266827,0.268852,0.270879,0.272906,0.274935,0.276965,0.278996,0.281028,0.283062,0.285096,0.287132,0.289169,0.291208,0.293247,0.295288,0.29733,0.299374,0.301418,0.303464,0.305512,0.30756,0.30961,0.311662,0.313714,0.315768,0.317824,0.31988,0.321939,0.323998,0.326059,0.328122,0.330186,0.332251,0.334318,0.336386,0.338456,0.340528,0.3426,0.344675,0.346751,0.348828,0.350907,0.352988,0.35507,0.357154,0.35924,0.361327,0.363416,0.365506,0.367598,0.369692,0.371787,0.373885,0.375984,0.378084,0.380187,0.382291,0.384397,0.386505,0.388614,0.390726,0.392839,0.394954,
0.397071,0.39919,0.40131,0.403433,0.405558,0.407684,0.409813,0.411943,0.414076,0.41621,0.418346,0.420485,0.422625,0.424768,0.426913,0.429059,0.431208,0.433359,0.435512,0.437668,0.439825,0.441985,0.444147,0.446311,0.448477,0.450646,0.452817,0.45499,0.457165,0.459343,0.461523,0.463706,0.465891,0.468078,0.470268,0.47246,0.474655,0.476852,0.479051,0.481253,0.483458,0.485665,0.487875,0.490088,0.492303,0.49452,0.49674,0.498963,0.501189,0.503417,0.505649,0.507883,0.510119,0.512359,0.514601,0.516846,0.519094,0.521345,0.523599,0.525856,0.528115,0.530378,0.532644,0.534912,0.537184,0.539459,0.541737,0.544018,0.546302,0.54859,0.55088,0.553174,0.555471,0.557772,0.560075,0.562382,0.564693,0.567006,0.569324,0.571644,0.573968,0.576296,0.578627,0.580962,0.5833,0.585642,0.587987,0.590337,0.592689,0.595046,0.597406,0.599771,0.602139,0.60451,0.606886,0.609266,0.61165,0.614037,0.616429,0.618825,
0.621224,0.623628,0.626036,0.628449,0.630865,0.633286,0.635711,0.638141,0.640575,0.643013,0.645456,0.647903,0.650355,0.652811,0.655272,0.657738,0.660208,0.662683,0.665163,0.667648,0.670138,0.672632,0.675132,0.677636,0.680146,0.68266,0.68518,0.687705,0.690235,0.692771,0.695312,0.697858,0.70041,0.702967,0.70553,0.708099,0.710673,0.713252,0.715838,0.718429,0.721027,0.72363,0.726239,0.728855,0.731476,0.734104,0.736737,0.739378,0.742024,0.744677,0.747337,0.750003,0.752675,0.755355,0.758041,0.760734,0.763434,0.766141,0.768855,0.771576,0.774304,0.77704,0.779783,0.782533,0.785291,0.788057,0.79083,0.793612,0.796401,0.799198,0.802003,0.804816,0.807638,0.810467,0.813306,0.816153,0.819008,0.821872,0.824745,0.827627,0.830519,0.833419,0.836328,0.839247,0.842176,0.845114,0.848062,0.85102,0.853988,0.856966,0.859954,0.862953,0.865962,0.868982,0.872012,0.875054,0.878107,0.881171,0.884246,0.887333,
0.890432,0.893543,0.896666,0.899801,0.902948,0.906108,0.909281,0.912467,0.915666,0.918879,0.922105,0.925345,0.928598,0.931866,0.935149,0.938446,0.941758,0.945085,0.948428,0.951786,0.95516,0.95855,0.961957,0.965381,0.968821,0.972279,0.975754,0.979248,0.982759,0.98629,0.989839,0.993407,0.996995,1.0006,1.00423,1.00788,1.01155,1.01524,1.01896,1.0227,1.02646,1.03024,1.03405,1.03788,1.04174,1.04562,1.04953,1.05346,1.05743,1.06142,1.06544,1.06948,1.07356,1.07767,1.08182,1.08599,1.0902,1.09444,1.09872,1.10303,1.10738,1.11177,1.1162,1.12067,1.12518,1.12973,1.13433,1.13897,1.14366,1.1484,1.15319,1.15803,1.16293,1.16788,1.17289,1.17796,1.1831,1.1883,1.19356,1.1989,1.20431,1.2098,1.21538,1.22103,1.22678,1.23261,1.23855,1.24459,1.25074,1.25701,1.2634,1.26992,1.27659,1.2834,1.29037,1.29752,1.30485,1.31238,1.32014,1.32814,
1.33641,1.34497,1.35386,1.36313,1.37283,1.38302,1.39379,1.40525,1.41755,1.43093,1.44571,1.46249,1.48238,1.50829,hpi_f,hpi_f        
    };

    // fast1/fast2 用时少[0%,75%]，误差不超过1e-4
    // fast3  用时少80%，误差不超过1e-2 (若输入在[-0.99,0.99]内时可达1e-4)
    inline float asin(float x, speed_option speed=FM_SPEED_DEFAULT){
        if(speed==ESpeedStd||speed==ESpeedNormal){
            return std::asin(x);
        }
        else if(speed==ESpeedFast1||speed==ESpeedFast2){
            if(abs(x)>1) return std::numeric_limits<float>::quiet_NaN();
            //保证fast1 fast2的精度要求
            if(abs(x)>0.99) return std::asin(x);

            //查表法
            x = (x+1) * (_bk/2);
            int32_t id = (int32_t)x;
            x -= id;
            return (1-x)*_asin_lut[id]+x*_asin_lut[id+1];   
        }
        else{ //EspeedFast3
            if(abs(x)>1) return std::numeric_limits<float>::quiet_NaN();

            //查表法
            x = (x+1) * (_bk/2);
            int32_t id = (int32_t)x;
            x -= id;
            return (1-x)*_asin_lut[id]+x*_asin_lut[id+1];            
        }
    }

    // fast1/fast2 用时少[0%,81%]，误差不超过1e-4
    // fast3 用时少85%，误差不超过1e-2 (若输入在[-0.99,0.99]内时可达1e-4)
    inline float acos(float x, speed_option speed=FM_SPEED_DEFAULT){
        if(speed==ESpeedStd||speed==ESpeedNormal){
            return std::acos(x);
        }
        else if(speed==ESpeedFast1||speed==ESpeedFast2){
            if(abs(x)>1) return std::numeric_limits<float>::quiet_NaN();
            //保证fast1 fast2的精度要求
            if(abs(x)>0.99) return std::acos(x);

            //查表法
            x = (x+1) * (_bk/2);
            int32_t id = (int32_t)x;
            x -= id;
            float asi = (1-x)*_asin_lut[id]+x*_asin_lut[id+1];
            return hpi_f - asi;              
        }
        else{ //EspeedFast3
            if(abs(x)>1) return std::numeric_limits<float>::quiet_NaN();

            //查表法
            x = (x+1) * (_bk/2);
            int32_t id = (int32_t)x;
            x -= id;
            float asi = (1-x)*_asin_lut[id]+x*_asin_lut[id+1];
            return hpi_f - asi;            
        }
    }

    inline float _Fast2ArcTan(const float x) {
        float xx = x * x;
        return ((0.0776509570923569f*xx -0.287434475393028f)*xx + ((pi_f/4.0f) -0.0776509570923569f +0.287434475393028))*x;
    }

    // fast2/fast3 用时少20%，误差不超过1e-3
    inline float atan(float x, speed_option speed=FM_SPEED_DEFAULT){
        if(speed==ESpeedStd||speed==ESpeedNormal||speed==ESpeedFast1){
            return std::atan(x);
        }
        else{ // EspeedFast2 EspeedFast3
            if(abs(x)<=1){
                return _Fast2ArcTan(x);
            }
            else{
                x=1/x;
                float t=_Fast2ArcTan(x);
                return ((x>0)-0.5f)*pi_f - t;
            }
        }
    }
    
    // fast2/fast3 用时少48%，误差不超过2e-4
    inline float atan2(float y,float x, speed_option speed=FM_SPEED_DEFAULT){
        if(speed==ESpeedStd||speed==ESpeedNormal||speed==ESpeedFast1){
            return std::atan2(y,x);
        }
        else{ // EspeedFast2 EspeedFast3
            float a = std::min (abs(x), abs(y)) / std::max (abs(x), abs(y));
            float s = a * a;
            float r = ((-0.0464964749f * s + 0.15931422f) * s - 0.327622764f) * s * a + a;
            if (abs(y) > abs(x)) r = hpi_f - r;
            if (x < 0) r = pi_f - r;
            if (y < 0) r = -r;
            return r;
        }
    }

    // fast1/fast2/fast3 用时少91%（10+倍速），随机检测的超1e8个测试case均0误差
    // 但是在极端情况下（fmod(x,y)极为接近0时，有可能因为精度而得到相差除数y的结果）
    // 无inf，nan的适配
    template <typename T>
    inline T fmod(T x,T y, speed_option speed=FM_SPEED_DEFAULT){
        if(speed==ESpeedStd||speed==ESpeedNormal){
            return std::fmod(x,y);
        }
        else{ //ESpeedFast1 ESpeedFast2 ESpeedFast3
            T t=(T)((double)x - std::trunc((double)x/y)*(double)y);
            return t;
        }
    }

    //经过测试，能够找到的快于标准库的实现精度太差（这些实现可见于DiscardedImpl.h）
    template <typename T>
    inline T pow(float x,float y, speed_option speed=FM_SPEED_DEFAULT){
        return std::pow(x,y);
    }

    // 废弃，因为max,min是algrothm中而非cmath中的内容，也没法优化。
    // using std::max; 
    // using std::min;





}//namespace fm