;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;; 
;; ISRI OCRspell Checker (C) 1995
;; Ocrspell OCR-based statistical spell checker
;; Copyright (C) 1995 Regents of the University of Nevada
;; Text Retrieval Group
;; Information Science Research Institute
;; University of Nevada, Las Vegas
;; Las Vegas, NV 89154-4021
;; isri-text@isri.unlv.edu.
;; A multi-featured ocrspell emacs-lisp program
;; Authors : Eric Stofsky et. al.
;; Some of the code and ideas encorporated into ocrspell.el
;; has been adopted from ispell.el which is part of GNU EMACS
;; ISRI OCRspell is distributed in the hope that it will be useful,
;; but WITHOUT ANY WARRANTY; without even the implied warranty of
;; MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. 
;; TR GROUP
;; Kazem Taghva, Eric 'JokerMAN' Stofsky, Jeff Gilbreth, Julie Borsack
;; Allen Condit
;; Version : for Ocrspell version 1.0.00
;; Bug Reports : stof@isri.unlv.edu
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;                         DESCRIPTION
; ISRI Ocrspell is an interactive spell checker designed to aid the
; process of correcting OCR based spelling errors occuring in text
; documents.  The end "ocrspelled" document will be more suited for
; both presentation and possible text retrieval.  The html mode feature
; along with the spelling engine, join features, and run-time dynamic
; confusion matrix allow for easy correction of ocr'd materials for
; presentation.  Other features provide for a robost, easy to use,
; system.
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;                          FEATURES                                  
; (1)  Full two level saturation statistical analysis of all incorrect
;      words
; (2)  Construction of run-time dynamic confusion matrix (LCS analysis
;      on all replacements)  
; (3)  Pure word insertion (inserted word will be fully involved in 
;      future queries)
; (4)  Learning Mode
; (5)  Fully menu driven with word choices being displayed in a window
;      at the top of the buffer (with mnemonic options)
; (6)  Soft character mappings
; (7)  Creates a .choices buffer, recording all changes in the buffer
;      in the form original -> replacement (queries user at the end to
;      save)
; (8)  Allows user to spell a word, region, or entire buffer
; (9)  True hyphenation handeling - with hyphen is whitespace toggle
;      switch option
; (10) Allows insertion of special character sequences
; (11) HTML mode which skips over tags and special character sequences
; (12) Dynamic confusion matrices can be saved and loaded at a future
;      time
; (13) User can bound the number of choices for each misspelling which
;      occurs in the document
; (14) Loads/Saves Session Files
; (15) Allow user the option of skipping numbers and index (reference)
;      expressions
; (16) Allow user to treat numbers within word boundaries as words
;      while skipping total number words
; (17) Correct ocr error tilda '~' handeling
; (18) Allow for ocrspell customized startup
;      user can specify: alternate frequency files
;                        alternate hashed dictionaries
;                        whether to use intelligent word detection
; (19) User can interactively join words (previous or next)
; (20) Heuristically based stemming/pluralization on error
; (21) Global replacement option
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;                         HELPFUL HINTS
; 1.  Make sure to set the appropriate toggle switches.
; 2.  If you wish to use an alternate dictionary or frequency file
;     make sure special-startup-mode is on.
; 3.  If you find that the dynamic confusion matrix has been 
;     corrupted or is simply too large, select clear confusions from 
;     the menu.
; 4.  If you have bounded the number of suggestions per word and  
;     wish to relax this restriction, use the set-number-word-choices
;     function and enter a negative number
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; Last Modified: Mon Jan 29 15:35:30 PST 1996
; Last Mofified by: Eric Stofsky
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; Last Modification:
; stof@isri.unlv.edu
; Wed Oct 18 18:40:30 PDT 1995
; Added functions for reading and writing a dynamic confusion matrix 
; to file. Disabled Ocrspell Region from menu when there isnt a 
; highlighted region.
;
; stof@isri.unlv.edu
; Mon Jan 15 16:15:24 PST 1996
; Added interactive join feature, hyphen is whitespace toggle-switch
; and function, and cleaned read-session-file up a bit.
;
; stof@isri.unlv.edu
; Wed Jan 17 18:12:14 PST 1996
; Added join previous feature, fixed bug that wouldn't allow a user 
; to quit until the last error on the current line was reached.
; Added global replacement feature.
;
; stof@isri.unlv.edu
; Fri Jan 19 13:11:04 PST 1996
; Added stemming feature, with toggle function, to the tilde handler.
; Added heuristic capitalization and pluralization as well.
; Modified mini-buffer prompt to display all of the current options.
;
; stof@isri.unlv.edu
; Mon Jan 29 15:35:30 PST 1996
; Added ocrspell-key-help function, expanded ocrspell-help.
; Added code to return the cursor to the buffer and beep if the 
; user quits in the middle of a document. Deleted inconsequential
; messages from this list for brevity.
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; Commentary:
;
; INSTRUCTIONS
; The following constants must contain the full location of the specified
; files. Ocrspell must be compiled first (the source is included in this
; distribution).  A copy of lcs2cnf is included as well.  After specifying
; the location of these files, to fully install ocrspell.el, it must be
; compiled by loading in into emacs and using the M-X byte-compile-file
; command.  Depending on usage it may be desirable to have many of the 
; functions listed below added to the appropriate init file via the
; autoload function
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; paths to executables
; ispell, lcs2cnf, ocrspell
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; to properly install the next 3 declarations must be changed to the 
; actual location of these executables
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

(defconst ispell-exec "/local/gnu/bin/ispell"
  "The full path to the ispell executable")

(defconst ocrspell-exec
  "/space/src/isri/ocrspell-23jan97/src/ocrspell"
  "The full path to the ocrspell executable")

(defconst lcs2cnf-exec 
  "/space/src/isri/ocrspell-23jan97/src/lcs2cnf"
  "The full path to the lcs2cnf executable")

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; Path to ispell.el GNU interface for ispell
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

(defconst ispell-el-file "/local/emacs/share/emacs/site-lisp/ispell/ispell.el"
  "The full path to the ispell.el file")

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;some ispell character mapping stuff
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

(autoload 'ispell-get-casechars  "ispell.el"
  "Load the character set." t)

(autoload 'ispell-get-not-casechars  "ispell.el"
  "Load the non-character set." t)

(autoload 'ispell-get-otherchars  "ispell.el"
  "Load the other character sets." t)

(autoload 'ispell-get-many-otherchars-p  "ispell.el"
  "Load entire other char set." t)

(load ispell-el-file)

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;define process and pipe filter vars
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

(defvar ocrspell-process nil "The process running OCRSPELL")

(defvar lcs2cnf-process nil
  "The process running lcs2cnf")

(defconst allowable-confusion-chars "[A-Z0-9a-z!@#$%^&*()-.,~;'\"]*"
  "The set of characters allowed in the confusion matrix")

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;define confusion matrices lists
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

 
(defvar global-ocrspell-dynamic-confusions nil
  "The dynamic confusions per session")

(defvar ocrspell-dynamic-confusions nil 
  "The dynamic confusions per entrie")

