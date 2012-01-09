/*
 * $Id: ocrgen.c,v 1.1.1.1 2000/08/28 21:11:38 slumos Exp $
 *	
 * Statistical Spell Checker Prototype
 * Eric Stofsky
 * file : ocrgen.c (driver module)
 *
 * Ocrspell OCR-based statistical spell checker
 * Copyright (C) 1995 Regents of the University of Nevada
 * Text Retrieval Group
 * Information Science Research Institute
 * University of Nevada, Las Vegas
 * Las Vegas, NV 89154-4021
 * isri-text@isri.unlv.edu.
 *
 * See copyright.h for more information
 *
 * ocrgen.c - An interactive Spell Checker
 * This file is part of OCRSpell a package designed to correct OCR and
 * other device dependent spelling errors in text.  To fully install
 * OCRSpell, this program and the included lcs2cnf program must be 
 * compiled.  Then the ocrspell.el file must be loaded into EMACS and
 * byte-compiled.  Instructions for installing the ocrspell.el file are
 * included in that file. 
 *
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif /* HAVE_CONFIG_H */

#ifdef STDC_HEADERS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#endif /* STDC_HEADERS */

#include "copyright.h"
#include "statcheck.h"
#include "levelsat.h"
#include "config.h"
#include "ocrgen.h"
#include "intelligent_nonalpha_handler.h"
#include "ispell_emulate.h"
#include "learn.h"
#include "stats.h"

static char rcsid[] = "$Id: ocrgen.c,v 1.1.1.1 2000/08/28 21:11:38 slumos Exp $";

static char OCRSPELL_VERSION[] = VERSION_ID;

extern int getopt();
extern FILE *fopen();
  
char *prog;
word_props a_word;       
struct ocrspell_configuration cfg;

static void output_greeting();
static void learning_greating();
static void version();
static void usage();
static int set_default_configuration();
static void print_current_options();
static void help();
static int start_up();
static void terminate_logfile(char *);


static void
output_greeting()
/*
 * Display the output greating with version id
 */
{
     fprintf(stderr, "\nOCRSpell System (ocrspell) version %s\n",
	     OCRSPELL_VERSION);
     return;
}


static void
learning_greating()
/* 
 * Display the learning mode version id
 */
{
     fprintf(stderr, "\nOcrLearn version %s\n", OCRSPELL_VERSION);
     return;
}


static void
version()
/*
 * Display version information
 */
{
     output_greeting();
     fprintf(stderr,
	     "Copyright (C) 1995 Regents of the University of Nevada\n\n");
     fprintf(stderr,	"Kazem Taghva, Eric 'JokerMAN' Stofsky, Jeff Gilbreth, Julie Borsack\n");
     fprintf(stderr, "Text Retrieval Group\n");
     fprintf(stderr, "Information Science Research Institute\n");
     fprintf(stderr, "University of Nevada, Las Vegas\n");
     fprintf(stderr, "<isri-text@isri.unlv.edu>\n\n");
     exit(0);
}


static void
usage()
/*
 * Display usage information
 */
{
     fprintf(stderr,
	     "Usage: ocrspell [-ahpuvIg] [-w <word>] [-L <log_file>] [-l <learn_database>]\n");
     fprintf(stderr,
	     "                [-f <frequency_file>] [-d <alt_dictionary>]\n");
     fprintf(stderr," -h   help\n");
     fprintf(stderr," -v   show version information\n");
     fprintf(stderr," -u   show usage information\n");
     fprintf(stderr," -w   spell check word\n");
     fprintf(stderr," -a   run interactively\n");
     fprintf(stderr," -I   intelligent word detection\n");
     fprintf(stderr," -p   run interactively with prompts and stat info\n");
     fprintf(stderr," -g   run in fast mode\n");
     fprintf(stderr," -l   statistical learning feature\n");
     fprintf(stderr," -d   use alternate ispell hashed dictionary\n");
     fprintf(stderr," -f   use alternate frequency file\n");
     fprintf(stderr," -L   genenerate log file\n");
     fprintf(stderr,"File and Word Usage:\n");
     fprintf(stderr," <word>           word or sentence to be ocrspelled\n");
     fprintf(stderr," <log_file>       keeps track of non-hit errors/word generation\n");
     fprintf(stderr," <learn_database> new statistical info is kept in this file (-l option) \n");
     fprintf(stderr," <frequency_file> alternate ocr static frequency file\n");
     fprintf(stderr," <alt_dictionary> alternate ispell(1) hashed dictionary\n");
     exit(1);
}


