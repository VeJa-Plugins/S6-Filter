#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <math.h>
#include <lv2.h>

#include "VCF.h"
#include "VCF_24DB.h"

#include "lv2/lv2plug.in/ns/ext/atom/atom.h"
#include "lv2/lv2plug.in/ns/ext/atom/util.h"
#include "lv2/lv2plug.in/ns/ext/midi/midi.h"
#include "lv2/lv2plug.in/ns/ext/urid/urid.h"
#include "lv2/lv2plug.in/ns/lv2core/lv2.h"

/**********************************************************************************************************************************************************/
#define PLUGIN_URI "http://VeJaPlugins.com/plugins/Release/m8vcf"
#define PI 3.14159265358979323846264338327950288
#define MAX_PORTS 8
#define MAX_OUTPUT_BUFFER_LENGHT 256
#define VCF_LOW_PASS_MODE 0


enum{
    INPUT,
    CvEnvInput,
    CvLFOInput,
    CvPitchInput,
    OUTPUT,
    Resonance,
    CutoffFreq,
    Slope,
    LFO_MOD,
    ENV_MOD,
    PITCH_MOD
};


class Mars_8{
public:
    Mars_8()
    {
        //filter init
        VCF_filter = new VCF();
        VCF_24DB_filter = new VCF_24DB();
    }
    ~Mars_8() {}
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
    float *cvenvinput;
    float *cvlfoinput;
    float *cvpitchinput;

        
    //control ports
    float *resonance;
    float *cutofffreq;
    float *slope;
    float *lfo_mod;
    float *env_mod;
    float *pitch_mod;

    //important stuff
    float Tau;
    float sampleRate;

    //VCF filter 12dB
    VCF * VCF_filter;

    //VCF filer 24dB
    VCF_24DB * VCF_24DB_filter;

};
/**********************************************************************************************************************************************************/


/**********************************************************************************************************************************************************/
LV2_Handle Mars_8::instantiate(const LV2_Descriptor*   descriptor,
double                              samplerate,
const char*                         bundle_path,
const LV2_Feature* const* features)
{
    Mars_8* self = new Mars_8();

    //12dB
    //clear random value's
    self->VCF_filter->ClearState();
    //init with default coeffs, 20Khz, 0.5 resonane, Samplefreq of 48Khz
    self->VCF_filter->SetCoeff(20000, 0.5);

    //24dB
    self->VCF_24DB_filter->SetCoeff(20000, 0.5);

    return (LV2_Handle)self; 
}
/**********************************************************************************************************************************************************/
void Mars_8::connect_port(LV2_Handle instance, uint32_t port, void *data)
{
    Mars_8* self = (Mars_8*)instance;
    switch (port)
    {
        case INPUT:
            self->input = (float*) data;
            break;
        case CvEnvInput:
            self->cvenvinput = (float*) data;
            break;
        case CvLFOInput:
            self->cvlfoinput = (float*) data;
            break;
        case CvPitchInput:
            self->cvpitchinput = (float*) data;
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
        case Slope:
            self->slope = (float*) data;
            break;
        case LFO_MOD:
            self->lfo_mod = (float*) data;
            break;
        case ENV_MOD:
            self->env_mod = (float*) data;
            break;
        case PITCH_MOD:
            self->pitch_mod = (float*) data;
            break;
    }
}
/**********************************************************************************************************************************************************/
void Mars_8::activate(LV2_Handle instance)
{
}

/**********************************************************************************************************************************************************/
void Mars_8::run(LV2_Handle instance, uint32_t n_samples)
{
    Mars_8* self = (Mars_8*)instance;
    float order = *self->slope;
    float freq  = *self->cutofffreq;
    float res = *self->resonance;
    float lfo = *self->cvlfoinput;
    float lfo_slider = *self->lfo_mod;
    float pitch = *self->cvpitchinput;
    float pitch_slider = *self->pitch_mod;
    float env = *self->cvenvinput;
    float env_slider = *self->env_mod;

    //set filters
    float lfo_modulation = (lfo * (10*lfo_slider));
    float pitch_modulation = (pitch * (10*pitch_slider));
    float env_modulation = (env * (10*env_slider));

    float fc = (freq + lfo_modulation + pitch_modulation + env_modulation);
    //limit value's
    if (fc < 1) fc =1;
    if (fc > 20000) fc = 20000;

    self->VCF_filter->SetCoeff(fc, res);
    self->VCF_24DB_filter->SetCoeff((fc + lfo_modulation + pitch_modulation + env_modulation), res);

  	//start audio processing
    for(uint32_t i = 0; i < n_samples; i++)
    {
        //12 db
        if (order == 0)
        {
            self->output[i] = self->VCF_filter->Process(VCF_LOW_PASS_MODE, self->input[i]);
        }
        //24 db
        else 
        {
            self->output[i] = 3.0f * (self->VCF_24DB_filter->Process(self->input[i]));
        }
    }
}   

/**********************************************************************************************************************************************************/
void Mars_8::deactivate(LV2_Handle instance)
{
    // TODO: include the deactivate function code here
}
/**********************************************************************************************************************************************************/
void Mars_8::cleanup(LV2_Handle instance)
{
  delete ((Mars_8 *) instance); 
}
/**********************************************************************************************************************************************************/
static const LV2_Descriptor Descriptor = {
    PLUGIN_URI,
    Mars_8::instantiate,
    Mars_8::connect_port,
    Mars_8::activate,
    Mars_8::run,
    Mars_8::deactivate,
    Mars_8::cleanup,
    Mars_8::extension_data
};
/**********************************************************************************************************************************************************/
LV2_SYMBOL_EXPORT
const LV2_Descriptor* lv2_descriptor(uint32_t index)
{
    if (index == 0) return &Descriptor;
    else return NULL;
}
/**********************************************************************************************************************************************************/
const void* Mars_8::extension_data(const char* uri)
{
    return NULL;
}
