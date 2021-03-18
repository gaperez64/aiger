/***************************************************************************
Copyright (c) 2021 Guillermo A. Perez, University of Antwerp

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to
deal in the Software without restriction, including without limitation the
rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
sell copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
IN THE SOFTWARE.
***************************************************************************/

#include "aiger.h"

#include <assert.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

static const char * USAGE =
"usage: bfuntots [-h][-v][-s <index>] <input> <output>\n"
"\n"
"Turns a boolean-function AIGER file into one encoding a\n"
"transition system. This is done by removing all outputs\n"
"and making them latches. The output with index '<index>'\n"
"is also 'sticky', that is, once true it will remain true.\n"
"the value of '<index>' defaults to 0. Removes the \n"
"output with index '<index>' from the AIGER model.\n"
"A new file will be generated with name '<output>'.\n"
;

static unsigned sticky;
static int verbose;
static aiger * src, * dst;

static void
die (const char *fmt, ...) {
  va_list ap;
  fputs ("*** [bfuntots] ", stderr);
  va_start (ap, fmt);
  vfprintf (stderr, fmt, ap);
  va_end (ap);
  fputc ('\n', stderr);
  fflush (stderr);
  exit (1);
}

static void
msg (int level, const char *fmt, ...) {
  va_list ap;
  if (verbose < level) return;
  fputs ("[bfuntots] ", stderr);
  va_start (ap, fmt);
  vfprintf (stderr, fmt, ap);
  va_end (ap);
  fputc ('\n', stderr);
  fflush (stderr);
}

static void 
bfuntots () {
  unsigned j;
  unsigned next_lit = (src->maxvar + 1) * 2;
  unsigned sticky_and = next_lit;
  next_lit += 2;
  unsigned sticky_latch = next_lit;
  next_lit += 2;
  aiger_and * a;

  msg (2, "loading the output file");

  // inputs
  for (j = 0; j < src->num_inputs; j++)
    aiger_add_input (dst, src->inputs[j].lit, src->inputs[j].name);
  // latches
  for (j = 0; j < src->num_latches; j++) {
    aiger_add_latch (dst, src->latches[j].lit, 
                          src->latches[j].next,
                          src->latches[j].name);
    aiger_add_reset (dst, src->latches[j].lit, src->latches[j].reset);
  }
  /* no outputs */
  // and gates
  for (j = 0; j < src->num_ands; j++) {
      a = src->ands + j;
      aiger_add_and (dst, a->lhs, a->rhs0, a->rhs1);
  }
  // bads
  for (j = 0; j < src->num_bad; j++)
    aiger_add_bad (dst, src->bad[j].lit, src->bad[j].name);
  // constraints
  for (j = 0; j < src->num_constraints; j++)
    aiger_add_constraint (dst,
                          src->constraints[j].lit,
                          src->constraints[j].name);
  // justices
  for (j = 0; j < src->num_justice; j++)
	aiger_add_justice (dst, 
                       src->justice[j].size,
                       src->justice[j].lits,
                       src->justice[j].name);
  // fairness
  for (j = 0; j < src->num_fairness; j++)
    aiger_add_fairness (dst,
                        src->fairness[j].lit,
                        src->fairness[j].name);
  // changing the outputs
  for (j = 0; j < src->num_outputs; j++) {
      if (j == sticky) {
        msg (2, "found sticky output: %s", src->outputs[j].name);
        aiger_add_and (dst, sticky_and, aiger_not(sticky_latch),
                            aiger_not(src->outputs[j].lit));
        aiger_add_latch (dst, sticky_latch,
                              aiger_not(sticky_and),
                              src->outputs[j].name);
      } else {
        aiger_add_latch (dst, next_lit,
                              src->outputs[j].lit,
                              src->outputs[j].name);
        next_lit += 2;
      }
  }

  msg (2, "prepared MILOA = %u %u %u %u %u", 
       dst->maxvar,
       dst->num_inputs,
       dst->num_latches,
       dst->num_outputs,
       dst->num_ands);

  msg (2, "prepared BCJF = %u %u %u %u",
       dst->num_bad,
       dst->num_constraints,
       dst->num_justice,
       dst->num_fairness);
}

int
main (int argc, char ** argv) {
  const char * input, * output, * err;
  unsigned i;

  // Default values
  verbose = sticky = 0;
  input = output = 0;

  for (i = 1; i < argc; i++) {
    if (!strcmp (argv[i], "-h")) {
	  printf ("%s", USAGE);
	  exit (0);
	}

    if (!strcmp (argv[i], "-v"))
	  verbose++;
    else if (!strcmp (argv[i], "-s")) {
	  if (++i == argc) die ("argument to '-s' missing");
      sticky = atoi(argv[i]);
    } else if (argv[i][0] == '-')
      die ("invalid command line option '%s'", argv[i]);
    else if (output)
	  die ("too many arguments");
    else if (input)
      output = argv[i];
    else
	  input = argv[i];
  }

  if (!input) 
    die ("no input specified");
  if (!output) 
    die ("no output specified");

  msg (1, "reading %s", input);
  src = aiger_init ();
  err = aiger_open_and_read_from_file (src, input);

  if (err)
    die ("read error: %s", err);

  msg (1, "read MILOA = %u %u %u %u %u", 
       src->maxvar,
       src->num_inputs,
       src->num_latches,
       src->num_outputs,
       src->num_ands);
  
  msg (1, "read BCJF = %u %u %u %u",
       src->num_bad,
       src->num_constraints,
       src->num_justice,
       src->num_fairness);
 
  dst = aiger_init();
  bfuntots ();

  aiger_open_and_write_to_file (dst, output);

  if (err)
    die ("write error: %s", err);

  msg (1, "wrote output file");

  aiger_reset (src);
  aiger_reset (dst);

  return 0;
}