static void
help()
/*
 * Display current ocrspell output syntax and usage information
 */
{
     output_greeting();
     fprintf(stderr, 
	     "If a word is not in the dictionary then a list of the ");
     fprintf(stderr, "form: \n");
     fprintf(stderr, "& misspelled-word number-of-misspellings offset: ");
     fprintf(stderr, "correction-list\n");
     fprintf(stderr, "is returned. If the word occurs in the lexicon ");
     fprintf(stderr, "* is returned.\n");
     usage();
}


int
set_default_configuration()
{
     /*
      * Set Ocrspell's default behavior.  Consult config.h for 
      * details for current settings.
      * USE_ALTERNATE_DICTIONARY :? alternate dictionary usage
      * SPELL_BY_LINE            :? spell by line or word
      * LOG_FILE                 :? generate log file
      * DICTIONARY_INSERTION     :? allow dictionary insertions
      * ISPELL_EVALUATION        :? generate/evaluate ocrspell's choices
      * MULTILEVEL_SATURATION    :? use level sat word generation
      * INTELLIGENT_WORD         :? use intelligent word boundary detection
      * INTERACTIVE_MODE         :? run in interactive mode
      * INTERACTIVE_PROMPT       :? interactive mode with stat analysis
      * INTERACTIVE_LEARNING     :? use device mapping generator
      * LINE_BREAK               :S line break sequence
      * OCR_STATIC_FREQFILE      :S path to ocr statistics
      * LETTER_NGRAM_FILE        :S path to n-gram statistics
      * ALT_DICTIONARY_PATH      :S path to alternate hashed dictionary
      */
     if (!(cfg.ispell_path =
	   (char *)malloc(strlen(ISPELL_PATH) + 1))){
	  perror(prog);
	  return(0);
     }
     strcpy(cfg.ispell_path,ISPELL_PATH);
     if (!(cfg.learn_database =
	   (char *)malloc(strlen(LEARN_DATABASE) + 1))){
	  perror(prog);
	  return(0);
     }
     strcpy(cfg.learn_database,LEARN_DATABASE);
     if (!(cfg.ocr_static_freqfile=
	   (char *)malloc(strlen(OCR_STATIC_FREQFILE) + 1))){
	  perror(prog);
	  return(0);
     }
     strcpy(cfg.ocr_static_freqfile,OCR_STATIC_FREQFILE);
     if (!(cfg.letter_ngram_file =
	   (char *)malloc(strlen(LETTER_NGRAM_FILE) + 1))){
	  perror(prog);
	  return(0);
     }
     strcpy(cfg.letter_ngram_file,LETTER_NGRAM_FILE);
     if (!(cfg.alt_dictionary_path =
	   (char *)malloc(strlen(ALT_DICTIONARY_PATH) + 1))){
	  perror(prog);
	  return(0);
     }
     strcpy(cfg.alt_dictionary_path,ALT_DICTIONARY_PATH);
     cfg.spell_by_line = SPELL_BY_LINE;
     if (!(cfg.line_break =
	   (char *)malloc(strlen(LINE_BREAK) + 1))){
	  perror(prog);
	  return(0);
     }
     strcpy(cfg.line_break,LINE_BREAK);
     cfg.use_alternate_dictionary = USE_ALTERNATE_DICTIONARY;
     cfg.log_file_p = LOG_FILE;
     cfg.dictionary_insertion = DICTIONARY_INSERTION;
     cfg.ispell_evaluation = ISPELL_EVALUATION;
     cfg.multilevel_saturation = MULTILEVEL_SATURATION;
     cfg.intelligent_word = INTELLIGENT_WORD;
     cfg.in_word = IN_WORD;
     cfg.interactive_mode = INTERACTIVE_MODE;
     cfg.interactive_prompt = INTERACTIVE_PROMPT;
     cfg.fast_stats_mode = FAST_STATS_MODE;
     cfg.interactive_learning = INTERACTIVE_LEARNING;
     cfg.evaluate_ispell_choices = EVALUATE_ISPELL_CHOICES;
     return(1);
}


