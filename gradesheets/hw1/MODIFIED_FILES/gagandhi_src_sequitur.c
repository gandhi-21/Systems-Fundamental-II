--- hw1/dev_repo/solution/src/sequitur.c

+++ hw1/repos/gagandhi/hw1/src/sequitur.c

@@ -51,7 +51,7 @@

 	//    abbbc  ==> abbc   (handle this first)

 	//      ^

 	//    abbbc  ==> abbc   (then check for this one)

-        //     ^

+        //     ^ 

 	if(next->prev && next->next &&

 	   next->value == next->prev->value && next->value == next->next->value)

 	    digram_put(next);

@@ -125,7 +125,7 @@

     // We are destroying any digram that starts at this symbol,

     // so we have to delete that from the table.

     digram_delete(this);

-

+    

     // Splice the body of the rule in place of the nonterminal symbol.

     join_symbols(left, first);

     join_symbols(last, right);

@@ -188,14 +188,18 @@

 static void process_match(SYMBOL *this, SYMBOL *match) {

     debug("Process matching digrams <%lu> and <%lu>",

 	  SYMBOL_INDEX(this), SYMBOL_INDEX(match));

-    SYMBOL *rule = NULL;

-

+    SYMBOL *rule = NULL; 

+

+  //  debug("checkign rule head");

     if(IS_RULE_HEAD(match->prev) && IS_RULE_HEAD(match->next->next)) {

 	// If the digram headed by match constitutes the entire right-hand side

 	// of a rule, then we don't create any new rule.  Instead we use the

 	// existing rule to replace_digram for the newly inserted digram.

+   // debug("checked rule head");

 	rule = match->prev->rule;

+   // debug("replace digram called");

 	replace_digram(this, match->prev->rule);

+   // debug("replace digram received");

     } else {

 	// Otherwise, we create a new rule.

 	// Note that only one digram is created by this rule, and the insert_after

@@ -203,10 +207,16 @@

 	// In fact, no digrams will be deleted during the construction of

 	// the new rule because the calls are being made in such a way that we are

 	// never overwriting any pointers that were previously non-NULL.

+  //  debug("else of rule heead");

 	rule = new_rule(next_nonterminal_value++);

+  //  debug("back from new rule");

 	add_rule(rule);

+  //  debug("back from add rule");

+  ///  debug("calling insert after 1");

 	insert_after(rule->prev, new_symbol(this->value, this->rule));

+  //  debug("calling insert after 2");

 	insert_after(rule->prev, new_symbol(this->next->value, this->next->rule));

+  //  debug("back from insert after 2");

 

 	// Now, replace the two existing instances of the right-hand side of the

 	// rule by nonterminals that refer to the rule.

@@ -217,8 +227,11 @@

 	// However, since the nonterminal symbol is a freshly created one that

 	// did not exist before, these replacements cannot result in the creation

 	// of digrams that duplicate already existing ones.

+  //  debug("replace digram 1");

 	replace_digram(match, rule);

+  //  debug("replace digram 2");

 	replace_digram(this, rule);

+   // debug("back from replace digram 2");

 

 	// Insert the right-hand side of the new rule into the digram table.

 	// Note that no other rules that might have been created as a result of the

@@ -226,7 +239,9 @@

 	// we are about to insert here, because, the right-hand sides of any of these

 	// other rules must contain the new nonterminal that is at the head of the

 	// current rule but not in the body of the current rule.

+   // debug("calling digram put");

 	digram_put(rule->next);

+   // debug("back from digram pur");

     }

 

     // We have now restored the "no repeated digram" property, but it might be that

@@ -279,15 +294,20 @@

 

     // If the "digram" is actually a single symbol at the beginning or

     // end of a rule, then there is no need to do anything.

+   // debug("checking rule head");

     if(IS_RULE_HEAD(this) || IS_RULE_HEAD(this->next))

 	return 0;

-

+   // debug("checked rule head");

     // Otherwise, look up the digram in the digram table, to see if there is

     // a matching instance.

+   // debug("digram get called");

     SYMBOL *match = digram_get(this->value, this->next->value);

+  //  debug("digram get recieved");

     if(match == NULL) {

         // The digram did not previously exist -- insert it now.

+       // debug("digram put called");

 	digram_put(this);

+   // debug("digram put recieved");

 	return 0;

     }

 

@@ -300,4 +320,4 @@

 	process_match(this, match);

 	return 1;

     }

-}

+}