/*
 * $Id: spell.h,v 1.1 2001/06/15 03:02:13 slumos Exp $
 *
 * Copyright (C) 1995 Regents of the University of Nevada
 *
 * See copyright.h for details.
 *
 */


/*
 * Call this function to create the spell checker process
 * and set up the necessary streams to talk to it.  Call
 * it before the first call to in_dictionary().  First arg
 * is the full path for the spell checker, second is for
 * an optional alternate dictionary path, use NULL for the
 * default dictionary.  Returns true (1) normally, false (0)
 * if it failed.
 */
int spell_checker_startup(char *spellcheckerpath, char *dictpath);

/*
 * Takes a string and returns true (1) if it's in the
 * dictionary and false (0) if not.  Also, if the 2nd
 * arg is non-NULL, it is taken to be an address of a char **
 * into which to store a list of alternative spellings,
 * if the word was misspelled.  The list of strings is
 * terminated with a NULL, a la argv.
 */
int in_dictionary(char *string, char ***possibilities);

/*
 * Call this function to shut down the spell checker process
 * and clean up.  Don't call in_dictionary() after calling this.
 * Returns true normally, false if it failed.
 */
int spell_checker_shutdown();
