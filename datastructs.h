#ifndef DATAshortRUCTS_H
#define DATAshortRUCTS_H

#include <cstddef>
#include <set>
#include <stack>
#include <map>
#include <string>

namespace drgxtokenizer
{
  const short ttSep = 1;
  const short ttBegin = 2;
  const short ttEnd = 4;
  const short ttFullMask = 7;

  template <class T_data> 
    struct Range
    {
      short special;
      T_data start;
      T_data end;
      Range(T_data,T_data, short special = 0);
      Range(const Range&);
      Range(short special);
      Range();
      bool operator>(const Range&) const;
      bool operator<(const Range&) const;
    };

  template <class T_data> 
    class State;

  //typedef std::set<State<T_data>*> States;

  template <class T_data> 
    struct Transition : public Range<T_data>
  {
    std::set<State<T_data>*> states;
    Transition(const Transition&);
    Transition(const Range<T_data>&, State<T_data>* = NULL);
    Transition(short spec);
    Transition(const Transition&, T_data, T_data);
    Transition(const std::set<State<T_data>*>&, T_data, T_data);
  };

  //typedef std::set<Transition<T_data> > Transitions;
  const short sfAccepting = 8;
  const short sfLambda = 16;

  template <class T_data> 
    class State
    {
      public:
        short tokenid;
        short flags; //should be removed, if possible (and replaced by flags)
        std::set<Transition<T_data> > transitions;

        void AddTransition(Range<T_data>, State*);
        State* GetNext(T_data c);
        State* GetSpec(short flag);
        State();

        inline bool Accepting(){return (flags & sfAccepting);};
        inline void SetAccepting(bool b){ flags = b ? flags | sfAccepting: (flags & (~sfAccepting)) ; };
        inline bool Lambda(){return (flags & sfLambda);};
        inline void SetLambda(bool b){ flags = b ?   flags | sfLambda : (flags & (~sfLambda)); };
        inline short TType(){
          return (flags & ttFullMask);
        };
        inline void SetTType(short m){ flags = (flags & (~ttFullMask)) | m; };
    };
}

#ifdef DRGXTOKENIZER_TEMPLATED
#include "datastructs.cpp"
#endif

#endif // DATAshortRUCTS_H
