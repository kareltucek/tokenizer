#include "datastructs.h"
#include "automaton.h"
#include <list>
#include <queue>
#include <iostream>
#include <assert.h>
#include <limits>

namespace drgxtokenizer
{
  template<class T_data>
    FA<T_data>::FA()
    : entering(NULL), accepting(NULL), convGetTable(), convGetState(), size(1)
    {
    }

  template<class T_data>
    FA<T_data>::FA(const FA<T_data>& d)
    : entering(d.entering), accepting(d.accepting), convGetTable(), convGetState(), size(d.size)
    {
    }

  template<class T_data>
    FA<T_data>::FA(bool empty)
    : convGetTable(), convGetState(), size(1)
    {
      entering = empty ? NULL : new State<T_data> ();
      accepting = empty ? NULL : new State<T_data> ();
      if(!empty)
        entering->AddTransition(Range<T_data>(0), accepting);
    }

  template<class T_data>
    FA<T_data>::FA( Range<T_data>  range)
    : convGetTable(),convGetState(), size(1)
    {
      entering = new State<T_data> ();
      accepting = new State<T_data> ();
      entering->AddTransition(range, accepting);
    }

  template<class T_data>
    FA<T_data>::StateClass::StateClass(int id_, State<T_data>  * representant_)
    : id(id_), representant(representant_)
    {
    }

  template<class T_data>
    FA<T_data>::StateClass::StateClass()
    : id(-1), representant(NULL)
    {
    }

  template<class T_data>
    bool FA<T_data>::StateClass::operator<(const StateClass& rhs) const
    {
      return this->id < rhs.id;
    }

  template<class T_data>
    FA<T_data>::FA(std::basic_string<T_data> string, bool range, bool case_sens)
    : convGetTable(),convGetState(), size(1)
    {
      if(range)
      {
        std::basic_string<T_data> dbg = string;
        entering = new State<T_data> ();
        accepting = new State<T_data> ();
        std::list<Range<T_data> > l;
        bool negative = false;
        if(!string.empty() && string[0] == '^')
        {
          string = string.substr(1);
          negative = true;
        }
        std::basic_string<T_data> str;
        while(!string.empty())
        {
          if(string[0] == '\\')
            string = string.substr(1);
          if(string.empty())
            std::wcout << "unmatched end of range " << std::endl;
          str.push_back(string[0]);
          string = string.substr(1);
        }

        while(!str.empty())
        {
          if(str.length() > 2 && str[1] == '-')
          {
            if(case_sens)
            {
              l.push_back(Range<T_data> ((T_data)str[0], (T_data)str[2]));
            }
            else
            {
              l.push_back(Range<T_data> (towlower((T_data)str[0]), towlower((T_data)str[2])));
              l.push_back(Range<T_data> (towupper((T_data)str[0]), towupper((T_data)str[2])));
            }
            str = str.substr(3);
          }
          else
          {
            if(case_sens)
            {
              l.push_back(Range<T_data> ((T_data)str[0], (T_data)str[0]));
            }
            else
            {
              l.push_back(Range<T_data> (towlower((T_data)str[0]), towlower((T_data)str[0])));
              l.push_back(Range<T_data> (towupper((T_data)str[0]), towupper((T_data)str[0])));
            }
            str = str.substr(1);
          }
        }

        l.sort();

        if(!negative)
        {
          for(typename std::list<Range<T_data> >::iterator itr = l.begin(); itr != l.end(); ++itr)
            entering->AddTransition(*itr, accepting);
        }
        else
        {
          T_data lastend = 1; //0,0 is reserved for lambda!
          for(typename std::list<Range<T_data> >::iterator itr = l.begin(); itr != l.end(); ++itr)
          {
            entering->AddTransition(Range<T_data> (lastend, (*itr).start-1), accepting);
            lastend = (*itr).end+1;
          }
          entering->AddTransition(Range<T_data> (lastend, std::numeric_limits<T_data>::max()), accepting);
        }
      }
      else
      {
        entering = new State<T_data> ();
        accepting = entering;
        while(!string.empty())
        {
          State<T_data>  * st = new State<T_data> ();
          if(case_sens)
          {
            accepting->AddTransition(Range<T_data> ((T_data)string[0],(T_data)string[0]), st);
          }
          else
          {
            accepting->AddTransition(Range<T_data> (towlower((T_data)string[0]),towlower((T_data)string[0])), st);
            accepting->AddTransition(Range<T_data> (towupper((T_data)string[0]),towupper((T_data)string[0])), st);
          }
          accepting = st;
          string = string.substr(1);
        }
      }
    }

