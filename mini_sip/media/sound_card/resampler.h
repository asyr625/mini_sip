#ifndef RESAMPLER_H
#define RESAMPLER_H

#include "my_types.h"
#include "sobject.h"
#include "splugin.h"
#include "ssingleton.h"


#include "mini_defines.h"

class Resampler : public SObject
{
public:
    virtual void resample( short * input, short * output ) = 0;

    virtual std::string get_mem_object_type() const {return "Resampler";}
};

class Resampler_Plugin : public SPlugin
{
public:
    Resampler_Plugin(SRef<Library*> lib);
    virtual ~Resampler_Plugin();

    virtual SRef<Resampler*> create_resampler( uint32_t inputFreq, uint32_t outputFreq,
                                               uint32_t duration, uint32_t nChannels ) const = 0;

    virtual std::string get_plugin_type() const{ return "Resampler"; }
};

class Resampler_Registry : public SPlugin_Registry, public SSingleton<Resampler_Registry>
{
public:
    virtual std::string get_plugin_type(){ return "Resampler"; }

    SRef<Resampler *> create( uint32_t inputFreq, uint32_t outputFreq,
                              uint32_t duration, uint32_t nChannels );
protected:
    Resampler_Registry();
private:
    friend class SSingleton<Resampler_Registry>;
};

#endif // RESAMPLER_H
