#include "sox.h"
#include <assert.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define oops(func) (fprintf(stderr, "%s\n", func), exit(1))

void
fprintformat(FILE * stream, sox_format_t * format)
{
    fprintf(stream, "Channels: %d\n",  format->signal.channels);
    fprintf(stream, "Rate: %1.0f\n",   format->signal.rate);
    fprintf(stream, "Precision: %d\n", format->signal.precision);
    fprintf(stream, "Bits per: %d\n",  format->encoding.bits_per_sample);
}

sox_format_t *
setup_format_input(char * in_file)
{
    sox_format_t * input;

    sox_signalinfo_t in_signal = {
        8000, // rate
        1,    // channels
        0,    // precision
        0,    // length
        NULL  // mult
    };
    sox_encodinginfo_t in_encoding = {
        SOX_ENCODING_ALAW,  // encoding
        8,                  // bits_per_sample
        0,                  // compression
        sox_option_default, // reverse_bytes
        sox_option_default, // reverse_nibbles
        sox_option_default, // reverse_bits
        sox_false           // opposite_endian
    };

    //if (!(input = sox_open_read(filename, NULL, NULL, NULL)))
    if (!(input = sox_open_read(in_file, &in_signal, &in_encoding, "raw")))
        oops("sox_open_read()");

    //fprintf(stdout, "-----------INPUT----------\n");
    //fprintformat(stdout, input);

    return input;
}

sox_format_t *
setup_format_output(char * out_file)
{
    sox_format_t * output;

    sox_signalinfo_t out_signal = {
        48000, // rate
        1,     // channels
        0,     // precision
        0,     // length
        NULL   // mult
    };
    sox_encodinginfo_t out_encoding = {
        SOX_ENCODING_SIGN2, // encoding
        32,                 // bits_per_sample
        0,                  // compression
        sox_option_default, // reverse_bytes
        sox_option_default, // reverse_nibbles
        sox_option_default, // reverse_bits
        sox_false           // opposite_endian
    };

    if (!(output = sox_open_write(out_file, &out_signal, &out_encoding, "alsa", NULL, NULL)))
        oops("sox_open_write()");

    //fprintf(stdout, "-----------OUTPUT---------\n");
    //fprintformat(stdout, output);

    return output;
}

sox_effects_chain_t *
setup_chain(sox_format_t * input, sox_format_t * output)
{
    sox_effects_chain_t * chain = NULL;

    if (!(chain = sox_create_effects_chain(&input->encoding, &output->encoding)))
        oops("sox_create_effects()");

    char * args[10];
    sox_effect_t * e;

    /* The first effect in the effect chain must be something that can source
     * samples; in this case, we have defined an input handler that inputs
     * data from an audio file */
    e = sox_create_effect(sox_find_effect("input"));
    args[0] = (char *) input;
    assert(sox_effect_options(e, 1, args) == SOX_SUCCESS);
    assert(sox_add_effect(chain, e, &input->signal, &input->signal) == SOX_SUCCESS);
    free(e);

    e = sox_create_effect(sox_find_effect("rate"));
    char * rate[] = {"48k"};
    assert(sox_effect_options(e, 1, rate) == SOX_SUCCESS);
    assert(sox_add_effect(chain, e, &input->signal, &output->signal) == SOX_SUCCESS);
    free(e);

    e = sox_create_effect(sox_find_effect("output"));
    args[0] = (char *) output;
    assert(sox_effect_options(e, 1, args) == SOX_SUCCESS);
    assert(sox_add_effect(chain, e, &output->signal, &output->signal) == SOX_SUCCESS);
    free(e);

    return chain;
}

int
decode_init()
{
    sox_globals.verbosity = 0;
    return sox_init();
}

void
decode_destroy()
{
    sox_quit();
}

void *
decode_playback(void * context)
{
    char* filename = context;

    sox_format_t * input;
    sox_format_t * output;
    sox_effects_chain_t * chain;

    input = setup_format_input(filename);
    output = setup_format_output("default");
    chain = setup_chain(input, output);

    /* Flow samples through the effects processing chain until EOF is reached */
    sox_flow_effects(chain, NULL, NULL);

    sox_delete_effects(chain);
    sox_close(input);
    sox_close(output);
}