static void
print_current_options()
/*
 * Display the current ocrspell configuration including any user
 * selected options via the -Q flag
 */
{
     fprintf(stderr,"\n\t\tCurrent Ocrspell Options\n");
     fprintf(stderr, "Ispell(1) path:%s\n",cfg.ispell_path);
     fprintf(stderr, "OCRspell Static Confusion file:%s\n",
	     cfg.ocr_static_freqfile);
     fprintf(stderr, "letter n-gram frequency file:%s\n",
	     cfg.letter_ngram_file);
     if (cfg.interactive_learning){
	  fprintf(stderr, "learning mode is on with learning database : ");
	  fprintf(stderr, "%s\n", cfg.learn_database);
     }
     else
     {
	  fprintf(stderr, "learning mode if off\n");
     }
     if (cfg.use_alternate_dictionary){
	  fprintf(stderr, "Alternate hashed dictionary:%s\n"
		  ,cfg.alt_dictionary_path);
     }
     else
     {
	  fprintf(stderr,
		  "There is no alternated hashed ispell(1) dictionary\n");
     }
     if (cfg.spell_by_line){
	  fprintf(stderr,"line break sequence is:%s\n",cfg.line_break);
     }
     else
     {
	  fprintf(stderr,"spell by line is off\n");
     }
     if (cfg.dictionary_insertion){
	  fprintf(stderr, "local dictionary insertion is on\n");
     }
     else
     {
	  fprintf(stderr, "local dictionary insertion is off\n");
     }
     if (cfg.ispell_evaluation){
	  fprintf(stderr, "Ispell(1) Evaluation is on\n");
     }
     if (cfg.multilevel_saturation){
	  fprintf(stderr, "Multilevel spelling correction is on\n");
     }
     if (cfg.interactive_mode){
	  fprintf(stderr, "Ocrspell is running in interactive mode\n");
     }
     if (cfg.interactive_prompt){
	  fprintf(stderr, "Ocrspell is running in interactive stats mode\n");
     }
     if (cfg.intelligent_word){
	  fprintf(stderr, "intelligent word boundary detection is on\n");
     }
     if (cfg.log_file_p){
	  fprintf(stderr, "log is being kept\n");
     }
}


static int
start_up()
/*
 * Perform typical ocrspell startup sequence:
 * (1) Read either the default or user selected frequency file(s)
 * (2) Read either the default or user selected n-gram statistics
 * (3) Create the n-gram lookup-table
 * (4) Start the external spell checking process
 */
{
     int no_of_ocr_errors;
     spell_checker_pid = -1;   /* initialize ispell process id */
     read_stat_list(&no_of_ocr_errors);
                               /* read ocr sub stats */
     open_letter_stats_file(); /* open normalization stats */
     create_letter_look_up();  /* create norm stat lookup struct */
     read_letter_file();       /* initialize norm stat lookup */
     
     if (!cfg.use_alternate_dictionary){
	  if (!spell_checker_startup(cfg.ispell_path,NULL)){
	       perror(prog);
	       exit(1);
	  }
     }
     else
     {
	  if (!spell_checker_startup(cfg.ispell_path,cfg.alt_dictionary_path)){
	       perror(prog);
	       exit(1);
	  }
      }
     return(no_of_ocr_errors);
}


static 
void terminate_logfile(queried_word)
char *queried_word;
/*
 * If ocrspell has been started with the log file feature selected,
 * check the input for the sentinel string.  This feature was used
 * primarily for debugging the first version of ocrspell and was
 * left in for evaluation purposes only.  This feature will be
 * enhanced in the future.
 */
{
     if (cfg.log_file_p){
	  fprintf(cfg.log_file,"\nnext iteration\n");
	  if (!strcmp(queried_word,"ocrspell_exit")){
	       fprintf(cfg.log_file,"\ndone\n");
	       fclose(cfg.log_file);
	       fprintf(stderr,"\nLog session completed: ");
	       execlp("date", "date", (char *) 0);
	       fprintf(stderr,"Problems with 'date'\n");
	       exit(1);
	  }
     }
}


