#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <setjmp.h>
 
#define AT(CHAR) ( *curr == CHAR && ++curr )
#define TEST(STR) ( strncmp( curr, STR, strlen(STR) ) == 0 \
  && ! isalnum(curr[strlen(STR)]) && curr[strlen(STR)] != '_' )
#define IS(STR) ( TEST(STR) && (curr += strlen(STR)) )
 
static char *curr;
static char *startPosition;
static jmp_buf jumpEnvironment;
 
// prints error messages
static int error(char *message) {
  printf("false  %s\n%*s^ %s\n", startPosition, curr - startPosition + 7, "", message);
  longjmp( jumpEnvironment, 1 );
}
 
// 
static int expr(int level) {
  while( isspace(*curr) ) {
    ++curr;
  }

  if( AT('(') ) {
    if( expr(0) && ! AT(')') ) error("missing close paren");
  }
  else if( level <= 4 && IS("not") && expr(6) ) { }
  else if( TEST("or") || TEST("and") || TEST("not") ) {
    error("expected a primary, found an operator");
  }
  else if( isdigit(*curr) ) curr += strspn( curr, "0123456789" );
  else if( isalpha(*curr) ) curr += strspn( curr, "0123456789_"
    "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ" );
  else error("expected a primary");
 
  do {
    while( isspace(*curr) ) ++curr;
  }

  while(
    level <= 2 && IS("or") ? expr(3) :
    level <= 3 && IS("and") ? expr(4) :
    level <= 4 && (AT('=') || AT('<')) ? expr(5) :
    level == 5 && (*curr == '=' || *curr == '<') ? error("non-associative") :
    level <= 6 && (AT('+') || AT('-')) ? expr(7) :
    level <= 7 && (AT('*') || AT('/')) ? expr(8) :
    0 );

  return 1;
}

// validates parse
static void parse(char *start) {
  startPosition = curr = start;

  if( setjmp(jumpEnvironment) ) return;

  expr(0);

  if( *curr ) {
    error("unexpected character following valid parse");
  }

  printf(" true  %s\n", start);
}
 
static char *tests[] = {
  "3 + not 5",
  "3 + (not 5)",
  "(42 + 3",
  "(42 + 3 some_other_syntax_error",
  "not 3 < 4 or (true or 3/4+8*5-5*2 < 56) and 4*3 < 12 or not true",
  "and 3 < 2",
  "not 7 < 2",
  "2 < 3 < 4",
  "2 < foobar - 3 < 4",
  "2 < foobar and 3 < 4",
  "4 * (32 - 16) + 9 = 73",
  "235 76 + 1",
  "a + b = not c and false",
  "a + b = (not c) and false",
  "a + b = (not c and false)",
  "ab_c / bd2 or < e_f7",
  "g not = h",
  "i++",
  "j & k",
  "l or _m",
  "wombat",
  "WOMBAT or monotreme",
  "a + b - c * d / e < f and not ( g = h )",
  "$",
  };


int main(int a, char *b[]) {
  for( int i = 0; i < sizeof(tests)/sizeof(*tests); i++ ){
    parse(tests[i]);
  } 
}