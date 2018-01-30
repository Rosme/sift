#pragma once

#define one two
#define my(x) \
great \
multiline \
define

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

std::string something_else("this string has #define in it");

// better be careful with #define comments too