  template<class T_data>
    const FA<T_data> FA<T_data>::plus()
    {
      FA<T_data> result = *this;
      result.accepting->AddTransition(Range<T_data> (0), result.entering);

      result.entering = new State<T_data> ();
      result.entering->AddTransition(Range<T_data> (0), this->entering);
      result.accepting = new State<T_data> ();
      this->accepting->AddTransition(Range<T_data> (0), result.accepting);
      return result;
    }

  template<class T_data>
    const FA<T_data> FA<T_data>::qmark()
    {
      FA<T_data> result = *this;
      result.entering->AddTransition(Range<T_data> (0), result.accepting);

      result.entering = new State<T_data> ();
      result.entering->AddTransition(Range<T_data> (0), this->entering);
      result.accepting = new State<T_data> ();
      this->accepting->AddTransition(Range<T_data> (0), result.accepting);
      return result;
    }

  template<class T_data>
    const FA<T_data> FA<T_data>::star()
    {
      FA<T_data> result = *this;
      result.accepting->AddTransition(Range<T_data> (0), result.entering);
      result.entering->AddTransition(Range<T_data> (0), result.accepting);

      result.entering = new State<T_data> ();
      result.entering->AddTransition(Range<T_data> (0), this->entering);
      result.accepting = new State<T_data> ();
      this->accepting->AddTransition(Range<T_data> (0), result.accepting);
      return result;
    }

  template<class T_data>
    const FA<T_data> FA<T_data>::operator|(const FA<T_data>& d)
    {
      FA<T_data> result = *this;
      result.size += d.size;
      result.entering = new State<T_data> ();
      result.accepting = new State<T_data> ();
      result.entering->AddTransition(Range<T_data> (0), this->entering);
      result.entering->AddTransition(Range<T_data> (0), d.entering);

      if(this->accepting != NULL)
        this->accepting->AddTransition(Range<T_data> (0), result.accepting);
      if(d.accepting != NULL)
        d.accepting->AddTransition(Range<T_data> (0), result.accepting);
      return result;
    }

  template<class T_data>
    const FA<T_data> FA<T_data>::operator-(const FA<T_data>& d)
    {
      FA<T_data> result = *this;
      result.size += d.size;
      result.accepting->AddTransition(Range<T_data> (0), d.entering); //note that this changes the old one too!
      result.accepting = d.accepting;
      return result;
    }

  template<class T_data>
    FA<T_data>& FA<T_data>::operator=(const FA<T_data>& d)
    {
      if(&d == this)
        return *this;
      entering = d.entering;
      accepting = d.accepting;
      size = d.size;
      return *this;
    }

  template<class T_data>
    void FA<T_data>::MakeAccepting(int id)
    {
      accepting->flags |= sfAccepting;
      accepting->tokenid = id;
    }

  template<class T_data>
    void FA<T_data>::Free()
    {
      if(entering == NULL)
        return;
      std::set<State<T_data> *> s = std::set<State<T_data> *>();
      std::queue<State<T_data> *> q = std::queue<State<T_data> *>();
      q.push(entering);
      s.insert(entering);
      while(!q.empty())
      {
        State<T_data> * c = q.front();
        q.pop();
        for(typename std::set<Transition<T_data> >::iterator itr = c->transitions.begin(); itr != c->transitions.end(); ++itr)
        {
          for(typename std::set<State<T_data>*>::iterator it = (*itr).states.begin(); it != (*itr).states.end(); ++it)
          {
            std::pair<typename std::set<State<T_data>*>::iterator,bool> result = s.insert(*it);
            if(result.second)
              q.push(*it);
          }
        }
      }

      for(typename std::set<State<T_data> *>::iterator itr = s.begin(); itr != s.end(); ++itr)
      {
        delete (*itr);
      }
      entering = NULL;
      accepting = NULL;
    }


