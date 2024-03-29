#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <math.h>
#include <lv2.h>

#include "VoltageControlledFilter12dB.h"

#include "lv2/lv2plug.in/ns/ext/atom/atom.h"
#include "lv2/lv2plug.in/ns/ext/atom/util.h"
#include "lv2/lv2plug.in/ns/ext/midi/midi.h"
#include "lv2/lv2plug.in/ns/ext/urid/urid.h"
#include "lv2/lv2plug.in/ns/lv2core/lv2.h"

/**********************************************************************************************************************************************************/
#define PLUGIN_URI "http://VeJaPlugins.com/plugins/Release/s6Filter"

#define MAX_PORTS 8
#define MAX_OUTPUT_BUFFER_LENGHT 256
#define VCF_LOW_PASS_MODE 0

//default defines, can we handle this more gracefully? 
#define DEFAULT_VCF_CUT_OFF             10000 
#define DEFAULT_VCF_RESONANCE           0.2

//MAP function used to scale modulation factors
#define MAP(in_min, in_max, out_min, out_max, x)    (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min


using namespace VeJa::Plugins::Filters;

enum{
    INPUT,
    CvMODInput,
    OUTPUT,
    Resonance,
    CutoffFreq,
    MOD_C,
    FIRST_ORDER_TYPE
};


class S6_filter{
public:
    S6_filter()
    {

    }
    ~S6_filter() {}
    static LV2_Handle instantiate(const LV2_Descriptor* descriptor, double samplerate, const char* bundle_path, const LV2_Feature* const* features);
    static void activate(LV2_Handle instance);
    static void deactivate(LV2_Handle instance);
    static void connect_port(LV2_Handle instance, uint32_t port, void *data);
    static void run(LV2_Handle instance, uint32_t n_samples);
    static void cleanup(LV2_Handle instance);
    static const void* extension_data(const char* uri);
    
    // Features
    LV2_URID_Map* map;
    LV2_URID urid_midiEvent;    
        
    //audio ports
    const LV2_Atom_Sequence* port_events_in;
    float *input;
    float *output;
    float *cvmodinput;

        
    //control ports
    float *resonance;
    float *cutofffreq;
    float *mod_c;
    float *fot;

    //important stuff
    float Tau;
    float sampleRate;

    float prev_resonance;
    float prev_cutofffreq;

    //Voltage Controlled Filter 12dB
    VoltageControlledFilter12dB<float> * _voltageControlledFIlter12DB;
};
/**********************************************************************************************************************************************************/


/**********************************************************************************************************************************************************/
LV2_Handle S6_filter::instantiate(const LV2_Descriptor*   descriptor,
double                              samplerate,
const char*                         bundle_path,
const LV2_Feature* const* features)
{
    S6_filter* self = new S6_filter();

    //instantiate voltage controlled filters
    self->_voltageControlledFIlter12DB = new VoltageControlledFilter12dB<float>(samplerate);

    return (LV2_Handle)self; 
}
/**********************************************************************************************************************************************************/
void S6_filter::connect_port(LV2_Handle instance, uint32_t port, void *data)
{
    S6_filter* self = (S6_filter*)instance;
    switch (port)
    {
        case INPUT:
            self->input = (float*) data;
            break;
        case CvMODInput:
            self->cvmodinput = (float*) data;
            break;
        case OUTPUT:
            self->output = (float*) data;
            break;
        case Resonance:
            self->resonance = (float*) data;
            break;
        case CutoffFreq:
            self->cutofffreq = (float*) data;
            break;
        case MOD_C:
            self->mod_c = (float*) data;
            break;
        case FIRST_ORDER_TYPE:
            self->fot = (float*) data;
            break;
    }
}
/**********************************************************************************************************************************************************/
void S6_filter::activate(LV2_Handle instance)
{
}

/**********************************************************************************************************************************************************/
void S6_filter::run(LV2_Handle instance, uint32_t n_samples)
{
    S6_filter* self = (S6_filter*)instance;
    float freq  = *self->cutofffreq;
    float res = *self->resonance;
    float mod_control = *self->mod_c;

    float lfo_vcf_mod = (MAP(0.f, 1.f, -1.f*(mod_control * 10.f), (mod_control * 10.f), self->cvmodinput[0] ) ) ;
    if (lfo_vcf_mod > 0) lfo_vcf_mod += 1.f;
    else lfo_vcf_mod = MAP(-1.f, 0.f, 0.5f, 1.f, lfo_vcf_mod);

    float fc = freq ;

    if (mod_control != 0)
    {
    	fc = freq * (lfo_vcf_mod) ;
    }

    ////limit value's
    if (fc < 20) 
    {
        fc =20.0f;
    }
    else if (fc > 20000)
    {
        fc = 20000.0f;
    }

    if (res < 0) res = 0.01f;
    if (res > 0.99) res = 0.99f;

    if ((res != self->prev_resonance) || (fc != self->prev_cutofffreq))
    {
    	self->_voltageControlledFIlter12DB->SetCoefficient(fc, res);

    	self->prev_resonance = res;
    	self->prev_cutofffreq = fc;
    }

  	////start audio processing
    for(uint32_t i = 0; i < n_samples; i++)
    {
        self->output[i] = self->_voltageControlledFIlter12DB->Process(self->input[i], *self->fot);
    }
}   

/**********************************************************************************************************************************************************/
void S6_filter::deactivate(LV2_Handle instance)
{
    // TODO: include the deactivate function code here
}
/**********************************************************************************************************************************************************/
void S6_filter::cleanup(LV2_Handle instance)
{
  delete ((S6_filter *) instance); 
}
/**********************************************************************************************************************************************************/
static const LV2_Descriptor Descriptor = {
    PLUGIN_URI,
    S6_filter::instantiate,
    S6_filter::connect_port,
    S6_filter::activate,
    S6_filter::run,
    S6_filter::deactivate,
    S6_filter::cleanup,
    S6_filter::extension_data
};
/**********************************************************************************************************************************************************/
LV2_SYMBOL_EXPORT
const LV2_Descriptor* lv2_descriptor(uint32_t index)
{
    if (index == 0) return &Descriptor;
    else return NULL;
}
/**********************************************************************************************************************************************************/
const void* S6_filter::extension_data(const char* uri)
{
    return NULL;
}
