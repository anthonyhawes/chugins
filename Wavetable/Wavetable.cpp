//-----------------------------------------------------------------------------
// Entaro ChucK Developer!
// This is a Chugin boilerplate, generated by chugerate!
//-----------------------------------------------------------------------------

// this should align with the correct versions of these ChucK files
#include "chugin.h"

// general includes
#include <stdio.h>
#include <limits.h>
#include <math.h>

#define DEFAULT_TABLE_SIZE 2048
#define DEFAULT_FREQ 220

// declaration of chugin constructor
CK_DLL_CTOR(wavetable_ctor);
// declaration of chugin desctructor
CK_DLL_DTOR(wavetable_dtor);

// example of getter/setter
CK_DLL_MFUN(wavetable_setFreq);
CK_DLL_MFUN(wavetable_setInterpolate);
CK_DLL_MFUN(wavetable_setSync);

CK_DLL_MFUN(wavetable_getFreq);
CK_DLL_MFUN(wavetable_getInterpolate);
CK_DLL_MFUN(wavetable_getSync);

CK_DLL_MFUN(wavetable_setTable);


// for Chugins extending UGen, this is mono synthesis function for 1 sample
CK_DLL_TICK(wavetable_tick);

// this is a special offset reserved for Chugin internal data
t_CKINT wavetable_data_offset = 0;

// class definition for internal Chugin data
// (note: this isn't strictly necessary, but serves as example
// of one recommended approach)
class Wavetable
{
public:
// constructor
Wavetable( t_CKFLOAT fs)
{
        srate = fs;
        table_pos = 0;
        internal_table = new double[DEFAULT_TABLE_SIZE];
        table_size = DEFAULT_TABLE_SIZE;
        make_default_table();
        freq = DEFAULT_FREQ;
        step = table_size * freq / srate;
        current_table = internal_table;
        interpolate = 0;
        sync = 0;
        //printf("step: %f\n", step);
}

~Wavetable() // (ge) added 1.5.2.0
{
    // see if current_table is pointing to internal
    bool same = current_table == internal_table;
    // clean up internal
    CK_SAFE_DELETE_ARRAY( internal_table );
    // if not same, clean up current
    if( !same ) CK_SAFE_DELETE_ARRAY( current_table );
}

// for Chugins extending UGen
SAMPLE tick ( SAMPLE in )
{
        // default: this passes whatever input is patched into Chugin
        if (sync == 0)
        {
        if (in > 0)
        {
          freq = in;
          step = table_size * freq / srate;
			}
        table_pos += step;
			}
			else if (sync == 1)
			table_pos = table_size * in;
        while (table_pos >= table_size) table_pos -= table_size;

        int y0, y1, y2, y3;
        y0 = (int) table_pos;
        y1 = (y0 + 1) % table_size;
        y2 = (y0 + 2) % table_size;
        y3 = (y0 + 3) % table_size;

        if (interpolate==1)
        {
                return LinearInterpolate(current_table[y0], current_table[y1],
                                          table_pos - y0);
        }
        else if (interpolate==2)
        {
                return LagrangeInterpolate(current_table[y0],
                                          current_table[y1], current_table[y2], current_table[y3],
                                          table_pos - y0);
        }
        else if (interpolate==3)
        {
                return CubicInterpolate(current_table[y0],
                                          current_table[y1], current_table[y2], current_table[y3],
                                          table_pos - y0);
        }
        else if (interpolate==4)
        {
                return HermiteInterpolate(current_table[y0],
                                          current_table[y1], current_table[y2], current_table[y3],
                                          table_pos - y0);
        }
        return current_table[y0];
}

// set parameter example
float setFreq( t_CKFLOAT p )
{
        freq = p;
        step = table_size * freq / srate;
        return freq;
}

float getFreq()
{
        return freq;
}

int setInterpolate (t_CKINT p)
{
        if (p > 4 || p < 0) p = 0;
        interpolate = p;
        return p;
}

int getInterpolate()
{
        return interpolate;
}

int setSync ( t_CKINT p)
{
  sync = p;
  // unsigned int never < 0
  // if (sync < 0) sync = 0;
  if (sync > 1) sync = 0;
  return sync;
}

int getSync ()
{
  return sync;
}

void setTable( Chuck_ArrayFloat * ckarray, CK_DL_API api )
{
    // clean up first
    CK_SAFE_DELETE_ARRAY( current_table );
    // get size
    table_size = api->object->array_float_size( ckarray );
    // allocate
    current_table = new t_CKFLOAT[table_size];
    // copy
    for( t_CKINT i = 0; i < table_size; i++ )
    {
        // get element using portable API and set
        current_table[i] = api->object->array_float_get_idx( ckarray, i );
    }
    // compute step
    step = table_size * freq / srate;
  /*
  printf("size of userArray: %d\n", table_size);
  for (int i=0; i<table_size; i++)
  {
    printf("i: %d, val: %f\n", i, current_table[i]);
  }*/
}

private:
// instance data
double table_pos;
double* internal_table;
double* current_table;
float freq;
double step;
unsigned int table_size, sync;
int srate;
unsigned int interpolate;

void make_default_table()
{
        for (int i=0; i<table_size; i++)
        {
                internal_table[i] = sin( CK_TWO_PI * i / table_size );
        }
}

/*double LinearInterpolate(double y0, double y1, double mu)
{
	return y1 + mu*(y1-y0);
}*/

double LinearInterpolate(double y1, double y2, double mu)
{
   return(y1*(1-mu)+y2*mu);
}

double CubicInterpolate(double y0, double y1, double y2, double y3, double mu)
{
        double a0,a1,a2,a3,mu2;
        mu2 = mu*mu;
        a0 = -0.5*y0 + 1.5*y1 - 1.5*y2 + 0.5*y3;
        a1 = y0 - 2.5*y1 + 2*y2 - 0.5*y3;
        a2 = -0.5*y0 + 0.5*y2;
        a3 = y1;

        return (a0*mu*mu2+a1*mu2+a2*mu+a3);
}

double LagrangeInterpolate(double y0, double y1, double y2, double y3, double mu)
{
        return (y1 + mu * (
            (y2 - y1) - 0.1666667f * (1.-mu) * (
                (y3 - y0 - 3.0f * (y2 - y1)) * mu + (y3 + 2.0f*y0 - 3.0f*y1))));
}

double HermiteInterpolate(double y0,double y1,
                         double y2,double y3,
                         double mu)
{
        double m0,m1,mu2,mu3;
        double a0,a1,a2,a3;

        mu2 = mu * mu;
        mu3 = mu2 * mu;
        m0  = (y1-y0)/2;
        m0 += (y2-y1)/2;
        m1  = (y2-y1)/2;
        m1 += (y3-y2)/2;
        a0 =  2*mu3 - 3*mu2 + 1;
        a1 =    mu3 - 2*mu2 + mu;
        a2 =    mu3 -   mu2;
        a3 = -2*mu3 + 3*mu2;

        return(a0*y1+a1*m0+a2*m1+a3*y2);
}
};


