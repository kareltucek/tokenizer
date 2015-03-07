#include "tokenizer.h"
#include <queue>
#include <deque>
#include <list>
#include <iostream>
#include <limits>

namespace drgxtokenizer
{
  template<class T_data>
    Tokenizer<T_data> ::token::token(type t_, std::basic_string<T_data> s_)
    : t(t_), s(s_)
    {
    }

  template<class T_data>
    Tokenizer<T_data>::token::token(FA<T_data> automaton)
    : d(automaton), t(toktype_automaton)
    {
    }

  template<class T_data>
    Tokenizer<T_data>::token::token()
    {
    }

  template<class T_data>
    Tokenizer<T_data>::Tokenizer (const std::locale& l)
    : engine(false), tokenCount(0), loc(l)
    {
    }

  template<class T_data>
    Tokenizer<T_data>::~Tokenizer<T_data> ()
    {
      Clear();
    }

  template<class T_data>
    FA<T_data> Tokenizer<T_data> ::ConstructNFA(std::basic_string<T_data> regex, int token_id)
    {
      std::queue<token> tokens;
      std::deque<token> st;

      tokens.push(token(toktype_pbegin, std::basic_string<T_data>()));

      //bool lasttext = false;
      while(!regex.empty())
      {
        token t = ReadToken(regex);
        //if(lasttext && (t.t == toktype_string || t.t == toktype_range))
        //  tokens.push(token(toktype_concat, ""));

        if(t.t==toktype_string)
        {
          std::basic_string<T_data> s = t.s;
          if(s.length() > 1)
          {
            tokens.push(token(FA<T_data>(s.substr(0, s.length()-1), false)));
            //     tokens.push(token(toktype_concat, ""));
          }
          tokens.push(token(FA<T_data>( s.substr(s.length()-1), false))); // abcd* != (abcd)*
        }
        else if(t.t==toktype_range)
          tokens.push(token(FA<T_data>(t.s,true)));
        else if(t.t==toktype_any)
          tokens.push(token(FA<T_data>(Range<T_data>(1,std::numeric_limits<T_data>::max()))));
        else
          tokens.push(t);

       // lasttext = (t.t == toktype_string || t.t == toktype_range); // god maybe knows the original purpose...
      }

      tokens.push(token(toktype_pend, std::basic_string<T_data>()));

      while(!tokens.empty())
      {
        type switchval = tokens.front().t;
        switch(switchval)
        {
          case toktype_expbegin:
            st.push_front(token(FA<T_data>(Range<T_data>(ttBegin))));
            break;
          case toktype_expend:
            st.push_front(token(FA<T_data>(Range<T_data>(ttEnd))));
            break;
          case toktype_sep:
            st.push_front(token(FA<T_data>(Range<T_data>(ttSep))));
            break;
          case toktype_pbegin:
          case toktype_automaton:
            //case toktype_concat:
            st.push_front(tokens.front());
            break;
          case toktype_star:
          case toktype_mark:
          case toktype_plus:
            {
              if(st.empty() || st.front().t != toktype_automaton)
                std::cout << "misplaced operator3" << std::endl;
              token t = st.front();
              st.pop_front();
              switch(switchval)
              {
                case toktype_star:
                  st.push_front(token(t.d.star()));
                  break;
                case toktype_mark:
                  st.push_front(token(t.d.qmark()));
                  break;
                case toktype_plus:
                  st.push_front(token(t.d.plus()));
                  break;
                default:
                  break;
              }
            }
            break;
          case toktype_pend:    // squash (A|B|C.D.E.F into (A|B|CDEF and then join into choice automaton; remove parentheses and push back
          case toktype_choice:  // squash (A|B|C.D.E.F into (A|B|CDEF
            {
              if(st[0].t==toktype_choice || st[0].t==toktype_pbegin) //cases like (a|) () (|)
              {
                st.push_front(token(FA<T_data>(false)));
              }
              while(st.size() > 1 && st[0].t == toktype_automaton && st[1].t == toktype_automaton) //concatenate last block of concatenable automatons
              {
                token t2 = st.front();
                st.pop_front();
                token t1 = st.front();
                st.pop_front();
                if(t1.t != toktype_automaton || t2.t != toktype_automaton)
                  std::cout << "misplaced operator3" << std::endl;
                st.push_front(token(t1.d-t2.d));
              }
              if(switchval == toktype_pend)
              {
                while(st.size() > 1 && st[1].t == toktype_choice ) //indexing from zero; pend is NOT on stack here // choice together last block of automatonst
                {
                  token t2 = st.front();
                  st.pop_front();
                  st.pop_front();
                  token t1 = st.front();
                  st.pop_front();
                  if(t1.t != toktype_automaton || t2.t != toktype_automaton)
                    std::cout << "misplaced operator2" << std::endl;
                  st.push_front(token(t1.d|t2.d));
                }
                //remove parentheses
                if(st.front().t != toktype_automaton) std::cout << "misplaced operator1" << std::endl;
                FA<T_data> a1 = st.front().d;
                st.pop_front();
                if(st.front().t != toktype_pbegin) std::cout << "unmatched parenthes" << std::endl;
                st.pop_front();
                st.push_front(a1);
              }
              else
              {
                st.push_front(token(toktype_choice, std::basic_string<T_data>()));
              }
            }
            break;
          default:
            break;
        }
        tokens.pop();
      }

      if(token_id < 0)
        std::cout << "token_id is supposed to be >= 0; ALWAYS" << std::endl;
      st.front().d.MakeAccepting(token_id);
      if(st.front().t != toktype_automaton)
        std::cout << "wrong regex!" << std::endl;
      return st.front().d;
    }

