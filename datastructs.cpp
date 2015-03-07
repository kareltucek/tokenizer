#include "datastructs.h"

namespace drgxtokenizer
{
  template <class T_data>
    Range<T_data>::Range (T_data a, T_data b, short spec)
    :start(a), end(b), special(spec)
    {}

  template <class T_data>
    Range<T_data>::Range ()
    :start(0), end(0), special(0)
    {}

  template <class T_data>
    Range<T_data>::Range (short spec)
    :start(0), end(0), special(spec)
    {}

  template <class T_data>
    Range<T_data>::Range (const Range<T_data> & r)
    : start(r.start), end(r.end), special(r.special)
    {}

  template <class T_data>
    Transition<T_data>::Transition (const Transition<T_data>& r)
    : Range<T_data>(r), states(r.states)
    {}

  template <class T_data>
    Transition<T_data>::Transition (short spec)
    : Range<T_data>(spec), states()
    {}

  template <class T_data>
    Transition<T_data>::Transition (const Range<T_data>& r, State<T_data>* s)
    : Range<T_data>(r), states()
    {
      if(s!=NULL)
        states.insert(s);
    }

  template <class T_data>
    Transition<T_data>::Transition (const Transition<T_data>& t, T_data a, T_data b)
    : states(t.states), Range<T_data>(a, b)
    {}

  template <class T_data>
    Transition<T_data>::Transition (const std::set<State<T_data>*>& t, T_data a, T_data b)
    : states(t), Range<T_data>(a, b)
    {}

  template <class T_data>
    State<T_data>::State()
    : tokenid(0), flags(0), transitions()
    {
    }

  template <class T_data>
    void State<T_data>::AddTransition (Range<T_data> r, State* s)
    {
      //FA::DumpState(this);
      typename std::set<Transition<T_data> >::iterator itr = transitions.find(Transition<T_data>(r));
      //std::wcout << L"adding transition: " << r.start << L" " << r.end << std::endl;
      if(itr != transitions.end())
      {
        const Transition<T_data>& t = *itr;
        if(t.start == r.start && t.end == r.end)
        {
          ((Transition<T_data>&)t).states.insert(s);
        }
        else if(t.start > r.start)
        {
          AddTransition (Range<T_data> (r.start, t.start-1),s);
          AddTransition (Range<T_data> (t.start, r.end),s);
        }
        else if(r.end > t.end)
        {
          AddTransition (Range<T_data> (r.start, t.end), s);
          AddTransition (Range<T_data> (t.end+1, r.end), s);
        }
        else if(t.start < r.start)
        {
          int a = t.start;
          int b = r.start-1;
          int c = r.start;
          int d = r.end;
          //t.start = c;
          transitions.erase(itr);
          transitions.insert(Transition<T_data> (t, c, d));
          transitions.insert(Transition<T_data> (t, a, b));
          AddTransition (Range<T_data> (a, b), s);
          AddTransition (Range<T_data> (c, d), s);
        }
        else if(t.end > r.end)
        {
          int a = t.start;
          int b = r.end;
          int c = r.end+1;
          int d = t.end;
          //t.end = b;
          transitions.erase(itr);
          transitions.insert(Transition<T_data> (t, a, b));
          transitions.insert(Transition<T_data> (t, c, d));
          AddTransition (Range<T_data> (a, b), s);
          AddTransition (Range<T_data> (c, d), s);
        }
        else
        {
          ((Transition<T_data> &)t).states.insert(s);
        }
      }
      else
      {
        transitions.insert(Transition<T_data> (r, s));
      }
      //FA::DumpState(this);
    }

  template <class T_data>
    bool Range<T_data> ::operator>(const Range<T_data> & d) const
    {
      return start > d.end || (end == 0 && d.end == 0 && special > d.special);
    }

  template <class T_data>
    bool Range<T_data> ::operator<(const Range<T_data> & d) const
    {
      return d.start > end || (end == 0 && d.end == 0 && special < d.special);
    }

  template <class T_data>
    drgxtokenizer::State<T_data>* State<T_data>::GetSpec(short spec)
    {
      typename std::set<Transition<T_data> >::iterator tit = transitions.find(Transition<T_data> (spec));
      if(tit != transitions.end())
      {
        typename std::set<State<T_data>*>::iterator sit = (*tit).states.begin();
        if(sit != (*tit).states.end())
        {
          return *sit;
        }
      }
      return NULL;
    }

  template <class T_data>
    drgxtokenizer::State<T_data>* State<T_data>::GetNext(T_data c)
    {
      typename std::set<Transition<T_data> >::iterator tit = transitions.find(Transition<T_data> (Range<T_data>(c, c)));
      if(tit != transitions.end())
      {
        typename std::set<State<T_data>*>::iterator sit = (*tit).states.begin();
        if(sit != (*tit).states.end())
        {
          return *sit;
        }
      }
      return NULL;
    }
}