void
main (argc,argv)
int argc; 
char *argv[];
{
     
     char original_word[MAX_WORD_CHAR];
     char *n;
     char *q;
     int current_word_offset;
     extern char *optarg;
     extern int optind;
     int IN_W1 = 1;
     stat_link f_p,f_q;
     stat_link head_stat;     /*pointer to statistic structure*/
     int number_of_mappings;
     int c;
          
     if ((q = strrchr(argv[0], '/'))){
	  q++;
     } else {
	  q = argv[0];
     }

     if (!(prog = (char *)malloc(strlen(q) + 1))){
	  perror(q);
	  exit(1);
     }
     
     strcpy(prog,q);
     if (!set_default_configuration()){
	  exit(1);
     }

     /*
      * List of used flags:
      *
      *    ABCDEFGHIJKLMNOPQRSTUVWXYZ abcdefghijklmnopqrstuvwxyz
      *            ^  ^    ^          ^  ^ ^^^   ^   ^    ^^^
      */
     
     while ((c = getopt(argc, argv, "ahpguvIw:L:l:f:d:Q")) != -1){
	  switch (c) {
	  case 'a':
	       /* run interactively */
	       cfg.interactive_mode = 1;
	       break;
	  case 'h':
	       /* help */
	       help();
	       break;
	  case 'p':
	       /* interactive with statistic information */
	       cfg.interactive_mode = 1;
	       cfg.interactive_prompt = 1;
	       break;
	  case 'g':
	       /* fast statistics mode */
	       cfg.interactive_mode = 1;
	       cfg.fast_stats_mode = 1;
	       cfg.evaluate_ispell_choices = 0;
	       break;
	  case 'u':
	       /* usage */
	       usage();
	       break;
	  case 'v':
	       /* version information */
	       version();
	       break;
	  case 'w':
	       /* spell check a single word */
	       strcpy(original_word, optarg);
	       break;
	  case 'I':
	       /* use intelligent word boundary detection */
	       cfg.interactive_mode = 1;
	       cfg.intelligent_word = 1;
	       break;
	  case 'L':
	       /* generate a log file */
	       cfg.interactive_mode = 1;
	       cfg.log_file_p = 1;
	       if (!(cfg.log_file = fopen(optarg,"w+"))){
		    perror(optarg);
		    exit(1);
	       }
	       break;
	  case 'l':
	       /* turn on learning mode */
	       cfg.interactive_learning = 1;
	       cfg.interactive_mode = 1;
	       learning_greating();
	       if (!(cfg.learn_database 
		     = (char *) malloc (strlen (optarg) + 1))){
		    perror(prog);
		    exit(1);
	       }
	       strcpy(cfg.learn_database,optarg);
	       break;
	  case 'f':
	       /* load an ocr frequency file */
	       if (!(cfg.ocr_static_freqfile
		     = (char *) malloc (strlen (optarg) + 1))){
		    perror(prog);
		    exit(1);
	       }
	       strcpy(cfg.ocr_static_freqfile,optarg);
	       break;
	  case 'd':
	       /* use an alternate hashed dictionary */
	       if (cfg.use_alternate_dictionary){
		    free(cfg.alt_dictionary_path);
	       }
	       if (!(cfg.alt_dictionary_path
		     = (char *) malloc (strlen(optarg) + 1))){
		    perror(prog);
		    exit(1);
	       }
	       strcpy(cfg.alt_dictionary_path,optarg);
	       break;
	  case 'Q':
	       /* dump user settings */
	       print_current_options();
	       exit(1);
	  case '?':
	  default:
	       usage();
	       break;
	  }
     }

     if ((optind <argc)|| (argc == 1)){
	  usage();
     }
     
     number_of_mappings = start_up();
     head_stat = NULL;
     dump_intro();

     while (IN_W1){
	  if (!cfg.interactive_mode){
	       IN_W1 = 0;
	  }
	  else
	  {
	       if (cfg.interactive_prompt){
		    dump_intro();
	       }
	       if (scanf("%s",original_word) <= 0) {
		 exit(0);
	       }
	  }
	  
	    
	  for (f_p = head_stat; f_p!= NULL;f_p = f_q){
	       f_q = f_p -> next; /* free each element of linked structure */
	       free(f_p);
	  }
	  
	  head_stat = NULL;  /*initialize statistics structure*/
	  
          terminate_logfile(original_word);
          	  
	  if (cfg.interactive_prompt){
	       printf("\n original word: %s\n", original_word);
	       printf("***********************************\n");
	  }
	  
	  if (!original_word) {
	       return;
	  }
	  
	  if (!in_punction_handler
	      /*
	       * Determine the word boundary for the current string.
	       * If intelligent nonalpha handling is on this will involve
	       * the statistical analysis of any nonalphabetic prefix or
	       * suffix.  If OCRSpell is running in standard mode, word
	       * boundary detection is done in a typical fashion.
	       */
	      (original_word,&current_word_offset,number_of_mappings)){
	       perror(prog);
	       exit(1);
	  }
	  
	  /*
	   * Copy isolated word into the string to be queried
	   */
	  strcpy(original_word,a_word.word_to_check);	  	       
	  	  	  
	  if (cfg.log_file_p){
	       fprintf(cfg.log_file,"\nOcrSpell %s Log File\n",
		       OCRSPELL_VERSION);
	  }
	  
	  if (!cfg.ispell_evaluation){
	       a_word.NO_ISPELL = 1;
	  }
	  else
	  {
	       if ((a_word.pure_character_word)
		   && (!a_word.dictionary_insertion)){
		    analyze_word_list(original_word,&n,&head_stat);
		    /* look at ispell output */
	       }
	  }
	  
	  if (!a_word.correct_spelling){
	       if ((cfg.log_file_p) && (!a_word.NO_ISPELL)){
		    dump_the_table(NULL,&n);
	       }
	       if (!a_word.pure_character_word){
		    a_word.NO_ISPELL = 1;
		    
	       }
	       
	       if ((!a_word.NO_ISPELL) && (cfg.evaluate_ispell_choices)){
		    /* generate statistics for previous word list */

		    /*
		     * Analyze any near misses that were generated by
		     * single character swapping
		     */
		    rate_the_choices_wrongletter
			 (original_word,&n,&head_stat,number_of_mappings);
		    
		    /*
		     * Anaylze any near misses that were generated by the
		     * deletion of single characters
		     */
		    rate_the_choices_extra_insertion
			 (original_word,&n,&head_stat,number_of_mappings);
		    
		    /*
		     * Anaylze any near misses that were generated by the
		     * insertion of single characters
		     */
		    rate_the_choices_missingletter
			 (original_word,&n,&head_stat,number_of_mappings);
	       }
	       

	       if (a_word.pure_character_word && cfg.evaluate_ispell_choices){
		    /* 
		     * generate new words, keeping track of the statistics
		     * for each string substitution
		     */
		    
		    /*
		     * generate near misses for simple device mapping 
		     * errors
		     */
		    ocrstringreplacement_generator
			 (original_word,&n,&head_stat,number_of_mappings);

		}    
	       
	       if (cfg.multilevel_saturation){
		    /*
		     * generate near misses for complex and compound
		     * device mapping errors
		     */
		    multilevelsaturation_generator
			 (original_word,&n,&head_stat,
			  cfg.fast_stats_mode,number_of_mappings);
	       }

	       if (cfg.interactive_prompt){
		    print_the_stat_list(head_stat);
	       }
	       
	       structure_setup(original_word,current_word_offset,head_stat);
	       /* ispell emulation */
	       
	       if (cfg.interactive_learning){
		    interactive_training(original_word,&number_of_mappings);
	       }
	  }
     }
     spell_checker_shutdown();
}