(defvar tilda-count 0
  "The number of unrecognized ocr characters in a given
word")

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; define filter, window, buffer, and pipe information
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;


(defvar ocrspell-filter nil
  "Output filter from piped calls to ocrspell.")

(defvar doing-ocrword nil "Are we checking a single word?")

(defvar ocrspell-filter-continue nil
  "Control variable for ocrspell filter function.")

(defvar ocrspell-keep-choices-win t
  "*When not nil, the *Choices* window remains for spelling session.
This minimizes redisplay thrashing.")

(defvar special-configure-flags nil
  "Contains all of the user defined options if the ocrspell subprocess
is started via start-ocrspell-with-special-configuration()")

(defvar ocrspell-following-word nil
  "*Check word under or following cursor when non-nil.
Otherwise the preceding word is checked by ocrspell-word (\\[ocrspell-word]).")

(defvar ocrspell-quietly nil
  "*Messages suppressed in ocrspell-word when non-nil and interactive.")

(defvar ocrspell-choices-win-default-height 2
  "*The default size of the *Choices* window, including status line.
Must be greater than 1.")

(defvar ocrspell-quit nil
  "When non-nil the spell session is terminated.
When numeric, contains cursor location in buffer, and cursor remains there.")

(defvar ocrspell-use-ptys-p nil
  "When non-nil, emacs will use pty's to communicate with ocrspell.
When nil, emacs will use pipes.")

(defconst ocrspell-choices-buffer "*Choices*")

(defvar ocrspell-keep-choices-win t
  "*When not nil, the *Choices* window remains for spelling session.
This minimizes redisplay thrashing.")

(defvar ocrspell-special-insertion nil
  "List containing all words inserted by the user which contain
non alphabetic characters.")

(defvar ocrspell-spec-mess-handle nil
  "Non-nil when we're checking a mail message")

(defvar special-config-startup-p nil
  "Non-nil when the user whats to run a customized version of ocrspell")

(defvar ocrspell-comment-check-p nil
  "Spelling of comments checked when non-nil.")

(defvar ocrspell-dynamic-stem-p nil
  "Perform dynamic stemming on tilde errors when non-nil")

(defvar ocrspell-html-mode-p nil
  "True if user has selected html mode which skips over tags")

(defvar ocrspell-skipnumbers-mode-p nil
  "True if user has selected skipnumbers mode which skips over
numbers and indexing expressions")

(defvar ocrspell-skip-number-words-only-p t
  "True if user has selected skipnumberwords mode which skips over
number words but not numbers occuring within alpha word boundaries")

(defvar ocrspell-hyph-whitespace-p nil
  "True if user has selected hyphens as whitespace mode which treats
all hyphens in the document as though they were white space")


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; Define query-replace procedure
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

(defvar ocrspell-query-replace-choices nil
  "*Corrections made throughout region when non-nil.
Uses query-replace (\\[query-replace]) for corrections.")

(defvar ocrspell-query-replace-marker (make-marker)
  "Marker for query-replace processing.")

(defvar ocrspell-region-end-mark (make-marker)
  "Marker that allows spelling continuations.")


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; Highlight misspelled words?
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

(defvar ocrspell-highlight-p t
  "*Highlight spelling errors when non-nil.")


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; Is there a user specified limit on the number of choices per 
; misspelling
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

(defvar max-number-word-choices nil
  "Holds the limit on the number of user choices")

(defvar max-number-word-choices-p nil
  "Non-nil is there is a limit on the number of choices")


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; Redefine ispell's menu to allow for ocrspell features
; Create a menu that has all of standard ispell 3.1 commands
; and all of the ocrspell 1.0.00 commands
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;


(cond
 ((and (string-lessp "19" emacs-version)
       (string-match "Lucid" emacs-version))

  (let ((dicts (cons (cons "default" nil) ispell-dictionary-alist))
	(current-menubar (or current-menubar default-menubar))

	(menu
	 '(["Help"		(describe-function 'ispell-help) t]
	   ;;["Help"		(popup-menu ispell-help-list)	t]
	   ["OCRspell Help"     ocrspell-help                   t]
	   ["OCRspell Word"     ocrspell-word                   t]
	   ["OCRspell Region"   ocrspell-region                 t]
	   ["OCRspell Buffer"   ocrspell-buffer                 t]
	   ["Clear Confusions"  clear-dynamic-confusions        t]
	   ["Read Dynamic"      read-dynamic-file               t]
	   ["Save Dynamic"      save-dynamic-file               t]
	   ["Bound # Of Choices"set-number-word-choices         t]
	   ["Check Message"	ispell-message			t]
	   ["Check Buffer"	ispell-buffer			t]
	   ["Check Word"	ispell-word			t]
	   ["Check Region"	ispell-region  (or (not zmacs-regions) (mark))]
	   ["Continue Check"	ispell-continue			t]
	   ["Complete Word Frag"ispell-complete-word-interior-frag t]
	   ["Complete Word"	ispell-complete-word		t]
	   ["Kill Process"	ispell-kill-ispell		t]
	   "-"
	   ["Save Dictionary"	(ispell-pdict-save t)		t]
	   ["Change Dictionary"	ispell-change-dictionary	t]))
	name)

    (while dicts
      (setq name (car (car dicts))
	    dicts (cdr dicts))

      (if (stringp name)
	  (setq menu (append menu
			     (list
			      (vector (concat "Select " (capitalize name))
				      (list 'ispell-change-dictionary name)
				      t))))))

    (defvar ispell-menu-lucid menu "Lucid's spelling menu.")

    (if current-menubar
	(progn
	  (delete-menu-item '("Edit" "Spell")) ; in case already defined
	  (add-menu '("Edit") "Spell" ispell-menu-lucid)))))
 

 ((and (featurep 'menu-bar)		
       (string-lessp "19" emacs-version))

  (let ((dicts (reverse (cons (cons "default" nil) ispell-dictionary-alist)))
	name)

    (defvar ispell-menu-map nil)
    ;; Can put in defvar when external defines are removed.
    (setq ispell-menu-map (make-sparse-keymap "Spell"))

    (while dicts
      (setq name (car (car dicts))
	    dicts (cdr dicts))
      (if (stringp name)
	  (define-key ispell-menu-map (vector (intern name))
	    (cons (concat "Select " (capitalize name))
		  (list 'lambda () '(interactive)
			(list 'ispell-change-dictionary name))))))

    ;; ALIAS MENU MAP
    (defalias 'ispell-menu-map ispell-menu-map)
    ;; Define commands in opposite order you want them to appear in menu.

    (define-key ispell-menu-map [ispell-change-dictionary]
      '("Change Dictionary" . ispell-change-dictionary))

    (define-key ispell-menu-map [ispell-kill-ispell]
      '("Kill Process" . ispell-kill-ispell))

    (define-key ispell-menu-map [ispell-pdict-save]
      '("Save Dictionary" . (lambda () (interactive) (ispell-pdict-save t))))

    (define-key ispell-menu-map [ispell-complete-word]
      '("Complete Word" . ispell-complete-word))

    (define-key ispell-menu-map [ispell-complete-word-interior-frag]
      '("Complete Word Frag" . ispell-complete-word-interior-frag))

    (define-key ispell-menu-map [ispell-continue]
      '("Continue Check" . ispell-continue))

    (define-key ispell-menu-map [ispell-region]
      '("Check Region" . ispell-region))

    (define-key ispell-menu-map [ispell-word]
      '("Check Word" . ispell-word))

    (define-key ispell-menu-map [ispell-buffer]
      '("Check Buffer" . ispell-buffer))

    (define-key ispell-menu-map [ispell-message]
      '("Check Message" . ispell-message))

    (define-key ispell-menu-map [set-number-word-choices]
      '("Bound # Of Choices" . set-number-word-choices))

    (define-key ispell-menu-map [save-dynamic-file]
      '("Save Dynamic" . save-dynamic-file))

    (define-key ispell-menu-map [read-dynamic-file]
      '("Read Dynamic" . read-dynamic-file))

    (define-key ispell-menu-map [clear-dynamic-confusions]
      '("Clear Confusions" . clear-dynamic-confusions))

    (define-key ispell-menu-map [ocrspell-buffer]
      '("OcrCheck Buffer" . ocrspell-buffer))

    (define-key ispell-menu-map [ocrspell-region]
      '("OcrCheck Region" . ocrspell-region))

    (define-key ispell-menu-map [ocrspell-word]
      '("OcrCheck Word" . ocrspell-word))

    (define-key ispell-menu-map [ocrspell-help]
      '("OcrSpell Help" . ocrspell-help))

    (define-key ispell-menu-map [ispell-help]
      '("Help" . (lambda () (interactive)
		   (describe-function 'ispell-help)
		  ;(x-popup-menu last-nonmenu-event(list "" ispell-help-list))
		   ))))

;; disable region functions when no region is selected
(put 'ocrspell-region 'menu-enable 'mark-active)
(put 'ispell-region 'menu-enable 'mark-active)))


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;                  Start of Function Definitions                 
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;




(defun ocrspell-help ()
  "Creates an *ocrspell-help* buffer and lists all of the available ocrspell
options"
  (interactive)
    (if (one-window-p)
      (split-window-vertically 25))
  (select-window (next-window))

  (set-buffer (get-buffer-create "*ocrspell-help*"))
  (set-window-buffer (selected-window) "*ocrspell-help*")
  (erase-buffer)

  (princ "OCRspell v1.0 HELP" (current-buffer))
  (terpri (current-buffer))
  (princ "ocrspell-version..........list version info"
	  (current-buffer))
  (terpri (current-buffer))
  (princ
   "ocrspell-word.............spell check word highlighted or closest to mouse"
   (current-buffer))
  (terpri (current-buffer))
  (princ 
   "ocrspell-region...........spell check highlighted region" (current-buffer))
  (terpri (current-buffer))
  (princ
   "ocrspell-buffer...........spell check current buffer" (current-buffer))
  (terpri (current-buffer))
  (princ
   "clear-dynamic-confusions..clears current dynamic confusion list" 
   (current-buffer))
  (terpri (current-buffer))
  (princ "set-number-word-choices...sets a limit on the maximum number of"
	 (current-buffer))
  (terpri (current-buffer))
  (princ "                          choices per misspelling" 
	 (current-buffer))
  (terpri (current-buffer))
  (princ
   "read-dynamic-file.........reads a dynamic confusion matrix from file "
   (current-buffer))
  (terpri (current-buffer))
  (princ
   "save-dynamic-file.........saves a dynamic confusion matrix from file" 
   (current-buffer))
  (terpri (current-buffer))
  (princ "ocrspell-html-mode........toggles into or out of html mode" 
	 (current-buffer))
  (terpri (current-buffer))
  (princ "ocrspell-skipnumbers-mode.toggles into or out of skip numbers mode"
	 (current-buffer))
  (terpri (current-buffer))
  (princ "  use ocrspell-skipnumberwords-mode to skip number words only"
	 (current-buffer))
  (terpri (current-buffer))
  (princ
   "special-startup-mode......toggles between default and custom ocrspell startup" 
   (current-buffer))
  (terpri (current-buffer))
  (princ "ocrspell-hyph-whitespace..toggles between hyphen handling modes"
	 (current-buffer))
  (terpri (current-buffer))
  (princ "dynamic-stem-mode.........toggles betweeen dynamic stemming handling"
	 (current-buffer))
  (terpri (current-buffer))
  (princ "                          on tilde error words"
	 (current-buffer))
  (terpri (current-buffer))
  (princ "save-session-file.........saves current session file"
	 (current-buffer))
  (terpri (current-buffer))
  (princ "read-session-file.........loads a session file"
	 (current-buffer))
  (terpri (current-buffer))
  (princ "ocrspell-key-help.........help for the ocrspell key options"
	 (current-buffer))
  (terpri (current-buffer))
  (princ "kill-ocrspell.............emergency use"
	 (current-buffer))
  (terpri (current-buffer))
  (princ "Further documentation for all functions can be obtained by simply selecting"
	 (current-buffer))
  (terpri (current-buffer))
  (princ "c-h f and typing in the name of the function" (current-buffer))

  (beep)(beep)
  (select-window (next-window))
)



(defun ocrspell-key-help ()
  "Creates an *ocrspell-menu-help* buffer and lists all of the 
available ocrspell options"
  (interactive)
    (if (one-window-p)
      (split-window-vertically 25))
  (select-window (next-window))

  (set-buffer (get-buffer-create "*ocrspell-menu-help*"))
  (set-window-buffer (selected-window) "*ocrspell-menu-help*")
  (erase-buffer)

  (princ "OCRspell v1.0 INTERACTIVE MODE HELP" (current-buffer))
  (terpri (current-buffer))
  (princ "[i] insert word into the lexicon for the session"
          (current-buffer))
  (terpri (current-buffer))
  (princ "[r] replace word, find confusions, and add to device mappings list"
	 (current-buffer))
  (terpri (current-buffer))
  (princ "[b] backward join \(Merges current and previous word\)"
	 (current-buffer))
  (terpri (current-buffer))
  (princ "[j] forward join  \(Merges current and next word\)" 
	 (current-buffer))
  (terpri (current-buffer))
  (princ "[g] same as [r] but also queries the user on other instances"
	 (current-buffer))
  (terpri (current-buffer))
  (princ "    of the word in the current document"
	 (current-buffer))
  (terpri (current-buffer))
  (princ "[<space>] skip the current word or highlighted region"
          (current-buffer))
  (terpri (current-buffer))
  (princ "[<character>] replace the current highlighted word with the"
	 (current-buffer))
  (terpri (current-buffer))
  (princ "              selection corresponding to <character> in the"
	  (current-buffer))
  (terpri (current-buffer))
  (princ "              choices list" (current-buffer))
   (terpri (current-buffer))
  (princ "[q] quit the current Ocrspell session"
	 (current-buffer))
  (terpri (current-buffer))
)



(defun ocrspell-html-mode ()
  "Sets ocrspell-html-mode-p to true, which makes ocrspell skip over HTML
tags"
  (interactive)
  (if (not ocrspell-html-mode-p)  
      (setq ocrspell-html-mode-p t)
    (setq ocrspell-html-mode-p nil))
)



(defun dynamic-stem-mode ()
  "Sets ocrspell-dynamic-stem-p to true, which causes all words with ~'s
in them to be dynamically stemmed, it already true the predicate is set to
false"
  (interactive)
  (if (not ocrspell-dynamic-stem-p)  
      (setq ocrspell-dynamic-stem-p t)
    (setq ocrspell-dynamic-stem-p nil))
)



(defun ocrspell-hyph-whitespace ()
  "Sets ocrspell-hyph-whitespace-p to true, which makes ocrspell 
treat hyphens as though they where whitespace"
  (interactive)
  (if (not ocrspell-hyph-whitespace-p)  
      (setq ocrspell-hyph-whitespace-p t)
    (setq ocrspell-hyph-whitespace-p nil))
)



(defun ocrspell-skipnumbers-mode ()
  "Toggles ocrspell-skipnumbers-mode-p  to true, which makes 
ocrspell skip over numbers and numeric indexes"
  (interactive)
  (if (not ocrspell-skipnumbers-mode-p)  
      (setq ocrspell-skipnumbers-mode-p t)
    (setq ocrspell-skipnumbers-mode-p nil))
)



(defun ocrspell-skipnumberwords-mode ()
  "Toggles ocrspell-skip-number-words-only-p to true, which makes 
ocrspell skip over numbers words but not words within word bounderies"
  (interactive)
  (if (not ocrspell-skip-number-words-only-p)  
      (setq ocrspell-skip-number-words-only-p t)
    (setq ocrspell-skip-number-words-only-p nil))
)



(defun special-startup-mode ()
  "Toggles special-config-startup-p to true/false, which makes 
ocrspell provide a customizable startup"
  (interactive)
  (if (not special-config-startup-p)  
      (setq special-config-startup-p t)
    (setq special-config-startup-p nil))
)


(defun clear-dynamic-confusions ()
  "Allow user to interactively clear the list of dynamic confusions
in between documents during a session."
  (interactive)
  (setq global-ocrspell-dynamic-confusions nil)
)




(defun ocrspell-version ()
  "Display current version number"
  (interactive)
  (message "Ocrspell System (ocrspell) version 1.0.0")
)



(defun kill-ocrspell ()
  "Kill the ocrspell process."
  (interactive)
  (if (not (and ocrspell-process
	       (eq (process-status ocrspell-process) 'run)))
	 (message "Ocrspell is not running..."))
  (if ocrspell-process
      (delete-process ocrspell-process))
  (setq ocrspell-process nil)
  (message "killed Ocrspell process")
  nil
)



(defun set-number-word-choices ()
  "Allow user to interactively select the maximum number of word choices. If
a negative or non-integer string is entered, the maximum will be unbounded."
  (interactive)
  (setq max-number-word-choices 
	(string-to-int(read-no-blanks-input 
		       "Maximum word choices [neg for unbounded]:")))
  (setq max-number-word-choices-p t)
  (if (< max-number-word-choices 1)
      (setq max-number-word-choices-p nil))
  (message "limit set")
)



(defun start-ocrspell ()
  "Start an ocrspell subprocess; display the greeting."
  (interactive)
  (message "Starting an ocrspell process...")
  (let ((buf (get-buffer "*ocrspell*"))))
  (setq ocrspell-process (start-process "ocrspell" "*ocrspell*" 
ocrspell-exec "-a"))
  ;;change nil to "*ocrspell*"
  (message "ocrspell started...")
  (process-kill-without-query ocrspell-process)
  (buffer-flush-undo (process-buffer ocrspell-process))
  ;; uncomment next 5 lines for buffer/pipe toggle
  ;;(accept-process-output ocrspell-process)
  ;;(set-buffer (process-buffer ocrspell-process))
  ;;(bury-buffer (current-buffer))
  ;;(setq last-char (- (point-max) 1))
  ;;(delete-region (point-min) last-char)
  (setq ocrspell-filter nil)
  (setq ocrspell-filter-continue nil)
  (set-process-filter ocrspell-process 'ocrspell-filter)
  (accept-process-output ocrspell-process)
  (message "%s" (car ocrspell-filter))
  (setq ocrspell-filter nil)
  (message "startup procedure complete")
)



(defun start-ocrspell-with-special-configuration ()
  "Start an ocrspell subprocess; display greating
   ocrspell currently has the following features:
   ocrspell [-ahpuvI] [-w <word>] [-L <log_file>] [-l <learn_database>]
                [-f <frequency_file>] [-d <alt_dictionary>]
   -w   spell check word
   -a   run interactively
   -I   intelligent word detection
   -p   run interactively with prompts and stat info
   -l   statistical learning feature
   -d   use alternate ispell hashed dictionary
   -f   use alternate frequency file
   -L   genenerate log file
    File and Word Usage:
     <word>           word or sentence to be ocrspelled
     <log_file>       keeps track of non-hit errors/word generation
     <learn_database> new statistical info is kept in this file (-l option) 
     <frequency_file> alternate ocr static frequency file
     <alt_dictionary> alternate ispell(1) hashed dictionary
  The -a option is used by default in start-ocrspell
  The -w option is emulated by the -a option in start-ocrspell
  The -l option, statistical learning feature, cannot be used with this
    version of the interface
  The -p option, is typically used in conjunction with the -l option and
    is also not supported by this version of the interface
  Use this function to:
    Run ocrspell with :
      (1) intelligent word detection
      (2) use alternate ispell hashed dictionary
      (3) use alternate frequency file
  Since the  .choices buffer is created by ocrspell.el the -L (log file
     feature) is also not supported but can be easily added"
  (interactive) 
  (message "Starting a custom ocrspell process...")
  (let*
      ((flag-I (yes-or-no-p "Do you wish to use intelligent word detection?"))
       (flag-d (yes-or-no-p "Do you with to use an alt ispell dictionary?"))
       (flag-d-path 
	(if flag-d (read-string
		    "What is the complete path to the dictionary?")))
       (flag-f (yes-or-no-p "Do you wish to use an alternate freq file ?"))
       (flag-f-path
	(if flag-f (read-string 
		    "What is the complete path to the freq file ?"))))
    (setq special-configure-flags "-a ")
    (if flag-I 
	(setq special-configure-flags 
	      (concat special-configure-flags " -I")))
    (if flag-d
	(setq special-configure-flags 
	      (concat special-configure-flags " -d " flag-d-path)))
    (if flag-f
	(setq special-configure-flags
	      (concat special-configure-flags " -f " flag-f-path)))   
    (message "ocrspell %s"  special-configure-flags)
    (let ((buf (get-buffer "*ocrspell*"))))
    (cond
     ((and flag-I (not flag-d) (not flag-f))
      (setq ocrspell-process (start-process "ocrspell" "*ocrspell*" 
					    ocrspell-exec
					    "-a" "-I")))
     ((and flag-I flag-d (not flag-f))
      (setq ocrspell-process (start-process "ocrspell" "*ocrspell*" 
					    ocrspell-exec
					    "-a" "-I" "-d" flag-d-path)))
     ((and flag-I flag-d flag-f)
      (setq ocrspell-process (start-process "ocrspell" "*ocrspell*" 
					    ocrspell-exec
					    "-a" "-I" "-d" flag-d-path
					    "-f" flag-f-path)))
     ((and (not flag-I) flag-d flag-f)
      (setq ocrspell-process (start-process "ocrspell" "*ocrspell*" 
					    ocrspell-exec
					    "-a" "-d" flag-d-path
					    "-f" flag-f-path)))
     ((and (not flag-I) (not flag-d) flag-f)
      (setq ocrspell-process (start-process "ocrspell" "*ocrspell*" 
					    ocrspell-exec
					    "-a" "-f" flag-f-path)))
     ((and (not flag-I) flag-d (not flag-f))
      (setq ocrspell-process (start-process "ocrspell" "*ocrspell*" 
					    ocrspell-exec
					    "-a" "-d" flag-d-path)))
     ((and flag-I (not flag-d) flag-f)
      (setq ocrspell-process (start-process "ocrspell" "*ocrspell*" 
					    ocrspell-exec
					    "-a" "-I" "-f" flag-f-path)))
     (t
      (setq ocrspell-process (start-process "ocrspell" "*ocrspell*" 
					    ocrspell-exec "-a"))))
    ;;change nil to "*ocrspell*"
    (message "ocrspell started...")
    (process-kill-without-query ocrspell-process)
    (buffer-flush-undo (process-buffer ocrspell-process))
    ;; uncomment next 5 lines for buffer/pipe toggle
    ;;(accept-process-output ocrspell-process)
    ;;(set-buffer (process-buffer ocrspell-process))
    ;;(bury-buffer (current-buffer))
    ;;(setq last-char (- (point-max) 1))
    ;;(delete-region (point-min) last-char)
    (setq ocrspell-filter nil)
    (setq ocrspell-filter-continue nil)
    (set-process-filter ocrspell-process 'ocrspell-filter)
    (accept-process-output ocrspell-process)
    (message "%s" (car ocrspell-filter))
    (setq ocrspell-filter nil)
    (message "startup procedure complete"))
)



(defun allow-confusion-chars-p (word1 word2)
  "Checks to see whether a longest substring subprocess is
necessary"
  (save-match-data 
    (string-match allowable-confusion-chars word1)
    (cond ((equal (match-end 0) (length word1))
	   (string-match allowable-confusion-chars word2)
	   (cond ((equal (match-end 0) (length word2))
		  t)
		 (t
		  nil)))
	  nil)
    )
)
	
     

(defun match-list (s1 s2)
  "Perform a regular expression search on string s2 with 
expression s1, allow for any number of wildcards"
  (cond ((and (null s1) (null s2)) t)

        ((or (null s1) (null s2)) nil)

        ((equal (car s1) (car s2))
         (match-list (cdr s1) (cdr s2)))

        ((equal  (car s1) '*)

         (cond ((match-list (cdr s1) (cdr s2)))
               ((match-list s1 (cdr s2))))))
)



(defun remove-duplicates (confusion-list)
  "Remove all duplicate confusions from the global confusion matrice
list in a fast efficient manner"
  (let ((working confusion-list)
	(temp nil))
	
	(while (not (null (car working)))
	  (if (and (not (match-list (list '*  (car working)
					  )  working))
		   (not (match-list (list '*  (car working) '*
					  )  working)))
	      (setq temp (append temp (list (car working)))))
	  
	  (setq working (cdr working)))
	(setq global-ocrspell-dynamic-confusions temp))
)



(defun start-lcs2cnf (string1 string2)
  "start lcs2cnf subprocess to determine the longest common substring
between string1 and string2"
  (interactive)
  (setq ocrspell-dynamic-confusions nil)
  (message "Starting an lcs2cnf process...")

  (let ((buf (get-buffer-create "*lcs2cnf*"))))
  (set-buffer "*lcs2cnf*")
  (erase-buffer)

  (setq lcs2cnf-process (start-process "lcs2cnf" "*lcs2cnf*" 
				       lcs2cnf-exec
                                       string1 string2))
  (message "lcs2cnf started...")
  (process-kill-without-query lcs2cnf-process)
  (buffer-flush-undo (process-buffer lcs2cnf-process))
  (accept-process-output lcs2cnf-process)
  (sit-for 2)
  (let ((start 0)
	(end 0)
	(working-window nil)
	(bufferstring nil))
    (setq working-window (current-buffer))
    (set-buffer (process-buffer lcs2cnf-process))
    (setq bufferstring (buffer-string))
    (setq start 0)
    ;; allow for special characters
    (while (not (equal (string-match (concat allowable-confusion-chars
					     (regexp-quote " -> ")
					     allowable-confusion-chars) 
				     bufferstring start)
		       nil))
      
      (setq start (match-beginning 0))
      (setq end (match-end 0))
      
      (setq ocrspell-dynamic-confusions (append ocrspell-dynamic-confusions
						(list (list (substring 
							     bufferstring 
							     start end)))))
      (setq start (+ 1 end)))
    
    (setq ocrspell-dynamic-confusions (cdr ocrspell-dynamic-confusions))
    (setq global-ocrspell-dynamic-confusions 
	  (append global-ocrspell-dynamic-confusions
		  ocrspell-dynamic-confusions))
    
    (remove-duplicates global-ocrspell-dynamic-confusions)
    (setq ocrspell-dynamic-confusions nil)
    (set-buffer working-window))
)




(defun ocrspell-filter (process output)
  "Output filter function for ocrspell"
  (let ((start 0)
	(continue t)
	end)

    (while continue
      (setq end (string-match "\n" output start)) ; get text up to the newline.
      (if (and ocrspell-filter-continue ocrspell-filter 
	       (listp ocrspell-filter))  
	  (setcar ocrspell-filter
		  (concat (car ocrspell-filter) (substring output start end)))

	(setq ocrspell-filter
	      (cons (substring output start end) ocrspell-filter)))
      (if (null end)	
	  (setq ocrspell-filter-continue t continue nil)

	(setq ocrspell-filter-continue nil end (1+ end))
	(if (= end (length output))	
	    (setq continue nil)	       
	  (setq start end)))))
)




(defun ocrspell-parse-output (output)
  "Parse the OUTPUT string of 'ocrspell' and return:
1: T for an exact match.
2: A list of possible correct spellings of the format:
   '(\"original-word\" offset miss-list guess-list)
   original-word is a string of the possibly misspelled word.
   offset is an integer giving the line offset of the word.
   miss-list and guess-list are possibly null lists of guesses and misses."
  (cond
   ((string= output "") t)		; for startup with pipes

   ((string= output "*") t)		; exact match
      
   (t					; need to process &, ?, and #'s
    (let ((type (substring output 0 1))	; &, ?, or #
	  (original-word (substring output 2 (string-match " " output 2)))
	  (cur-count 0)			; contains number of misses + guesses
	  count miss-list guess-list offset)
      (setq output (substring output (match-end 0))) ; skip over misspelling

      (if (string= type "#")
	  (setq count 0)		; no misses for type #
	(setq count (string-to-int output) ; get number of misses.
	      output (substring output (1+ (string-match " " output 1)))))
      (setq offset (string-to-int output))

      (if (string= type "#")		; No miss or guess list.
	  (setq output nil)
	(setq output (substring output (1+ (string-match " " output 1)))))

      (while output
	(let ((end (string-match ", \\|\\($\\)" output))) ; end of miss/guess.
	  (setq cur-count (1+ cur-count))
	  (if (> cur-count count)
	      (setq guess-list (cons (substring output 0 end) guess-list))
	    (setq miss-list (cons (substring output 0 end) miss-list)))
	  (if (match-end 1)		; True only when at end of line.
	      (setq output nil)		; no more misses or guesses
	    (setq output (substring output (+ end 2))))))

      (list original-word offset miss-list guess-list))))
)



(defun apply-dynamic-confusions (string1)
  "Apply all of the confusions in the global confusion matrice list
to word string1, checking to see if the newly formed word is in fact
a correctly spelled word."
  (cond
   ((< (length string1) 3) ; Don't try if small word, too many possibilities
    nil)

   (t
    (setq working global-ocrspell-dynamic-confusions)
    (setq generated-word-list nil)

    (while (not (null (car working)))
      (setq confusion (car (car working)))
      (string-match "->" confusion)
      (setq generated-chars (substring confusion 0 (- (match-beginning 0) 1)))
      (setq correct-chars (substring confusion (+ (match-end 0) 1)
				     (length confusion)))
      (setq spoint 0)
      (while (and (/= (length generated-chars) 0)
	      (not (equal (string-match (regexp-quote generated-chars)
				       string1 spoint) 
			 nil)))

	(setq spoint (match-beginning 0))
	(setq epoint (match-end 0))

	(setq new-word (concat (substring string1 0 spoint)
			       correct-chars (substring string1 epoint 
							(length string1))))
	(if (and 
	     (equal (spell-string new-word) (concat new-word " is correct"))
	     (not (tilda-in-word new-word))) 
	    (setq generated-word-list
		  (append generated-word-list
			  (list (concat (substring string1 0 spoint)
					correct-chars 
					(substring string1 epoint
						   (length string1)))))
		  )
	  )
	
	(setq spoint epoint))

      (setq working (cdr working)))
    (message "new words %s to %s" string1 generated-word-list)
    generated-word-list)
   )
)



;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; Display the current global confusion matrice at each evaluation of 
; ocrspell.el
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

(message "global-dynamic :%s" global-ocrspell-dynamic-confusions)


(defun confusion-matrix-gen (string1 string2)
  "Implementation of explode and implode for confusion matrix
generation"
  (interactive)
  (let ((correct-word nil)
	(generated-word '())
	(common-substrings '())
	(i 0))
	(cond
	 ((string= string1 string2) nil)
	 
	 (t
	  (setq i 0)
	  (while (< i (length string1))
	    (setq generated-word 
		  (append generated-word (list 
					  (car 
					   (read-from-string string1 
							     i (+ i 1))))))
	    (setq i (+ 1 i)))
	  
	  (setq i 0)
	  (while (< i (length string2))
	    (setq correct-word 
		  (append correct-word (list 
					(car 
					 (read-from-string string2 
							   i (+ i 1))))))
	    (setq i (+ 1 i)))
	  
	  (message "%s" generated-word)
	  (message "%s" correct-word))))
)



(defun query-special-word (queried-word word-list)
  "Looks for words with special characters that were accepted
interactively by the user"
  (cond
   ((null word-list)
    nil)
   ((equal queried-word (car word-list))
    't)
   (t
    (query-special-word queried-word (cdr word-list))))
)



(defun insert-special-word (insert-word)
  "Inserts words with special characters that were accepted
interactively by the user"
  (setq ocrspell-special-insertion
	(append ocrspell-special-insertion (list insert-word)))
)



(defun check-hyphenated-word (hypho-word)
  "Checks if all the components of a hyphenated word are correct, if
so the word is correct"
  (cond
   ((equal (string-match "-" hypho-word) nil)
    nil)
   ((tilda-in-word hypho-word)
    nil)

   (t
    (let* ((wrd-start 0)
	  (wrd-end 0)
	  (mspelled 1)
	  (wrd-end (string-match "-" hypho-word))
	  (check-wrd (substring hypho-word wrd-start wrd-end)))
      (if (not (equal (spell-string check-wrd) 
		      (concat check-wrd " is correct")))
	  (setq mspelled 0))
      (setq wrd-start (+ 1 wrd-end))
      (setq wrd-end (length hypho-word))
      (setq check-wrd (substring hypho-word wrd-start wrd-end)) 
      
      (if (not (equal (spell-string check-wrd) 
		      (concat check-wrd " is correct")))
	  (setq mspelled 0))
      (cond
       ((equal mspelled 1)
	(not nil))
       (t
	nil))
      )))
)
  


(defun ocrspell-word (&optional following quietly)
  "Check the spelling of the word under the cursor.  See 'ocrspell'
for more documentation."
  (interactive)
  (if (interactive-p)
      (setq following ocrspell-following-word
	    quietly ocrspell-quietly))

  (let ((cursor-location (point))
	ocrspell-keep-choices-win
	(word (ocrspell-get-word following))
	start end poss replace)
    (setq start (car (cdr word))
	  end (car (cdr (cdr word)))
	  word (car word))

    (message "Checking spelling of %s..." word)
    (start-ocrspell)
    (process-send-string ocrspell-process (concat word "\n"))
    (message "beggining parsing...")
    (accept-process-output ocrspell-process)
    (message "Done parsing...")
    (message "poss: %s" (car ocrspell-filter))
    (if (listp ocrspell-filter)
	(setq poss (ocrspell-parse-output (car ocrspell-filter))))
    (message "possibilities: %s" poss)

    (cond ((eq poss t)
	   (or quietly
	       (message "%s is correct." word)))
	  ((stringp poss)
	   (or quietly
	       (message "%s is correct because of root %s"
			word
			poss)))
	  ((null poss) (message "Error in ocrspell process"))
	  (t				; prompt for correct word.
	   (unwind-protect
	       (progn
		 (ocrspell-highlight-spelling-error start end t)
		 (setq replace (ocrselect-word-gui
				(car (cdr (cdr poss)))
				(car (cdr (cdr (cdr poss))))
				(car poss))))
	     ;; will query user on choices
	     
	     (ocrspell-highlight-spelling-error start end))
	   (cond ((equal 0 replace) ;; insert to ispell-add-per-file-word-list
		  (ispell-add-per-file-word-list (car poss)))
		 (replace
		  (delete-region start end)
		  (setq word (if (atom replace) replace (car replace))
			cursor-location (+ (- (length word) (- end start))
					   cursor-location))

		  (insert word)
		  (if (not (atom replace)) ; recheck spelling of replacement
		      (progn
			(goto-char cursor-location)
			(ocrspell-word following quietly)))))
	   (if (get-buffer ocrspell-choices-buffer)
	       (kill-buffer ocrspell-choices-buffer))))
    (goto-char cursor-location)		; return to original location

    (kill-ocrspell)
    (if ocrspell-quit (setq ocrspell-quit nil)))
)



(defun gen-empty (x)
  "Generates the empty string of length x"
  (let ((s ""))
    (while (> x 0)
      (setq s (concat s " "))
      (setq x (- x 1))
      )
    s)
)


(defun remove-html-specials-tags (string1)
  "Skip all tags <...> in the buffer when ocrspelling an html document"
  (cond
   ((string-match (concat (regexp-quote "<") "[^<>]*" 
			  (regexp-quote ">")) string1)
    (setq beggining (match-beginning 0))
    (setq s-end (match-end 0))
    (setq string1
	  (concat (substring string1 
			     0 beggining) (gen-empty (- s-end beggining))
			     (substring string1
					s-end (length string1))))
    (remove-html-specials-tags string1))
   (t
    (remove-html-special-characters string1)))
)



(defun remove-html-special-characters (string1)
  "Skip all special character sequences in the buffer when ocrspelling an html
document"
  (cond
   ((string-match (concat (regexp-quote "&") "[^&;]*" 
			  (regexp-quote ";")) string1)
    (setq beggining (match-beginning 0))
    (setq s-end (match-end 0))
      (setq string1
	    (concat (substring string1 
			       0 beggining) (gen-empty (- s-end beggining))
			       (substring string1
					  s-end (length string1))))
      (remove-html-special-characters string1))
   (t
    string1))
)



(defun ocrspell-remove-numbers (string1)
  "Skip all numbers in the buffer when ocrspelling a document"
  (cond
   ((string-match (concat (regexp-quote " ") "?" "[^a-zA-Z]?" "[0-9]+"
			  "[^a-zA-Z]+" (regexp-quote " ")"?") string1)
    (setq beggining (match-beginning 0))
    (setq s-end (match-end 0))
    (setq string1
	  (concat (substring string1 
			     0 beggining) (gen-empty (- s-end beggining))
			     (substring string1
					s-end (length string1))))
    (ocrspell-remove-numbers string1))
   (t
    string1))
)



(defun ocrspell-hyph-whitespace-r (string)
  "Strips out the unquery-able special characters"
  (cond ((string-match (regexp-quote "-") string)
	 (let* ((beggining (match-beginning 0))
		(s-end (match-end 0))
		(string
		 (concat (substring string 
				    0 beggining) " "
				    (substring string
					       s-end (length string)))))
	   (ocrspell-hyph-whitespace-r string)))
	(t
	 string))
)



(defun ocrspell-skip-number-words-only (string1)
  "Skip only numbers in the buffer that do do occur within a word boundary
   There are three posibilites:
   1: A pure number word seperated by white space
   2: A pure number word at the beggining of a line
   3: A pure number word at the end of a line"
  (interactive)
  (cond
   ;; Remove pure number word at the beggining of line, if present
   ((string-match  (concat "^[0-9]+" (regexp-quote " ")) string1)
    (setq beggining (match-beginning 0))
    (setq s-end (match-end 0))
    (setq string1
	  (concat (substring string1 
			     0 beggining) (gen-empty (- s-end beggining))
			     (substring string1
					s-end (length string1))))
    (ocrspell-skip-number-words-only string1))
   ;; Remove pure number word at the end of line, if present
   ((string-match  (concat "^[0-9]+" (regexp-quote " ")) string1)
    (setq beggining (match-beginning 0))
    (setq s-end (match-end 0))
    (setq string1
	  (concat (substring string1 
			     0 beggining) (gen-empty (- s-end beggining))
			     (substring string1
					s-end (length string1))))
    (ocrspell-skip-number-words-only string1))
   ;; Remove pure number word at the end of line, if present
   ((string-match (concat (regexp-quote " ") "[0-9]+$") string1)
    (setq beggining (match-beginning 0))
    (setq s-end (match-end 0))
    (setq string1
	  (concat (substring string1 
			     0 beggining) (gen-empty (- s-end beggining))
			     (substring string1
					s-end (length string1))))
    (ocrspell-skip-number-words-only string1))
   ;; Remove pure number words seperated by white space, if present
   ((string-match (concat (regexp-quote " ") "[0-9]+" (regexp-quote " ")) 
		  string1)
    (setq beggining (match-beginning 0))
    (setq s-end (match-end 0))
    (setq string1
	  (concat (substring string1 
			     0 beggining) (gen-empty (- s-end beggining))
			     (substring string1
					s-end (length string1))))
    (ocrspell-skip-number-words-only string1))    
   (t
    string1))
)




(defun special-chars-in-word-tilda (string)
  "Strips out the unquery-able special characters"
  (cond ((string-match (regexp-quote "~") string)
	 (let* ((beggining (match-beginning 0))
		(s-end (match-end 0))
		(string
		 (concat (substring string 
				    0 beggining) "1"
				    (substring string
					       s-end (length string)))))
	   (special-chars-in-word-tilda string)))
	(t
	 string))
)



(defun special-chars-in-word-p (string)
  "Checks to see if the word has any special characters, which
do not occur in the dictionary"
  (let* ((striped-string (special-chars-in-word-tilda string)))
    (cond ((string-match "\\Sw" striped-string)
	 t)
	(t
	 nil)))
)
   


(defun tilda-in-word (word)
  "Checks to see if the word has an unrecognized ocr character,
represented as a '~'"
  (cond ((string-match (regexp-quote "~") word)
	 t)
	(t
	 nil))
)


(defun filter-tilda-lookup(word-list size tilda-count)
  "Filter out all words where:
 |new word length| > |old word length| + (2 * |number of tildas|)
all other generated words are appended to the choices list"
  (let ((the-new-list nil)
	(the-max-tilda-choices 0)
	(total-max-tilda-choices 30))

    (while (and (not (null word-list))
		(/= the-max-tilda-choices total-max-tilda-choices))
      (cond 
       ((< (length (car word-list)) (+ size (* tilda-count 2)))
	(setq the-new-list (append the-new-list
				   (list (car word-list))))
	(setq the-max-tilda-choices (+ 1 the-max-tilda-choices)))
       (t
	nil))

      (setq word-list (cdr word-list))
      )
    the-new-list)
)



(defun ocrspell-tilda-handler (tilda-word)
  "Calls the tilda handling routines"
  (cond
   ((string-match (concat "[^" (regexp-quote "~") "]+") tilda-word)
    (setq tilda-count 0)
    (ocrspell-tilda-handler-r tilda-word))
   (t
    ;; DON'T TRY TO LOOKUP TILDA ONLY WORDS
    nil))
)



(defun ocrspell-tilda-handler-r (tilda-word)
  "Generates possibilities for words with ~'s.  The list is later filtered
and appended to the choices list"
   (cond
   ((string-match (regexp-quote "~") tilda-word)
    (setq beggining (match-beginning 0))
    (setq s-end (match-end 0))
    (setq tilda-count (+ 1 tilda-count))
    (setq tilda-word
	  (concat (substring tilda-word 
			     0 beggining) "*"
			     (substring tilda-word
					s-end (length tilda-word))))
    (ocrspell-tilda-handler-r tilda-word))
   (t
    (filter-tilda-lookup (lookup-words tilda-word) (length tilda-word) 
			 tilda-count)))
)



(defun ocrspell-overlay-window (height)
  "Create a (usually small) window covering the top HEIGHT lines of the
current window. Ensure that the line above point is still visible but
otherwise avoid scrolling the current window.  Should leave the old
window selected."
(interactive)
  (save-excursion

    (let ((oldot (save-excursion (forward-line -1) (point)))
	  (top (save-excursion (move-to-window-line height) (point))))

      (if (< oldot top) (setq top oldot))
      (if (string-match "19\.9.*Lucid" (emacs-version))
	  (setq height (1+ height)))
      (split-window nil height)
      (set-window-start (next-window) top)))
)




(defun ocrspell-highlight-spelling-error (start end &optional highlight)
  "Highlight the word from START to END by deleting and reinserting it
while toggling the variable \"inverse-video\".  When the optional
third arg HIGHLIGHT is set, the word is highlighted otherwise it is
displayed normally."
(interactive)

  (let ((modified (buffer-modified-p))	; don't allow this fn to modify buffer
	(buffer-read-only nil)		; Allow highlighting read-only buffers.
	(text (buffer-substring start end)) ; Save highlight region
	(inhibit-quit t)		; inhibit interrupt processing here.
	(buffer-undo-list nil))		; don't clutter the undo list.

    (delete-region start end)
    (insert-char ?  (- end start))	; mimimize amount of redisplay
    (sit-for 0)				; update display
    (if highlight (setq inverse-video (not inverse-video))) ; toggle video

    (delete-region start end)		; delete whitespace
    (insert text)			; insert text in inverse video.
    (sit-for 0)				; update display showing inverse video.
    (if highlight (setq inverse-video (not inverse-video))) ; toggle video

    (set-buffer-modified-p modified))	; don't modify if flag not set.
)






(defun perform-stemming-on-list-of-objects (unstemmed-list new-stem)
  "This function takes an unstemmed-list of words as objects
applies a new stem to each of the words and returns the list"
  (cond
   ((null unstemmed-list)
    nil)
   (t
    (let* ((end-of-word
	    (substring (prin1-to-string (car unstemmed-list))
		       -1 nil)))
      (cond
       ((or
	 (equal end-of-word "a")
	 (equal end-of-word "e")
	 (equal end-of-word "i")
	 (equal end-of-word "o")
	 (equal end-of-word "u"))

	(append (list (car (read-from-string
			    (concat
			     (substring (prin1-to-string (car unstemmed-list))
					0 -1)
			     new-stem))))
		(perform-stemming-on-list (cdr unstemmed-list) new-stem)))
       (t
	(append (list (car (read-from-string
			    (concat
			     (prin1-to-string (car unstemmed-list))
			     new-stem))))
		(perform-stemming-on-list (cdr unstemmed-list) new-stem)))))))
)




(defun perform-pluralization-on-list-of-objects (unstemmed-list)
  "This function takes an unstemmed-list of words as objects and pluralizes
each of the words and returns the list"
  (cond
   ((null unstemmed-list)
    nil)
   (t
    (let* ((end-of-word
	    (substring (prin1-to-string (car unstemmed-list))
		       -1 nil)))
      (cond
       ((or
	 (equal end-of-word "a")
	 (equal end-of-word "e")
	 (equal end-of-word "i")
	 (equal end-of-word "o")
	 (equal end-of-word "u")
	 (< (length (prin1-to-string (car unstemmed-list))) 4))
	
	(append (list (car (read-from-string
			    (concat
			     (prin1-to-string (car unstemmed-list))
			     "s"))))
		(perform-pluralization-on-list (cdr unstemmed-list))))
       ((equal end-of-word "s")
	(append (list (car (read-from-string
			    (concat
			     (substring 
			      (prin1-to-string (car unstemmed-list)) 0 -1)
			     "es"))))
		(perform-pluralization-on-list (cdr unstemmed-list))))
       ((equal end-of-word "y")
	(append (list (car (read-from-string
			    (concat
			     (substring
			      (prin1-to-string (car unstemmed-list)) 0 -1)
			     "ies"))))
		(perform-pluralization-on-list (cdr unstemmed-list))))
       (t
	(append (list (car (read-from-string
			    (concat
			     (prin1-to-string (car unstemmed-list))
			     "s"))))
		(perform-pluralization-on-list 
		 (cdr unstemmed-list))))))))
)



(defun ocrspell-strip-tildas (string)
  "Strips out all ~'s and replaces with *'s"
  (cond ((string-match (regexp-quote "~") string)
	 (let* ((beggining (match-beginning 0))
		(s-end (match-end 0))
		(string
		 (concat (substring string 
				    0 beggining) "*"
				    (substring string
					       s-end (length string)))))
	   (ocrspell-strip-tildas string)))
	(t
	 string))
)




(defun perform-stemming-on-list (unstemmed-list new-stem)
  "This function takes an unstemmed-list of words applies a new stem to
each of the words and returns the list"
  (cond
   ((null unstemmed-list)
    nil)
   (t
    (let* ((end-of-word
	    (substring (car unstemmed-list)
		       -1 nil)))
      (cond
       ((or
	 (equal end-of-word "a")
	 (equal end-of-word "e")
	 (equal end-of-word "i")
	 (equal end-of-word "o")
	 (equal end-of-word "u"))

	(append (list (concat
			     (substring (car unstemmed-list)
					0 -1)
			     new-stem))
		(perform-stemming-on-list (cdr unstemmed-list) new-stem)))
       (t
	(append (list 
		 (concat
		  (car unstemmed-list)
		  new-stem))
		(perform-stemming-on-list 
		 (cdr unstemmed-list) new-stem)))))))
)




(defun perform-pluralization-on-list (unstemmed-list)
  "This function takes an unstemmed-list of words and pluralizes
each of the words and returns the list"
  (cond
   ((null unstemmed-list)
    nil)
   (t
    (let* ((end-of-word
	    (substring (car unstemmed-list)
		       -1 nil)))
      (cond
       ((or
	 (equal end-of-word "a")
	 (equal end-of-word "e")
	 (equal end-of-word "i")
	 (equal end-of-word "o")
	 (equal end-of-word "u")
	 (< (length (car unstemmed-list)) 4))
	
	(append (list (concat
		       (car unstemmed-list)
		       "s"))
		(perform-pluralization-on-list (cdr unstemmed-list))))
       ((equal end-of-word "s")
	(append (list (concat
		       (substring 
			(car unstemmed-list) 0 -1)
		       "es"))
		(perform-pluralization-on-list (cdr unstemmed-list))))
       ((equal end-of-word "y")
	(append (list (concat
		       (substring
			(car unstemmed-list) 0 -1)
		       "ies"))
		(perform-pluralization-on-list (cdr unstemmed-list))))
       (t
	(append (list (concat
		       (car unstemmed-list)
		       "s"))
		(perform-pluralization-on-list 
		 (cdr unstemmed-list))))))))
)



(defun perform-spelling-correctness-for-stem-r (unstemmed-word)
  "This function is passed an unstemmed word and returns nil if it
is not in the dictionary"
  (cond
   ((equal (spell-string unstemmed-word) (concat unstemmed-word 
						 " is correct"))
    (not nil))
   (t
    nil))
)



(defun perform-stemming (stemmed-word)
  "This function takes a stemmed word and heuristically makes stemming
substitutions in order of likelihood, returning the unstemmed word
if found or the original word if no probable stem was found."
  (cond
   ((perform-spelling-correctness-for-stem-r stemmed-word)
    stemmed-word)

   ((< (length stemmed-word) 4)
    stemmed-word)

   (t
    (let ((word-length (length stemmed-word))
	  (possible-stem nil)
	  (unstemmed-word stemmed-word)
	  (end-of-word1 (substring stemmed-word -2 nil))
	  (end-of-word2 (substring stemmed-word -3 nil)))

      (cond
       ((or
	 (equal end-of-word1 "er")
	 (equal end-of-word1 "ly")
	 (equal end-of-word1 "ed"))
	(setq unstemmed-word (substring stemmed-word 0 -2))
	(if (not (perform-spelling-correctness-for-stem-r unstemmed-word))
	    (setq unstemmed-word (concat unstemmed-word "e")))
	(if (not (perform-spelling-correctness-for-stem-r unstemmed-word))
	    (setq unstemmed-word stemmed-word)))

       ((and (equal unstemmed-word stemmed-word) 
	     (or
	      (equal end-of-word2 "ing")
	      (equal end-of-word2 "ize")
	      (equal end-of-word2 "est")
	      (equal end-of-word2 "eer")))
	(setq unstemmed-word (substring stemmed-word 0 -3))
	
	(if (not (perform-spelling-correctness-for-stem-r unstemmed-word))
	    (setq unstemmed-word (concat unstemmed-word "e")))
	(if (not (perform-spelling-correctness-for-stem-r unstemmed-word))
	    (setq unstemmed-word stemmed-word)))
       (t
	nil))
      unstemmed-word)))
)



(defun perform-stemming-lookup (stemmed-word)
  "This function takes a stemmed word and heuristically makes stemming
substitutions in order of likelihood, returning the unstemmed word
if the choice generates possibilities or the original word if no probable 
stem was found."  
  (cond
   ((lookup-words stemmed-word)
    stemmed-word)

   ((< (length stemmed-word) 4)
    stemmed-word)

   (t
    (let ((word-length (length stemmed-word))
	  (possible-stem nil)
	  (unstemmed-word stemmed-word)
	  (stem-used nil)
	  (end-of-word1 (substring stemmed-word -2 nil))
	  (end-of-word2 (substring stemmed-word -3 nil)))

      (cond
       ((or
	 (equal end-of-word1 "er")
	 (equal end-of-word1 "ly")
	 (equal end-of-word1 "ed"))
	(setq unstemmed-word (substring stemmed-word 0 -2))
	(setq stem-used (substring stemmed-word -2))

	(if (not (lookup-words unstemmed-word))
	    (setq unstemmed-word (concat unstemmed-word "e")))
	(if (not (lookup-words unstemmed-word))
	    (setq unstemmed-word stemmed-word)))
       ((and (equal unstemmed-word stemmed-word) 
	     (or
	      (equal end-of-word2 "ing")
	      (equal end-of-word2 "ize")
	      (equal end-of-word2 "est")
	      (equal end-of-word2 "eer")))
	(setq unstemmed-word (substring stemmed-word 0 -3))
	(setq stem-used (substring stemmed-word -3))

	(if (not (lookup-words unstemmed-word))
	    (setq unstemmed-word (concat unstemmed-word "e")))
	(if (not (lookup-words unstemmed-word))
	    (setq unstemmed-word stemmed-word)))
       (t
	nil))
      (list unstemmed-word stem-used))))
)



(defun affix-queried-word-capitalization (queried-word generated-word-list)
  "This function returns capitalizes each term in generated-word-list
 and returns the new list"
  (cond
   ((null generated-word-list)
    nil)
   (t
    (append (list (car (read-from-string
		  (capitalize (prin1-to-string (car generated-word-list)))))) 
	    (affix-queried-word-capitalization queried-word 
					       (cdr generated-word-list))))) 
	    
)


 
(defun queried-word-capital (c-queried-word)
  "This function returns nil if either the first character of the word
is not an alphabet character or it is a lower case alphabet character"
  (cond
   ((equal (substring c-queried-word 0 1)
	   (downcase  (substring c-queried-word 0 1)))
    nil)

   ((save-match-data
      (string-match "[A-Z]+" (substring c-queried-word 0 1)))
    (not nil))
   (t
    nil))
)
   
  

(defun capital-form-handler-r (original-word generated-word-list)
  "This function checks to see if the original-word is capitalized.  If
it is then each of the words in the generated-word-list is capiatlized"
  (cond
   ((null generated-word-list)
    nil)

   ((queried-word-capital original-word)
    (affix-queried-word-capitalization original-word generated-word-list))

   (t
    generated-word-list))    
)



(defun plural-form-handler-r (original-word)
  "This function examines the original-word.  If the original-word 
appears to be plural, the plural extension of the original word is 
removed and the word is queried again.  If this is successful, the 
new word is returned.  We assume that irregular plurals are going to be in
the dictionary anyway (like opus/opera, cactus/cacti)."
  (cond
   
   ((ocrspell-tilda-handler-r original-word)
    original-word)

   (t
    (let ((word-length (length original-word))
	  (possible-plural nil)
	  (singular-form original-word))
      (cond
       ((and (> word-length 4) (equal (substring original-word -1) "s"))
	(if (equal (substring original-word -3) "ies")
	    (let ((l-token (substring original-word -4 -3)))

	      (cond ((or
		      (equal l-token "a")
		      (equal l-token "e")
		      (equal l-token "i")
		      (equal l-token "o")
		      (equal l-token "u"))
		     nil)

		    (t
		     (setq singular-form (concat 
					  (substring original-word 0 -3)
					  "y"))
		     
		     (if (not (ocrspell-tilda-handler-r singular-form))
			 (setq singular-form (concat
					      (substring original-word 0 -3)
					      "ie")))
		     (if (not (ocrspell-tilda-handler-r singular-form))
			 (setq singular-form original-word))))))
	 
	(if (equal singular-form original-word)
	    (let ((l-token (substring original-word -4 -2)))
	      (if (or
		   (equal l-token "ss")
		   (equal l-token "th")
		   (equal l-token "ch")
		   (equal l-token "sh")
		   (equal (substring l-token 1 2) "x"))
		  (setq singular-form (substring
				       original-word 0 -2)))))

	(if (not (ocrspell-tilda-handler-r singular-form))
	    (setq singular-form original-word))

	(if (equal singular-form original-word)
	    (setq singular-form (concat
				 (substring
				  original-word
				  0 -2) "e")))
	
	(if (not (ocrspell-tilda-handler-r singular-form))
	    (setq singular-form original-word))
      
	(if (equal singular-form original-word)
	    (setq singular-form (substring
				 original-word 0 -1)))
      
	(if (not (ocrspell-tilda-handler-r singular-form))
	    (setq singular-form original-word))

    singular-form)

   (t
    nil)))))
)



(defun ocrspell-get-word (following &optional extra-otherchars)
  "Return the word for spell-checking according to ocrspell syntax.
If optional argument FOLLOWING is non-nil or if ocrspell-following-word
is non-nil when called interactively, then the following word
\(rather than preceeding\) will be checked when the cursor is not over a word.
Optional second argument contains otherchars that can be included in word
many times.

Word syntax described by ispell-dictionary-alist (which see)."
(interactive)

  (let* ((ocrspell-casechars (ispell-get-casechars))
	 (ocrspell-not-casechars (ispell-get-not-casechars))
	 (ocrspell-otherchars (ispell-get-otherchars))
	 (ocrspell-many-otherchars-p (ispell-get-many-otherchars-p))
	 (word-regexp (concat ocrspell-casechars
			      "+\\("
			      ocrspell-otherchars
			      "?"
			      (if extra-otherchars
				  (concat extra-otherchars "?"))
			      ocrspell-casechars
			      "+\\)"
			      (if (or ocrspell-many-otherchars-p
				      extra-otherchars)
				  "*" "?")))
	 did-it-once
	 start end word)

    ;; find the word
    (if (not (looking-at ocrspell-casechars))
	(if following
	    (re-search-forward ocrspell-casechars (point-max) t)
	  (re-search-backward ocrspell-casechars (point-min) t)))
    ;; move to front of word
    (re-search-backward ocrspell-not-casechars (point-min) 'start)

    (while (and (or (looking-at ocrspell-otherchars)
		    (and extra-otherchars (looking-at extra-otherchars)))
		(not (bobp))
		(or (not did-it-once)
		    ocrspell-many-otherchars-p))
      (if (and extra-otherchars (looking-at extra-otherchars))
	  (progn
	    (backward-char 1)
	    (if (looking-at ocrspell-casechars)
		(re-search-backward ocrspell-not-casechars (point-min) 'move)))
	(setq did-it-once t)

	(backward-char 1)
	(if (looking-at ocrspell-casechars)
	    (re-search-backward ocrspell-not-casechars (point-min) 'move)
	  (backward-char -1))))
    ;; Now mark the word and save to string.

    (or (re-search-forward word-regexp (point-max) t)
	(error "No word found to check!"))
    (setq start (match-beginning 0)
	  end (point)
	  word (buffer-substring start end))
    (list word start end))
)



(defun is-in-choices-list-p (choices-word choices-list)
  "Checks to see if the choices-word is in the choices-list"
  (cond
   ((null choices-list)
    nil)
   ((equal (car choices-list) choices-word)
    t)
   (t
    (is-in-choices-list-p choices-word (cdr choices-list))))
)
 


(defun remove-duplicates-from-choices-list-r (current-choices-list)
  "Constructs a new choices list that contains no duplicates"
  (cond
   ((null current-choices-list)
    nil)
   ((is-in-choices-list-p (car current-choices-list)
			  (cdr current-choices-list))
    (remove-duplicates-from-choices-list-r (cdr current-choices-list)))
   (t
    (append (list (car current-choices-list))
	    (remove-duplicates-from-choices-list-r 
	     (cdr current-choices-list)))))
)



(defun ocrselect-word-gui (miss guess word)
  "Display the generated word list in a window and allow for user
quick key insertion.  Allow for all the standard features such as
insertion, replacement, etc. Generate dynamic confusion replacements."
  (setq temp (current-buffer))

  ;; Put word list back in statistically sorted order
  (setq miss (nreverse miss))

  ;; append words generated by dynamic confusions, if any
  (setq miss (append miss (apply-dynamic-confusions word)))

  ;; append words generated by tilda handler, if any
  ;; since the tilde handler doesn't make any character substitutions
  ;; other than ~->**, affix capitalization when appropriate
  (if (and (tilda-in-word word) (not (special-chars-in-word-p word))) 
      (setq miss (append miss 
			 (capital-form-handler-r word
			  (ocrspell-tilda-handler word)))))

  ;; Now, if no choices are delivered and the word contains 
  ;; at least one tilde, see if we can unstem the word to generate
  ;; choices.  If we can establish a stem to remove we will heuristically
  ;; append the stem to all the generated choices

  ;; Lets see if it is an unusual standard plural
  (if (and (tilda-in-word word) 
	  (not (special-chars-in-word-p word))
	  (null miss)
	  (save-match-data
	    (string-match 
	     (concat "[^" (regexp-quote "~") "]+") word))
	  (not (equal (plural-form-handler-r word) nil)))
      (setq miss (append miss 
			 (capital-form-handler-r 
			  word
			  (perform-pluralization-on-list
			   (ocrspell-tilda-handler 
			    (plural-form-handler-r word)))))))
      			 
  ;; if there is still no choices, lets try a very primitive stemming
  ;; process for suffixes, since this process tends to slow performance
  ;; make sure the ocrspell-dynamic-stem predicate is true as well
  (cond
   ((and ocrspell-dynamic-stem-p
	 (tilda-in-word word)
	 (not (special-chars-in-word-p word))
	 (null miss)
	 (save-match-data
	   (string-match 
	    (concat "[^" (regexp-quote "~") "]+") word))
	 (not (equal (perform-stemming-lookup 
		      (ocrspell-strip-tildas word)) nil)))
    (let* ((current-stemming-info (perform-stemming-lookup 
				   (ocrspell-strip-tildas word))))
      (if (listp current-stemming-info)
	  (progn
	    (let*
		((new-unstemmed-word (car current-stemming-info))
		 (stem-to-use (car (cdr current-stemming-info))))
      
	      (setq miss (append miss
				 (capital-form-handler-r
				  word
				  (perform-stemming-on-list
				   (lookup-words
				    new-unstemmed-word)
				   stem-to-use)))))))))
   (t
    nil))
  
 
  ;; next delete any duplicates from the list, although this is
  ;; statistically improbable, you never know.
  (setq miss (remove-duplicates-from-choices-list-r miss))


  (unwind-protect
      (save-window-excursion
	(let ((count ?0)
	      (line 2)
	      (max-lines (- (window-height) 4))
	      (choices miss)
	      (window-min-height (min window-min-height
				      ocrspell-choices-win-default-height))
	      ;; include all of ispell's command characters set to allow for
	      ;; future commands and to provide a nice consistancy between
	      ;; ocrspell's user interface and ispell's
	      (command-characters
	       '( ?  ?i ?j ?J ?b ?B ?a ?A ?r ?R ?? ?g ?G ?x ?X ?q ?l ?u ?m ))
	      (skipped 0)
	      char num result)

	  (save-excursion
	    (select-window (previous-window))
	    (set-buffer (get-buffer-create ocrspell-choices-buffer))
	    (erase-buffer)
	    ;; uncomment for debugging
	    ;;(setq last-char (- (point-max) 1))
	    ;;(delete-region (point-min) last-char)
	    (setq mode-line-format "--  %b  --")
	    ;;(insert "**" (car miss) "**\n")
	    ;;(insert "**" word "**\n")
	    (setq ch_counter 0)
	    (while (and choices
			(< (if (> (+ 7 (current-column) (length (car choices))
				     (if (> count ?~) 3 0))
				  (window-width))
			       (progn
				 (insert "\n")
				 (setq line (1+ line)))
			     line)
			   max-lines))

	      ;; not so good if there are over 20 or 30 options, but then, if
	      ;; there are that many you don't want to scan them all anyway.
	      ;; lets have complete ispell behavior...
	      (while (memq count command-characters) 
					; skip all ispell command characters.
		(setq count (1+ count)
		      skipped (1+ skipped)))
	      (insert "(" count ") " (car choices) "  ")
	      (setq choices (cdr choices)
		    count (1+ count))
	      (setq ch_counter (+ ch_counter 1)) 
	      ;; check for user specified limit on the number of choices
	      (if (and max-number-word-choices-p
		       (equal ch_counter max-number-word-choices))
		  (setq choices nil)))
	    (setq count (- count ?0 skipped)))

	  (progn
	    (switch-to-buffer ocrspell-choices-buffer)
	    (select-window (next-window))
	    (save-excursion
	      (let ((cur-point (point)))
		(move-to-window-line line)
		(if (<= (point) cur-point)
		    (set-window-start (selected-window) (point)))))

	    (select-window (previous-window))
	    (enlarge-window line)
	    (goto-char (point-min)))
	  (ocrspell-overlay-window (max line
				      ocrspell-choices-win-default-height))

	  (switch-to-buffer ocrspell-choices-buffer)
	  (goto-char (point-min))
	  (select-window (next-window))
	  (switch-to-buffer temp)
	  (while
	      (eq
	       t
	       (setq
		result
		(progn
		  (undo-boundary)
		  (message 
		   (concat
		    "SPC to leave "
		    "unchanged, Character to replace word [i,r,j,b,g,q]"))

		  (let ((inhibit-quit t))
		    (setq char (if (fboundp 'read-char-exclusive)
				   (read-char-exclusive)
				 (read-char)))
		    (if (or quit-flag (= char ?\C-g)) ; C-g is like typing q
			(setq char ?q
			      quit-flag nil)))

		  (setq skipped 0)
		  (let ((com-chars command-characters))
		    (while com-chars
		      (if (and (> (car com-chars) ?0) (< (car com-chars) char))
			  (setq skipped (1+ skipped)))
		      (setq com-chars (cdr com-chars)))
		    (setq num (- char ?0 skipped)))

		   (cond
		    ((= char ? ) nil)	; accept word this time only

		    ((or (= char ?j) (= char ?J)) ; join next word
		     'join-next-word)

		    ((or (= char ?b) (= char ?B)) ; join previous word
		     'join-previous-word)

		    ((= char ?i)	; accept and insert word into pers dict
		     (if (not (string-match (regexp-quote "/") word))
			 (process-send-string ocrspell-process
					      (concat "@" word "\n")))
		     (insert-special-word word)
		     nil)

		    ((or (= char ?r) (= char ?R)) ; type in replacement
		    (if (or (= char ?R) ocrspell-query-replace-choices)
			(list (read-string "Query-replacement for: " word) t)
		      (cons (read-string "Replacement for: " word) nil)))

		    ((or (= char ?g) (= char ?G)) ; global replacement
		     (if (or (= char ?g) ocrspell-query-replace-choices)
			(list (read-string "Query-replacement for: " word) t)
		      (cons (read-string "Replacement for: " word) nil)))


		    ((= char ?q)
		     (if (y-or-n-p "Really quit ignoring changes? ")
			(progn
			  (setq ocrspell-filter nil)
			  (setq cursor-in-echo-area nil); put cursor in buffer
			  (kill-ocrspell) ; terminate process.
			)
		      t))		; continue if they don't quit.    

		    ((and (>= num 0) (< num count))
		     (if ocrspell-query-replace-choices ; Query replace?
			 (list (nth num miss) 'query-replace)
		       (nth num miss))))))))
	  result))

    (if (not ocrspell-keep-choices-win) (bury-buffer ocrspell-choices-buffer)))
)




(defun ocrspell-region (reg-start reg-end)  
  "Interactively check a region for spelling errors."
  (interactive "r")
  (if (not (and ocrspell-process
		(eq (process-status ocrspell-process) 'run)))
      (cond
       (special-config-startup-p
	(start-ocrspell-with-special-configuration))
       (t
	(start-ocrspell))))

  (unwind-protect
      (save-excursion
	(message "Spelling %s..."
		 (if (and (= reg-start (point-min)) (= reg-end (point-max)))
		     (buffer-name) "region"))

	(get-buffer-create (concat (buffer-name) ".choices"))

	(sit-for 0)
	(save-window-excursion
	  (if nil        ;ocrspell-keep-choices-win 

	      (let ((ocb (current-buffer))
		    (window-min-height ocrspell-choices-win-default-height))
		(or (eq ocb (window-buffer (selected-window)))
		    (error
		     "current buffer is not visible in selected window: %s"
		     ocb))
		(setq ocrspell-keep-choices-win
		      ocrspell-choices-win-default-height)
		(ocrspell-overlay-window ocrspell-choices-win-default-height)
		(switch-to-buffer (get-buffer-create ocrspell-choices-buffer))
		(setq mode-line-format "--  %b  --")
		(erase-buffer)
		(select-window (next-window))
		(or (eq (current-buffer) ocb)
		    (error "ocrspell is confused about current buffer!"))
		(sit-for 0)))

	  (goto-char reg-start)
	  (message "starting to scan the buffer......")
	  (let ((transient-mark-mode nil))
	    (while (and (not ocrspell-quit) (< (point) reg-end))
	      (let ((start (point))
		    (offset-change 0)
		    (end (save-excursion (end-of-line) (min (point) reg-end)))
		    (ispell-casechars (ispell-get-casechars))
		    string)

		(cond	
		 ((eolp)
		  (forward-char 1))
		 ((and (null ocrspell-comment-check-p) ; SKIPING COMMENTS
		       comment-start	; skip comments that start on the line.
		       (search-forward comment-start end t)) ; or found here.
		  (if (= (- (point) start) (length comment-start))
		      ;; comment starts the line.  Skip entire line or region
		      (if (string= "" comment-end) ; skip to next line
			  (beginning-of-line 2)	; or jump to comment end.
			(search-forward comment-end reg-end 'limit))
		    ;; Comment later in line.  Check spelling before comment.
		    (let ((limit (- (point) (length comment-start))))
		      (goto-char (1- limit))
		      (if (looking-at "\\\\") ; "quoted" comment, don't skip
			  ;; quoted comment.  Skip over comment-start
			  (if (= start (1- limit))
			      (setq limit (+ limit (length comment-start)))
			    (setq limit (1- limit))))
		      (goto-char start)

		      ;; Only check when "casechars" or math before comment
		      (if (or (re-search-forward ispell-casechars limit t)
			      (re-search-forward "[][()$]" limit t))
			  (setq string
				(concat (buffer-substring start limit)
					"\n")))
		      (goto-char limit))))
		 
		 ((looking-at "[---#@*+!%^]") 
					; SKIP SPECIAL OCRSPELL CHARACTERS
		  (forward-char 1))

		 ((and (looking-at (concat (regexp-quote "<") "[^<>]*"
					   (regexp-quote ">")))
		       ocrspell-html-mode-p)
		  (goto-char (match-end 0)))
		  
		 ((or (re-search-forward ispell-casechars end t) ; TEXT EXISTS
		      (re-search-forward "[][()$]" end t)) ; or MATH COMMANDS
		  (setq string (concat (buffer-substring start end) "\n"))
		  (goto-char end))
		 (t (beginning-of-line 2))) ; EMPTY LINE, skip it.
		
		(setq end (point))	; "end" tracks end of region to check.
		(message "examing text stream...")
		(if (stringp string)
		    (message "ocrspelling... %s" string))

		;; if in html mode skip over all tags and special chars
		(setq send-string string)
		(save-match-data
		  (if (and ocrspell-html-mode-p string)
		      (setq send-string
			    (remove-html-specials-tags send-string))))
		(save-match-data
		  (if (and ocrspell-skipnumbers-mode-p string)
		      (setq send-string 
			    (ocrspell-remove-numbers send-string))))
		(save-match-data
		  (if (and ocrspell-skip-number-words-only-p string)
		      (setq send-string (ocrspell-skip-number-words-only
					 send-string))))

		(save-match-data
		  (if (and ocrspell-hyph-whitespace-p string)
		      (setq send-string (ocrspell-hyph-whitespace-r
					 send-string))))

		(if string          ; (stringp string) there is 
				    ; something to spell!
		    (let (poss)
		      ;; send string to spell process and get input.
		      (process-send-string ocrspell-process
					   (concat send-string " ^^^\n"))
		     (while (progn
			       (accept-process-output ocrspell-process)
			       ;;Last item of output contains a blank line.
			    (not (string= "" (car ocrspell-filter)))))
		      ;; parse all inputs from the stream one word at a time.
		      ;; Place in FIFO order and remove the blank item.
		      (setq ocrspell-filter (nreverse (cdr ocrspell-filter)))
		      (setq ocrspell-quit nil)
		      (setq parse-string 0)

		      (while (and (not ocrspell-quit) ocrspell-filter)
			(setq poss (ocrspell-parse-output
				    (car ocrspell-filter)))
			;(message "is this a word?: %s" (car poss))
			(if (and (listp poss) 
				 (not (query-special-word
				       (car poss) ocrspell-special-insertion))
				 (not (check-hyphenated-word (car poss))))
					   
					; spelling error occurred.
			    ; hack for emulation problems
			    (let* ((word-start (+ start offset-change 
						  (string-match
						   (concat 
						   (regexp-quote (car poss))
						   "[^A-Za-z]")
						   string parse-string)))
				   (word-end (+ word-start
						(length (car poss))))
				   replace)
			      (goto-char word-start)
			      ;; Adjust the horizontal scroll & point
			      (ocrspell-horiz-scroll)
			      (goto-char word-end)
			      (ocrspell-horiz-scroll)
			      (goto-char word-start)
			      (ocrspell-horiz-scroll)

			      (unwind-protect
				  (progn
				    (setq parse-string
					  (+ parse-string (length (car poss))))
				    ;(if (string-match "[^a-z0-9A-Z]" (car poss) (- (length (car poss)) 1))
					;(setq word-end (- word-end 1)))
				    (if ocrspell-highlight-p
					(ispell-highlight-spelling-error
					 word-start word-end t))
				    
				    (sit-for 0)	; update screen display
				    (message "spell checking %s" (car poss))
				    (setq replace (ocrselect-word-gui
						   (car (cdr (cdr poss)))
						   (car (cdr (cdr (cdr poss))))
						   (car poss))))
				  
				  (if ocrspell-highlight-p
				      (ispell-highlight-spelling-error
				       word-start word-end)))

			      (cond
			       
			       ((equal replace 'join-previous-word)
				;; Join Request.  See if join is possible,
				;; if so join and recheck WORD
				(setq replace nil)
				(let ((space-position word-start))
				  (while (equal (buffer-substring
						 space-position
						 (+ 1 space-position)) " ")
				    (setq space-position 
					  (+ space-position 1)))
				  (while (and
					  (not (bolp))
					  (not (bobp))
					  (not (equal (buffer-substring
						       space-position
						       (- space-position 1))
						      "\n"))
					  (not (equal (buffer-substring
						       space-position
						       (- space-position 1))
						      " ")))
				    (setq space-position
					  (- space-position 1))
				    (backward-char 1))
				  (cond 
				   ((and (not (bolp))
					 (not (equal 
					       (buffer-substring 
						space-position
						(- space-position 1)) "\n"))) 
				    (delete-region space-position
						   (- space-position 1))
				    (setq ocrspell-filter nil
					  string (buffer-substring 
						  space-position
						  word-end))
				    (setq reg-end (- reg-end 1)
					  offset-change (- offset-change 1)))
				   (t
				    (beep)
				    (message "illegal join request")
				    (sit-for 1)))
				  (while (and
					  (not (bolp))
					  (not (bobp))
					  (not (equal (buffer-substring 
						       (point) (+ 1 (point)))
						      " ")))
				    (backward-char 1))
				  (setq string (buffer-substring
						(point) word-end)
					word-start (point))
				  (setq end (point))))
			      
				    			       
			       ((equal replace 'join-next-word)
				;; Join Request.  See if join is possible,
				;; if so join and recheck WORD
				(setq replace nil)
				(let ((space-position word-end))
				  (while (and
					  (< (point) end)
					  (not (equal (buffer-substring
						       space-position
						       (+ space-position 1))
						      "\n"))
					  (not (equal (buffer-substring
						       space-position
						       (+ space-position 1))
						      " ")))
				    (setq space-position
					  (+ space-position 1))
				    (forward-char 1))
				  (cond 
				   ((and (< (point) end)
					(not (equal 
					      (buffer-substring 
					       space-position
					       (+ space-position 1)) "\n"))) 
				    (delete-region space-position
						   (+ 1 space-position))
				    (setq ocrspell-filter nil
					  string (buffer-substring 
						  word-start
						  space-position))
				    (setq reg-end (- reg-end 1)
					  offset-change (- offset-change 1)))
				   (t
				    (beep)
				    (message "illegal join request")
				    (sit-for 1)))

					;(backward-char (- space-position
					; word-start))

				  (while (and
					  (not (bolp))
					  (not (bobp))
					  (not (equal (buffer-substring 
						       (point) (+ 1 (point)))
						      " ")))
				    (backward-char 1))

				  (setq end (point))))
				    
				    
			       ((and replace (listp replace))
				;; REPLACEMENT WORD entered.  Recheck line
				;; starting with the replacement word.
				(setq ocrspell-filter nil
				      string (buffer-substring word-start
							       word-end))
				(let ((change (- (length (car replace))
						 (length (car poss)))))
				  (setq reg-end (+ reg-end change)
					offset-change (+ offset-change
							 change)))

				;;insert into .choices buffer
				(setq workin-window (current-buffer))
				(setq orig-word
				      (buffer-substring word-start word-end))
				(set-buffer (concat (buffer-name) ".choices"))
				(insert-buffer-substring workin-window
							 word-start word-end)
				(set-buffer workin-window)
				(delete-region word-start word-end)
				(setq workin-window (current-buffer))
				(set-buffer (concat (buffer-name) ".choices"))
				(insert (concat " -> " (car replace) "\n"))

				;;update dynamic confusion list
				(if 
				    (allow-confusion-chars-p 
				     orig-word (car replace)) 
				(start-lcs2cnf orig-word (car replace)))
				(setq orig-word nil)
				(set-buffer workin-window)
				(insert (car replace))

				;; Need to recheck typed-in replacements
				(if (not (eq 'query-replace
					     (car (cdr replace))))
				    (backward-char (length (car replace))))
				(setq end (point)) ; reposition for recheck
				;; when second arg exists, query-replace, 
				;; saving regions


				(if (car (cdr replace))
				    (unwind-protect
					(save-window-excursion
					  (set-marker
					   ocrspell-query-replace-marker
					   reg-end)

					  ;; Assume case-replace &
					  ;; case-fold-search correct?
					  (query-replace string (car replace)
							 t))
				      (setq reg-end
					    (marker-position
					     ocrspell-query-replace-marker))
				      (set-marker ocrspell-query-replace-marker
						  nil))))
			       
			       
			       (replace	; STRING REPLACEMENT for this word.
				(setq workin-window (current-buffer))
				(set-buffer (concat (buffer-name) ".choices"))
				(insert-buffer-substring
				 workin-window word-start word-end)
				;;insert into .choices buffer
				(set-buffer workin-window)
				(setq workin-window (current-buffer))
				(set-buffer (concat (buffer-name) ".choices"))
				(insert (concat " -> " replace "\n"))
				(set-buffer workin-window)
				(delete-region word-start word-end)
				(insert replace)

				(let ((change (- (length replace)
						 (length (car poss)))))
				  (setq reg-end (+ reg-end change)
					offset-change (+ offset-change change)
					end (+ end change)))))

			      (if (not ocrspell-quit)
				  (message "continuing spelling check..."))
			      (sit-for 0)))
			;; finished with line!

			(setq ocrspell-filter (cdr ocrspell-filter)))))
		  (goto-char end)))))
	(not ocrspell-quit))
    ;; protected

    (if (get-buffer ocrspell-choices-buffer)
	(kill-buffer ocrspell-choices-buffer))

    (if ocrspell-quit
	(progn
	  ;; preserve or clear the region for ocrspell-continue.
	  (if (not (numberp ocrspell-quit))
	      (set-marker ocrspell-region-end-mark nil)
	    ;; Enable ocrspell-continue.
	    (set-marker ocrspell-region-end-mark reg-end)
	    (goto-char ocrspell-quit))
	  ;; Check for aborting
	  (if (and ocrspell-spec-mess-handle (numberp ocrspell-quit))
	      (progn
		(setq ocrspell-quit nil)
		(error "Message send aborted.")))
	  (setq ocrspell-quit nil))
      (set-marker ocrspell-region-end-mark nil)
      (kill-ocrspell)

      ;; Only save if successful exit.
      (setq workin-window (current-buffer))
      (set-buffer (concat (buffer-name) ".choices"))
      (setq buffer-offer-save t)
      (set-buffer workin-window)
      (message "OcrSpell done.")))
)



(defun ocrspell-buffer ()
  "Check the current buffer for spelling errors interactively."
  (interactive)
  ;;may want to set cursor to white
  ;;(set-cursor-color "white")
  (setq cursor-in-echo-area t)
  (ocrspell-region (point-min) (point-max))
  (setq cursor-in-echo-area nil)
)



(defun ocrspell-horiz-scroll ()
  "Places the point within the horizontal visibility of its window area."
  (if truncate-lines			; display truncating lines?
      ;; See if display needs to be scrolled.
      (let ((column (- (current-column) (max (window-hscroll) 1))))

	(if (and (< column 0) (> (window-hscroll) 0))
	    (scroll-right (max (- column) 10))

	  (if (>= column (- (window-width) 2))
	      (scroll-left (max (- column (window-width) -3) 10))))))
)




(defun read-dynamic-file ()
  "read a dynamic confusion matrix from a file"
  (interactive)
  (setq working-window (current-buffer))
  (set-buffer (find-file-noselect (read-file-name "Dynamic confusion file:")))
  (setq bufferstring (buffer-string))
  (let ((start 0)
	(end 0))
    (setq start 0)
    ;; allow for special characters
    
    (cond ((string-match "OCRspell v1.0 Dynamic Table" bufferstring)
	   (while (not (equal (string-match (concat allowable-confusion-chars
						    (regexp-quote " -> ")
						    allowable-confusion-chars)
					    bufferstring start)
			      nil))
	     
	     (setq start (match-beginning 0))
	     (setq end (match-end 0))
	     
	     (setq ocrspell-dynamic-confusions 
		   (append ocrspell-dynamic-confusions
			   (list (list (substring bufferstring start end)))))
	     (setq start (+ 1 end)))
	   
	   
	   (setq ocrspell-dynamic-confusions (cdr ocrspell-dynamic-confusions))
	   
	   (setq global-ocrspell-dynamic-confusions 
		 (append global-ocrspell-dynamic-confusions
			 ocrspell-dynamic-confusions))
	   
	   (remove-duplicates global-ocrspell-dynamic-confusions)
	   (setq ocrspell-dynamic-confusions nil))
	  (t
	   (beep)(beep)
	   (message "wrong file format")))
    
    (kill-buffer (current-buffer))
    (set-buffer working-window))
)



(defun save-dynamic-file ()
  "save a dynamic confusion matrix to a file"
  (interactive)
  (let ((pt-list nil))
    (setq pt-list global-ocrspell-dynamic-confusions)
    
    (setq working-window (current-buffer))
    (set-buffer (get-buffer-create "*confusionsave*"))
    
    (princ "OCRspell v1.0 Dynamic Table" (current-buffer))
    (terpri (current-buffer))
    (princ "generated -> correct" (current-buffer))
    (terpri (current-buffer))
    
    (while (not (null pt-list))
      (princ (car (car pt-list)) (current-buffer))
      (terpri (current-buffer))
      (setq pt-list (cdr pt-list)))
    
    (save-buffer)
    (kill-buffer (current-buffer))
    (set-buffer working-window))
)



(defun save-session-file ()
  "save all words inserted during session to a file"
  (interactive)
  (let ((pt-list nil))
    (setq pt-list ocrspell-special-insertion)
    
    (setq working-window (current-buffer))
    (set-buffer (get-buffer-create "*sessionsave*"))
    (erase-buffer)
    
    (princ "OCRspell v1.0 Special Insertions" (current-buffer))
    (terpri (current-buffer))
    
    (while (not (null pt-list))
      (princ (car pt-list) (current-buffer))
      (terpri (current-buffer))
      (setq pt-list (cdr pt-list)))
    
    (save-buffer)
    (kill-buffer (current-buffer))
    (set-buffer working-window))
)



(defun read-session-file ()
  "load a session file"
  (interactive)
  (cond
   (special-config-startup-p
    (start-ocrspell-with-special-configuration))
   (t
    (start-ocrspell)))

  (let ((start 0)
	(eost nil))
    (setq working-window (current-buffer))
    (set-buffer (find-file-noselect (read-file-name "Session file:")))
    (setq bufferstring (buffer-string))
    (setq start 0)
    ;; allow for special characters
    (setq eost nil)
    (cond ((string-match "OCRspell v1.0 Special Insertions" bufferstring)
	   (setq start (match-end 0))
	   (while (not eost)
	    	     	     
	     (message "processing: %s" (car (read-from-string 
					     bufferstring start)))
	     (if (not (string-match (regexp-quote "/")
				    (prin1-to-string (car 
						      (read-from-string 
						       bufferstring start)))))
		 (process-send-string ocrspell-process
				      (concat "@" (prin1-to-string 
						   (car 
						    (read-from-string 
						     bufferstring 
						     start)))
					      "\n")))
	     (insert-special-word (prin1-to-string (car (read-from-string 
					bufferstring start))))
	     (if (eq (+ 1 (cdr (read-from-string bufferstring start)))
		     (length bufferstring))
		 (setq eost t))
	     (setq start (cdr (read-from-string bufferstring start)))))
	  (t
	   (beep)(beep)
	   (message "wrong file format")))
    
    (kill-buffer (current-buffer))
    (set-buffer working-window))
  (ocrspell-buffer)
)

  
  
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;                    End of Function Definitions
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
