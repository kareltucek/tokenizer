#ifndef automaton_H
#define automaton_H

#include <string>
#include <queue>
#include <locale>
#include "datastructs.h"

namespace drgxtokenizer
{
  template <class T_data>
    class State ;

  /*
   * FA represents a finite automaton (both deterministic and nondeterministic automatons are represented by this class and provides method to:
   * join automatons and iterate over them (the regex operator functionality)
   * do actual matching
   * conversion to Deterministic automaton
   * reduction of deterministic automaton
   *
   =============
   *reduction follows this scheme
   * input:
   *   full list of states (tempStates)
   *   create initial equivalence classes from tokenids (and masks<) -> mapClass, created by ReduceInit;
   * algorithm:
   *   We call ReduceStep until everything gets stable;
   *     in each iteration we compare transition tables of all states with the transition table of representant of this class. If they do not correspond (do not have transition endpoints in same classes), we split them and add the new classes into the classNew index. That one is merged into the classMap at the end of ReduceStep();
   =============
   * NFA->DFA conversion follows this scheme:
   * ConvertToDFA()
   *   create new mapping representing the subset construction - convGetTable and convGetState; These map old set of states to new converted state and vice versa
   *   get lambda closure of entering state and create new entering state from it; push it onto a local queue
   *   now perform BFS on the queue (in fact BFS on states of the new automaton)
   *     each iteration takes a new state from queue of unprocessed states and iterates:
   *     while ( GetRangeBounds() ) //iterate over intersections of ranges of different states
   *       Move()
   *         creates subset of old states to which lead transitions from the old set of states
   *       LambdaClosure()
   *         gets lambda closure of the set obtained from move; Also checks if there is an accepting state in this group; If so, the resulting state is made accepting
   *       look for existing state  with these properties;
   **/

  template <class T_data> 
    class FA
    {
      private:

        State<T_data>* entering;
        State<T_data>* accepting;


        struct state_set_comparer
        {
          bool operator() (const std::set<State<T_data>*>* lhs, const std::set<State<T_data>*>* rhs) const;
        };

        //reduction related stuff
        int classid;

        struct StateClass
        {
          StateClass();
          StateClass(int tokenid, State<T_data> * representant);
          State<T_data>* representant;
          int id; //starts as tokenid; later may become whatever
          bool operator<(const StateClass&) const;
        };
        std::map<State<T_data>*, StateClass > classMap; //maps states to their classes for reduction
        std::set<StateClass > classNew; //tmp storage for new states during step of reduction algorithm
        std::set<State<T_data>*> tempStates; //full list of states; serves for reduction; currently filled during NFA->DFA conversion

        void JoinRanges();
        void ReduceInit();
        bool ReduceStep();
        void ReduceFinish();
        void ReduceInsertNew(State<T_data>  * state); //new equivalence class
        bool Correspond(State<T_data>  * a, State<T_data>  * b);

        //NFA->DFA conversion related stuff
        typedef std::map<State<T_data> *, std::set<State<T_data>*>*> map_st_to_tab_t;
        typedef std::map<std::set<State<T_data>*>*, State<T_data> *, state_set_comparer> map_tab_to_st_t;
        map_st_to_tab_t convGetTable;
        map_tab_to_st_t convGetState;

        State<T_data>* SolveClosure(std::set<State<T_data>*>& s, std::queue<State<T_data>*>& q, const Range<T_data>& r, State<T_data> * state);
        State<T_data>* SetupNewState(std::set<State<T_data>*> * closure, bool accepting, int acceptingid, const Range<T_data>& r, State<T_data>* state,  std::queue<State<T_data> *>& queue) ;
        bool GetRangeBounds(State<T_data> * state, T_data base, T_data& lower, T_data& upper);
        void Move(State<T_data> * state, Range<T_data>  move, std::set<State<T_data>*>& acumulator);
        std::set<State<T_data>*>* LambdaClosure(const std::set<State<T_data>*>& states, bool& accepting, int& acceptingid, short ttallowed, short& ttfound);
        State<T_data>  * CreateNewState(std::set<State<T_data>*> * states, bool accepting, bool& newstate, int tokenid);
        void FreeConvertTables();

        //define ANALYSIS
#ifdef ANALYSIS
        int _main_states ;
        int _main_iterations ;
        int _rangebounds_outern;
        int _rangebounds;
        int _move;
        int _lambdaclosure;
        int _lambdaclosure_outern;
        int _newstate;
#endif

      public:
        void Reduce();
        FA ConvertToDFA();
        //bool Match(std::basic_string<T_data>);
        template<class iterator_type>
          bool Tokenize(iterator_type& itr, const iterator_type& itrend, int& token_id); //itr will get moved to the end of parsed token; itrend marks where parsing should end; return value means whether a valid token was matched
        void MakeAccepting(int id);
        void Free();

        int size;

        FA();
        FA(bool empty);
        FA(const FA&);
        FA(Range<T_data> );
        FA(std::basic_string<T_data>, bool range, bool pcase); //either a trivial string or content of a range

        const FA plus();
        const FA star();
        const FA qmark();
        const FA operator|(const FA&);
        const FA operator-(const FA&);
        FA& operator=(const FA&);

        //debug functions
        static void DumpState(State<T_data> * state);
    };

  template<class T_data>
    template<class iterator_type>
    bool FA<T_data>::Tokenize(iterator_type& itr, const iterator_type& itrend, int& tokenid)
    {
      iterator_type tmpit = itr;
      State<T_data>  * state = entering;
      State<T_data>  * accept = NULL;

      if(entering->Accepting())
      {
        accept = entering;
        tmpit = itr;
      }

      bool first = true;

      T_data d = '\0';
      while(state != NULL && (itr != itrend || state->Lambda()))
      {
        if(state->Lambda())
        {
          std::locale loc;
          short flags = (first ? ttBegin : 0) | ( itr == itrend ? ttEnd : 0) | ( first || itr == itrend || (std::isalnum<T_data>(d,loc) ^ std::isalnum<T_data>(*itr,loc)) ? ttSep : 0);
          state = state->GetSpec(state->TType() & flags);
        }
        else
        {
          state = state->GetNext(*itr);
          d = *itr;
          ++itr;
          first = false;
        }
        if(state != NULL && state->Accepting())
        {
          accept = state;
          tmpit = itr;
        }
      }

      itr = tmpit;
      if(accept != NULL)
      {
        tokenid = accept->tokenid;
        return true;
      }
      return false;
    }
}

#ifdef DRGXTOKENIZER_TEMPLATED
#include "automaton.cpp"
#endif

#endif // automaton_H
