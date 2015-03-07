#define  DRGXTOKENIZER_TEMPLATED
#include "tokenizer.h"

template class drgxtokenizer::Tokenizer<wchar_t>;
template class drgxtokenizer::FA<wchar_t>;
template class drgxtokenizer::Range<wchar_t>;
template class drgxtokenizer::Transition<wchar_t>;
template class drgxtokenizer::State<wchar_t>;

template class drgxtokenizer::Tokenizer<char>;
template class drgxtokenizer::FA<char>;
template class drgxtokenizer::Range<char>;
template class drgxtokenizer::Transition<char>;
template class drgxtokenizer::State<char>;

namespace drgxtokenizer
{
  typedef Tokenizer<wchar_t> WTokenizer;
  typedef Tokenizer<char> CTokenizer;
}