// query function: chuck calls this when loading the Chugin
// NOTE: developer will need to modify this function to
// add additional functions to this Chugin
CK_DLL_QUERY( Wavetable )
{
        QUERY->setname(QUERY, "Wavetable");
        QUERY->begin_class(QUERY, "Wavetable", "UGen");
        QUERY->add_ctor(QUERY, wavetable_ctor);
        QUERY->add_dtor(QUERY, wavetable_dtor);
        QUERY->add_ugen_func(QUERY, wavetable_tick, NULL, 1, 1);

        // set methods
        QUERY->add_mfun(QUERY, wavetable_setFreq, "float", "freq");
        QUERY->add_arg(QUERY, "float", "arg");

        QUERY->add_mfun(QUERY, wavetable_setSync, "int", "sync");
        QUERY->add_arg(QUERY, "int", "arg");

        QUERY->add_mfun(QUERY, wavetable_setInterpolate, "int", "interpolate");
        QUERY->add_arg(QUERY, "int", "arg");

        QUERY->add_mfun(QUERY, wavetable_setTable, "void", "setTable");
        QUERY->add_arg(QUERY, "float[]", "table");

        // get methods
        QUERY->add_mfun(QUERY, wavetable_getFreq, "float", "freq");
        QUERY->add_mfun(QUERY, wavetable_getSync, "int", "sync");
        QUERY->add_mfun(QUERY, wavetable_getInterpolate, "int", "interpolate");

        wavetable_data_offset = QUERY->add_mvar(QUERY, "int", "@w_data", false);
        QUERY->end_class(QUERY);
        return TRUE;
}

CK_DLL_CTOR(wavetable_ctor)
{
        OBJ_MEMBER_INT(SELF, wavetable_data_offset) = 0;
        Wavetable * w_obj = new Wavetable(API->vm->srate(VM));
        OBJ_MEMBER_INT(SELF, wavetable_data_offset) = (t_CKINT) w_obj;
}
CK_DLL_DTOR(wavetable_dtor)
{
        Wavetable * w_obj = (Wavetable *) OBJ_MEMBER_INT(SELF, wavetable_data_offset);
        if( w_obj )
        {
                // clean up
                delete w_obj;
                OBJ_MEMBER_INT(SELF, wavetable_data_offset) = 0;
                w_obj = NULL;
        }
}

CK_DLL_TICK(wavetable_tick)
{
        Wavetable * w_obj = (Wavetable *) OBJ_MEMBER_INT(SELF, wavetable_data_offset);
        if(w_obj) *out = w_obj->tick(in);
        return TRUE;
}

CK_DLL_MFUN(wavetable_setFreq)
{
        Wavetable * w_obj = (Wavetable *) OBJ_MEMBER_INT(SELF, wavetable_data_offset);
        RETURN->v_float = w_obj->setFreq(GET_NEXT_FLOAT(ARGS));
}

CK_DLL_MFUN(wavetable_getFreq)
{
        Wavetable * w_obj = (Wavetable *) OBJ_MEMBER_INT(SELF, wavetable_data_offset);
        RETURN->v_float = w_obj->getFreq();
}

CK_DLL_MFUN(wavetable_setInterpolate)
{
        Wavetable * w_obj = (Wavetable *) OBJ_MEMBER_INT(SELF, wavetable_data_offset);
        RETURN->v_int = w_obj->setInterpolate(GET_NEXT_INT(ARGS));
}

CK_DLL_MFUN(wavetable_getInterpolate)
{
        Wavetable * w_obj = (Wavetable *) OBJ_MEMBER_INT(SELF, wavetable_data_offset);
        RETURN->v_int = w_obj->getInterpolate();
}

CK_DLL_MFUN(wavetable_setSync)
{
        Wavetable * w_obj = (Wavetable *) OBJ_MEMBER_INT(SELF, wavetable_data_offset);
        RETURN->v_int = w_obj->setSync(GET_NEXT_INT(ARGS));
}

CK_DLL_MFUN(wavetable_getSync)
{
        Wavetable * w_obj = (Wavetable *) OBJ_MEMBER_INT(SELF, wavetable_data_offset);
        RETURN->v_int = w_obj->getSync();
}

CK_DLL_MFUN(wavetable_setTable)
{
        Wavetable * w_obj = (Wavetable *) OBJ_MEMBER_INT(SELF, wavetable_data_offset);
        w_obj->setTable((Chuck_ArrayFloat *)GET_NEXT_OBJECT(ARGS), API);
}