  template<class T_data>
    bool Tokenizer<T_data> ::is_operator(T_data c)
    {
      switch(c) //oh yeah lets rely on the compiler optimization... 
      {
        case '$':
        case '^':
        case '(':
        case ')':
        case '*':
        case '?':
        case '+':
        case '|':
        case '[':
        case '.':
          return true;
        default:
          return false;
      }
    }

  template<class T_data>
    typename Tokenizer<T_data> ::token Tokenizer<T_data> ::ReadToken(std::basic_string<T_data>& str)
    {
      if(str[0] == '[')
      {
        std::basic_string<T_data> res;
        if(str[1] == ']')
        {
          res.push_back(']');
          str = str.substr(1);
        }
        str = str.substr(1);
        while(!str.empty() && str[0] != ']')
        {
          if(str[0] == '\\')
          {
            res.push_back(str[0]);
            str = str.substr(1);
          }
          res.push_back(str[0]);
          str = str.substr(1);
        }
        str = str.substr(1);
        return token(toktype_range, res);
      }
      else if (str.length() > 1 && str[0] == '\\' && str[1] == 'b')
      {
        token result;
        result.t = toktype_sep;
        str = str.substr(2);
        return result;
      }
      else if(is_operator(str[0]))
      {
        token result;
        switch(str[0])
        {
          case '$':
            result.t = toktype_expend;
            break;
          case '^':
            result.t = toktype_expbegin;
            break;
          case '.':
            result.t = toktype_any;
            break;
          case '(':
            result.t = toktype_pbegin;
            break;
          case ')':
            result.t = toktype_pend;
            break;
          case '*':
            result.t = toktype_star;
            break;
          case '+':
            result.t = toktype_plus;
            break;
          case '?':
            result.t = toktype_mark;
            break;
          case '|':
            result.t = toktype_choice;
            break;
        }
        str = str.substr(1);
        //result.s = std::basic_string<T_data>();
        return result;
      }
      else
      {
        std::basic_string<T_data> res;
        while(!str.empty() && !is_operator(str[0]))
        {
          if(str[0] == '\\' && str.length() > 1)
          {
            res.push_back(str[1]);
            str = str.substr(2);
          }
          else
          {
            res.push_back(str[0]);
            str = str.substr(1);
          }
        }
        return token(toktype_string, res);
      }
    }

  template<class T_data>
    void Tokenizer<T_data> ::AddToken(std::basic_string<T_data> token_rgx, int id, int base)
    {
#if defined(LINEAR_COMPOSITION)
      engine = engine | ConstructNFA(token_rgx, id);
      tokenCount++;
      if((tokenCount & 511) == 511)
        AddTokensSubmit();
#elif defined(BINARY_COMPOSITION)
      FA<T_data> newtok = ConstructNFA(token_rgx, id); 
      FA<T_data> newtokD = newtok.ConvertToDFA();
      newtok.Free();
      FAstack.push_back(newtokD);
      while( FAstack.size() >= base && (*(FAstack.end()-1)).size >= (*(FAstack.end()-base)).size )
      {
        FA<T_data> fa1 = FAstack.back();
        FAstack.pop_back();
        for(int i = 1; i < base; i++)
        {
          FA<T_data> fa2 = FAstack.back();
          FAstack.pop_back();
          fa1 = fa1 | fa2;
        }
        FA<T_data> fa4 = fa1.ConvertToDFA();
        fa1.Free();
        FAstack.push_back(fa4);
      }
#endif
    }

  template<class T_data>
    void Tokenizer<T_data> ::AddTokensSubmit()
    {
#if defined(LINEAR_COMPOSITION)
      FA<T_data> NFA = engine;
      engine = engine.ConvertToDFA();
      NFA.Free();
      engine.Reduce();
#elif defined(BINARY_COMPOSITION)
      while( FAstack.size() >= 2)
      {
        FA<T_data> fa1 = FAstack.back();
        FAstack.pop_back();
        FA<T_data> fa2 = FAstack.back();
        FAstack.pop_back();
        FA<T_data> fa3 = fa1 | fa2;
        FA<T_data> fa4 = fa3.ConvertToDFA();
        fa3.Free();
        FAstack.push_back(fa4);
      }
      engine = FAstack.back();
      FAstack.pop_back();
      engine.Reduce();
      FAstack.push_back(engine);
#endif
    }

  template<class T_data>
    bool Tokenizer<T_data> ::Match(std::basic_string<T_data> string, std::basic_string<T_data> pattern)
    {
      std::basic_string<T_data> pattern2;
      T_data s[4];
      T_data t[2];
      s[0]=(T_data)'.';
      s[1]=(T_data)'*';
      s[2]=(T_data)'(';
      s[3]=(T_data)'\0';
      t[0]=(T_data)')';
      t[1]=(T_data)'\0';
      pattern2.append(s).append(pattern).append(t);
      //pattern2 = pattern;
      FA<T_data> NFA = ConstructNFA(pattern2, 0);
      FA<T_data> DFA = NFA.ConvertToDFA();
      //DFA.Reduce(); //just for test
      typename std::basic_string<T_data>::iterator b = string.begin();
      typename std::basic_string<T_data>::iterator e = string.end();
      int id;
      bool retval = DFA.template Tokenize<typename std::basic_string<T_data>::iterator>(b, e, id);
      //bool retval = DFA.Match(string);
      NFA.Free();
      DFA.Free();
      return retval;
    }

  template<class T_data>
    void Tokenizer<T_data> ::Clear()
    {
      while (FAstack.size() > 0)
      {
        FAstack.back().Free();
        FAstack.pop_back();
      }
    }

}