  template<class T_data>
    bool FA<T_data>::GetRangeBounds(State<T_data> * state, T_data base, T_data& lower, T_data& upper)
    {
      std::set<State<T_data>*>* st = convGetTable[state];
      Transition<T_data> probe(Range<T_data>(base, base, 0));
      bool found = false;
#ifdef ANALYSIS
      _rangebounds_outern++;
#endif
      for(typename std::set<State<T_data>*>::iterator itr = st->begin(); itr != st->end(); ++itr)
      {
#ifdef ANALYSIS
        _rangebounds++;
#endif
        typename std::set<Transition<T_data> >::iterator tran = (*itr)->transitions.lower_bound(probe);
        if( tran != (*itr)->transitions.end())
        {
          found = true;
          Range<T_data>  r = *tran;
          if(lower < r.start && r.start <= upper)
            upper = r.start-1;
          if(r.start < lower && lower <= r.end && base < lower)
            upper = lower-1;
          if(r.start < lower)
            lower = base > r.start ? base : r.start;
          if(r.end < upper)
            upper = r.end;
          if(upper == base )
          {
            return found;
          }
        }
      }
      return found;
    }

  template<class T_data>
    void FA<T_data>::Move(State<T_data> * state, Range<T_data>  move, std::set<State<T_data>*>& acumulator)
    {
      std::set<State<T_data>*>* st = convGetTable[state];
      for(typename std::set<State<T_data>*>::iterator sit = st->begin(); sit != st->end(); ++sit)
      {
#ifdef ANALYSIS
        _move++;
#endif
        typename std::set<Transition<T_data> >::iterator tit = (*sit)->transitions.find(Transition<T_data>(move));
        if(tit == (*sit)->transitions.end())
          continue;
        const Transition<T_data>& t = *tit;
        for(typename std::set<State<T_data>*>::iterator it = t.states.begin(); it != t.states.end(); ++it)
        {
          acumulator.insert(*it);
        }
      }
    }

  template<class T_data>
    std::set<State<T_data>*>* FA<T_data>::LambdaClosure(const std::set<State<T_data>*>& states, bool& accepting, int& acceptingid, short allowedTransitionTypes, short& transitionTypesFound ) //O(links from the initial set)
    {
      std::set<State<T_data>*>* newStates = new std::set<State<T_data>*>();
      std::queue<State<T_data> *> q = std::queue<State<T_data> *>();
      for(typename std::set<State<T_data>*>::iterator itr = states.begin(); itr != states.end(); itr++)
        q.push(*itr);

#ifdef ANALYSIS
      _lambdaclosure_outern++;
#endif
      while(!q.empty())
      {
#ifdef ANALYSIS
        _lambdaclosure++;
#endif
        State<T_data> * s = q.front();
        accepting |= s->Accepting();
        acceptingid = s->tokenid > acceptingid ? s->tokenid : acceptingid;
        q.pop();
        std::pair<typename std::set<State<T_data>*>::iterator, bool> res = newStates->insert(s);
        for(int f = 0; f <= ttFullMask; f++)
        {
          if(res.second && (f == 0 || (f & allowedTransitionTypes)))
          {
            typename std::set<Transition<T_data> >::iterator transition = s->transitions.find(Transition<T_data>(Range<T_data>(0,0,f)));
            if(transition != s->transitions.end())
            {
              transitionTypesFound |= f;
              for(typename std::set<State<T_data>*>::iterator itr = (*transition).states.begin(); itr != (*transition).states.end(); ++itr)
                q.push(*itr);
            }
          }
        }
      }
      return newStates;
    }

  template<class T_data>
    State<T_data>  * FA<T_data>::CreateNewState(std::set<State<T_data>*> * states, bool accepting, bool& newstate, int acceptingid) // O(log(#states))
    {
      typename std::map<std::set<State<T_data>*>*, State<T_data> *>::iterator st = convGetState.find(states);
      newstate = false;
      if(st == convGetState.end())
      {
        newstate = true;
        State<T_data>  * newState = new State<T_data> ();
        tempStates.insert(newState);
        newState->SetAccepting(accepting);
        newState->tokenid = acceptingid;
        convGetTable[newState] = states;
        convGetState[states] = newState;
        return newState;
      }
      else
        return st->second;
    }

