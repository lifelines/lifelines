/*
 * @progname       pafcompat.ll
 * @version        2.0
 * @author         Eggert
 * @category       
 * @output         Text
 * @description    

This LifeLines report program checks a LifeLines database for
compatibility with PAF.  I used the Mac PAF manual for field length
specification, and Mac PAF v2.11 for a little testing.

pafcompat - a LifeLines PAF compatibility checker
	by Jim Eggert (eggertj@ll.mit.edu)
	Version 1, 2 January 1993
	Version 2, 7 January 1993 added 80 character max for all values

This program checks:
1.  Number (<=3) and length (<=16) of given names.
2.  Length of surname. (<=16)
3.  Whether something is after the surname.
4.  Length of title (<=16) and reference number (<=10).
5.  Whether sex is "M" or "F" or " " or not.
6.  Number (<=4) and length (<=16) of place fields.
7.  Length of date. (<=23)
8.  Legal tags at each level, including legal heirarchical structure.
9.  No more than one of each tag at each heirarchical level.
10. Values must be less than 80 characters.

Allowed tags are:
    NAME, TITL, SEX, BIRT, CHR, DEAT, BURI, NOTE, FAMS, FAMC, REFN,
    BAPL, ENDL, CONT, DATE, PLAC, TEMP, SLGC, HUSB, WIFE, CHIL,
    MARR, DIV, SLGS

This version doesn't parse dates per se, it only checks the length of
the date string.  Thus it doesn't know what PAF can understand in a
date string.

It also doesn't check for valid content of any of the LDS entries.

*/

global(n_place_tokens)
global(longest_token_length)
global(n_givens)
global(post_surname_token)

proc parse_place(eplace) {
    set(n_place_tokens,0)
    set(longest_token_length,0)
    set(len,1)
    set(last_len,0)
    while (lt(len,strlen(eplace))) {
	set(head,save(trim(eplace,len)))
	set(len,add(len,1))
	if (not(strcmp(concat(head,","),trim(eplace,len)))) {
	    set(n_place_tokens,add(n_place_tokens,1))
	    set(this_token_length,sub(len,add(last_len,1)))
	    set(last_len,len)
	    if (gt(this_token_length,longest_token_length)) {
		set(longest_token_length,this_token_length)
	    }
	}
    }
}


proc parse_names(pname) {
    set(longest_token_length,0)
    set(post_surname_token,0)
    set(n_givens,0)
    set(len,1)
    set(last_len,1)
    set(sep_level,0)
    set(last_name,0)
    set(gsep," ")
    set(lsep,"/")
    while (lt(len,strlen(pname))) {
	set(head,save(trim(pname,len)))
	set(len,add(len,1))
	if (not(strcmp(concat(head,gsep),trim(pname,len)))) {
	    if (eq(last_name,0)) {
		set(this_token_length,sub(len,add(last_len,1)))
		set(last_len,len)
		if (gt(this_token_length,longest_token_length)) {
		    set(longest_token_length,this_token_length)
		}
		if (or(gt(this_token_length,0),lt(n_givens,3))) {
		    set(n_givens,add(n_givens,1))
		}
	    }
	}
	elsif (ge(last_name,2)) {
	    set(post_surname_token,1)
	}
	elsif (not(strcmp(concat(head,lsep),trim(pname,len)))) {
	    set(this_token_length,sub(len,add(last_len,1)))
	    set(last_len,len)
	    if (gt(this_token_length,longest_token_length)) {
		set(longest_token_length,this_token_length)
	    }
	    if (and(eq(last_name,0),gt(this_token_length,0))) {
		set(n_givens,add(n_givens,1))
	    }
	    set(last_name,add(last_name,1))
	}
    }
}


proc report_indi(person) {
    " " key(person) " " name(person) "\n"
}


proc report_fam(family) {
t    " family "
    key(husband(family)) " " name(husband(family)) " & "
    key(wife(family))    " " name(wife(family)) "\n"
}


