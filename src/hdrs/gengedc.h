/*
 gengedc module
 
 This is a complete rewrite by Perry
 of the original gengedcom functionality by Tom Wetmore
 
  This version closes the node subset - that is, it outputs
  records for all the pointers it outputs, and does not
  output any invalid pointers.

  There are five modes in this version.

  Original - no S,E,X records
    but pointers are not trimmed
    (except family-linkage pointers are trimmed)
  The other four modes are combinations of two axes:
  Weak mode - no S,E,X records
  Strong mode - all S,E,X records referenced are included
  Trim mode - invalid (external) pointers are removed from the value line
  Dump mode - nodes with invalid (external) pointers are skipped

  NB: In trim mode, if the value line has been trimmed down to
   nothing, then the node is dumped

  eg, if 
  1 _ABOUT @S14@ @I12@ @I4500@

  if the indiset being output includes only I12,
  dump modes will skip this node entirely
   (including all its children)

  trim modes will remove the @I12@ from the output
  (weak trim will also remove the @S14@ from the output,
   because weak mode considers all S,E,X records as external)

*/


#ifndef _GENGEDC_H
#define _GENGEDC_H

enum { 
	GENGEDCOM_ORIGINAL,
	GENGEDCOM_WEAK_DUMP,
	GENGEDCOM_WEAK_TRIM,
	GENGEDCOM_STRONG_DUMP,
	GENGEDCOM_STRONG_TRIM
};

void gen_gedcom (INDISEQ seq, int gengedcl, BOOLEAN *eflg);

#endif /* _GENGEDC_H */