  template<class T_data>
    FA<T_data> FA<T_data>::ConvertToDFA()
    {
      FA<T_data> result = FA<T_data>();
      result.size = this->size;
#ifdef ANALYSIS
      _main_states = 0;
      _main_iterations = 0;
      _rangebounds = 0;
      _rangebounds_outern = 0;
      _move = 0;
      _lambdaclosure = 0;
      _lambdaclosure_outern = 0;
      _newstate = 0;
#endif

      //TODO: connect every accepting state to a new "accepting" output by lambda transition

      std::queue<State<T_data> *> q = std::queue<State<T_data> *>();

      std::set<State<T_data>*> s = std::set<State<T_data>*>();
      s.insert(entering);
      //std::set<State<T_data>*> * closure = LambdaClosure(s, accepting, acceptingid, ttAllowed, ttFound);
      //State<T_data>  * state = CreateNewState(closure, accepting, newstate, acceptingid);
      result.entering = SolveClosure(s, q, Range<T_data>(), NULL);
      assert(result.entering != NULL);
      //q.push(result.entering);

      while(!q.empty())
      {
#ifdef ANALYSIS
        _main_states++;
#endif
        State<T_data> * state = q.front();
        assert(state != NULL);
        q.pop();
        T_data base = 1;
        T_data lower = std::numeric_limits<T_data>::max();
        T_data upper = std::numeric_limits<T_data>::max();
        while(GetRangeBounds(state, base, lower, upper))
        {
#ifdef ANALYSIS
          _main_iterations++;
#endif
          s = std::set<State<T_data>*>();
          Range<T_data> r(lower,upper,0);
          Move(state, r, s);
          SolveClosure(s, q, r, state);
          if(upper == std::numeric_limits<T_data>::max())
            break;
          base = upper+1;
          lower = std::numeric_limits<T_data>::max();
          upper = std::numeric_limits<T_data>::max();
        }
      }

      FreeConvertTables();
      return result;
    }

  template<class T_data>
    State<T_data>* FA<T_data>::SolveClosure(std::set<State<T_data>*>& s, std::queue<State<T_data>*>& q, const Range<T_data>& r, State<T_data> * state)
    {
      /*
         For each of the operators $, ^ and \b a special deterministic lambda state is created. Every time crawler encounters such a state in final automaton, crawler switches without asking to the target lambda closure. Here these lambda closures are constructed.
         */
      int acceptingid = -1;
      bool accepting = false;
      short ttAllowed = ttFullMask;
      short ttFound = 0;

      std::set<State<T_data>*> * closure = LambdaClosure(s, accepting, acceptingid, ttAllowed, ttFound);
      if(ttFound == 0)
      {
        /*hooray, the simple case*/
        return SetupNewState( closure, accepting, acceptingid, r, state, q);
      }
      else
      {
        /* create the lambda state, set its flag and set up all necessary transitions*/
        State<T_data>  * lambdaState = new State<T_data> ();
        tempStates.insert(lambdaState);
        lambdaState->SetAccepting(false);
        lambdaState->SetLambda(true);
        lambdaState->SetTType(ttFound);
        lambdaState->tokenid = -1;
        if(state != NULL)
          state->AddTransition(r, lambdaState);
        SetupNewState( closure, accepting, acceptingid, Range<T_data>(0,0,ttFound), lambdaState, q);
        for(int i = 0; i < ttFound; i++)
        {
          if( ((~ttFound) & i))
            continue;
          acceptingid = -1;
          accepting = false;
          short iDontCare;
          closure = LambdaClosure(s, accepting, acceptingid, i, iDontCare);
          SetupNewState( closure, accepting, acceptingid, Range<T_data>(0,0,i), lambdaState, q);
        }
        return lambdaState;
      }
    }

