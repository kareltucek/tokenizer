#ifndef TOKENIZER_H
#define TOKENIZER_H

#include <string>
#include <map>
#include <vector>
#include <locale>
#include "automaton.h"
#include "datastructs.h"

#define BINARY_COMPOSITION
#define DEFAULT_BASE 64

namespace drgxtokenizer
{
  enum type{toktype_expbegin, toktype_expend, toktype_pbegin, toktype_pend, toktype_star, toktype_mark, toktype_plus, toktype_choice, toktype_string, toktype_range, toktype_concat, toktype_automaton, toktype_any, toktype_sep};

  /*
     see example on how to use this in main.cpp; Note using AddTokensSubmit() after specifying all tokens is obligatory!. Token ids you actually intend to use should start by 1 (0/-1 is reserved as implicit). Higher token id is always preferred for an ambiguous match, longer token is always preffered.
     */

  template <class T_data>
    class Tokenizer
    {
      private:

        //drgxtokenizer::Ranges GetRanges(std::basic_string<T_data> str); //???
        int tokenCount;
        drgxtokenizer::FA<T_data> engine;
        std::vector<drgxtokenizer::FA<T_data> > FAstack;
        static drgxtokenizer::FA<T_data> ConstructNFA(std::basic_string<T_data> regex, int token_id = -1);

        std::locale loc;

        struct token //just regex construction token :-)
        {
          drgxtokenizer::type t;
          std::basic_string<T_data> s;
          drgxtokenizer::FA<T_data> d;
          token(drgxtokenizer::type,std::basic_string<T_data>);
          token(drgxtokenizer::FA<T_data>);
          token();
        };
        static token ReadToken(std::basic_string<T_data>& str);
        static bool is_operator(T_data c);

      public:
        Tokenizer(const std::locale& loc = std::locale());
        ~Tokenizer();


        static bool Match(std::basic_string<T_data> string, std::basic_string<T_data> pattern);
        void AddToken(std::basic_string<T_data> token_rgx, int id, int base = DEFAULT_BASE);
        void AddTokensSubmit();
        template<class iterator_type>
          bool NextToken(iterator_type& itr, const iterator_type& itrend, int& token_id);

        void Clear();

    };


  template<class T_data>
    template<class iterator_type>
    bool Tokenizer<T_data>::NextToken(iterator_type& itr, const iterator_type& itrend, int& token_id)
    {
      return engine.template Tokenize<iterator_type>(itr, itrend, token_id);
    }
}

#ifdef DRGXTOKENIZER_TEMPLATED
#include "tokenizer.cpp"
#endif

namespace drgxtokenizer
{
  typedef Tokenizer<wchar_t> WTokenizer;
  typedef Tokenizer<char> CTokenizer;
}

#endif // TOKENIZER_H
