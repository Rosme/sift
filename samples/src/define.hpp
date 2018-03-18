#pragma once

// This file has 8 macros
// with 4 macro 'functions'

#define one two
#define my(x) \
great \
multiline \
define

/* This line should not count as a #define macro(x) */
/* This line should not count as a #define macro */

/* Here's a comment */ #define wow(x) a define
      #define this_one is just really towards the right
              #define tabs_too

                            #define very \
ugly\
defines\
i must say\

   /* this one is also technically valid */ #define although(x) it\
   is\
        very \
        disgusting
        
        
  #define dont_forget(x) about these ones too 
  
  
std::string something_else("this string has #define in it");

// better be careful with #define comments too