  template<class T_data>
    State<T_data>* FA<T_data>::SetupNewState(std::set<State<T_data>*> * closure, bool accepting, int acceptingid, const Range<T_data>& r, State<T_data>* state,  std::queue<State<T_data> *>& q) 
    {
      bool newstate = false;
      State<T_data>  * tmp = CreateNewState(closure, accepting, newstate, acceptingid);
      if(state != NULL)
        state->AddTransition(r, tmp);
      if(newstate)
        q.push(tmp);
      return tmp;
    }

  template<class T_data>
    void FA<T_data>::FreeConvertTables()
    {
      for(typename std::map<State<T_data> *, std::set<State<T_data>*>*>::iterator itr = convGetTable.begin(); itr!= convGetTable.end(); ++itr)
      {
        delete itr->second;
      }
      convGetTable.clear();
      convGetState.clear();
    }


  /*
     template<class T_data>
     bool FA<T_data>::Match(std::basic_string<T_data> string)
     {
     typename std::basic_string<T_data>::iterator itr = string.begin();
     State<T_data>  * state = entering;
     State<T_data>  * accept = NULL;
     if(entering->Accepting())
     accept = entering;

     bool first = true;
     while(state != NULL && (itr != string.end() || state->Lambda()))
     {
     if(state->Lambda())
     {
     short flags = (first ? ttBegin : 0) | ( itr == string.end() ? ttEnd : 0) | ( false ? ttSep : 0);
     state = state->GetSpec(state->TType() & flags);
     }
     else
     {
     state = state->GetNext(*itr);
     ++itr;
     first = false;
     }

     if(state != NULL && state->Accepting())
     {
     accept = state;
     return true; // if we dont care about the rest of string
     }
     }

     return accept != NULL;
  //moved to the templated function...
  }
  */


  template<class T_data>
    bool FA<T_data>::state_set_comparer::operator ()(const std::set<State<T_data>*> * lhs, const std::set<State<T_data>*> * rhs) const
    {
      //just compare the to set of states as if they were references; (otherwise would be compared by the value of pointer; But we want to search for corresponding set)
      typename std::set<State<T_data>*>::iterator l = lhs->begin();
      typename std::set<State<T_data>*>::iterator r = rhs->begin();
      while(l != lhs->end() || r != rhs->end())
      {
        if(l == lhs->end() || *l < *r)
          return true;
        if(r == rhs->end() || *l > *r)
          return false;
        l++;
        r++;
      }
      return false;
    }

  template<class T_data>
    void FA<T_data>::JoinRanges() // #states * their transitions
    {
      //For each state of this automaton ...
      for(typename std::set<State<T_data>*>::iterator itr = tempStates.begin(); itr != tempStates.end(); itr++)
      {
        State<T_data>  * state = *itr;
        if(state->transitions.size() < 2)
          continue;
        //... go through its transitions ...
        for(typename std::set<Transition<T_data> >::iterator sec = ++(state->transitions.begin()), fir = state->transitions.begin(); sec != state->transitions.end();)
        {
          //... and check if there is a neighbouring pair which can be joined.
          if((*fir).end > 0 && (*fir).end + 1 == (*sec).start && (*fir).states == (*sec).states)
          {
            //If so, then join em;
            typename std::set<Transition<T_data> >::iterator tmpi = sec;
            ++tmpi;
            std::set<State<T_data>*> tmps = (*fir).states;
            T_data tmpb = (*fir).start;
            T_data tmpe = (*sec).end;
            state->transitions.erase(fir);
            state->transitions.erase(sec);
            state->transitions.insert(Transition<T_data>(tmps,tmpb, tmpe));
            if(tmpi == state->transitions.end())
              break;
            sec = tmpi;
            fir = sec;
            --fir;
          }
          else
          {
            ++sec;
            ++fir;
          }
        }
      }
    }

  template<class T_data>
    void FA<T_data>::ReduceInit()
    {
      classid = 0;
      for(typename std::set<State<T_data>*>::iterator itr = tempStates.begin(); itr != tempStates.end(); ++itr)
      {
        classMap[(*itr)] = StateClass((*itr)->tokenid, *itr);
        if((*itr)->tokenid > classid)
          classid = (*itr)->tokenid;
      }
    }