proc main() {
    list(indi_tags)
    list(indi_tag_counts)
    list(indi_tag_value)
    list(indi_tag_subtags)
    list(fam_tags)
    list(fam_tag_counts)
    list(fam_tag_value)
    list(fam_tag_subtags)
    list(event_subtags)
    list(note_subtags)
    list(lds_subtags)
    list(slgc_subtag)
    list(empty)
    list(subtags)
    list(subtag_counts)  /* as long as the longest subtag list */
    list(subsubtag_counts) /* only for SLGC under family */

    enqueue(note_subtags,"CONT")
    enqueue(event_subtags,"DATE")
    enqueue(event_subtags,"PLAC")
    enqueue(lds_subtags,"DATE")
    enqueue(lds_subtags,"TEMP")
    enqueue(slgc_subtag,"SLGC")  /* this one has no value! */
    enqueue(subtag_counts,0)
    enqueue(subtag_counts,0)
    enqueue(subsubtag_counts,0)
    enqueue(subsubtag_counts,0)

    enqueue(indi_tags,"NAME") /* 1 */
      enqueue(indi_tag_subtags,empty)
      enqueue(indi_tag_counts,0)
      enqueue(indi_tag_value,neg(2))
    enqueue(indi_tags,"TITL") /* 2 */
      enqueue(indi_tag_subtags,empty)
      enqueue(indi_tag_counts,0) 
      enqueue(indi_tag_value,16)
    enqueue(indi_tags,"SEX")  /* 3 */
      enqueue(indi_tag_subtags,empty)
      enqueue(indi_tag_counts,0)
      enqueue(indi_tag_value,1)
    enqueue(indi_tags,"BIRT") /* 4 */
      enqueue(indi_tag_subtags,event_subtags)
      enqueue(indi_tag_counts,0)
      enqueue(indi_tag_value,0)
    enqueue(indi_tags,"CHR")  /* 5 */
      enqueue(indi_tag_subtags,event_subtags)
      enqueue(indi_tag_counts,0)
      enqueue(indi_tag_value,0)
    enqueue(indi_tags,"DEAT") /* 6 */
      enqueue(indi_tag_subtags,event_subtags)
      enqueue(indi_tag_counts,0)
      enqueue(indi_tag_value,0)
    enqueue(indi_tags,"BURI") /* 7 */
      enqueue(indi_tag_subtags,event_subtags)
      enqueue(indi_tag_counts,0)
      enqueue(indi_tag_value,0)
    enqueue(indi_tags,"NOTE") /* 8 */
      enqueue(indi_tag_subtags,note_subtags)
      enqueue(indi_tag_counts,0)
      enqueue(indi_tag_value,neg(1))
    enqueue(indi_tags,"FAMC") /* 9 */
      enqueue(indi_tag_subtags,empty)
      enqueue(indi_tag_counts,0)
      enqueue(indi_tag_value,neg(2))
    enqueue(indi_tags,"FAMS") /* 10 */
      enqueue(indi_tag_subtags,empty)
      enqueue(indi_tag_counts,0)
      enqueue(indi_tag_value,neg(2))
    enqueue(indi_tags,"REFN") /* 11 */
      enqueue(indi_tag_subtags,empty)
      enqueue(indi_tag_counts,0)
      enqueue(indi_tag_value,10)
    enqueue(indi_tags,"BAPL") /* 12 */
      enqueue(indi_tag_subtags,lds_subtags)
      enqueue(indi_tag_counts,0)
      enqueue(indi_tag_value,0)
    enqueue(indi_tags,"ENDL") /* 13 */
      enqueue(indi_tag_subtags,lds_subtags)
      enqueue(indi_tag_counts,0)
      enqueue(indi_tag_value,0)
    enqueue(indi_tags,"SLGC") /* 14 */
      enqueue(indi_tag_subtags,lds_subtags)
      enqueue(indi_tag_counts,0)
      enqueue(indi_tag_value,0)

    enqueue(fam_tags,"HUSB") /* 1 */
      enqueue(fam_tag_subtags,empty)
      enqueue(fam_tag_counts,0)
      enqueue(fam_tag_value,neg(2))
    enqueue(fam_tags,"WIFE") /* 2 */
      enqueue(fam_tag_subtags,empty)
      enqueue(fam_tag_counts,0)
      enqueue(fam_tag_value,neg(2))
    enqueue(fam_tags,"CHIL") /* 3 */
      enqueue(fam_tag_subtags,slgc_subtag)
      enqueue(fam_tag_counts,0)
      enqueue(fam_tag_value,neg(2))
    enqueue(fam_tags,"MARR") /* 4 */
      enqueue(fam_tag_subtags,event_subtags)
      enqueue(fam_tag_counts,0)
      enqueue(fam_tag_value,0)
    enqueue(fam_tags,"DIV") /* 5 */
      enqueue(fam_tag_subtags,empty)
      enqueue(fam_tag_counts,0)
      enqueue(fam_tag_value,1)
    enqueue(fam_tags,"SLGS") /* 6 */
      enqueue(fam_tag_subtags,lds_subtags)
      enqueue(fam_tag_counts,0)
      enqueue(fam_tag_value,0)

    print("Checking individuals ")
    set(next_print,0)
    forindi(person,pnum) {
	if (ge(pnum,next_print)) {
	    print(d(pnum)) print(" ")
	    set(next_print,add(next_print,100))
	}
	forlist(indi_tag_counts,count,cnum) {
	    setel(indi_tag_counts,cnum,0)
	}
	fornodes(inode(person),node) {
	    set(tag_ok,0)
	    set(node_tag,save(tag(node)))
	    forlist(indi_tags,vtag,vnum) {
		if (not(strcmp(node_tag,vtag))) {
		    set(tag_ok,vnum)
		    set(subtags,getel(indi_tag_subtags,vnum))
		    set(tag_count,add(getel(indi_tag_counts,vnum),1))
		    setel(indi_tag_counts,vnum,tag_count)
		    set(tag_value,getel(indi_tag_value,vnum))
		}
	    }
	    if (not(tag_ok)) {
		"Illegal tag " node_tag
		call report_indi(person)
	    }
	    else {
		if (and(gt(tag_count,1),
			and(strcmp(node_tag,"NOTE"),
			    strcmp(node_tag,"FAMS")))) {
		    "Duplicate " node_tag
		    call report_indi(person)
		}
		if (not(tag_value)) {
		    if (strcmp(value(node),"")) {
			"Illegal " node_tag " value " value(node)
			call report_indi(person)
		    }
		}
		elsif (gt(tag_value,0)) {
		    if (gt(strlen(value(node)),tag_value)) {
			node_tag " too long " value(node)
			call report_indi(person)
		    }
		}
		elsif (eq(tag_value,neg(2))) {
		    if (not(strcmp(value(node),""))) {
			"Empty " node_tag
			call report_indi(person)
		    }
		}
		if (lt(tag_value,0)) {
		    if (gt(strlen(value(node)),80)) {
			node_tag " >80 characters " value(node)
			call report_indi(person)
		    }
		}
		if (eq(tag_ok,3)) { /* "SEX" */
		    if (and(and(strcmp(value(node),"M"),
			        strcmp(value(node),"F")),
				strcmp(value(node)," "))) {
			"Illegal sex " value(node)
			call report_indi(person)
		    }
		}
		elsif (eq(tag_ok,1)) { /* "NAME" */
		    call parse_names(value(node))
		    if (gt(n_givens,3)) {
			"Too many given names"
			call report_indi(person)
		    }
		    if (gt(longest_token_length,16)) {
			"Name too long"
			call report_indi(person)
		    }
		    elsif (eq(longest_token_length,0)) {
			"No name" call report_indi(person)
		    }
		    if (post_surname_token) {
			"Stuff after surname" call report_indi(person)
		    }
		}
		forlist(subtags,vstag,vsnum) {
		    setel(subtag_counts,vsnum,0)
		}
		fornodes(node,subnode) {
		    set(subnode_tag,save(tag(subnode)))
		    set(subtag_count,0)
		    forlist(subtags,vstag,vsnum) {
			if (not(strcmp(subnode_tag,vstag))) {
			    set(subtag_count,add(getel(subtag_counts,vsnum),1))
			    setel(subtag_counts,vsnum,subtag_count)
			}
		    }
		    if (not(subtag_count)) {
			"Illegal subtag " node_tag " " subnode_tag
			call report_indi(person)
		    }
		    else {
			if (and(gt(subtag_count,1),
				strcmp(subnode_tag,"CONT"))) {
			    "Duplicate subtag " subnode_tag " " node_tag
			    call report_indi(person)
			}
			if (not(strcmp(subnode_tag,"DATE"))) {
			    if (gt(strlen(value(subnode)),40)) {
				"Date too long " node_tag " "
				value(subnode)
				call report_indi(person)
			    }
			}
			elsif (not(strcmp(subnode_tag,"PLAC"))) {
			    call parse_place(value(subnode))
			    if (gt(n_place_tokens,4)) {
				"Too many place levels " node_tag " "
				value(subnode)
				call report_indi(person)
			    }
			    if (gt(longest_token_length,16)) {
				"Place too long " node_tag " "
				value(subnode)
				call report_indi(person)
			    }
			}
			elsif (gt(strlen(value(subnode)),80)) {
			    subnode_tag " >80 characters " value(subnode)
			    call report_indi(person)
			}
		    }
		    fornodes(subnode,subsubnode) {
			"Illegal node depth "
			node_tag " " subnode_tag " " tag(subsubnode)
			call report_indi(person)
		    }
		}
	    }
	}
	if (not(getel(indi_tag_counts,1))) {
	    "No name"  call report_indi(person)
	}
    }
    print("\nChecking families ")
    set(next_print,0)
    forfam(family,fnum) {
	if (ge(fnum,next_print)) {
	    print(d(fnum)) print(" ")
	    set(next_print,add(next_print,100))
	}
	forlist(fam_tag_counts,count,cnum) {
	    setel(fam_tag_counts,cnum,0)
	}
	fornodes(fnode(family),node) {
	    set(tag_ok,0)
	    set(node_tag,save(tag(node)))
	    forlist(fam_tags,vtag,vnum) {
		if (not(strcmp(node_tag,vtag))) {
		    set(tag_ok,vnum)
		    set(subtags,getel(fam_tag_subtags,vnum))
		    set(tag_count,add(getel(fam_tag_counts,vnum),1))
		    setel(fam_tag_counts,vnum,tag_count)
		    set(tag_value,getel(fam_tag_value,vnum))
		}
	    }
	    if (not(tag_ok)) {
		"Illegal tag " node_tag
		call report_fam(family)
	    }
	    else {
		if (and(gt(tag_count,1),
			strcmp(node_tag,"CHIL"))) {
		    "Duplicate " node_tag
		    call report_fam(family)
		}
		if (not(tag_value)) {
		    if (strcmp(value(node),"")) {
			"Illegal " node_tag " value " value(node)
			call report_fam(family)
		    }
		}
		elsif (gt(tag_value,0)) {
		    if (gt(strlen(value(node)),tag_value)) {
			node_tag " too long " value(node)
			call report_fam(family)
		    }
		}
		elsif (eq(tag_value,neg(2))) {
		    if (not(strcmp(value(node),""))) {
			"Empty " node_tag
			call report_fam(family)
		    }
		}
		if (lt(tag_value,0)) {
		    if (gt(strlen(value(node)),80)) {
			node_tag " >80 characters " value(node)
			call report_fam(family)
		    }
		}
		if (eq(tag_ok,5)) { /* "DIV" */
		    if (strcmp(value(node),"Y")) {
			"Illegal divorce value " value(node)
			call report_fam(family)
		    }
		}
		forlist(subtags,vstag,vsnum) {
		    setel(subtag_counts,vsnum,0)
		}
		fornodes(node,subnode) {
		    set(subnode_tag,save(tag(subnode)))
		    set(subtag_count,0)
		    forlist(subtags,vstag,vsnum) {
			if (not(strcmp(subnode_tag,vstag))) {
			    set(subtag_count,add(getel(subtag_counts,vsnum),1))
			    setel(subtag_counts,vsnum,subtag_count)
			}
		    }
		    if (not(subtag_count)) {
			"Illegal subtag " node_tag " " subnode_tag
			call report_fam(family)
		    }
		    else {
			if (gt(subtag_count,1)) {
			    "Duplicate subtag " node_tag " " subnode_tag
			    call report_fam(family)
			}
			if (not(strcmp(subnode_tag,"DATE"))) {
			    if (gt(strlen(value(subnode)),40)) {
				"Date too long "
				node_tag " " value(subnode)
				call report_fam(family)
			    }
			}
			elsif (not(strcmp(subnode_tag,"PLAC"))) {
			    call parse_place(value(subnode))
			    if (gt(n_place_tokens,4)) {
				"Too many place levels "
				node_tag " " value(subnode)
				call report_fam(family)
			    }
			    if (gt(longest_token_length,16)) {
				"Place too long "
				node_tag " " value(subnode)
				call report_fam(family)
			    }
			}
		    }
		    if (not(strcmp(subnode_tag,"SLGC"))) {
			forlist(subsubtag_counts,count,cnum) {
			    setel(subsubtag_counts,cnum,0)
			}
			fornodes(subnode,subsubnode) {
			    set(subsubtag_count,0)
			    set(subsubnode_tag,save(tag(subsubnode)))
			    forlist(lds_subtags,vstag,vsnum) {
				if (not(strcmp(subsubnode_tag,vstag))) {
				    set(subsubtag_count,
					add(getel(subsubtag_counts,vsnum),1))
				    setel(subsubtag_counts,vsnum,
					subsubtag_count)
				}
			    }
			    if (not(subsubtag_count)) {
				"Illegal subsubtag "
				node_tag " " subnode_tag " "
				tag(subsubnode)
				call report_fam(family)
			    }
			    if (gt(subsubtag_count,1)) {
				"Duplicate subsubtag "
				node_tag " " subnode_tag " "
				tag(subsubnode)
				call report_fam(family)
			    }
			}
		    }
		    else {
			fornodes(subnode,subsubnode) {
			    "Illegal node depth "
			    node_tag " " subnode_tag " "
			    tag(subsubnode)
			    call report_fam(family)
			}
		    }
		}
	    }
	}
    }
}
