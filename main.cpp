#include <iostream>
#include <assert.h>
#include <fstream>
#include <ctime>

/*
 * this library can be used in two modes:
 *
 * static version
 *   In this case you are restriced to the wchar_t and char versions of tokenizer. You compile using 'g++ main.cpp -ldrgxtokenizer'
 *
 * templated version
 *   If you want to use templated version of this library with arbitrary template types, you have to define DRGXTOKENIZER_TEMPLATED *before* including the drgxtokenizer/tokenizer.h header. In that case you compile just using 'g++ main.cpp', because all library codes are (and due to templatization of the library have to be) included as part of header files and compiled with the rest of your code.
 * */

//#define DRGXTOKENIZER_TEMPLATED
//#include <drgxtokenizer/tokenizer.h>

#include "tokenizer.h"

using namespace std;
using namespace drgxtokenizer;

//runs the performance test repeatedly with increasing base and prints out the times
//define PROFILING 
//number of tokens to be loaded from the 'testdata' file
#define DATA_SIZE 10000

/*
   this file serves as an example and a unit test
   */

int main()
{

  wcout << L"testing regex matching" << endl;

  //create an instance of tokenizer; It is not neccessary to create an instance in order to match a simple regex, but you need one to tokenize
  //these four declarations are equivalent except for string types accepted (all these are already precompiled in the static library)
  Tokenizer<wchar_t> tokenizer; //wide char (is to be initialized using std::wstring (respectively by std::basic_string<wchar_t>))
  WTokenizer tokenizer4;
  Tokenizer<char> tokenizer5; //char (is to be initialized using std::string (std::basic_string<char>))
  CTokenizer tokenizer3;

  /*
   *regex usage example
   */
  //test trivial matching
  assert(tokenizer.Match(L"a b", L"^.\\b.\\b.$"));

  assert(!tokenizer.Match(L"hello world", L"l$"));
  assert(tokenizer.Match(L"hello world", L"^h"));
  assert(!tokenizer.Match(L"hello world", L"^e"));
  assert(tokenizer.Match(L"hello world", L"d$"));
  assert(!tokenizer.Match(L"acb", L"^.\\b.\\b.$"));
  assert(tokenizer.Match(L"abc", L"a.c"));
  assert(WTokenizer::Match(L"hello    world", L"(H|h)el[lk]o *w"));
  assert(CTokenizer::Match("hello    world", "(H|h)el[lk]o *w"));
  assert(Tokenizer<wchar_t>::Match(L"world", L"((H|h)el[lk]o *w|w[orl]+d)")); //can call a as static function
  assert(tokenizer.Match(L"hel world", L"((H|h)el[lk]o *w|w[orl]+d)")); //or a nonstatic function
  assert(!tokenizer.Match(L"hel world", L"((H|h)el[lk]o *w|w[orl]+dd)")); //or a nonstatic function
  assert(tokenizer3.Match("hello world", "((H|h)el[lk]. * ?w|w[orl]+d)"));
  assert(tokenizer.Match(L"hello world", L"h.*o"));
  bool matched = tokenizer.Match(L"hello world", L"h.*o"); //assert macros serve here just for the purpose of this file as a unit test... You can retrieve value like simply...

  /*
   *tokenizer example
   */
  wcout << L"testing tokenizer" << endl;
  //Tokenizer can be used with an arbitrary iterator handled string which has implemented standard functions and dereferences to reference to your templated type (e.g. char& ) (or anything that is automatically castable to that type). Using template argument is necessary.
  //First we need to pass token definitions with desired token ids. More tokens can share the same id.
  tokenizer.AddToken(L"[a-z]*", 3);
  tokenizer.AddToken(L"[A-Z]*", 4);
  tokenizer.AddToken(L"[A-Za-z]*", 2);
  tokenizer.AddToken(L"[0-9]*", 5);
  tokenizer.AddToken(L".", 1); //fallback match, higher id is always prefered; Your token specification should ALWAYS match at least one character!
  tokenizer.AddTokensSubmit(); //is obligatory at the end of definitions - performs the subset transformation and reduces automaton, so it can be used. (there is implemented NO algorithm for nondeterministic crawling! - crawling in nondeterministic automaton will lead to undefined behaviour:w

  //lets setup a test case
  wstring buffer = L"test TEST TeSt 123";
  int result[7] = {3,1,4,1,2,1,5}; //resulting sequence for the test buffer
  wstring::iterator tmp;
  wstring::iterator begin = buffer.begin();
  wstring::iterator end = buffer.end();
  int tokenid;
  bool succ;

  //and now lets go
  int i;
  for(i = 0; begin != end; i++)
  {
    tmp = begin;
    succ = tokenizer.NextToken<wstring::iterator>(begin, end, tokenid); //parse one token from begin. Do not parse after end. Returns true if match was successfull (should ALWAYS be). begin is set to point behind end of matched token. If you need the token's value, you should make copy of begin beforehand:
    std::wcout << "'" << wstring(tmp, begin).c_str() << L"' was matched to " << tokenid << std::endl;

    assert(succ);
    assert(tokenid == result[i]);
  }
  assert(i == 7);

  /*
   * a performance test with some output for analysis - nothing interesting here
   */

  int base = 2;
  std::vector<std::pair<int, double> > profiling_results;

repeat:
  clock_t time_begin = clock();
  string line;

  Tokenizer<wchar_t> tokenizer2 = Tokenizer<wchar_t>();
  ifstream file("testdata");
  if(file.is_open())
  {
    wcout << L"construction performance test" << endl;
    int i = 0;
    while(getline(file,line))
    {
#ifdef PROFILING
      tokenizer2.AddToken(std::wstring(line.begin(), line.end()), i & 63, base);
#else
      tokenizer2.AddToken(std::wstring(line.begin(), line.end()), i & 63);
#endif
      i++;
      if((i & 2047) == 2047)
        std::wcout << "processed " << i << " tokens" << std::endl;
      if(i == DATA_SIZE)
        break;
    }
    clock_t time_end = clock();
    double elapsed_secs = double(time_end - time_begin) / CLOCKS_PER_SEC;
    profiling_results.push_back(std::pair<int,double>(base, elapsed_secs));
    wcout << L" finished with base " << base << endl;
    wcout << L"performance test took " << elapsed_secs << endl;
  }
  else
    wcout << L"failed to open file testdata" << std::endl;

#ifdef PROFILING
  if(base < 10000)
  {
    base *= 2;
    goto repeat;
  }

  std::wcout << "profiling results" << std::endl;
  std::wcout << "base   time" << std::endl;

  for(std::vector<std::pair<int, double> >::iterator itr = profiling_results.begin(); itr != profiling_results.end(); ++itr)
  {
    std::wcout << (*itr).first << "   " << (*itr).second << std::endl;
  }
#endif

  return 0;
}