  template<class T_data>
    void FA<T_data>::Reduce()
    {
      if(tempStates.empty())
        return;
      //on input we already have full list of all states of automaton from the NFA<T_data>->DFA<T_data> conversion
      //In Init we will construct new map of representants of every class; The initiall equivalence classes will be defined by token ids of accepting states
      ReduceInit();
      //then we reduce...
      while( ReduceStep() )
        ;
      //at last we change all transition to lead just to representants of all equivalence classes, and delete the other ones
      ReduceFinish();
      tempStates.clear();
      classMap.clear();
    }

  template<class T_data>
    bool FA<T_data>::ReduceStep()
    {
      bool changed = false;
      //in each reduction iteration we go over all states and compare their transitions to the transitions of the representant of their class
      for(typename std::set<State<T_data>*>::iterator itr = tempStates.begin(); itr != tempStates.end(); ++itr)
      {
        State<T_data> * rep = classMap[*itr].representant;
        State<T_data> * state = *itr;
        //the representants are static and thus need not be checked
        if(rep == state)
          continue;

        if(!Correspond(rep, state))
        {
          //if the transition tables do not correspond then we try to find a class that matches our transition table;
          //If we cant find matching transition table (aka state), we will just change the classMap mapping, otherwise we create new class
          //If matching transition already exists it must have been created during this pass (no two equal classes could have existed before, and thus there was no class equal to the class of our state); We index these in the classNew for this purpose (we do not want to search through all states!);
          ReduceInsertNew(state);
          changed = true;
        }
      }
      return changed;
    }

  template<class T_data>
    bool FA<T_data>::Correspond(State<T_data>  * rep, State<T_data>  * state)
    {
      typename std::set<Transition<T_data> >::iterator tit1 = rep->transitions.begin();
      typename std::set<Transition<T_data> >::iterator tit2 = state->transitions.begin();
      while(tit1 != rep->transitions.end() || tit2 != state->transitions.end())
      {
        if(tit1 == rep->transitions.end() || tit2 == state->transitions.end() ||
            (*tit1).start != (*tit2).start || (*tit1).end != (*tit2).end ||
            classMap[*(*tit1).states.begin()].representant != classMap[*(*tit2).states.begin()].representant )
        {
          return false;
          break;
        }
      }
      return true;
    }

  template<class T_data>
    void FA<T_data>::ReduceInsertNew(State<T_data>  * state)
    {
      for(typename std::set<StateClass>::iterator itr = classNew.begin(); itr != classNew.end(); ++itr)
      {
        if(Correspond(state, (*itr).representant))
        {
          classMap[state] = (*itr);
          return;
        }
      }
      classNew.insert(StateClass(++classid, state));
      classMap[state] = StateClass(classid, state); //(already incremented!)
    }

  template<class T_data>
    void FA<T_data>::ReduceFinish()
    {
      for(typename std::set<State<T_data>*>::iterator sit = tempStates.begin(); sit != tempStates.end(); ++sit)
      {
        for(typename std::set<Transition<T_data> >::iterator tit = (*sit)->transitions.begin(); tit != (*sit)->transitions.end(); ++tit)
        {
          assert((*tit).states.size() == 1);
          State<T_data>  * rep = classMap[*(*tit).states.begin()].representant;
          if( *(*tit).states.begin() != rep)
          {
            ((std::set<State<T_data>*>)((*tit).states)).erase((*tit).states.begin());
            ((std::set<State<T_data>*>)((*tit).states)).insert(rep);
          }
        }
      }

      for(typename std::set<State<T_data>*>::iterator sit = tempStates.begin(); sit != tempStates.end(); ++sit)
      {
        if(*sit != classMap[*sit].representant)
          delete *sit;
      }
    }

  template<class T_data>
    void FA<T_data>::DumpState(State<T_data> * state)
    {
      std::wcout << L"state dummp:" << std::endl;
      std::wcout << L"  id: " << state->tokenid << std::endl;
      for(typename std::set<Transition<T_data> >::iterator itr = state->transitions.begin(); itr != state->transitions.end(); ++itr)
      {
        std::wcout << L"  tran: " << (*itr).start << L" " << (*itr).end << std::endl;
      }
    }
}
